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
*/
#include <prefix/SimCorePrefix-src.h>
#ifdef AGEIA_PHYSICS
//#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>
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
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/PortalActor.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

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
      , mPerformAboveGroundSafetyCheck(true)
      , mPublishLinearVelocity(true)
      , mPublishAngularVelocity(true)
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
         Platform::OnEnteredWorld();

         GetPhysicsHelper()->SetAgeiaUserData(mPhysicsHelper.get());

         // Register with the Physics Component
         dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->
            GetComponentByName("NxAgeiaWorldComponent"))->RegisterAgeiaHelper(*mPhysicsHelper.get());

         if(IsRemote())
         {
            GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
            GetPhysicsHelper()->SetObjectAsKinematic();
         }
         else // Local
         {
            // Disable gravity until the map has loaded terrain under our feet...
            // Note - you can probably do this on remote entities too, but they probably aren't kinematic anyway
            GetPhysicsHelper()->TurnObjectsGravityOff("Default");
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::IsTerrainPresent()
      {
         // DEBUG: std::cout << "Terrain loaded." << std::endl;

         NxActor* physicsObject = GetPhysicsHelper()->GetPhysXObject();
         if( physicsObject == NULL )
         {
            // DEBUG: std::cout << "\tVehicle physics object not loaded :(\n" << std::endl;
            return false;
         }

         // Check to see if we are currently up under the earth, if so, snap them back up.
         osg::Vec3 terrainPoint;
         NxVec3 pos = physicsObject->getGlobalPosition();
         osg::Vec3 location( pos.x, pos.y, pos.z );

         // DEBUG: std::cout << "\tAttempting detection at [" << location << "]...";

         // If a point was detected on the terrain...
         bool terrainDetected = GetTerrainPoint( location, terrainPoint );
         if( terrainDetected )
         {
            // DEBUG: std::cout << "DETECTED!" << std::endl;

            // ...and snap just above that point.
            physicsObject->setGlobalPosition(
               NxVec3( pos.x, pos.y, terrainPoint.z() + 5.0f ) );

            // And turn gravity on if it is off...
            if( physicsObject->readBodyFlag(NX_BF_DISABLE_GRAVITY) )
            {
               // DEBUG: std::cout << "\t\tTurning vehicle gravity ON.\n" << std::endl;

               GetPhysicsHelper()->TurnObjectsGravityOn();
            }
         }
         // DEBUG:
         /*else
         {
            std::cout << "NOT detected :(\n" << std::endl;
         }*/

         return terrainDetected;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ApplyForce( const osg::Vec3& force, const osg::Vec3& location )
      {
         // NOTE - Fix this so it uses the location provided
         //GetPhysicsHelper()->GetPhysXObject()->addForce( NxVec3(force[0],force[1],force[2]) );
         GetPhysicsHelper()->GetPhysXObject()->addForce( NxVec3(force[0],force[1],force[2]), NX_SMOOTH_IMPULSE );
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ApplyForce( const osg::Vec3& force)
      {
         GetPhysicsHelper()->GetPhysXObject()->addForce( NxVec3(force[0],force[1],force[2]), NX_SMOOTH_IMPULSE );
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::TickLocal(const dtGame::Message &tickMessage)
      {
         NxActor* physicsObject = GetPhysicsHelper()->GetPhysXObject();
         if(physicsObject == NULL)
         {
            LOG_ERROR("BAD PHYSXOBJECT ON VEHICLE!");
            return;
         }

         if(physicsObject->isSleeping())
            physicsObject->wakeUp(1e30);

         // Check if terrain is available. (For startup)
         if( ! mHasFoundTerrain )
         {
            // Terrain has not been found. Check for it again.
            mHasFoundTerrain = IsTerrainPresent();
         }
         // Check to see if we are currently up under the earth, if so, snap them back up.
         else if( GetPerformAboveGroundSafetyCheck() == true)
         {
            KeepAboveGround(physicsObject);
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
         Platform::TickLocal(tickMessage);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateSoundEffects(float deltaTime)
      {
         // Do nothing in the base. That's yer job.
      }

      bool BasePhysicsVehicleActor::CompareVectors( const osg::Vec3& op1, const osg::Vec3& op2, float epsilon )
      {
         return std::abs(op1.x() - op2.x()) < epsilon
            && std::abs(op1.y() - op2.y()) < epsilon
            && std::abs(op1.z() - op2.z()) < epsilon;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool CompareVectors( const osg::Vec3& op1, const osg::Vec3& op2, float epsilon )
      {
         return std::abs(op1.x() - op2.x()) < epsilon
            && std::abs(op1.y() - op2.y()) < epsilon
            && std::abs(op1.z() - op2.z()) < epsilon;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateDeadReckoning(float deltaTime)
      {
         // Increment how long it's been since our last DR check. We can only send out an update
         // so many times a second.
         mTimeForSendingDeadReckoningInfoOut += deltaTime;


         ///// DELETE THE FOLLOWING!!!!


         // A full update may not be required. Prevent any further network updates.
         // Let the following code determine if this vehicle should be flagged
         // for a full actor update.
         //mNotifyFullUpdate = false;
         //mNotifyPartialUpdate = false;

/*
         float amountChange = 0.5f;
         float glmat[16];
         NxActor* physxObj = mPhysicsHelper->GetPhysXObject();

         if(physxObj == NULL)
         {
            LOG_ERROR("No physics object on the hmmwv, no doing dead reckoning");
            return;
         }

         NxMat33 rotation = physxObj->getGlobalOrientation();
         rotation.getColumnMajorStride4(glmat);
         glmat[12] = physxObj->getGlobalPosition()[0];
         glmat[13] = physxObj->getGlobalPosition()[1];
         glmat[14] = physxObj->getGlobalPosition()[2];
         glmat[15] = 1.0f;
         //float zoffset = 0.0;
         float zoffset = 1.1;
         osg::Matrix currentMatrix(glmat);
         osg::Vec3 globalOrientation;
         dtUtil::MatrixUtil::MatrixToHpr(globalOrientation, currentMatrix);
         osg::Vec3 physTranslationVec;
         physTranslationVec.set(physxObj->getGlobalPosition().x, physxObj->getGlobalPosition().y, physxObj->getGlobalPosition().z + zoffset);

         const osg::Vec3 &drTranslationVec = GetDeadReckoningHelper().GetLastKnownTranslation();//GetCurrentDeadReckonedTranslation();
         const osg::Vec3 &drOrientationVec = GetDeadReckoningHelper().GetLastKnownRotation();//GetCurrentDeadReckonedRotation();

         bool changedTrans = CompareVectors(physTranslationVec, drTranslationVec, amountChange);//!dtUtil::Equivalent<osg::Vec3, float>(physTranslationVec, translationVec, 3, amountChange);
         bool changedOrient = !dtUtil::Equivalent<osg::Vec3, float>(globalOrientation, drOrientationVec, 3, 3.0f);

         const osg::Vec3 &drAngularVelocity = GetDeadReckoningHelper().GetAngularVelocityVector();
         const osg::Vec3 &drLinearVelocity = GetDeadReckoningHelper().GetVelocityVector();

         const osg::Vec3 physAngularVelocity(physxObj->getAngularVelocity().x, physxObj->getAngularVelocity().y, physxObj->getAngularVelocity().z);
         const osg::Vec3 physLinearVelocity(physxObj->getLinearVelocity().x, physxObj->getLinearVelocity().y, physxObj->getLinearVelocity().z);

         float linearVelocityDiff = (drLinearVelocity - physLinearVelocity).length();
         bool physVelocityNearZero = physLinearVelocity.length() < 0.1;
         bool changedLinearVelocity = linearVelocityDiff > 0.2 || (physVelocityNearZero && drLinearVelocity.length() > 0.1 );

         float angularVelocityDiff = (drAngularVelocity - physAngularVelocity).length();
         bool physAngularVelocityNearZero = physAngularVelocity.length() < 0.1;
         bool changedAngularVelocity = angularVelocityDiff > 0.2 || (physAngularVelocityNearZero && drAngularVelocity.length() > 0.1 );

         if( changedTrans || changedOrient || changedLinearVelocity || changedAngularVelocity)
         {
            SetLastKnownTranslation(physTranslationVec);
            SetLastKnownRotation(globalOrientation);

            if( physVelocityNearZero )
            {
               SetVelocityVector( osg::Vec3(0.f, 0.f, 0.f) );
            }
            else
            {
               SetVelocityVector(physLinearVelocity);
            }

            if ( physAngularVelocityNearZero )
            {
               SetAngularVelocityVector( osg::Vec3(0.f, 0.f, 0.f) );
            }
            else
            {
               SetAngularVelocityVector(physAngularVelocity);
            }

            // do not send the update message here but rather flag this vehicle
            // to send the update via the base class though ShouldForceUpdate.
            mNotifyFullUpdate = true;
         }
         else if( GetArticulationHelper() != NULL && GetArticulationHelper()->IsDirty() )
         {
            mNotifyPartialUpdate = true;
            // DEBUG: std::cout << "Articulation Update\n" << std::endl;
         }
         */
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

            NxActor* physxObj = mPhysicsHelper->GetPhysXObject();
            if(physxObj == NULL)
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
                  NxVec3 linearVelVec3 = physxObj->getLinearVelocity();
                  const osg::Vec3 physLinearVelocity(linearVelVec3.x, linearVelVec3.y, linearVelVec3.z);
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
                  NxVec3 angularVelVec3 = physxObj->getAngularVelocity();
                  const osg::Vec3 physAngularVelocity(angularVelVec3.x, angularVelVec3.y, angularVelVec3.z);
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
      void BasePhysicsVehicleActor::TickRemote(const dtGame::Message &tickMessage)
      {
         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         UpdateSoundEffects(ElapsedTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::AgeiaPrePhysicsUpdate()
      {
         NxActor* physicsActor = GetPhysicsHelper()->GetPhysXObject();

         // The PRE physics update is only trapped if we are remote. It updates the physics
         // engine and moves the vehicle to where we think it is now (based on Dead Reckoning)
         // We do this because we don't own remote vehicles and naturally can't just go
         // physically simulating them however we like. But, the physics scene needs them to interact with.
         // Only called if we are remote, but check for safety. Local objects are moved by the physics engine...
         if (IsRemote() && physicsActor != NULL)
         {
            osg::Matrix rot = GetMatrixNode()->getMatrix();

            // In order to make our local vehicle bounce on impact, the physics engine needs the velocity of
            // the remote entities. Essentially remote entities are kinematic (physics isn't really simulating),
            // but we want to act like their not.
            osg::Vec3 velocity = GetVelocityVector();
            NxVec3 physVelocity(velocity[0], velocity[1], velocity[2]);
            physicsActor->setLinearVelocity(physVelocity );

            // Move the remote physics object to its dead reckoned position/rotation.
            physicsActor->setGlobalPosition(NxVec3(rot.operator ()(3,0), rot.operator ()(3,1), rot.operator ()(3,2)));
            physicsActor->setGlobalOrientation(
               NxMat33( NxVec3(rot.operator ()(0,0), rot.operator ()(0,1), rot.operator ()(0,2)),
                        NxVec3(rot.operator ()(1,0), rot.operator ()(1,1), rot.operator ()(1,2)),
                        NxVec3(rot.operator ()(2,0), rot.operator ()(2,1), rot.operator ()(2,2))));
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
            NxActor* physicsActor = GetPhysicsHelper()->GetPhysXObject();
            if(physicsActor != NULL && !physicsActor->isSleeping())
            {
               dtCore::Transform ourTransform;
               //GetTransform(ourTransform);

               // Rotation
               float glmat[16];
               memset(glmat, 0, 16*sizeof(float));
               NxMat33 rotation = physicsActor->getGlobalOrientation();
               rotation.getColumnMajorStride4(glmat);
               // Translation
               glmat[12] = physicsActor->getGlobalPosition()[0];
               glmat[13] = physicsActor->getGlobalPosition()[1];
               glmat[14] = physicsActor->getGlobalPosition()[2];
               glmat[15] = 1.0f;
               osg::Matrix currentMatrix(glmat);
               ourTransform.Set(currentMatrix);

               // Translation
               //ourTransform.SetTranslation(physicsActor->getGlobalPosition()[0],
               //   physicsActor->getGlobalPosition()[1], physicsActor->getGlobalPosition()[2]);

               SetTransform(ourTransform);
            }
         }

      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport&
         contactReport, NxActor& ourSelf, NxActor& whatWeHit)
      {
         //printf("VehicleCollision\n");
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
      {
         // nothing by default
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::ResetVehicle()
      {
         GetPhysicsHelper()->ResetToStarting();
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::UpdateVehicleTorquesAndAngles(float deltaTime)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      float BasePhysicsVehicleActor::GetMPH()
      {
         return GetVelocityVector().length() * 2.236936291;
         //return GetPhysicsHelper()->GetMPH();
         //return 0.0f;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::RepositionVehicle(float deltaTime)
      {
         // This behavior is semi duplicated in the four wheel vehicle helper.
         // It has been moved here to allow vehicles to have separate behavior. Eventually,
         // we need a base vehicle helper and it should be moved there.
         // The code in PhysicsHelper::RepositionVehicle can be refactored out

         NxActor* physActor = GetPhysicsHelper()->GetPhysXObject();
         float glmat[16];

         for(int i = 0 ; i < 16; i++)
            glmat[i] = 0.0f;
         glmat[0]  = glmat[5]  = glmat[10] = glmat[15] = 1.0f;
         glmat[12] = physActor->getGlobalPosition()[0];
         glmat[13] = physActor->getGlobalPosition()[1];
         glmat[14] = physActor->getGlobalPosition()[2];
         glmat[10] = 1.0f;

         float xyOffset = deltaTime;
         float zOffset = deltaTime * 5.0f;

         GetPhysicsHelper()->SetTransform(NxVec3(physActor->getGlobalPosition()[0] + xyOffset,
            physActor->getGlobalPosition()[1] + xyOffset, physActor->getGlobalPosition()[2] + zOffset), osg::Matrix(glmat));

         GetPhysicsHelper()->ResetForces();
         //GetPhysicsHelper()->RepositionVehicle(deltaTime);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void BasePhysicsVehicleActor::KeepAboveGround( NxActor* physicsObject )
      {
         bool underearth = false;
         std::vector<dtDAL::ActorProxy*> toFill;
         GetGameActorProxy().GetGameManager()->FindActorsByName("Terrain", toFill);
         dtDAL::ActorProxy* terrainNode = NULL;
         if(!toFill.empty())
            terrainNode = (dynamic_cast<dtDAL::ActorProxy*>(&*toFill[0]));

         NxVec3 position = physicsObject->getGlobalPosition();

         osg::Vec3 hp;
         dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
         iSector->SetScene( &GetGameActorProxy().GetGameManager()->GetScene() );
         iSector->SetQueryRoot(terrainNode->GetActor());
         dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
         osg::Vec3 pos( position.x, position.y, position.z );
         osg::Vec3 endPos = pos;
         pos[2] -= 100.0f;
         endPos[2] += 100.0f;
         float offsettodo = 5.0f;
         SingleISector.SetSectorAsLineSegment(pos, endPos);
         if( iSector->Update(osg::Vec3(0,0,0), true) )
         {
            if( SingleISector.GetNumberOfHits() > 0 )
            {
               SingleISector.GetHitPoint(hp);

               if(position[2] + offsettodo < hp[2])
               {
                  underearth = true;
               }
            }
         }

         if(underearth)
         {
            physicsObject->setGlobalPosition(
               NxVec3(position[0], position[1], hp[2] + offsettodo));
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      bool BasePhysicsVehicleActor::GetTerrainPoint(
         const osg::Vec3& location, osg::Vec3& outPoint )
      {
         NxActor* physxObject = GetPhysicsHelper()->GetPhysXObject();
         if( physxObject == NULL )
         {
            return false;
         }

         NxRay ray;
         ray.orig = NxVec3(location.x(),location.y(),location.z());
         ray.orig.z += 10000.0f;
         ray.dir = NxVec3(0.0f,0.0f,-1.0f);

         // Create a raycast report that should ignore this vehicle.
         dtAgeiaPhysX::SimpleRaycastReport report( this );
         static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);

         NxU32 numHits = physxObject->getScene().raycastAllShapes(
            ray, report, NX_ALL_SHAPES, GROUPS_FLAGS );

         if( numHits > 0 && report.HasHit() )
         {
            report.GetClosestHit( outPoint );
            return true;
         }
         return false;
      }


      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      BasePhysicsVehicleActorProxy::BasePhysicsVehicleActorProxy()
      {
         //SetClassName("NxAgeiaFourWheeledVehicleActor");
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
      /*void BasePhysicsVehicleActorProxy::CreateActor()
      {
         SetActor(*new BasePhysicsVehicleActor(*this));

         BaseEntity* entityActor = dynamic_cast<BaseEntity*> (GetActor());
         if( entityActor != NULL )
         {
            entityActor->InitDeadReckoningHelper();
         }
      }*/

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
#endif
