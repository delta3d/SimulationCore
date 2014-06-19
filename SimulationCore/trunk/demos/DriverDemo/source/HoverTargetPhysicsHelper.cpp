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
#include <osg/MatrixTransform>
#include <dtGame/exceptionenum.h>
#include <dtCore/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/matrixutil.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/PhysicsTypes.h>
#include <dtPhysics/primitivetype.h>
#include <dtPhysics/palphysicsworld.h>
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>


namespace DriverDemo
{

   ////////////////////////////////////////////////////////////////////////////////////
   HoverTargetPhysicsActComp::HoverTargetPhysicsActComp()
   : dtPhysics::PhysicsActComp()
   , mGroundClearance(4.0)
   {
      // non properties
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::SPHERE);
      physicsObject->SetMass(100.0f);
      physicsObject->SetExtents(osg::Vec3(1.0f, 1.0f, 1.0f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
   }

   ////////////////////////////////////////////////////////////////////////////////////
   HoverTargetPhysicsActComp::~HoverTargetPhysicsActComp(){}

   //////////////////////////////////////////////////////////////////////////////////////
   //                                  Vehicle Meths                                   //
   //////////////////////////////////////////////////////////////////////////////////////

   // These statics are put here as test variables. They need to be made permanent or
   // made into properties on the actor.
//   static float testCorrection = 0.01f;
//   static float testQuicknessAdjustment = 2.8f; // 1.0 gives you a sluggish feel

   ////////////////////////////////////////////////////////////////////////////////
   void HoverTargetPhysicsActComp::ApplyTargetHoverForces(float deltaTime, osg::Vec3 &goalLocation)
   {
      dtPhysics::PhysicsObject* physicsObject = GetMainPhysicsObject();  // Tick Local protects us from NULL.
      deltaTime = (deltaTime > 0.2) ? 0.2 : deltaTime;  // cap at 0.2 second to avoid rare 'freak' outs.
      float weight = physicsObject->GetMass();

      // First thing we do is try to make sure we are hovering...
      osg::Vec3 velocity = physicsObject->GetBodyWrapper()->GetLinearVelocity();
      osg::Vec3 pos = physicsObject->GetTranslation();
      osg::Vec3 posLookAhead = pos + velocity * 0.4; // where would our vehicle be in the future?
//      float speed = dtUtil::Min(velocity.length(), 300.0f);  // cap for silly values - probably init values


      // Adjust position so that we are 'hovering' above the ground. The look ahead position
      // massively helps smooth out the bounciness
      osg::Vec3 direction( 0.0f, 0.0f, -1.0f);
      float distanceToHit = 0.0;
      float futureAdjustment = ComputeEstimatedForceCorrection(posLookAhead, direction, distanceToHit);
      float currentAdjustment = ComputeEstimatedForceCorrection(pos, direction, distanceToHit);

      // modulate how we correct and other stuff, so that our vehicle
      // behaves differently as we move faster. It also gives a little downward dip as we speed up
      float modulation = 0.15;
      // Add an 'up' impulse based on the weight of the vehicle, our current time slice, and the adjustment.
      // Use current position and estimated future position to help smooth out the force.
      float finalAdjustment = currentAdjustment * (1.0f - modulation) + futureAdjustment * (modulation);
      float upForce = weight * finalAdjustment;// * deltaTime;
      osg::Vec3 dir(0.0, 0.0, 1.0);
      osg::Vec3 upImpulse = dir * upForce * deltaTime;
      physicsObject->ApplyImpulse(dir * upForce * deltaTime);


      // Now, figure out how to move our target toward our goal. We want it to sort of oscillate a bit,
      // so we play with the forces a tad.
      osg::Vec3 targetVector =  goalLocation - pos;
      osg::Vec3 targetVectorLookAhead = goalLocation - posLookAhead;
      float distanceAway = targetVector.length() * 0.4 + targetVectorLookAhead.length() * 0.6;
      float distanceAwayPercent = (distanceAway) / mGroundClearance;
      float forceAdjustment = dtUtil::Min(10.0f, distanceAwayPercent);
      targetVector.normalize();
      targetVector[2] = 0.01; // cancel out the z - that's up above.
      targetVector[1] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
      targetVector[0] += dtUtil::RandFloat(0.0, 0.15); // cause minor fluctuations
      osg::Vec3 hoverForce = targetVector * weight * forceAdjustment * deltaTime; 
      physicsObject->ApplyImpulse(hoverForce);

      mTotalForceAppliedLastTime = upImpulse + hoverForce;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void HoverTargetPhysicsActComp::ApplyForceFromLastFrame(float deltaTime)
   {
      dtPhysics::PhysicsObject* physicsObject = GetMainPhysicsObject();  // Tick Local protects us from NULL.
      physicsObject->ApplyImpulse(mTotalForceAppliedLastTime);
   }

   ////////////////////////////////////////////////////////////////////////////////
   float HoverTargetPhysicsActComp::ComputeEstimatedForceCorrection(const osg::Vec3& location,
      const osg::Vec3&  direction, float& distanceToHit)
   {
      static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
      float estimatedForceAdjustment = -dtPhysics::PhysicsWorld::GetInstance().GetGravity().z();
      osg::Vec3 terrainHitLocation;

      distanceToHit = TraceRay(location,
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
         {
            estimatedForceAdjustment *= (0.01f + ((1.0f+distanceToCorrect) * 0.99)); // part gravity, part reduced
         }
         else // opportunity to counteract free fall if we want.
         {
            estimatedForceAdjustment = 0.0f;//0.01f;
         }
      }
      else
      {
         estimatedForceAdjustment = 0.0f;
      }

      return estimatedForceAdjustment;

   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                               Vehicle Initialization                             //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   bool HoverTargetPhysicsActComp::CreateTarget(const dtCore::Transform& transformForRot,
      osg::Node* bodyNode)
   {
      dtPhysics::PhysicsObject *physObj = GetMainPhysicsObject();
      physObj->CreateFromProperties(bodyNode);
      physObj->SetTransform(transformForRot);
      physObj->SetActive(true);

      // LOCAL - Configure Physics
      //if(!isRemote)
      //{
      //   SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_GET_COLLISION_REPORT |
      //      dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
      //}
      // REMOTE- Configure Physics
      //else // -- Flags set in the base class.
      //GetPhysicsActComp()->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);

      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverTargetPhysicsActComp::BuildPropertyMap()
   {
      BaseClass::BuildPropertyMap();
      const std::string GROUP = "Vehicle Property Values";

      AddProperty(new dtCore::FloatActorProperty("Ground Clearance", "Ground Clearance",
               dtCore::FloatActorProperty::SetFuncType(this, &HoverTargetPhysicsActComp::SetGroundClearance),
               dtCore::FloatActorProperty::GetFuncType(this, &HoverTargetPhysicsActComp::GetGroundClearance),
         "The height we should try to leave beneath our vehicle (cause we hover...).", GROUP));

   }

   ////////////////////////////////////////////////////////////////////////////////////
   float HoverTargetPhysicsActComp::GetSphereRadius()
   {
      dtPhysics::PhysicsObject* physicsObject = GetMainPhysicsObject();
      if (physicsObject != NULL)
      {
         return physicsObject->GetExtents().x();
      }
      else
      {
         return 0.0;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   float HoverTargetPhysicsActComp::GetGroundClearance() const
   {
      return mGroundClearance;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverTargetPhysicsActComp::SetGroundClearance(float value)
   {
      mGroundClearance = value;
   }

} // end namespace

