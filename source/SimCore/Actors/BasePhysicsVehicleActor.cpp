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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>


#ifdef AGEIA_PHYSICS

#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>

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
#include <dtGame/basemessages.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>
#include <osgViewer/View>
#include <SimCore/Components/ArticulationHelper.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/PortalActor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <osg/io_utils>

namespace SimCore
{
   namespace Actors
   {

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActor ::BasePhysicsVehicleActor(PlatformActorProxy &proxy)
         : Platform(proxy)
         , mHasDriver(false)
         , mHasFoundTerrain(false)
         , mNotifyFullUpdate(true)
         , mNotifyPartialUpdate(true)
         , mPerformAboveGroundSafetyCheck(false)
         , mPublishLinearVelocity(true)
         , mPublishAngularVelocity(true)
         , mPushTransformToPhysics(false)
      {
         mTimeForSendingDeadReckoningInfoOut = 0.0f;
         mTimesASecondYouCanSendOutAnUpdate  = 3.0f;

         // If you subclass this actor, you MUST do something like the following in the constructor.
         // The actor can't do it's job without having a physics helper! Might even crash!!!
         //mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaVehiclePhysicsHelper(proxy);
         //mPhysicsHelper->SetBaseInterfaceClass(this);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActor::~BasePhysicsVehicleActor(void)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();


         // Register with the Physics Component
         dtPhysics::PhysicsComponent* physicsComp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComp);
         physicsComp->RegisterHelper(*mPhysicsHelper.get());


#ifdef AGEIA_PHYSICS
         if(IsRemote())
         {
            GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
            GetPhysicsHelper()->SetObjectAsKinematic();
         }
         else // Local
         {
            GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
            // Disable gravity until the map has loaded terrain under our feet...
            // Note - you can probably do this on remote entities too, but they probably aren't kinematic anyway
            GetPhysicsHelper()->TurnObjectsGravityOff("Default");
         }
#else
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
            // Note - you can probably do this on remote entities too, but they probably aren't kinematic anyway
            //GetPhysicsHelper()->TurnObjectsGravityOff("Default");
         }
#endif
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
#ifdef AGEIA_PHYSICS
         NxVec3 pos = physicsObject->getGlobalPosition();
         osg::Vec3 location( pos.x, pos.y, pos.z );
#else
         dtPhysics::TransformType xform;
         dtPhysics::VectorType location;
         physicsObject->GetTransform(xform);
         xform.GetTranslation(location);
#endif
         osg::Vec3 terrainPoint;

         // DEBUG: std::cout << "\tAttempting detection at [" << location << "]...";

         // If a point was detected on the terrain...
         bool terrainDetected = GetTerrainPoint( location, terrainPoint );
         if( terrainDetected )
         {
            // DEBUG: std::cout << "DETECTED!" << std::endl;

            // ...and snap just above that point.

            terrainPoint.z() += 5.0f;
#ifdef AGEIA_PHYSICS
            physicsObject->setGlobalPosition(
               NxVec3( pos.x, pos.y, terrainPoint.z()) );

            // And turn gravity on if it is off...
            if( physicsObject->readBodyFlag(NX_BF_DISABLE_GRAVITY) )
            {
               // DEBUG: std::cout << "\t\tTurning vehicle gravity ON.\n" << std::endl;

               GetPhysicsHelper()->TurnObjectsGravityOn();
            }
#else
            xform.SetTranslation(terrainPoint);
            physicsObject->SetTransform(xform);
#endif
         }
         // DEBUG:
         /*else
         {
            std::cout << "NOT detected :(\n" << std::endl;
         }*/

         return terrainDetected;
      }

#ifdef AGEIA_PHYSICS
      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ApplyForce(const osg::Vec3& force, const osg::Vec3& location, bool isImpulse)
      {
         if (isImpulse)
            GetPhysicsHelper()->GetMainPhysicsObject()->addForce(NxVec3(force[0],force[1],force[2]), NX_SMOOTH_IMPULSE);
         else
            GetPhysicsHelper()->GetMainPhysicsObject()->addForce(NxVec3(force[0],force[1],force[2]), NX_FORCE);
      }

#else
      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ApplyForce(const osg::Vec3& force, const osg::Vec3& location, bool isImpulse)
      {
         if (isImpulse)
            GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->AddForce(force);
         else
            GetPhysicsHelper()->GetMainPhysicsObject()->GetBodyWrapper()->ApplyImpulse(force);
      }

#endif
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
            LOG_ERROR("BAD Physics OBJECT ON VEHICLE! May occur naturally if the application is shutting down.");
            return;
         }
