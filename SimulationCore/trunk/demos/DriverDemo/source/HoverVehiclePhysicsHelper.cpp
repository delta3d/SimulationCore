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
#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaRaycastReport.h>
#include <osg/MatrixTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/NxCollisionGroupEnum.h>
#include <dtUtil/matrixutil.h>
#include <dtABC/application.h>

// Curt - hack
#include <dtCore/keyboard.h>

namespace DriverDemo
{
   ///a workaround for the transform inefficiency
   void FindMatrix(osg::Node* node, osg::Matrix& wcMatrix);

   ////////////////////////////////////////////////////////////////////////////////////
   HoverVehiclePhysicsHelper::HoverVehiclePhysicsHelper(dtGame::GameActorProxy &proxy) :
      NxAgeiaPhysicsHelper(proxy)
      , mVehicleMaxForwardMPH(120.0f)            
      , mVehicleMaxStrafeMPH(40.0f)     
      , mVehicleBaseWeight(1000.0f)
      , mSphereRadius(1.5f)
      , mGroundClearance(0.5)
      , mForceBoostFactor(0.25)
   {
      // non properties


   }

   ////////////////////////////////////////////////////////////////////////////////////
   HoverVehiclePhysicsHelper::~HoverVehiclePhysicsHelper(){}

   ////////////////////////////////////////////////////////////////////////////////////
   dtAgeiaPhysX::NxAgeiaWorldComponent *HoverVehiclePhysicsHelper::GetPhysicsComponent() 
   {
      if (!mWorldComponent.valid())
      {
         mWorldComponent = FindWorldComponent();
         if (!mWorldComponent.valid())
         {
            LOG_ERROR("Fatal error - could not find Physics Component! Hover Vehicle will not work and may crash.");
         }
      }

      // NOTE - This class assumes the component exists.  If it doesn't, it will probably crash.
      return mWorldComponent.get();
   }

   //??? Curt What the heck??? 
   ///////////////////////////////////////////////////////////////////////////////////
   void FindMatrix(osg::Node* node, osg::Matrix& wcMatrix)
   {
      osg::NodePathList nodePathList = node->getParentalNodePaths();
      if( !nodePathList.empty() )
      {
         osg::NodePath nodePath = nodePathList[0];

         if( std::string( nodePath[0]->className() ) == std::string("CameraNode") )
         {
            nodePath = osg::NodePath( nodePath.begin()+1, nodePath.end() );
         }

         wcMatrix.set( osg::computeLocalToWorld(nodePath) );
      }
   }

   ////////////////////////////////////////////////////////////////////////////////////
   /// Reset to starting position
   void HoverVehiclePhysicsHelper::ResetToStarting()
   {
      NxAgeiaPhysicsHelper::ResetToStarting();
   }

   ////////////////////////////////////////////////////////////////////////////////////
   /// Turns it up and moves up
   void HoverVehiclePhysicsHelper::RepositionVehicle(float deltaTime)
   {
      // Do whatever you need here in addition to the base class... 
   }

   
   //////////////////////////////////////////////////////////////////////////////////////
   //                                  Vehicle Meths                                   //
   //////////////////////////////////////////////////////////////////////////////////////

   // These statics are put here as test variables. They need to be made permanent or 
   // made into properties on the actor.
   static float testCorrection = 0.01f;
   //static float testForceBoost = 0.25f;
   static float testJumpBoost = -dtAgeiaPhysX::DEFAULT_GRAVITY_Z;

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::DoJump(float deltaTime)
   {
      NxActor* physicsObject = GetPhysXObject();  // Tick Local protects us from NULL.
      float weight = GetVehicleBaseWeight();
      NxVec3 dir(0.0, 0.0, 1.0);
      physicsObject->addForce(dir * (weight * testJumpBoost), NX_SMOOTH_IMPULSE);

   }

