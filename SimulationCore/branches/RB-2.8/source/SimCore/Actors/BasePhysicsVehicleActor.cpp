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
#include <dtGame/drpublishingactcomp.h>


#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>


#include <dtCore/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
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
#include <SimCore/Utilities.h>

//Test
//#include <osg/io_utils>
//#include <iostream>

namespace SimCore
{
   namespace Actors
   {

      static const std::string CONF_TIME_WAIT_TERRAIN_PAGING("SimCore.Vehicle.SecsToWaitForTerrainPaging");

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActor::BasePhysicsVehicleActor(PlatformActorProxy& proxy)
         : Platform(proxy)
         , mTerrainPresentDropHeight(0.5f)
         , mTimeToWaitBeforeDroppingConf(1.0f)
         , mTimeToWaitBeforeDropping(1.0f)
         , mHasDriver(false)
         , mHasFoundTerrain(false)
         , mPerformAboveGroundSafetyCheck(false)
         , mPushTransformToPhysics(false)
      {
         // If you subclass this actor, you MUST do something like the following in the constructor.
         // The actor can't do it's job without having a physics helper! Might even crash!!!
         //mPhysicsActComp = new dtPhysics::PhysicsActComp(proxy);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActor::~BasePhysicsVehicleActor(void)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnEnteredWorld()
      {
         // This makes the results smoother when sending updates at a high rate.
         // This is just a default value. It can be overridden in the base class via config options.
         GetComponent<dtGame::DeadReckoningHelper>()->SetUseFixedSmoothingTime(true);

         std::string timeConfig = GetGameActorProxy().GetGameManager()->GetConfiguration().GetConfigPropertyValue(CONF_TIME_WAIT_TERRAIN_PAGING);
         if (!timeConfig.empty())
         {
            mTimeToWaitBeforeDroppingConf = dtUtil::ToType<float>(timeConfig);
            // If it's garbage, don't want to make the app get stuck.
            dtUtil::Clamp(mTimeToWaitBeforeDroppingConf, 0.0f, 60.0f);
         }
         mTimeToWaitBeforeDropping = mTimeToWaitBeforeDroppingConf;

         BaseClass::OnEnteredWorld();

         if(IsRemote()) // Remote
         {
            GetPhysicsActComp()->SetPrePhysicsCallback(
                     dtPhysics::PhysicsActComp::UpdateCallback(this, &BasePhysicsVehicleActor::PrePhysicsUpdate));
         }
         else // Local
         {
            // Post on local. Pre on both.
            GetPhysicsActComp()->SetPostPhysicsCallback(
               dtPhysics::PhysicsActComp::UpdateCallback(this, &BasePhysicsVehicleActor::PostPhysicsUpdate));
            GetPhysicsActComp()->SetPrePhysicsCallback(
               dtPhysics::PhysicsActComp::UpdateCallback(this, &BasePhysicsVehicleActor::PrePhysicsUpdate));

            dtPhysics::PhysicsObject* po = GetPhysicsActComp()->GetMainPhysicsObject();
            if (po->GetMechanicsType() == dtPhysics::MechanicsType::DYNAMIC)
            {
               // Disable gravity until the map has loaded terrain under our feet...
               // Note - you can probably do this on remote entities too, but they probably are kinematic anyway
               po->SetGravityEnabled(false);
               po->SetCollisionResponseEnabled(false);
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::IsTerrainPresent()
      {
         // DEBUG: std::cout << "Terrain loaded." << std::endl;

         dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();
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
            GetPhysicsActComp()->GetMainPhysicsObject()->ApplyImpulse(force);
         }
         else
         {
            GetPhysicsActComp()->GetMainPhysicsObject()->AddForce(force);
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
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();

         if(physicsObject == NULL)
         {
            LOG_WARNING("BAD Physics OBJECT ON VEHICLE! May occur naturally if the application is shutting down.");
            return;
         }
         bool isDynamic = true;

         if (!mHasFoundTerrain && physicsObject->GetMechanicsType() != dtPhysics::MechanicsType::DYNAMIC)
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
            if (IsTerrainPresent())
            {
               // This will set mHasFoundTerrain to true when it thinks the terrain loading has settled down.
               KeepOnGround(tickMessage.GetDeltaRealTime());

               // just to be safe, kill the velocity.  Something could be affecting the motion since the object is dynamic.
               physicsObject->SetLinearVelocity(osg::Vec3(0.0f, 0.0f, 0.0f));
               physicsObject->SetAngularVelocity(osg::Vec3(0.0f, 0.0f, 0.0f));

               if (mHasFoundTerrain)
               {
                  physicsObject->SetGravityEnabled(true);
                  physicsObject->SetCollisionResponseEnabled(true);
               }
            }
            else
            {
               physicsObject->SetGravityEnabled(false);
               physicsObject->SetCollisionResponseEnabled(false);
            }
         }
         // Check to see if we are currently up under the earth, if so, snap them back up.
         else if (GetPerformAboveGroundSafetyCheck())
         {
            KeepOnGround(tickMessage.GetDeltaRealTime());
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
         dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();
         if (physObj != NULL)
         {
            osg::Vec3 physAngularVelocity;
            physAngularVelocity = physObj->GetAngularVelocity();
            GetComponent<dtGame::DRPublishingActComp>()->SetCurrentAngularVelocity(physAngularVelocity);
         }
         else
         {
            LOG_ERROR("No physics object found. Cannot update Dead Reckoning values from physics.");
         }


         // Allow the base class to handle expected base functionality.
         // NOTE: This is called last since the vehicle's position will be final.
         //       The base TickLocal currently queries the vehicle's position and orientation.
         BaseClass::OnTickLocal(tickMessage);
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateSoundEffects(float deltaTime)
      {
         // Do nothing in the base. That's your job.
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
      {
         BaseClass::OnTickRemote(tickMessage);

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
         dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();

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
               osg::Vec3 velocity = GetComponent<dtGame::DeadReckoningHelper>()->GetLastKnownVelocity();
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
         if (!IsRemote() && GetPhysicsActComp() != NULL)
         {
            // The base behavior is that we want to pull the translation and rotation off the object
            // in our physics scene and apply it to our 3D object in the visual scene.
            dtPhysics::PhysicsObject* physicsObject = GetPhysicsActComp()->GetMainPhysicsObject();

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
         // The code in PhysicsActComp::RepositionVehicle can be refactored out

         dtPhysics::PhysicsObject* physObj = GetPhysicsActComp()->GetMainPhysicsObject();

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
            return GetComponent<dtGame::DeadReckoningHelper>()->GetLastKnownVelocity().length() * METERSPS_TO_MILESPH;
         }
         else
         {
            return GetPhysicsActComp()->GetMainPhysicsObject()->GetLinearVelocity().length()
                  * METERSPS_TO_MILESPH;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::KeepOnGround(float dt)
      {
         dtCore::Transform xform;
         GetTransform(xform);

         if (!mHasFoundTerrain)
         {
            if (GetPerformAboveGroundSafetyCheck())
            {
               if (SimCore::Utils::KeepBodyOnGround(xform, GetBoundingBox().zMax() - GetBoundingBox().zMin(), mTerrainPresentDropHeight, mTerrainPresentDropHeight * 0.9f, mTerrainPresentDropHeight * 1.1f))
               {
                  SetTransform(xform);
                  // TODO should this be an accessor and have a constant?
                  // Reset since the body was found to be out of bounds for the drop.
                  // this COULD mean that the terrain is still paging in.
                  mTimeToWaitBeforeDropping = mTimeToWaitBeforeDroppingConf;
               }
            }

            if (mTimeToWaitBeforeDropping <= 0)
            {
               mHasFoundTerrain = true;
            }
            else
            {
               mTimeToWaitBeforeDropping -= dt;
            }
         }
         else
         {
            if (SimCore::Utils::KeepBodyOnGround(xform, GetBoundingBox().zMax() - GetBoundingBox().zMin(), mTerrainPresentDropHeight))
            {
               SetTransform(xform);
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::GetTerrainPoint(
         const osg::Vec3& location, osg::Vec3& outPoint )
      {
         dtPhysics::PhysicsObject* physObject = GetPhysicsActComp()->GetMainPhysicsObject();

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
         GetPhysicsActComp()->TraceRay(ray, hits);
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
      dtPhysics::PhysicsActComp* BasePhysicsVehicleActor::GetPhysicsActComp() const
      {
         dtPhysics::PhysicsActComp* ac = NULL;
         GetComponent(ac);
         return ac;
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

         AddProperty(new dtCore::BooleanActorProperty("Perform_Above_Ground_Safety_Check",
            "Perform above ground safety check",
            dtCore::BooleanActorProperty::SetFuncType(&actor, &BasePhysicsVehicleActor::SetPerformAboveGroundSafetyCheck),
            dtCore::BooleanActorProperty::GetFuncType(&actor, &BasePhysicsVehicleActor::GetPerformAboveGroundSafetyCheck),
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
         {
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE,
               dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }

         PlatformActorProxy::OnEnteredWorld();
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtCore::ActorProperty> BasePhysicsVehicleActorProxy::GetDeprecatedProperty(const std::string& name)
      {
         dtCore::RefPtr<dtCore::ActorProperty> depProp = BaseClass::GetDeprecatedProperty(name);
         return depProp;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActorProxy::BuildActorComponents()
      {
         // Create the default if a subclass didn't create one.  The base physics vehicle does not use the
         // platform default physics actor component.
         if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
         {
            AddComponent(*new dtPhysics::PhysicsActComp());
         }

         BaseClass::BuildActorComponents();

         // DEFAULT the Dead Reckoning Algorithm to Velocity And Acceleration. It's a prop so will
         // be overwriten from the map, unless this is a new vehicle object.
         // For a default, static would be dumb. Velocity might be OK.
         dtCore::RefPtr<dtGame::DeadReckoningHelper> drAC;
         GetComponent(drAC);
         if (drAC.valid())
         {
            // default to velocity only.  Humans walk.
            drAC->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////

   } // namespace
}// namespace

