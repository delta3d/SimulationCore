/* -*-c++-*-
* Simulation Core
* Copyright 2008, Alion Science and Technology
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
* @author Curtiss Murphy
* @author David Guthrie
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/Actors/DRPublishingActComp.h>


#ifdef AGEIA_PHYSICS

#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>
#include <PhysicsGlobals.h>

#else

#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>

#endif


#include <dtDAL/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/batchisector.h>
#include <dtCore/keyboard.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/PortalActor.h>
#include <SimCore/CollisionGroupEnum.h>


//Test
#include <osg/io_utils>
#include <iostream>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActor::BasePhysicsVehicleActor(PlatformActorProxy& proxy)
         : Platform(proxy)
         , mTerrainPresentDropHeight(0.5f)
         , mHasDriver(false)
         , mHasFoundTerrain(false)
         , mPerformAboveGroundSafetyCheck(false)
         , mPushTransformToPhysics(false)
      {
         // If you subclass this actor, you MUST do something like the following in the constructor.
         // The actor can't do it's job without having a physics helper! Might even crash!!!
         //mPhysicsHelper = new dtPhysics::PhysicsHelper(proxy);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActor::~BasePhysicsVehicleActor(void)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();

         // DEFAULT the Dead Reckoning Algorithm to Velocity Only. It's a prop so will 
         // be overwriten from the map, unless this is a new vehicle object. 
         // For a default, static would be dumb and it is very difficult to make 
         // 'Velocity and Acceleration' be smooth because acceleration is so finicky.
         if (!HasComponent(dtGame::DeadReckoningHelper::TYPE))
         {
            GetDeadReckoningHelper().SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnEnteredWorld()
      {
         // This makes the results smoother when sending updates at a high rate.
         // This is just a default value. It can be overridden in the base class via config options.
         GetDeadReckoningHelper().SetAlwaysUseMaxSmoothingTime(true);

         BaseClass::OnEnteredWorld();

         // Register with the Physics Component
         dtPhysics::PhysicsComponent* physicsComp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComp);
         physicsComp->RegisterHelper(*mPhysicsHelper.get());

         if(IsRemote()) // Remote
         {
            GetPhysicsHelper()->SetPrePhysicsCallback(
                     dtPhysics::PhysicsHelper::UpdateCallback(this, &BasePhysicsVehicleActor::PrePhysicsUpdate));
         }
         else // Local
         {
            // Post on local. Pre on both.
            GetPhysicsHelper()->SetPostPhysicsCallback(
               dtPhysics::PhysicsHelper::UpdateCallback(this, &BasePhysicsVehicleActor::PostPhysicsUpdate));
            GetPhysicsHelper()->SetPrePhysicsCallback(
               dtPhysics::PhysicsHelper::UpdateCallback(this, &BasePhysicsVehicleActor::PrePhysicsUpdate));

            // Disable gravity until the map has loaded terrain under our feet...
            // Note - you can probably do this on remote entities too, but they probably are kinematic anyway
            GetPhysicsHelper()->GetMainPhysicsObject()->SetGravityEnabled(false);
            GetPhysicsHelper()->GetMainPhysicsObject()->SetCollisionResponseEnabled(false);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();
         dtPhysics::PhysicsComponent* physicsComp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComp);
         physicsComp->UnregisterHelper(*mPhysicsHelper.get());
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::IsTerrainPresent()
      {
         // DEBUG: std::cout << "Terrain loaded." << std::endl;

         dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();
         if( physicsObject == NULL )
         {
            // DEBUG: std::cout << "\tVehicle physics object not loaded :(\n" << std::endl;
            return false;
         }

         // Check to see if we are currently up under the earth, if so, snap them back up.
         dtPhysics::TransformType xform;
         dtPhysics::VectorType location;
         physicsObject->GetTransformAsVisual(xform);
         xform.GetTranslation(location);

         osg::Vec3 terrainPoint;

         // If a point was detected on the terrain...
         bool terrainDetected = GetTerrainPoint( location, terrainPoint );
         if( terrainDetected )
         {
            // ...and snap just above that point.

            terrainPoint.z() += mTerrainPresentDropHeight;
            xform.SetTranslation(terrainPoint);
            physicsObject->SetTransformAsVisual(xform);
            physicsObject->SetGravityEnabled(true);
            physicsObject->SetCollisionResponseEnabled(true);
         }

         return terrainDetected;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::SetTerrainPresentDropHeight(float zDistance)
      {
         mTerrainPresentDropHeight = zDistance;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float BasePhysicsVehicleActor::GetTerrainHeightDropHeight() const
      {
         return mTerrainPresentDropHeight;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ApplyForce(const osg::Vec3& force, const osg::Vec3& location, bool isImpulse)
      {
         if (isImpulse)
         {
            GetPhysicsHelper()->GetMainPhysicsObject()->ApplyImpulse(force);
         }
         else
         {
            GetPhysicsHelper()->GetMainPhysicsObject()->AddForce(force);
         }
      }


      ///////////////////////////////////////////////////////////////////////////////////
      /// Overridden so that it will flag the actor as being transformed when you set the position.
      void BasePhysicsVehicleActor::SetTransform(const dtCore::Transform& xform, dtCore::Transformable::CoordSysEnum cs)
      {
         Platform::SetTransform(xform, cs);
         mPushTransformToPhysics = true;
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();

         if(physicsObject == NULL)
         {
            LOG_WARNING("BAD Physics OBJECT ON VEHICLE! May occur naturally if the application is shutting down.");
            return;
         }
         bool isDynamic = true;

         if (physicsObject->GetMechanicsType() != dtPhysics::MechanicsType::DYNAMIC)
         {
            isDynamic = false;
            //No need to look for terrain on static objects.
            mHasFoundTerrain = true;
         }

         if (isDynamic && !physicsObject->IsActive())
         {
            physicsObject->SetActive(true);
         }

         // Check if terrain is available. (For startup)
         if (!mHasFoundTerrain)
         {
            // Terrain has not been found. Check for it again.
            mHasFoundTerrain = IsTerrainPresent();
         }
         // Check to see if we are currently up under the earth, if so, snap them back up.
         else if (GetPerformAboveGroundSafetyCheck())
         {
            KeepAboveGround();
         }

         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         //                                          Update everything else                                      //
         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         float elapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();

         // Previously - updated DR values here.


         UpdateVehicleTorquesAndAngles(elapsedTime);
         UpdateRotationDOFS(elapsedTime, true);
         UpdateSoundEffects(elapsedTime);


         // ANGULAR VELOCITY - uses physics engine
         dtPhysics::PhysicsObject* physObj = mPhysicsHelper->GetMainPhysicsObject();
         if (physObj != NULL)
         {
            osg::Vec3 physAngularVelocity;
            physAngularVelocity = physObj->GetAngularVelocity();
            GetDRPublishingActComp()->SetCurrentAngularVelocity(physAngularVelocity);
         }
         else
         {
            LOG_ERROR("No physics object found. Cannot update Dead Reckoning values from physics.");
         }


         // Allow the base class to handle expected base functionality.
         // NOTE: This is called last since the vehicle's position will be final.
         //       The base TickLocal currently queries the vehicle's position and orientation.
         Platform::OnTickLocal(tickMessage);
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateSoundEffects(float deltaTime)
      {
         // Do nothing in the base. That's yer job.
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         float ElapsedTime = tickMessage.GetDeltaSimTime();
         UpdateSoundEffects(ElapsedTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::GetPushTransformToPhysics() const
      {
         return mPushTransformToPhysics;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::SetPushTransformToPhysics(bool flag)
      {
         mPushTransformToPhysics = flag;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::PrePhysicsUpdate()
      {
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();

         // The PRE physics update is only trapped if we are remote. It updates the physics
         // engine and moves the vehicle to where we think it is now (based on Dead Reckoning)
         // We do this because we don't own remote vehicles and naturally can't just go
         // physically simulating them however we like. But, the physics scene needs them to interact with.
         // Only called if we are remote, but check for safety. Local objects are moved by the physics engine...
         if (physicsObject != NULL  && (IsRemote() || GetPushTransformToPhysics()))
         {
            SetPushTransformToPhysics(false);

            if (IsRemote())
            {
               // In order to make our local vehicle bounce on impact, the physics engine needs the velocity of
               // the remote entities. Essentially remote entities are kinematic (physics isn't really simulating),
               // but we want to act like their not.
               osg::Vec3 velocity = GetDeadReckoningHelper().GetLastKnownVelocity();
               physicsObject->SetLinearVelocity(velocity);
            }

            dtCore::Transform xform;
            GetTransform(xform);
            physicsObject->SetTransformAsVisual(xform);
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::PostPhysicsUpdate()
      {
         // This is ONLY called if we are LOCAL (we put the check here just in case... )
         if (!IsRemote() && GetPhysicsHelper() != NULL)
         {
            // The base behavior is that we want to pull the translation and rotation off the object
            // in our physics scene and apply it to our 3D object in the visual scene.
            dtPhysics::PhysicsObject* physicsObject = GetPhysicsHelper()->GetMainPhysicsObject();

            //TODO: Ask if the object is activated.  If not, the transform should not be pushed.

            if (!GetPushTransformToPhysics())
            {
               if(physicsObject != NULL)
               {
                  dtCore::Transform xform;
                  physicsObject->GetTransformAsVisual(xform);
                  SetTransform(xform);
                  SetPushTransformToPhysics(false);
               }
            }
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
      {
         // nothing by default
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ResetVehicle()
      {
         //this is a stupid method.
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::RepositionVehicle(float deltaTime)
      {
         // This behavior is semi duplicated in the four wheel vehicle helper.
         // It has been moved here to allow vehicles to have separate behavior. Eventually,
         // we need a base vehicle helper and it should be moved there.
         // The code in PhysicsHelper::RepositionVehicle can be refactored out

         dtPhysics::PhysicsObject* physObj = GetPhysicsHelper()->GetMainPhysicsObject();

         float xyOffset = deltaTime;
         float zOffset = deltaTime * 5.0f;

         dtCore::Transform xform;
         physObj->GetTransform(xform);
         osg::Vec3 pos;
         xform.GetTranslation(pos);
         pos.x() += xyOffset;
         pos.y() += xyOffset;
         pos.z() += zOffset;
         xform.SetTranslation(pos);
         physObj->SetTransform(xform);

         physObj->GetBodyWrapper()->ResetForces();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float BasePhysicsVehicleActor::GetMPH() const
      {
         static const float METERSPS_TO_MILESPH = 2.236936291;
         if (IsRemote())
         {
            return GetDeadReckoningHelper().GetLastKnownVelocity().length() * METERSPS_TO_MILESPH;
         }
         else
         {
            return GetPhysicsHelper()->GetMainPhysicsObject()->GetLinearVelocity().length()
                  * METERSPS_TO_MILESPH;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::KeepAboveGround()
      {
         // assume we are under earth unless we get proof otherwise.
         // because some checks could THINK we are under earth, especially if you drive / move
         // under a bridge or something
         bool underearth = true;
         dtDAL::ActorProxy* terrainNode = NULL;
         GetGameActorProxy().GetGameManager()->FindActorByName("Terrain", terrainNode);
         if (terrainNode == NULL)
         {
            return;
         }

         dtCore::Transform xform;

         GetTransform(xform);
         osg::Vec3 pos;
         xform.GetTranslation(pos);

         osg::Vec3 hp;
         dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
         iSector->SetScene( &GetGameActorProxy().GetGameManager()->GetScene() );
         iSector->SetQueryRoot(terrainNode->GetActor());
         dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
         osg::Vec3 endPos = pos;
         osg::Vec3 startPos = pos;
         startPos[2] -= 100.0f;
         endPos[2] += 100.0f;
         float offsettodo = 5.0f;
         SingleISector.SetSectorAsLineSegment(startPos, endPos);
         if (iSector->Update(osg::Vec3(0,0,0), true))
         {
            for (unsigned i = 0; i < SingleISector.GetNumberOfHits(); ++i)
            {
               SingleISector.GetHitPoint(hp, i);

               if (pos[2] + offsettodo > hp[2])
               {
                  underearth = false;
                  break;
               }
            }
         }
         else
         {
            // can't be under earth if there is no earth...
            underearth = false;
         }

         if (underearth)
         {
            // This should just set the regular position.  When the physics engine should get
            // prephysics every time, not just for remote.
            pos.z() = hp[2] + mTerrainPresentDropHeight;
            xform.SetTranslation(pos);
            SetTransform(xform);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::GetTerrainPoint(
         const osg::Vec3& location, osg::Vec3& outPoint )
      {
         dtPhysics::PhysicsObject* physObject = GetPhysicsHelper()->GetMainPhysicsObject();

         if( physObject == NULL )
         {
            return false;
         }

         osg::Vec3 rayStart(location);
         rayStart.z() += 10000.0f;
         dtPhysics::RayCast ray;
         std::vector<dtPhysics::RayCast::Report> hits;
         ray.SetOrigin(rayStart);
         ray.SetDirection(osg::Vec3(0.0f, 0.0f, -20000.0f));
         static const dtPhysics::CollisionGroupFilter GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
         ray.SetCollisionGroupFilter(GROUPS_FLAGS);
         GetPhysicsHelper()->TraceRay(ray, hits);
         if (!hits.empty())
         {
            const dtPhysics::RayCast::Report* closestReport = NULL;
            std::vector<dtPhysics::RayCast::Report>::const_iterator i, iend;
            i = hits.begin();
            iend = hits.end();
            for (; i != iend; ++i)
            {
               const dtPhysics::RayCast::Report& report = *i;
               // Get the report closest to the location of the vehicle.
               if (closestReport == NULL
                        || (location - report.mHitPos).length2() < (location - closestReport->mHitPos).length2())
               {
                  closestReport = &report;
               }
            }
            // closestReport can't be null here because it can't get in this branch
            // if hits is empty.
            outPoint = closestReport->mHitPos;

            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::SetPhysicsHelper(dtPhysics::PhysicsHelper* newHelper)
      {
         mPhysicsHelper = newHelper;
      }

      //////////////////////////////////////////////////////////////////////
      dtPhysics::PhysicsHelper* BasePhysicsVehicleActor::GetPhysicsHelper() const
      {
         return mPhysicsHelper.get();
      }

      //////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::SetHasDriver(bool hasDriver)
      {
         mHasDriver = hasDriver;
      }
      //////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::GetHasDriver() const
      {
         return mHasDriver;
      }

      //////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::SetPerformAboveGroundSafetyCheck(bool enable)
      {
         mPerformAboveGroundSafetyCheck = enable;
      }
      //////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::GetPerformAboveGroundSafetyCheck() const
      {
         return mPerformAboveGroundSafetyCheck;
      }

      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActorProxy::BasePhysicsVehicleActorProxy()
      {
         SetClassName("BasePhysicsVehicleActorProxy");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActorProxy::BuildPropertyMap()
      {
         const dtUtil::RefString VEH_GROUP("Vehicle Property Values");

         PlatformActorProxy::BuildPropertyMap();

         BasePhysicsVehicleActor& actor = static_cast<BasePhysicsVehicleActor &>(GetGameActor());

         // Add all the properties from the physics helper class
         std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
         actor.GetPhysicsHelper()->BuildPropertyMap(toFillIn);
         for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
         {
            AddProperty(toFillIn[i].get());
         }

         AddProperty(new dtDAL::BooleanActorProperty("Perform_Above_Ground_Safety_Check",
            "Perform above ground safety check",
            dtDAL::BooleanActorProperty::SetFuncType(&actor, &BasePhysicsVehicleActor::SetPerformAboveGroundSafetyCheck),
            dtDAL::BooleanActorProperty::GetFuncType(&actor, &BasePhysicsVehicleActor::GetPerformAboveGroundSafetyCheck),
            "Use an Isector as a safety check to keep the vehicle above ground if the collision detection fails.",
            VEH_GROUP));

      }

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActorProxy::~BasePhysicsVehicleActorProxy(){}

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActorProxy::OnEnteredWorld()
      {
         //RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

         if (IsRemote())
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE,
               dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);

         PlatformActorProxy::OnEnteredWorld();
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::ActorProperty> BasePhysicsVehicleActorProxy::GetDeprecatedProperty(const std::string& name)
      {
#ifndef AGEIA_PHYSICS
         BasePhysicsVehicleActor* actor = NULL;
         GetActor(actor);
         return actor->GetPhysicsHelper()->GetDeprecatedProperty(name);
#else
         return NULL;
#endif
      }

   } // namespace
}// namespace

