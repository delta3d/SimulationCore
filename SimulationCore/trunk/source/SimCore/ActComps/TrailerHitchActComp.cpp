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
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/environmentactor.h>
#include <dtCore/scene.h>

#include <pal/palFactory.h>
#include <pal/palLinks.h>

#include <dtUtil/nodecollector.h>
#include <dtUtil/matrixutil.h>
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
      , mHitchType(&HitchTypeEnum::HITCH_TYPE_5TH_WHEEL)
      , mHitchNodeNameTractor(TrailerHitchActComp::REAR_HITCH_DOF_NAME_DEFAULT)
      , mHitchNodeNameTrailer(TrailerHitchActComp::FRONT_HITCH_DOF_NAME_DEFAULT)
      , mTrailerActorId("")
      , mCurrentHitchRotHPR(osg::Vec3(0.0f, 0.0f, 0.0f))
      , mCascadeDeletes(true)
      , mUseCurrentHitchRotToMoveTrailerWhenRemote(false)
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

         DT_REGISTER_PROPERTY(
                  CurrentHitchRotHPR,
                  "The current rotation of the hitch for the trailer.",
                  PropRegType, propRegHelper);
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::OnEnteredWorld()
      {
         // This will do nothing unless everything is set.
         dtGame::GameActor* ga = NULL;
         GetOwner(ga);
         Attach();
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
            if (!mTrailerActor.valid())
            {
               return;
            }

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


            if (!mTrailerActor->IsRemote())
            {
               dtGame::GameActor* ga = NULL;
               GetOwner(ga);
               if (ga->IsPublished() && !mTrailerActor->IsPublished())
               {
                  // TODO make this listen for the publish message from the tractor.
                  ga->GetGameActorProxy().GetGameManager()->PublishActor(mTrailerActor->GetGameActorProxy());
               }

               osg::Vec3d hitchWorldPos = WarpTrailerToTractor();
               palFactory* factory = dtPhysics::PhysicsWorld::GetInstance().GetPalFactory();
               if (*mHitchType == HitchTypeEnum::HITCH_TYPE_SPHERICAL)
               {
                  palSphericalLink* psl = factory->CreateSphericalLink(
                           &physicsObjects.first->GetBodyWrapper()->GetPalBodyBase(), &physicsObjects.second->GetBodyWrapper()->GetPalBodyBase(),
                           Float(hitchWorldPos.x()), Float(hitchWorldPos.y()), Float(hitchWorldPos.z()));

                  psl->SetLimits(osg::DegreesToRadians(mRotationMaxCone), osg::DegreesToRadians(mRotationMaxYaw));

                  mHitchJoint = psl;
               }
               else if (*mHitchType == HitchTypeEnum::HITCH_TYPE_5TH_WHEEL)
               {
                  float rotationMaxConeRad = osg::DegreesToRadians(mRotationMaxCone);
                  float rotationMaxYawRad = osg::DegreesToRadians(mRotationMaxYaw);

                  palGenericLink* pgl = factory->CreateGenericLink(
                           &physicsObjects.first->GetBodyWrapper()->GetPalBodyBase(), &physicsObjects.second->GetBodyWrapper()->GetPalBodyBase(),
                           palVector3(Float(hitchWorldPos.x()), Float(hitchWorldPos.y()), Float(hitchWorldPos.z())),
                           palVector3(-FLT_EPSILON, -FLT_EPSILON, -FLT_EPSILON), palVector3(FLT_EPSILON, FLT_EPSILON, FLT_EPSILON), // Lock the linear motion.
                           palVector3(-rotationMaxConeRad, 0.0f, -rotationMaxYawRad),
                           palVector3(rotationMaxConeRad, 0.0f, rotationMaxYawRad));

                  mHitchJoint = pgl;
               }
            }
            else
            {
               WarpTrailerToTractor(mUseCurrentHitchRotToMoveTrailerWhenRemote);
            }
         }
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::CalcTransformsForTractorHitchAndTrailerVisual(dtCore::Transform& tractorHitchTransform, dtCore::Transform& trailerTransform)
      {
         if (!mTrailerActor.valid())
         {
            LOG_WARNING("Unable to calculate relevant transforms for attaching a trailer for a tractor without a trailer actor");
         }

         std::pair<osg::Group*, osg::Group* > nodes = GetHitchNodes();
         if (nodes.first == NULL || nodes.second == NULL)
         {
            LOG_WARNING("Unable to attach trailer; either the tractor hitch \"" + mHitchNodeNameTractor
                     + "\"  or trailer hitch \"" + mHitchNodeNameTrailer + "\" nodes cannot be found.");
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

         osg::Matrix hitchRotation;
         dtUtil::MatrixUtil::HprToMatrix(hitchRotation, mCurrentHitchRotHPR);
         tractorHitchMat = hitchRotation * tractorHitchMat;

         osg::Matrix newTrailerTransform =  trailerRelToHitch * tractorHitchMat;


         tractorHitchTransform.Set(tractorHitchMat);
         trailerTransform.Set(newTrailerTransform);
      }

      //////////////////////////////////////////////////
      std::pair<osg::Group*, osg::Group* > TrailerHitchActComp::GetHitchNodes() const
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
      std::pair<dtPhysics::PhysicsObject*, dtPhysics::PhysicsObject*> TrailerHitchActComp::GetPhysicsObjects() const
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
         if (actor != NULL)
         {
            return dynamic_cast<SimCore::Actors::IGActor*>(actor->GetDrawable());
         }
         return NULL;
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::Detach()
      {
         if (mTrailerActor != NULL)
         {
            ResetTrailerActor();
            
            if(mTrailerActor->GetParent() != NULL)          
            {
               mTrailerActor->Emancipate();
            }
         }

         delete mHitchJoint;
         mHitchJoint = NULL;
         mTrailerActor = NULL;
      }

      //////////////////////////////////////////////////
      bool TrailerHitchActComp::GetAttached() const
      {
         return mTrailerActor.valid() && (mHitchJoint != NULL || mTrailerActor->IsRemote());
      }

      //////////////////////////////////////////////////
      dtGame::GameActor* TrailerHitchActComp::GetTrailer()
      {
         return mTrailerActor.get();
      }

      //////////////////////////////////////////////////
      void TrailerHitchActComp::ResetTrailerActor()
      {
         if (mTrailerActor == NULL)
         {
            return;
         }

         dtGame::GameActor* ga = NULL;
         GetOwner(ga);

         bool needDetach = mTrailerActor->GetParent() == ga;

         if (needDetach)
         {
            mTrailerActor->Emancipate();
         }

         if ( mTrailerActor->GetParent() == NULL)
         {
            dtGame::GameManager* gm = ga->GetGameActorProxy().GetGameManager();
            if (gm->GetEnvironmentActor() != NULL)
            {
               dtGame::IEnvGameActor* ienv = NULL;
               gm->GetEnvironmentActor()->GetDrawable(ienv);
               ienv->AddActor(*mTrailerActor);
            }
            else
            {
               gm->GetScene().AddChild(mTrailerActor.get());
            }
         }

         dtGame::DeadReckoningHelper* drHelper = NULL;
         mTrailerActor->GetComponent(drHelper);

         dtGame::DeadReckoningHelper* drHelperOwner = NULL;
         ga->GetComponent(drHelperOwner);
         if (drHelper != NULL && drHelperOwner != NULL)
         {
            // pick up the dr algorithm from the tractor.
            drHelper->SetDeadReckoningAlgorithm(drHelperOwner->GetDeadReckoningAlgorithm());
         }
      }

      //////////////////////////////////////////////////
      osg::Vec3d TrailerHitchActComp::WarpTrailerToTractor(bool addAsChild)
      {
         osg::Vec3d result(0.0, 0.0, 0.0);
         if (mTrailerActor != NULL)
         {
            dtCore::Transform tractorHitchTransform, trailerWorld;
            CalcTransformsForTractorHitchAndTrailerVisual(tractorHitchTransform, trailerWorld);

            dtGame::DeadReckoningHelper* drHelper = NULL;
            mTrailerActor->GetComponent(drHelper);

            dtGame::GameActor* ga = NULL;
            GetOwner(ga);

            bool parentIsTractor = mTrailerActor->GetParent() == ga;

            // really an xor.  if you want it to be a child and the parent is not the tractor
            // or you don't want it to be a child and the parent IS the tractor.
            if (addAsChild != parentIsTractor)
            {
               mTrailerActor->Emancipate();
            }

            if (addAsChild && !parentIsTractor)
            {
               // adding as a child so it will stay in the right place
               // between updates to the hitch rotation.
               ga->AddChild(mTrailerActor.get());
               drHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::NONE);
            }
            else if (mTrailerActor->GetParent() == NULL)
            {
               ResetTrailerActor();
            }

            // Move the trailer so it is exactly the right position.
            mTrailerActor->SetTransform(trailerWorld);

            // this does not mean if it doesn't drHelper, but rather that it's not remote
            // OR it has no drHelper.
            if (!mTrailerActor->IsRemote())
            {
               dtPhysics::PhysicsActComp* physACTrailer = NULL;
               mTrailerActor->GetComponent(physACTrailer);
               if (physACTrailer != NULL)
               {
                  physACTrailer->SetMultiBodyTransformAsVisual(trailerWorld);
               }
            }
            else
            {
               drHelper->SetLastKnownTranslation(trailerWorld.GetTranslation());
               drHelper->SetLastKnownRotation(trailerWorld.GetRotation());
            }

            tractorHitchTransform.GetTranslation(result);
         }
         return result;
      }

      ////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, float, RotationMaxYaw);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, float, RotationMaxCone);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, dtUtil::EnumerationPointer<HitchTypeEnum>, HitchType);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, std::string, HitchNodeNameTractor);

      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, std::string, HitchNodeNameTrailer);

      DT_IMPLEMENT_ACCESSOR_GETTER(TrailerHitchActComp, dtCore::UniqueId, TrailerActorId);

      ////////////////////////////////////////////////////////////////
      void TrailerHitchActComp::SetTrailerActorId(const dtCore::UniqueId& id)
      {
         bool wasAttached = mTrailerActor != NULL;
         if (wasAttached)
         {
            Detach();
         }
         mTrailerActorId = id;
         if (wasAttached)
         {
            Attach();
         }
      }

      ////////////////////////////////////////////////////////////////
      osg::Vec3 TrailerHitchActComp::GetCurrentHitchRotHPR() const
      {
         osg::Vec3 result(mCurrentHitchRotHPR);
         if (GetAttached())
         {
            std::pair<osg::Group*, osg::Group* > nodes = GetHitchNodes();
            if (nodes.first == NULL || nodes.second == NULL)
            {
               LOG_WARNING("The trailer seems to think it's attached, but either the tractor hitch \"" + mHitchNodeNameTractor
                        + "\"  or trailer hitch \"" + mHitchNodeNameTrailer + "\" nodes cannot be found.");
            }
            else
            {
               osg::Matrix tractorHitchMat, trailerHitchMat;
               dtCore::Transformable::GetAbsoluteMatrix(nodes.first->getParent(0), tractorHitchMat);
               dtCore::Transformable::GetAbsoluteMatrix(nodes.second->getParent(0), trailerHitchMat);

               osg::Matrix tractorHitchMatInv;
               tractorHitchMatInv.invert(tractorHitchMat);

               osg::Matrix hitchRelativeMatrix = trailerHitchMat * tractorHitchMatInv;
               dtUtil::MatrixUtil::MatrixToHpr(result, hitchRelativeMatrix);
            }
         }
         return result;
      }

      ////////////////////////////////////////////////////////////////
      void TrailerHitchActComp::SetCurrentHitchRotHPR(const osg::Vec3& hpr)
      {
         mCurrentHitchRotHPR = hpr;
         if (mUseCurrentHitchRotToMoveTrailerWhenRemote && mTrailerActor != NULL && mTrailerActor->IsRemote())
         {
            WarpTrailerToTractor(true);
         }
      }

      ////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(TrailerHitchActComp, bool, CascadeDeletes);

      DT_IMPLEMENT_ACCESSOR_GETTER(TrailerHitchActComp, bool, UseCurrentHitchRotToMoveTrailerWhenRemote);

      ////////////////////////////////////////////////////////////////
      void TrailerHitchActComp::SetUseCurrentHitchRotToMoveTrailerWhenRemote(bool moveTrailerWhenRemote)
      {
         mUseCurrentHitchRotToMoveTrailerWhenRemote = moveTrailerWhenRemote;
      }

   }

}
