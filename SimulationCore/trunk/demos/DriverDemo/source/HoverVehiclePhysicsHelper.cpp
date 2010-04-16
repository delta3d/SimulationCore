/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
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
#include <HoverVehiclePhysicsHelper.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
#include <osg/MatrixTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/CollisionGroupEnum.h>
#include <dtUtil/matrixutil.h>
#include <SimCore/CollisionGroupEnum.h>
#include <dtPhysics/primitivetype.h>
#include <dtPhysics/palphysicsworld.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palphysicsworld.h>
#include <dtPhysics/physicsmaterials.h>
#include <osg/Vec3>
#include <dtGame/gameactor.h>


namespace DriverDemo
{
   ///a workaround for the transform inefficiency
   //void FindMatrix(osg::Node* node, osg::Matrix& wcMatrix);

   ////////////////////////////////////////////////////////////////////////////////////
   HoverVehiclePhysicsHelper::HoverVehiclePhysicsHelper(dtGame::GameActorProxy &proxy) :
      dtPhysics::PhysicsHelper(proxy)
      , mVehicleMaxForwardMPH(120.0f)
      , mVehicleMaxStrafeMPH(40.0f)
      //, mSphereRadius(1.5f)
      , mGroundClearance(0.5)
      , mForceBoostFactor(0.25)
   {
      // non properties
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      AddPhysicsObject(*physicsObject);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::SPHERE);
      physicsObject->SetMass(1000.0f);
      physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);

   }

   ////////////////////////////////////////////////////////////////////////////////////
   HoverVehiclePhysicsHelper::~HoverVehiclePhysicsHelper(){}

   //////////////////////////////////////////////////////////////////////////////////////
   //                                  Vehicle Meths                                   //
   //////////////////////////////////////////////////////////////////////////////////////

   // These statics are put here as test variables. They need to be made permanent or
   // made into properties on the actor.
   static float testCorrection = 0.01f;
   //static float testForceBoost = 0.25f;
   static float testJumpBoost = 2.3 * -dtPhysics::DEFAULT_GRAVITY_Z;
   static float testQuicknessAdjustment = 2.8f; // 1.0 gives you a sluggish feel

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::DoJump(float deltaTime)
   {
      dtPhysics::PhysicsObject* physicsObject = GetMainPhysicsObject();

      float weight = physicsObject->GetMass();
      osg::Vec3 dir(0.0, 0.0, 1.0);
      physicsObject->ApplyImpulse(dir * weight * testJumpBoost);
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::UpdateVehicle(float deltaTime,
      bool accelForward, bool accelReverse, bool accelLeft, bool accelRight)
   {
      deltaTime = (deltaTime > 0.2) ? 0.2 : deltaTime;  // cap at 0.2 second to avoid rare 'freak' outs.
      dtPhysics::PhysicsObject* physicsObject = GetMainPhysicsObject();  // Tick Local protects us from NULL.
      float weight = physicsObject->GetMass();

      // First thing we do is try to make sure we are hovering...
      osg::Vec3 velocity = physicsObject->GetBodyWrapper()->GetLinearVelocity();
      osg::Vec3 pos = physicsObject->GetTranslation();
      osg::Vec3 posLookAhead = pos + velocity * 0.4; // where would our vehicle be in the future?
      float speed = dtUtil::Min(velocity.length(), 300.0f);  // cap for silly values - probably init values

      // Adjust position so that we are 'hovering' above the ground. The look ahead position
      // massively helps smooth out the bouncyness
      osg::Vec3 direction( 0.0f, 0.0f, -1.0f);
      float distanceToHit = 0.0;
      float futureAdjustment = ComputeEstimatedForceCorrection(posLookAhead, direction, distanceToHit);
      float currentAdjustment = ComputeEstimatedForceCorrection(pos, direction, distanceToHit);

      // modulate how we correct and other stuff, so that our vehicle
      // behaves differently as we move faster. It also gives a little downward dip as we speed up
      float modulation = 0.05;
      // Add an 'up' impulse based on the weight of the vehicle, our current time slice, and the adjustment.
      // Use current position and estimated future position to help smooth out the force.
      float finalAdjustment = currentAdjustment * (1.0f - modulation) + futureAdjustment * (modulation);
      float upForce = weight * finalAdjustment;// * deltaTime;
      osg::Vec3 dir(0.0, 0.0, 1.0);
      physicsObject->ApplyImpulse(dir * upForce * deltaTime);

      // Get the forward vector and the perpendicular side (right) vector.
      dtGame::GameActor* actor = NULL;
      GetGameActorProxy()->GetActor( actor );
      osg::Matrix matrix;
      dtCore::Transformable::GetAbsoluteMatrix( actor->GetOSGNode(), matrix);
      osg::Vec3 lookDir = dtUtil::MatrixUtil::GetRow3(matrix, 1);
      osg::Vec3 rightDir = dtUtil::MatrixUtil::GetRow3(matrix, 0);
      lookDir[2] = 0.0f; // remove Z component so we don't fly...
      lookDir.normalize();
      rightDir[2] = 0.0f; // remove Z component so we don't fly...
      rightDir.normalize();

      // basic force calc - the 2.0 makes it handle 'quicker' and wind below takes that out.
      float speedModifier = testQuicknessAdjustment * GetVehicleMaxForwardMPH() * GetForceBoostFactor();
      float strafeModifier = testQuicknessAdjustment * GetVehicleMaxStrafeMPH() * GetForceBoostFactor();

      // FORWARD
      if(accelForward)
      {
         physicsObject->AddForce(lookDir * (weight * speedModifier));
      }
      // REVERSE
      else if(accelReverse)
      {
         physicsObject->AddForce(-lookDir * (weight * strafeModifier));
      }

      // LEFT
      if(accelLeft)
      {
         physicsObject->AddForce(-rightDir * (weight * strafeModifier));
      }
      // RIGHT
      else if(accelRight)
      {
         physicsObject->AddForce(rightDir * (weight * strafeModifier));
      }

      // Apply a 'wind' resistance force based on velocity. This is what causes you to slow down and
      // prevents you from achieving unheard of velocities.
      if(speed > 0.001)
      {
         float windResistance = testQuicknessAdjustment * speed * testCorrection;

         // If we aren't doing any movement, then add an extra flavor in there so that we 'coast' to a stop.
         // The slower we're going, the more the 'flavor' boost kicks in, else we get linearly less slowdown.
         // Don't slow down if we're in the air dummy!
         if(!accelForward && !accelReverse && !accelLeft && !accelRight &&
            distanceToHit < (mGroundClearance + GetSphereRadius()))
         {
            if(speed < 4.0f) // if we're almost stopped, shut us down fast
               windResistance *= 4.0f * (GetVehicleMaxForwardMPH() + 0.2f)/ (speed + 0.2f);
            else
               windResistance *= (GetVehicleMaxForwardMPH() + 0.2f)/ (speed + 0.2f);
         }

         // Slow us down! Wind or coast effect
         //physicsObject->addForce(-velocity * (weight * windResistance * deltaTime), NX_SMOOTH_IMPULSE);
         physicsObject->AddForce(-velocity * (weight * windResistance));
      }

      // TEST HACK STUFF
      /*dtCore::Keyboard *keyboard = GetPhysicsGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
      if(keyboard == NULL)
         return;
      if(keyboard->GetKeyState('q'))
      {
         testJumpBoost *= (1.0f - 0.3 * deltaTime);
         std::cout << "Jump Boost[" << testJumpBoost << "]." << std::endl;
      }
      if(keyboard->GetKeyState('e'))
      {
         testJumpBoost *= (1.0f + 0.3 * deltaTime);
         std::cout << "Jump Boost[" << testJumpBoost << "]." << std::endl;
      }
      if(keyboard->GetKeyState('y'))
      {
         mForceBoostFactor *= (1.0f - 0.3 * deltaTime);
         std::cout << "Force Boost[" << mForceBoostFactor << "]." << std::endl;
      }
      if(keyboard->GetKeyState('u'))
      {
         mForceBoostFactor *= (1.0f + 0.3 * deltaTime);
         std::cout << "Force Boost[" << mForceBoostFactor << "]." << std::endl;
      }*/
   }

   ////////////////////////////////////////////////////////////////////////////////
   float HoverVehiclePhysicsHelper::ComputeEstimatedForceCorrection(const osg::Vec3 &location,
      const osg::Vec3 &direction, float &distanceToHit)
   {
      static const int GROUPS_FLAGS = (1 << SimCore::CollisionGroup::GROUP_TERRAIN);
      float estimatedForceAdjustment = -dtPhysics::PhysicsWorld::GetInstance().GetGravity().z();
      osg::Vec3 terrainHitLocation;

      distanceToHit = TraceRay(location, direction, terrainHitLocation, GROUPS_FLAGS);

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
   bool HoverVehiclePhysicsHelper::CreateVehicle(const dtCore::Transform& transformForRot,
      osgSim::DOFTransform* bodyNode)
   {
      dtPhysics::PhysicsObject *physObj = GetMainPhysicsObject();
      physObj->CreateFromProperties(bodyNode);
      physObj->SetTransform(transformForRot);
      physObj->SetActive(true);

      // Remove friction from our vehicle so that the physics body doesn't spin and cause
      // wierd stagger steps on the terrain
      // - NOTE - This does not help at all.
      //if(!IsRemote())
      //{
         dtPhysics::Material *material = dtPhysics::PhysicsWorld::GetInstance().GetMaterials().GetMaterial("FrictionLessMaterial");
         if (material == NULL )
         {
            dtPhysics::MaterialDef fallbackFriction;
            fallbackFriction.SetStaticFriction(0.0f);
            fallbackFriction.SetRestitution (0.3f);
            dtPhysics::PhysicsWorld::GetInstance().GetMaterials().NewMaterial ("FrictionLessMaterial", fallbackFriction );
            material = dtPhysics::PhysicsWorld::GetInstance().GetMaterials().GetMaterial("FrictionLessMaterial");
         }

         physObj->SetMaterial(material);
      //}


      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
   {
      dtPhysics::PhysicsHelper::BuildPropertyMap(toFillIn);

      const std::string& VEHICLEGROUP = "Vehicle Physics";

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Forward MPH", "Max Forward MPH",
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetVehicleMaxForwardMPH),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetVehicleMaxForwardMPH),
         "The theoretical max speed this vehicle can go (using forward thrust) in any direction under normal conditions.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Strafe Or ReverseMPH", "Max Strafe Or ReverseMPH",
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetVehicleMaxStrafeMPH),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetVehicleMaxStrafeMPH),
         "The theoretical max speed this vehicle would go using just reverse or strafe thrust under normal conditions.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Ground Clearance", "Ground Clearance",
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetGroundClearance),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetGroundClearance),
         "The height we should try to leave beneath our vehicle (cause we hover...).", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("ForceBoostFactor", "Force Boost Factor",
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetForceBoostFactor),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetForceBoostFactor),
         "Multiplied times the max speeds to determine force to apply (ex. 0.25).", VEHICLEGROUP));

   }
} // end namespace