   //static float timeTillPrint = 0.0f;
   ////////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::UpdateVehicle(float deltaTime, 
      bool accelForward, bool accelReverse, bool accelLeft, bool accelRight)
   {
      deltaTime = (deltaTime > 0.2) ? 0.2 : deltaTime;  // cap at 0.2 second to avoid rare 'freak' outs.
      float weight = GetVehicleBaseWeight();

      // First thing we do is try to make sure we are hovering...
      NxActor* physicsObject = GetPhysXObject();  // Tick Local protects us from NULL.
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
      //float futureAdjustment = currentAdjustment;

      // Add an 'up' impulse based on the weight of the vehicle, our current time slice, and the adjustment.
      // Use current position and estimated future position to help smooth out the force. 
      float finalAdjustment = currentAdjustment * 0.90 + futureAdjustment * 0.10; 
      NxVec3 dir(0.0, 0.0, 1.0);
      physicsObject->addForce(dir * (weight * finalAdjustment * deltaTime), NX_SMOOTH_IMPULSE);
      //ApplyForceToActor(dir, weight * multiplier); // old method

      // Debug code... delete me
      //timeTillPrint += deltaTime;
      //if (timeTillPrint > 0.2)
      //{
      //   timeTillPrint = 0.0f;
      //   std::cout << "Height [" << (distanceToHit) << "], Multiplier[" << finalAdjustment << "]." << std::endl;
      //}

      // Figure out which way we are looking...
      osg::Matrix cameraMatrix;
      dtCore::Transformable::GetAbsoluteMatrix(GetPhysicsGameActorProxy().
         GetGameManager()->GetApplication().GetCamera()->GetOSGNode(), cameraMatrix); 
      osg::Vec3 lookDir = dtUtil::MatrixUtil::GetRow3(cameraMatrix, 1);
      osg::Vec3 rightDir = dtUtil::MatrixUtil::GetRow3(cameraMatrix, 0);
      lookDir[2] = 0.0f; // remove Z component so we don't fly...
      lookDir.normalize();
      rightDir[2] = 0.0f; // remove Z component so we don't fly...
      rightDir.normalize();

      // basic force calc
      float speedModifier = GetVehicleMaxForwardMPH() * GetForceBoostFactor();
      float strafeModifier = GetVehicleMaxStrafeMPH() * GetForceBoostFactor();

      // FORWARD
      if(accelForward)
      {
         NxVec3 dir(lookDir[0], lookDir[1], lookDir[2]);
         physicsObject->addForce(dir * (weight * speedModifier * deltaTime), NX_SMOOTH_IMPULSE);
      }
      // REVERSE
      else if (accelReverse)
      {
         NxVec3 dir(-lookDir[0], -lookDir[1], -lookDir[2]);
         physicsObject->addForce(dir * (weight * strafeModifier * deltaTime), NX_SMOOTH_IMPULSE);
      }

      // LEFT
      if (accelLeft)
      {
         NxVec3 dir(-rightDir[0], -rightDir[1], -rightDir[2]);
         physicsObject->addForce(dir * (weight * strafeModifier * deltaTime), NX_SMOOTH_IMPULSE);
      }
      // RIGHT
      else if (accelRight)
      {
         NxVec3 dir(rightDir[0], rightDir[1], rightDir[2]);
         physicsObject->addForce(dir * (weight * strafeModifier * deltaTime), NX_SMOOTH_IMPULSE);
      }

      // Apply a 'wind' resistance force based on velocity. This is what causes you to slow down and 
      // prevents you from achieving unheard of velocities. 
      //NxVec3 velocity = physicsObject->getLinearVelocity();
      float speed = dtUtil::Min(velocity.magnitude(), 300.0f);  // cap for silly values - probably init values
      if (speed > 0.001)
      {
         float windResistance = speed * testCorrection;

         // If we aren't doing any movement, then add an extra flavor in there so that we 'coast' to a stop.
         // The slower we're going, the more the 'flavor' boost kicks in, else we get linearly less slowdown.
         // Don't slow down if we're in the air dummy!
         if (!accelForward && !accelReverse && !accelLeft && !accelRight && 
            distanceToHit < (mGroundClearance + GetSphereRadius()))
         {
            if (speed < 4.0f) // if we're almost stopped, shut us down fast
               windResistance *= 4.0f * (GetVehicleMaxForwardMPH() + 0.2f)/ (speed + 0.2f); 
            else 
               windResistance *= (GetVehicleMaxForwardMPH() + 0.2f)/ (speed + 0.2f); 
         }

         // Slow us down! Wind or coast effect
         physicsObject->addForce(-velocity * (weight * windResistance * deltaTime), NX_SMOOTH_IMPULSE);
      }
 // test
      // HACK STUFF
      dtCore::Keyboard *keyboard = GetPhysicsGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();
      if(keyboard == NULL)
         return;
      if (keyboard->GetKeyState('q'))
      {
         testJumpBoost *= (1.0f - 0.3 * deltaTime);
         std::cout << "Jump Boost[" << testJumpBoost << "]." << std::endl;
      }
      if (keyboard->GetKeyState('e'))
      {
         testJumpBoost *= (1.0f + 0.3 * deltaTime);
         std::cout << "Jump Boost[" << testJumpBoost << "]." << std::endl;
      }
      if (keyboard->GetKeyState('y'))
      {
         mForceBoostFactor *= (1.0f - 0.3 * deltaTime);
         std::cout << "Force Boost[" << mForceBoostFactor << "]." << std::endl;
      }
      if (keyboard->GetKeyState('u'))
      {
         mForceBoostFactor *= (1.0f + 0.3 * deltaTime);
         std::cout << "Force Boost[" << mForceBoostFactor << "]." << std::endl;
      }

   }

   ////////////////////////////////////////////////////////////////////////////////////
   float HoverVehiclePhysicsHelper::GetClosestIntersectionUsingDirection(
      const osg::Vec3& location, const osg::Vec3& direction , osg::Vec3& outPoint, int groupFlags)
   {
      float closestHitDistance = 0.0f;
      NxScene& scene = GetPhysicsComponent()->GetPhysicsScene(dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_SCENE_NAME);

      NxRay ray;
      ray.orig = NxVec3(location.x(),location.y(),location.z());
      ray.dir = NxVec3(direction.x(), direction.y(), direction.z());

      // CURT - NOTE - You can pass in the owner of the intersection if it's a drawable. 
      // This is useful for not intersecting yourself!
      // Create a raycast report that should ignore this vehicle.
      dtAgeiaPhysX::SimpleRaycastReport report( &GetPhysicsGameActorProxy().GetGameActor() );

      NxU32 numHits = scene.raycastAllShapes(ray, report, NX_ALL_SHAPES, groupFlags );
      if(numHits > 0 && report.HasHit())
      {
         closestHitDistance = report.GetClosestRaycastHit().distance;
         //std::cout << "Hit distance[" << closestHitDistance << "]." << std::endl;
         report.GetClosestHit( outPoint );
      }         

      // Returns 0 if no hit or distance to closest hit.  Actual hit is in outPoint
      return closestHitDistance;
   }

   ////////////////////////////////////////////////////////////////////////////////
   float HoverVehiclePhysicsHelper::ComputeEstimatedForceCorrection(const osg::Vec3 &location, 
      const osg::Vec3 &direction, float &distanceToHit)
   {
      static const int GROUPS_FLAGS = (1 << SimCore::NxCollisionGroup::GROUP_TERRAIN);
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
         else // counteract half of gravity :)
            estimatedForceAdjustment *= 0.01f; 
      }
      else 
         estimatedForceAdjustment = 0.0f;

      return estimatedForceAdjustment;

   }

