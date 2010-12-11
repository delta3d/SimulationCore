/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 * 
 * David Guthrie
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/ActComps/TrailerHitchActComp.h>

#include <dtPhysics/palphysicsworld.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <pal/palFactory.h>
#include <pal/palLinks.h>

#include <dtUtil/nodecollector.h>
#include <dtDAL/propertymacros.h>

#include <dtGame/gamemanager.h>

#include <osg/Group>
#include <osgSim/DOFTransform>

namespace SimCore
{

   namespace ActComps
   {

      IMPLEMENT_ENUM(HitchTypeEnum);

      HitchTypeEnum HitchTypeEnum::HITCH_TYPE_SPHERICAL("HITCH_TYPE_SPHERICAL");
      HitchTypeEnum HitchTypeEnum::HITCH_TYPE_5TH_WHEEL("HITCH_TYPE_5TH_WHEEL");

      HitchTypeEnum::HitchTypeEnum(const std::string& name)
      : dtUtil::Enumeration(name)
      {
         AddInstance(this);
      }

      const dtUtil::RefString TrailerHitchActComp::TYPE("TrailerHitch");

      const dtUtil::RefString TrailerHitchActComp::FRONT_HITCH_DOF_NAME_DEFAULT = "dof_hitch_front";
      const dtUtil::RefString TrailerHitchActComp::REAR_HITCH_DOF_NAME_DEFAULT = "dof_hitch_rear";

      TrailerHitchActComp::TrailerHitchActComp()
      : BaseClass(TYPE)
      , mRotationMaxYaw(60.0f)
      , mRotationMaxCone(20.0f)
      , mHitchType(&HitchTypeEnum::HITCH_TYPE_SPHERICAL)
      , mHitchNodeNameTractor(TrailerHitchActComp::REAR_HITCH_DOF_NAME_DEFAULT)
      , mHitchNodeNameTrailer(TrailerHitchActComp::FRONT_HITCH_DOF_NAME_DEFAULT)
      , mTrailerActorId("")
      , mCascadeDeletes(true)
      , mHitchJoint(NULL)
      {
      }

      TrailerHitchActComp::~TrailerHitchActComp()
      {
      }

