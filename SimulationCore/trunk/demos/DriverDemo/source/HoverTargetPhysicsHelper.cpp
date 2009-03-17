/*
* Copyright, 2009, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Curtiss Murphy
*/
#include <HoverTargetPhysicsHelper.h>
#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>
#include <osg/MatrixTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/CollisionGroupEnum.h>
#include <dtUtil/matrixutil.h>

#include <SimCore/PhysicsTypes.h>

namespace DriverDemo
{

   ////////////////////////////////////////////////////////////////////////////////////
   HoverTargetPhysicsHelper::HoverTargetPhysicsHelper(dtGame::GameActorProxy &proxy) :
      NxAgeiaPhysicsHelper(proxy)
      , mVehicleBaseWeight(100.0f)
      , mSphereRadius(1.0f)
      , mGroundClearance(4.0)
   {
      // non properties
   }

   ////////////////////////////////////////////////////////////////////////////////////
   HoverTargetPhysicsHelper::~HoverTargetPhysicsHelper(){}

   //////////////////////////////////////////////////////////////////////////////////////
   //                                  Vehicle Meths                                   //
   //////////////////////////////////////////////////////////////////////////////////////

   // These statics are put here as test variables. They need to be made permanent or
   // made into properties on the actor.
   static float testCorrection = 0.01f;
   static float testQuicknessAdjustment = 2.8f; // 1.0 gives you a sluggish feel

   ////////////////////////////////////////////////////////////////////////////////
   void HoverTargetPhysicsHelper::ApplyTargetHoverForces(float deltaTime, osg::Vec3 &goalLocation)
   {
      deltaTime = (deltaTime > 0.2) ? 0.2 : deltaTime;  // cap at 0.2 second to avoid rare 'freak' outs.
      float weight = GetVehicleBaseWeight();

      // First thing we do is try to make sure we are hovering...
      dtPhysics::PhysicsObject* physicsObject = GetMainPhysicsObject();  // Tick Local protects us from NULL.
      NxVec3 velocity = physicsObject->getLinearVelocity();
      NxVec3 pos = physicsObject->getGlobalPosition();
      NxVec3 posLookAhead = pos + velocity * 0.5; // where would our vehicle be in .5 seconds?


      // Adjust position so that we are 'hovering' above the ground. The look ahead position
      // massively helps smooth out the bouncyness
      osg::Vec3 location(pos.x, pos.y, pos.z);
      osg::Vec3 locationLookAhead(posLookAhead.x, posLookAhead.y, posLookAhead.z);
      osg::Vec3 direction( 0.0f, 0.0f, -1.0f);
      float distanceToHit = 0.0;
      float futureAdjustment = ComputeEstimatedForceCorrection(locationLookAhead, direction, distanceToHit);
      float currentAdjustment = ComputeEstimatedForceCorrection(location, direction, distanceToHit);

      // Add an 'up' impulse based on the weight of the vehicle, our current time slice, and the adjustment.
      // Use current position and estimated future position to help smooth out the force.
      float finalAdjustment = currentAdjustment * 0.85 + futureAdjustment * 0.15;
      NxVec3 dir(0.0, 0.0, 1.0);
      physicsObject->addForce(dir * (weight * finalAdjustment * deltaTime), NX_SMOOTH_IMPULSE);

      // Now, figure out how to move our target toward our goal. We want it to sort of oscillate a bit,
      // so we play with the forces a tad.
      osg::Vec3 targetVector =  goalLocation - location;
      osg::Vec3 targetVectorLookAhead = goalLocation - locationLookAhead;
      float distanceAway = targetVector.length() * 0.4 + targetVectorLookAhead.length() * 0.6;
      float distanceAwayPercent = (distanceAway) / mGroundClearance;
      float forceAdjustment = dtUtil::Min(10.0f, distanceAwayPercent);
      targetVector.normalize();
      targetVector[2] = 0.01; // cancel out the z - that's up above.
      targetVector[1] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
      targetVector[0] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
      NxVec3 hoverDir(targetVector[0], targetVector[1], targetVector[2]);
      physicsObject->addForce(hoverDir * (weight * forceAdjustment * deltaTime), NX_SMOOTH_IMPULSE);

   }