/*   ////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::CurtTestMethod(float timeDelta)
   {
      static osg::Vec3 lastXYZ = osg::Vec3(0.0,0.0,0.0);
         NxScene& nxScene = GetPhysicsComponent()->GetPhysicsScene(std::string("Default"));
         dtCore::Transform ourTransform;
         vehicle->GetTransform(ourTransform);
         osg::Vec3 curXYZ;
         ourTransform.GetTranslation(curXYZ);
         if (lastXYZ[0] == 0.0)
            lastXYZ = curXYZ;

         dtAgeiaPhysX::NxAgeiaFourWheelVehiclePhysicsHelper *physicsHelper = 
            vehicle->GetPhysicsHelper();
         float weight = physicsHelper->GetVehicleBaseWeightAmount() + physicsHelper->GetVehicleGearWeightAmount() + 
            physicsHelper->GetVehicleCrewWeightAmount() * physicsHelper->GetVehicleCrewMembersOnBoard();
         //NxVec3 dir(0.0, 0.0, 180.0);
         //physicsHelper->ApplyForceToActor(dir, 10000.0);


         // Do a ray cast to see how close we are to the ground or other 'stuff'.
         NxRay ourRay;
         ourRay.orig = NxVec3(lastXYZ[0],lastXYZ[1],lastXYZ[2]);
         ourRay.dir = NxVec3(0.0, 0.0, -1.0);
         dtAgeiaPhysX::SimpleRaycastReport myReport(vehicle);//(mWeapon.valid() ? mWeapon->GetOwner() : NULL));
         // CR: Create the bit mask once rather than every time the method is called.
         static const int GROUPS_FLAGS = (1 << SimCore::NxCollisionGroup::GROUP_TERRAIN) | 
            (1 << SimCore::NxCollisionGroup::GROUP_WATER)  | (1 << SimCore::NxCollisionGroup::GROUP_VEHICLE_WATER);
         NxU32 numHits = nxScene.raycastAllShapes(ourRay, myReport, NX_ALL_SHAPES, GROUPS_FLAGS);
         if(numHits > 0 && myReport.HasHit())
         {
            if (myReport.mClosestHit.distance <= 6.0)
            {
               float multiplier = dtUtil::Max(0.0f, 2 * (6.0f - myReport.mClosestHit.distance));
               NxVec3 dir(0.0, 0.0, 1.0); // Why am I using 180???? 
               //physicsHelper->ApplyForceToActor(dir, weight * multiplier);
            }
         }         

         lastXYZ = curXYZ;
      }
   }
*/


   //////////////////////////////////////////////////////////////////////////////////////
   //                               Vehicle Initialization                             //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   bool HoverVehiclePhysicsHelper::CreateVehicle(const dtCore::Transform& transformForRot, 
      osgSim::DOFTransform* bodyNode)
   {
      // Create our vehicle with a starting position
      osg::Vec3 startVec = GetVehicleStartingPosition();
      NxVec3 startPos(startVec[0], startVec[1],startVec[2]);
      SetCollisionSphere(startPos, GetSphereRadius(), 0, 
         mVehicleBaseWeight, 0, "Default", "Default", false);

      // Reorient physics to our Y is forward system.
      NxMat33 orient;
      orient.setRow(0, NxVec3(1,0,0));
      orient.setRow(1, NxVec3(0,0,-1));
      orient.setRow(2, NxVec3(0,1,0));
      SwitchCoordinateSystem(orient);

      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   ////////////////////////////////////////////////////////////////////////////////////
   void HoverVehiclePhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
   {
      NxAgeiaPhysicsHelper::BuildPropertyMap(toFillIn);

      const std::string& VEHICLEGROUP = "Vehicle Physics";

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Forward MPH", "Max Forward MPH", 
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetVehicleMaxForwardMPH),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetVehicleMaxForwardMPH),
         "The theoretical max speed this vehicle can go (using forward thrust) in any direction under normal conditions.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Strafe Or ReverseMPH", "Max Strafe Or ReverseMPH", 
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetVehicleMaxStrafeMPH),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetVehicleMaxStrafeMPH),
         "The theoretical max speed this vehicle would go using just reverse or strafe thrust under normal conditions.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("BaseWeight", "BaseWeight", 
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetVehicleBaseWeight),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetVehicleBaseWeight),
         "The base weight of this vehicle.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Sphere Radius", "Sphere Radius", 
         dtDAL::MakeFunctor(*this, &HoverVehiclePhysicsHelper::SetSphereRadius),
         dtDAL::MakeFunctorRet(*this, &HoverVehiclePhysicsHelper::GetSphereRadius),
         "The radius of the hover sphere.", VEHICLEGROUP));

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