#ifdef AGEIA_PHYSICS
         if(physicsObject->isSleeping())
            physicsObject->wakeUp();

#else
         if (!physicsObject->IsActive())
         {
            physicsObject->SetActive(true);
         }
#endif
         // Check if terrain is available. (For startup)
         if( ! mHasFoundTerrain )
         {
            // Terrain has not been found. Check for it again.
            mHasFoundTerrain = IsTerrainPresent();
         }
         // Check to see if we are currently up under the earth, if so, snap them back up.
         else if( GetPerformAboveGroundSafetyCheck() == true)
         {
            KeepAboveGround();
         }

         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         //                                          Update everything else                                      //
         //////////////////////////////////////////////////////////////////////////////////////////////////////////
         float elapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         UpdateVehicleTorquesAndAngles(elapsedTime);
         UpdateRotationDOFS(elapsedTime, true);
         UpdateSoundEffects(elapsedTime);
         UpdateDeadReckoning(elapsedTime);

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
      void BasePhysicsVehicleActor::UpdateDeadReckoning(float deltaTime)
      {
         // Increment how long it's been since our last DR check. We can only send out an update
         // so many times a second.
         mTimeForSendingDeadReckoningInfoOut += deltaTime;

      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::ShouldForceUpdate(
         const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
      {
         bool forceUpdateResult = fullUpdate; // if full update set, we assume we will publish
         bool enoughTimeHasPassed = (mTimesASecondYouCanSendOutAnUpdate > 0.0f &&
            (mTimeForSendingDeadReckoningInfoOut > 1.0f / mTimesASecondYouCanSendOutAnUpdate));

         if(fullUpdate || enoughTimeHasPassed)
         {
            // Let parent determine if we are outside our threshold values
            if (!fullUpdate)
               forceUpdateResult = Platform::ShouldForceUpdate(pos, rot, fullUpdate);

            dtPhysics::PhysicsObject* physObj = mPhysicsHelper->GetMainPhysicsObject();
            if(physObj == NULL)
            {
               LOG_ERROR("No physics object on BasePhysicsVehicleActor::ShouldForceUpdate(), no doing dead reckoning");
               return false;
            }

            // If we are going to update, then set our velocities so others can do remote dead reckoning.
            if (forceUpdateResult)
            {
               // Linear Velocity - set our linear velocity from the physics object in preparation for a publish
               if (mPublishLinearVelocity)
               {
                  osg::Vec3 physLinearVelocity;
#ifdef AGEIA_PHYSICS
                  NxVec3 linearVelVec3 = physObj->getLinearVelocity();
                  physLinearVelocity.set(linearVelVec3.x, linearVelVec3.y, linearVelVec3.z);
#else
                  physLinearVelocity = physObj->GetLinearVelocity();
#endif
                  // If the value is very close to 0, set it to zero to prevent warbling
                  bool physVelocityNearZero = physLinearVelocity.length() < 0.1;
                  if( physVelocityNearZero )
                     SetVelocityVector( osg::Vec3(0.f, 0.f, 0.f) );
                  else
                     SetVelocityVector(physLinearVelocity);
               }
               else
                  SetVelocityVector( osg::Vec3(0.f, 0.f, 0.f) );


               // Angular Velocity - set our angular velocity from the physics object in preparation for a publish
               if (mPublishAngularVelocity)
               {
                  osg::Vec3 physAngularVelocity;
#ifdef AGEIA_PHYSICS
                  NxVec3 angVelVec3 = physObj->getAngularVelocity();
                  physAngularVelocity.set(angVelVec3.x, angVelVec3.y, angVelVec3.z);
#else
                  physAngularVelocity = physObj->GetAngularVelocity();
#endif
                  // If the value is very close to 0, set it to zero to prevent warbling
                  bool physAngularVelocityNearZero = physAngularVelocity.length() < 0.1;
                  if ( physAngularVelocityNearZero ) //
                     SetAngularVelocityVector( osg::Vec3(0.f, 0.f, 0.f) );
                  else
                     SetAngularVelocityVector(physAngularVelocity);
               }
               else
                  SetAngularVelocityVector( osg::Vec3(0.f, 0.f, 0.f) );

               // Since we are about to publish, set our time since last publish back to 0.
               // This allows us to immediately send out a change the exact moment it happens (ex if we
               // were moving slowly and hadn't updated recently).
               mTimeForSendingDeadReckoningInfoOut = 0.0f;
            }

         }

         return forceUpdateResult;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float BasePhysicsVehicleActor::GetPercentageChangeDifference(float startValue, float newValue)
      {
         if(std::abs(startValue) < 0.01f && std::abs(newValue) < 0.01f)
            return 1.0;

         if(startValue == 0)
            startValue = 1.0f;

         return std::abs((((newValue - startValue) / startValue) * 100.0f));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::OnTickRemote(const dtGame::TickMessage &tickMessage)
      {
         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
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

#ifdef AGEIA_PHYSICS
      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::AgeiaPrePhysicsUpdate()
      {
         NxActor* physObject = GetPhysicsHelper()->GetMainPhysicsObject();

         // The PRE physics update is only trapped if we are remote. It updates the physics
         // engine and moves the vehicle to where we think it is now (based on Dead Reckoning)
         // We do this because we don't own remote vehicles and naturally can't just go
         // physically simulating them however we like. But, the physics scene needs them to interact with.
         if (physObject != NULL && (IsRemote() || GetPushTransformToPhysics()))
         {
            mPushTransformToPhysics = false;
            dtCore::Transform xform;
            GetTransform(xform);
            osg::Matrix mat;
            xform.Get(mat);

            // In order to make our local vehicle bounce on impact, the physics engine needs the velocity of
            // the remote entities. Essentially remote entities are kinematic (physics isn't really simulating),
            // but we want to act like their not.
            if (IsRemote())
            {
               osg::Vec3 velocity = GetVelocityVector();
               NxVec3 physVelocity(velocity[0], velocity[1], velocity[2]);
               physObject->setLinearVelocity(physVelocity );
            }
            // Move the remote physics object to its dead reckoned position/rotation.
            physObject->setGlobalPosition(NxVec3(mat(3,0), mat(3,1), mat(3,2)));
            physObject->setGlobalOrientation(
                                     NxMat33( NxVec3(mat(0,0), mat(1,0), mat(2,0)),
                                              NxVec3(mat(0,1), mat(1,1), mat(2,1)),
                                              NxVec3(mat(0,2), mat(1,2), mat(2,2))));
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::AgeiaPostPhysicsUpdate()
      {
         // This is ONLY called if we are LOCAL (we put the check here just in case... )
         if (!IsRemote() && GetPhysicsHelper() != NULL)
         {
            // The base behavior is that we want to pull the translation and rotation off the object
            // in our physics scene and apply it to our 3D object in the visual scene.
            dtPhysics::PhysicsObject* physicsActor = GetPhysicsHelper()->GetMainPhysicsObject();
            if (physicsActor != NULL && !GetPushTransformToPhysics() && !physicsActor->isSleeping())
            {
               dtCore::Transform ourTransform;
               //GetTransform(ourTransform);

               // Rotation
               float glmat[16];
               memset(glmat, 0, 16 * sizeof(float));
               NxMat33 rotation = physicsActor->getGlobalOrientation();
               rotation.getColumnMajorStride4(glmat);
               // Translation
               glmat[12] = physicsActor->getGlobalPosition()[0];
               glmat[13] = physicsActor->getGlobalPosition()[1];
               glmat[14] = physicsActor->getGlobalPosition()[2];
               glmat[15] = 1.0f;
               osg::Matrix currentMatrix(glmat);
               ourTransform.Set(currentMatrix);
               //std::cout << ourTransform.GetTranslation() << std::endl;

               // Translation
               //ourTransform.SetTranslation(physicsActor->getGlobalPosition()[0],
               //   physicsActor->getGlobalPosition()[1], physicsActor->getGlobalPosition()[2]);

               SetTransform(ourTransform);
               SetPushTransformToPhysics(false);
            }
         }
      }

#else

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
               osg::Vec3 velocity = GetVelocityVector();
               physicsObject->GetBodyWrapper()->SetLinearVelocity(velocity);
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

#endif

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
      {
         // nothing by default
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
      {
      }

#ifdef AGEIA_PHYSICS
      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ResetVehicle()
      {
         GetPhysicsHelper()->ResetToStarting();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::RepositionVehicle(float deltaTime)
      {
         // This behavior is semi duplicated in the four wheel vehicle helper.
         // It has been moved here to allow vehicles to have separate behavior. Eventually,
         // we need a base vehicle helper and it should be moved there.
         // The code in PhysicsHelper::RepositionVehicle can be refactored out

         dtPhysics::PhysicsObject* physObj = GetPhysicsHelper()->GetMainPhysicsObject();
         float glmat[16];

         for(int i = 0 ; i < 16; i++)
            glmat[i] = 0.0f;
         glmat[0]  = glmat[5]  = glmat[10] = glmat[15] = 1.0f;
         glmat[12] = physObj->getGlobalPosition()[0];
         glmat[13] = physObj->getGlobalPosition()[1];
         glmat[14] = physObj->getGlobalPosition()[2];
         glmat[10] = 1.0f;

         float xyOffset = deltaTime;
         float zOffset = deltaTime * 5.0f;

         GetPhysicsHelper()->SetTransform(NxVec3(physObj->getGlobalPosition()[0] + xyOffset,
            physObj->getGlobalPosition()[1] + xyOffset, physObj->getGlobalPosition()[2] + zOffset), osg::Matrix(glmat));


         GetPhysicsHelper()->ResetForces();
      }
      ///////////////////////////////////////////////////////////////////////////////////
      float BasePhysicsVehicleActor::GetMPH()
      {
         static const float METERSPS_TO_MILESPH = 2.236936291;
         if (IsRemote())
         {
            return GetVelocityVector().length() * METERSPS_TO_MILESPH;
         }
         else
         {
            return GetPhysicsHelper()->GetMainPhysicsObject()->getLinearVelocity().magnitude()
                  * METERSPS_TO_MILESPH;
         }
      }

#else
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
      float BasePhysicsVehicleActor::GetMPH()
      {
         static const float METERSPS_TO_MILESPH = 2.236936291;
         if (IsRemote())
         {
            return GetVelocityVector().length() * METERSPS_TO_MILESPH;
         }
         else
         {
            return GetPhysicsHelper()->GetMainPhysicsObject()->GetLinearVelocity().length()
                  * METERSPS_TO_MILESPH;
         }
      }
#endif
      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::KeepAboveGround()
      {
         bool underearth = false;
         std::vector<dtDAL::ActorProxy*> toFill;
         GetGameActorProxy().GetGameManager()->FindActorsByName("Terrain", toFill);
         dtDAL::ActorProxy* terrainNode = NULL;
         if(!toFill.empty())
            terrainNode = (dynamic_cast<dtDAL::ActorProxy*>(&*toFill[0]));


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
         if( iSector->Update(osg::Vec3(0,0,0), true) )
         {
            if( SingleISector.GetNumberOfHits() > 0 )
            {
               SingleISector.GetHitPoint(hp);

               if(pos[2] + offsettodo < hp[2])
               {
                  underearth = true;
               }
            }
         }

         if (underearth)
         {
            // This should just set the regular position.  When the physics engine should get
            // prephysics every time, not just for remote.
            pos.z() = hp[2] + offsettodo;
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
#ifdef AGEIA_PHYSICS
         NxRay ray;
         ray.orig = NxVec3(rayStart.x(), rayStart.y(), rayStart.z());
         ray.dir = NxVec3(0.0f,0.0f,-1.0f);

         // Create a raycast report that should ignore this vehicle.
         dtAgeiaPhysX::SimpleRaycastReport report( this );
         static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);

         unsigned int numHits = physObject->getScene().raycastAllShapes(
            ray, report, NX_ALL_SHAPES, GROUPS_FLAGS );

         if( numHits > 0 && report.HasHit() )
         {
            report.GetClosestHit( outPoint );
#else
         dtPhysics::RayCast ray;
         ray.SetOrigin(rayStart);
         ray.SetDirection(osg::Vec3(0.0f, 0.0f, -20000.0f));
         dtPhysics::RayCast::Report report;
         if (dtPhysics::PhysicsWorld::GetInstance().TraceRay(ray, report))
         {
            outPoint = report.mHitPos;
#endif

            return true;
         }
         return false;
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
         const std::string& VEH_GROUP   = "Vehicle Property Values";

         PlatformActorProxy::BuildPropertyMap();

         BasePhysicsVehicleActor  &actor = static_cast<BasePhysicsVehicleActor &>(GetGameActor());

         // Add all the properties from the physics helper class
         std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
         actor.GetPhysicsHelper()->BuildPropertyMap(toFillIn);
         for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
            AddProperty(toFillIn[i].get());

         AddProperty(new dtDAL::BooleanActorProperty("Perform_Above_Ground_Safety_Check",
            "Perform above ground safety check",
            dtDAL::MakeFunctor(actor, &BasePhysicsVehicleActor::SetPerformAboveGroundSafetyCheck),
            dtDAL::MakeFunctorRet(actor, &BasePhysicsVehicleActor::GetPerformAboveGroundSafetyCheck),
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

   } // namespace
}// namespace