      void TrailerHitchActComp::BuildPropertyMap()
      {
         typedef dtDAL::PropertyRegHelper<TrailerHitchActComp&, TrailerHitchActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Trailer Hitch");

         DT_REGISTER_PROPERTY(
                  RotationMaxYaw,
                  "The max twist rotation for the hitch from straight behind the vehicle until it hits something.\n"
                  "Half the total allowed rotation.",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
                  RotationMaxCone,
                  "The max pitch and roll (roll only for spherical hitch).",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
                  HitchType,
                  "The kind of hitch to create. Defaults to HITCH_TYPE_SPHERICAL",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
                  HitchNodeNameTractor,
                  "The name of the DOF node used for the hitch on the tractor or pulling trailer",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
                  HitchNodeNameTrailer,
                  "The name of the DOF node used for the hitch on the trailer",
                  PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
                  CascadeDeletes,
                  "true or false for removing the attached actors from the GM when the parent is removed.",
                  PropRegType, propRegHelper);
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::OnEnteredWorld()
      {
         // This will do nothing unless everything is set.
         dtGame::GameActor* ga = NULL;
         GetOwner(ga);
         // remote versions should be dead-reckoned together, or otherwise moved another way.
         if (ga != NULL && !ga->IsRemote())
         {
            Attach();
         }
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::OnRemovedFromWorld()
      {
         dtGame::GameActor* ga = NULL;
         GetOwner(ga);

         if (mCascadeDeletes && mTrailerActor.valid())
         {
            if (ga != NULL)
            {
               ga->GetGameActorProxy().GetGameManager()->DeleteActor(mTrailerActor->GetGameActorProxy());
            }
         }
         // remote versions should be dead-reckoned together, or otherwise moved another way.
         if (ga != NULL && !ga->IsRemote())
         {
            Detach();
         }
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::Attach()
      {
         if (mHitchJoint == NULL && GetIsInGM() && !mTrailerActorId.ToString().empty())
         {
            mTrailerActor = LookupTrailer();

            std::pair<dtPhysics::PhysicsObject*, dtPhysics::PhysicsObject* > physicsObjects = GetPhysicsObjects();

            if (physicsObjects.first == NULL || physicsObjects.second == NULL)
            {
               LOG_WARNING("In order to attach a trailer, both actors must have both a physics actor component and a main physics object.");
               return;
            }

            if (physicsObjects.first->GetBodyWrapper() == NULL || physicsObjects.second->GetBodyWrapper() == NULL)
            {
               LOG_WARNING("Physics objects for the tractor and trailer body must be initalized.");
               return;
            }

            std::pair<osg::Group*, osg::Group* > nodes = GetHitchNodes();
            if (nodes.first == NULL || nodes.second == NULL)
            {
               LOG_WARNING("Unable to attach trailer, either the tractor or trailer hitch nodes cannot be found.");
               return;
            }

            // We have all the data we need, now to do the actual work.
            osg::Matrix tractorHitchMat, trailerHitchMat;
            dtCore::Transformable::GetAbsoluteMatrix(nodes.first->getParent(0), tractorHitchMat);
            dtCore::Transformable::GetAbsoluteMatrix(nodes.second->getParent(0), trailerHitchMat);

            dtCore::Transform trailerWorld;
            mTrailerActor->GetTransform(trailerWorld);

            osg::Matrix trailerWorldMat;
            trailerWorld.Get(trailerWorldMat);

            osg::Matrix trailerHitchMatInv;
            trailerHitchMatInv.invert(trailerHitchMat);

            osg::Matrix trailerRelToHitch =  trailerWorldMat * trailerHitchMatInv;
            osg::Matrix newTrailerTransform =  trailerRelToHitch * tractorHitchMat;

            // Move the trailer so it is exactly the right position.
            trailerWorld.Set(newTrailerTransform);
            mTrailerActor->SetTransform(trailerWorld);
            dtPhysics::PhysicsActComp* physACTrailer = NULL;
            mTrailerActor->GetComponent(physACTrailer);
            if (physACTrailer != NULL)
            {
               physACTrailer->SetMultiBodyTransformAsVisual(trailerWorld);
            }

            palFactory* factory = dtPhysics::PhysicsWorld::GetInstance().GetPalFactory();
            if (*mHitchType == HitchTypeEnum::HITCH_TYPE_SPHERICAL)
            {
               osg::Vec3f hitchWorldPos = tractorHitchMat.getTrans();
               palSphericalLink* psl = factory->CreateSphericalLink(
                        &physicsObjects.first->GetBodyWrapper()->GetPalBodyBase(), &physicsObjects.second->GetBodyWrapper()->GetPalBodyBase(),
                        hitchWorldPos.x(), hitchWorldPos.y(), hitchWorldPos.z());

               psl->SetLimits(osg::DegreesToRadians(mRotationMaxCone), osg::DegreesToRadians(mRotationMaxYaw));

               mHitchJoint = psl;
            }
            else if (*mHitchType == HitchTypeEnum::HITCH_TYPE_5TH_WHEEL)
            {
               osg::Vec3f hitchWorldPos = tractorHitchMat.getTrans();

               float rotationMaxConeRad = osg::DegreesToRadians(mRotationMaxCone);
               float rotationMaxYawRad = osg::DegreesToRadians(mRotationMaxYaw);

               palGenericLink* pgl = factory->CreateGenericLink(
                        &physicsObjects.first->GetBodyWrapper()->GetPalBodyBase(), &physicsObjects.second->GetBodyWrapper()->GetPalBodyBase(),
                        palVector3(hitchWorldPos.x(), hitchWorldPos.y(), hitchWorldPos.z()),
                        palVector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON), palVector3(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON), // Lock the linear motion.
                        palVector3(-rotationMaxConeRad, 0.0f, -rotationMaxYawRad),
                        palVector3(rotationMaxConeRad, 0.0f, rotationMaxYawRad));

               mHitchJoint = pgl;
            }
         }
      }

      //////////////////////////////////////////////////
      std::pair<osg::Group*, osg::Group* > TrailerHitchActComp::GetHitchNodes()
      {
         std::pair<osg::Group*, osg::Group* > result(NULL, NULL);
         SimCore::Actors::IGActor* igDraw = NULL;
         GetOwner(igDraw);

         dtUtil::NodeCollector* nc = NULL;
         dtUtil::NodeCollector* ncTrailer = NULL;

         if (igDraw != NULL)
         {
            nc = igDraw->GetNodeCollector();
         }

         if (mTrailerActor.valid())
         {
            ncTrailer = mTrailerActor->GetNodeCollector();
         }

         if (nc != NULL && ncTrailer != NULL)
         {
            osgSim::DOFTransform* dofTractor = nc->GetDOFTransform(mHitchNodeNameTractor);
            osgSim::DOFTransform* dofTrailer = ncTrailer->GetDOFTransform(mHitchNodeNameTrailer);
            return std::pair<osg::Group*, osg::Group* >(dofTractor, dofTrailer);
         }
         return result;
      }

      //////////////////////////////////////////////////
      std::pair<dtPhysics::PhysicsObject*, dtPhysics::PhysicsObject*> TrailerHitchActComp::GetPhysicsObjects()
      {
         std::pair<dtPhysics::PhysicsObject*, dtPhysics::PhysicsObject*> result(NULL, NULL);

         SimCore::Actors::IGActor* igDraw = NULL;
         GetOwner(igDraw);

         dtPhysics::PhysicsActComp* physAC;
         igDraw->GetComponent(physAC);

         if (physAC != NULL)
         {
            result.first = physAC->GetMainPhysicsObject();
         }

         physAC = NULL;
         if (mTrailerActor.valid())
         {
            mTrailerActor->GetComponent(physAC);
         }

         if (physAC != NULL)
         {
            result.second = physAC->GetMainPhysicsObject();
         }

         return result;
      }

      //////////////////////////////////////////////////
      SimCore::Actors::IGActor* TrailerHitchActComp::LookupTrailer()
      {
         if (mTrailerActorId.ToString().empty())
         {
            return NULL;
         }

         SimCore::Actors::IGActor* igDraw = NULL;
         GetOwner(igDraw);
         dtDAL::BaseActorObject* actor = igDraw->GetGameActorProxy().GetGameManager()->FindActorById(mTrailerActorId);

         return dynamic_cast<SimCore::Actors::IGActor*>(actor->GetDrawable());
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::Detach()
      {
         if (mHitchJoint != NULL)
         {
            delete mHitchJoint;
            mHitchJoint = NULL;
         }
      }

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, float, RotationMaxYaw);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, float, RotationMaxCone);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, dtUtil::EnumerationPointer<HitchTypeEnum>, HitchType);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, std::string, HitchNodeNameTractor);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, std::string, HitchNodeNameTrailer);

      DT_IMPLEMENT_ACCESSOR_GETTER(TrailerHitchActComp, dtCore::UniqueId, TrailerActorId);

      void TrailerHitchActComp::SetTrailerActorId(const dtCore::UniqueId& id)
      {
         mTrailerActorId = id;
         Detach();
         Attach();
      }

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, bool, CascadeDeletes);

   }

}