   ////////////////////////////////////////////////////////////////////////////////
   float HoverTargetPhysicsHelper::ComputeEstimatedForceCorrection(const osg::Vec3 &location,
      const osg::Vec3 &direction, float &distanceToHit)
   {
      static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
      float estimatedForceAdjustment = -dtAgeiaPhysX::DEFAULT_GRAVITY_Z; // gravity
      osg::Vec3 terrainHitLocation;

      distanceToHit = GetClosestIntersectionUsingDirection(location,
         direction, terrainHitLocation, GROUPS_FLAGS);

      if (distanceToHit > 0.0f)
      {
         // Apply gravity force at the exact height we want. More below, less above.
         float distanceToCorrect = GetSphereRadius() + mGroundClearance - distanceToHit;
         if (distanceToCorrect >= 0.0f) // allow a 1 meter buffer to smoothly decay force
         {
            float distanceAwayPercent = distanceToCorrect / mGroundClearance;
            estimatedForceAdjustment *= (1.0f + (distanceAwayPercent * distanceAwayPercent));
         }
         else if (distanceToCorrect > -1.0f) // if we are slightly too high, then slow down our force
            estimatedForceAdjustment *= (0.01f + ((1.0f+distanceToCorrect) * 0.99)); // part gravity, part reduced
         else // opportunity to counteract free fall if we want.
            estimatedForceAdjustment = 0.0f;//0.01f;
      }
      else
         estimatedForceAdjustment = 0.0f;

      return estimatedForceAdjustment;

   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                               Vehicle Initialization                             //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   bool HoverTargetPhysicsHelper::CreateTarget(osg::Vec3 &startVec, bool isRemote)
   {
      // Create our Physics Sphere!
      NxVec3 startPos(startVec[0], startVec[1],startVec[2]);
      SetCollisionSphere(startPos, mSphereRadius, 0, mVehicleBaseWeight, 0, "Default", "Default", false);
      dtPhysics::PhysicsObject *physActor = GetMainPhysicsObject();
      if (physActor == NULL)
      {
         throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_ACTOR_STATE,
            "Critical Error! Failed to create the Physics Object during Target creation.", __FILE__, __LINE__);
      }

      // Reorient physics to our Y is forward system.
      NxMat33 orient;
      orient.setRow(0, NxVec3(1,0,0));
      orient.setRow(1, NxVec3(0,0,-1));
      orient.setRow(2, NxVec3(0,1,0));
      SwitchCoordinateSystem(orient);

      // LOCAL - Configure Physics
      if(!isRemote)
      {
         SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_GET_COLLISION_REPORT |
            dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
         //GetHoverPhysicsHelper()->TurnObjectsGravityOff("Default");
      }
      // REMOTE- Configure Physics
      //else // -- Flags set in the base class.
      //GetPhysicsHelper()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);


      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverTargetPhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
   {
      NxAgeiaPhysicsHelper::BuildPropertyMap(toFillIn);

      const std::string& GROUP = "Vehicle Property Values";

      toFillIn.push_back(new dtDAL::FloatActorProperty("BaseWeight", "BaseWeight",
         dtDAL::MakeFunctor(*this, &HoverTargetPhysicsHelper::SetVehicleBaseWeight),
         dtDAL::MakeFunctorRet(*this, &HoverTargetPhysicsHelper::GetVehicleBaseWeight),
         "The base weight of this vehicle.", GROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Sphere Radius", "Sphere Radius",
         dtDAL::MakeFunctor(*this, &HoverTargetPhysicsHelper::SetSphereRadius),
         dtDAL::MakeFunctorRet(*this, &HoverTargetPhysicsHelper::GetSphereRadius),
         "The radius of the hover sphere.", GROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Ground Clearance", "Ground Clearance",
         dtDAL::MakeFunctor(*this, &HoverTargetPhysicsHelper::SetGroundClearance),
         dtDAL::MakeFunctorRet(*this, &HoverTargetPhysicsHelper::GetGroundClearance),
         "The height we should try to leave beneath our vehicle (cause we hover...).", GROUP));

   }
} // end namespace

