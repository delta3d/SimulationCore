/* -*-c++-*-
 * Simulation Core
 * Copyright 2007-2008, Alion Science and Technology
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
 * @author Allen Danklefsen, Bradley Anderegg, Stephen Westin
 */

#include <SimCore/BaseWheeledVehiclePhysicsHelper.h>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osgSim/DOFTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtCore/transform.h>

#ifndef AGEIA_PHYSICS
#include <dtPhysics/bodywrapper.h>
#include <pal/palVehicle.h>
#include <pal/palFactory.h>
#endif

namespace SimCore
{

   /// Constructor
   BaseVehiclePhysicsHelper::BaseVehiclePhysicsHelper(dtGame::GameActorProxy &proxy)
   : dtPhysics::PhysicsHelper(proxy)
   , mEngineTorque(300.0f)
   , mVehicleTopSpeed(120.0f)
   , mVehicleTopSpeedReverse(40.0f)
   , mHorsePower(150)
   , mVehicleMass(3500.0f)
   , mWheelTurnRadiusPerUpdate(0.03f)
   , mWheelInverseMass(0.15)
   , mWheelRadius(0.5f)
   , mWheelWidth(0.25f)
   , mMaxSteerAngle(45.0f)
   , mWheelSuspensionTravel(0.35f)
   , mSuspensionSpringCoef(600.0f)
   , mSuspensionSpringDamper(15.0f)
   , mSuspensionSpringTarget(0.0f)
   , mTireExtremumSlip(1.0f)
   , mTireExtremumValue(2.0f)
   , mTireAsymptoteSlip(2.0f)
   , mTireAsymptoteValue(1.0f)
   , mTireStiffnessFactor(1000.0f)
   , mTireRestitution(0.5f)
   {
#ifndef AGEIA_PHYSICS
//Create the vehicle here so we can add wheels any time.
      mVehicle = dynamic_cast<palVehicle*>(palFactory::GetInstance()->CreateObject("palVehicle"));
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("chassis");
      AddPhysicsObject(*physicsObject);
#endif
   }

   /// Destructor
   BaseVehiclePhysicsHelper::~BaseVehiclePhysicsHelper()
   {
#ifndef AGEIA_PHYSICS
      delete mVehicle;
#endif
   }

   /// A workaround for the transform inefficiency
   void BaseVehiclePhysicsHelper::GetLocalMatrix(osgSim::DOFTransform* node, osg::Matrix& wcMatrix)
   {
      if(node->getNumParents() > 0)
      {
         osg::MatrixTransform* parentNode = dynamic_cast<osg::MatrixTransform*>(node->getParent(0));
         if(parentNode != NULL)
         {
            wcMatrix = parentNode->getMatrix();
         }
      }
   }


   // ///////////////////////////////////////////////////////////////////////////////////
   //                               Vehicle Initialization                             //
   // ///////////////////////////////////////////////////////////////////////////////////

#ifdef AGEIA_PHYSICS

   /// Adds a single wheel to the vehicle.
   ///
   /// @param actor       NXActor for the vehicle body
   /// @param position    3D offset from body CG to tire CG
   ///
   /// @retval            Pointer to the new wheel, which is already attached to
   ///                    actor.
   /// @retval            NULL if the wheel could not be created.
   WheelType BaseVehiclePhysicsHelper::AddWheel(const osg::Vec3& position)
   {
      if (GetMainPhysicsObject() == NULL)
         return NULL;

      WheelType wheel;
      wheel.mPowered = powered;
      wheel.mSteered = steered;
      wheel.mBraked = braked;

      NxWheelShapeDesc wheelShapeDesc;

      NxTireFunctionDesc lngTFD;
      lngTFD.extremumSlip = mTireExtremumSlip;
      lngTFD.extremumValue = mTireExtremumValue;
      lngTFD.asymptoteSlip = mTireAsymptoteSlip;
      lngTFD.asymptoteValue = mTireAsymptoteValue;
      lngTFD.stiffnessFactor = mTireStiffnessFactor; // effects acceleration

      std::string NameofMaterial = "WheelNoFrictionMaterial";
      dtPhysics::PhysicsComponent* worldComponent = NULL;
      mGameProxy->GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, worldComponent);

      if(worldComponent != NULL)
      {
         worldComponent->RegisterMaterial(NameofMaterial, mTireRestitution, 0.2f, 0.1f);
         unsigned flags = worldComponent->GetMaterial(NameofMaterial, "Default", true).getFlags();

         flags |= NX_MF_DISABLE_FRICTION;
         worldComponent->GetMaterial(NameofMaterial, "Default", true).setFlags(flags);
         wheelShapeDesc.materialIndex = worldComponent->GetMaterialIndex(NameofMaterial, "Default", true);
      }
      else
      {
         wheelShapeDesc.materialIndex = 0;
      }

      osg::Vec3 newPosition = position + GetLocalOffSet();
      wheelShapeDesc.localPose.t = dtPhysics::VectorType(newPosition[0], newPosition[1], newPosition[2]);

      wheelShapeDesc.suspension.spring       = mSuspensionSpringCoef;
      wheelShapeDesc.suspension.damper       = mSuspensionSpringDamper;
      wheelShapeDesc.suspension.targetValue  = mSuspensionSpringTarget;
      wheelShapeDesc.radius                  = mWheelRadius;
      wheelShapeDesc.suspensionTravel        = mWheelSuspensionTravel;
      wheelShapeDesc.inverseWheelMass        = mWheelInverseMass;

      //TODO- do we need to different versions of the properties to handle this?
      wheelShapeDesc.longitudalTireForceFunction = lngTFD;
      wheelShapeDesc.lateralTireForceFunction = lngTFD;

      NxWheelShape* wheelShape = static_cast<NxWheelShape*>(GetMainPhysicsObject()->createShape(wheelShapeDesc));

      if(wheelShape == NULL)
      {
         dtUtil::Log::GetInstance().LogMessage ( dtUtil::Log::LOG_ERROR, std::string(__FILE__), "Could not create a wheel; was the inverse mass zero?" );
      }

      WheelType wheel;
      wheel.mPowered = powered;
      wheel.mSteered = steered;
      wheel.mBraked = braked;
      wheel.mWheel = wheelShape

      mWheels.push_back(wheel);

      return wheel;
   }
#else
   WheelType BaseVehiclePhysicsHelper::AddWheel(const osg::Vec3& position, bool powered, bool steered, bool braked)
   {
      WheelType wheel;
      //pal keeps track of this, but I set the anyway for completeness.
      wheel.mPowered = powered;
      wheel.mSteered = steered;
      wheel.mBraked = braked;

      wheel.mWheel = mVehicle->AddWheel();

      wheel.mWheel->Init(position.x(), position.y(), position.z(),
               mWheelRadius, mWheelWidth, mSuspensionSpringTarget,
               mSuspensionSpringCoef,
               mSuspensionSpringDamper, powered, steered, braked,
               mWheelSuspensionTravel, mTireRestitution);

      mWheels.push_back(wheel);

      return wheel;
   }
#endif

   /////////////////////////////////////////////////////////
   bool BaseVehiclePhysicsHelper::CreateChassis(const dtCore::Transform& transformForRot, osgSim::DOFTransform* bodyNode)
   {
      // make sure bad data wasn't passed in
      if(bodyNode == NULL)
      {
         return false;
      }

      osg::Matrix BodyMatrix;
      GetLocalMatrix(bodyNode, BodyMatrix);


#ifdef AGEIA_PHYSICS
      osg::Matrix rot;
      transformForRot.GetRotation(rot);

      // we set the actor's relative position to the position of the dof_chassis
      // by setting the translation of this matrix
      NxMat34 sendInMatrix(NxMat33( dtPhysics::VectorType(rot(0,0), rot(0,1), rot(0,2)),
               dtPhysics::VectorType(rot(1,0), rot(1,1), rot(1,2)),
               dtPhysics::VectorType(rot(2,0), rot(2,1), rot(2,2))),
               dtPhysics::VectorType(BodyMatrix.getTrans()[0],
                        BodyMatrix.getTrans()[1],
                        BodyMatrix.getTrans()[2]));

      //we set the local offset to zero so the convex mesh below will be created relative to
      //the position of the actor which is set to be position of the dof_chassis above
      SetLocalOffSet(osg::Vec3(0.0f, 0.0f, 0.0f));

      dtPhysics::PhysicsObject* physicsObject = SetCollisionConvexMesh(bodyNode, sendInMatrix, 0, mVehicleMass, false, "", "Default", "Default", 0);

      dtPhysics::VectorType massPos = physicsObject->getCMassLocalPosition();
      massPos.x += GetVehiclesCenterOfMass()[0];
      massPos.y += GetVehiclesCenterOfMass()[1];
      massPos.z += GetVehiclesCenterOfMass()[2];
      physicsObject->setCMassOffsetLocalPosition(massPos);
#else
      GetMainPhysicsObject()->CreateFromProperties(bodyNode);
#endif

      return true;

   }

   void BaseVehiclePhysicsHelper::Control(float acceleration, float normalizedWheelAngle, float normalizedBrakes)
   {

#ifdef AGEIA_PHYSICS
      float maxWheelAngle = GetMaxSteerAngle() * osg::PI / 180.0f;   // convert from deg to rad
      float steeringAngle = maxWheelAngle * normalizeWheelAngle;

      dtUtil::Clamp(normalizedBrakes, 0.0f, 1.0f);

      float brakeTorque = normalizedBrakes * GetMaxBrakeTorque();  //!< @todo Need to use max brake pressure value that includes air pressure for trucks or malfunctions

      wheelTorque = -mAccelerator * GetEngineTorque();

      for(int i = 0 ; i < mWheels.size(); ++i)
      {
         WheelType& curWheel = mWheels[i];
         if (curWheel.mBraked)
         {
            curWheel.mWheel->setBrakeTorque(brakeTorque);
         }

         if (curWheel.mPowered)
         {
            curWheel->setMotorTorque(wheelTorque);
         }

         if (curWheel.mSteered)
         {
            curWheel->setSteerAngle(-steeringAngle);
         }
      }
#else
      mVehicle->ForceControl(normalizedWheelAngle, acceleration, normalizedBrakes);
#endif

   }

   void BaseVehiclePhysicsHelper::FinalizeInitialization()
   {
#ifndef AGEIA_PHYSICS
      mVehicle->Init(&GetMainPhysicsObject()->GetBodyWrapper()->GetPalBody(), GetEngineTorque(),
               GetMaxBrakeTorque());
#endif
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   /// Builds the property map for this vehicle.
   ///
   /// @param toFillIn    dtDAL::ActorProperty for this vehicle

   void BaseVehiclePhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
   {
      dtPhysics::PhysicsHelper::BuildPropertyMap(toFillIn);

      static const dtUtil::RefString VEHICLEGROUP("Vehicle Physics");
      static const dtUtil::RefString WHEELGROUP("Wheel Physics");

      toFillIn.push_back(new dtDAL::FloatActorProperty("Engine Torque", "Engine Torque",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetEngineTorque),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetEngineTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Top Speed (MPH)", "Top Speed (MPH)",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleTopSpeed),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleTopSpeed),
               "Top speed of vehicle", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("mVehicleTopSpeedReverse", "mVehicleTopSpeedReverse",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleTopSpeedReverse),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleTopSpeedReverse),
               "Top speed in reverse", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::IntActorProperty("Horsepower", "Horsepower",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleHorsePower),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleHorsePower),
               "Currently unused", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("mWheelTurnRadiusPerUpdate", "mWheelTurnRadiusPerUpdate",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleTurnRadiusPerUpdate),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleTurnRadiusPerUpdate),
               "", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Vehicle Mass", "Vehicle Mass",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleMass),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleMass),
               "Mass of entire vehicle.  Don't use this for dtPhysics.  The physics object itself has a mass.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Inverse Mass of Wheel", "Inverse Mass of Wheel",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelInverseMass),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelInverseMass),
               "This is not used for dtPhysics.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Wheel Radius", "Wheel Radius",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelRadius),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelRadius),
               "Rolling radius of wheel", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Wheel Width", "Wheel Width",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelWidth),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelWidth),
               "Width of the wheel.  PhysX doesn't use this.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Travel", "Suspension Travel",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelSuspensionTravel),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelSuspensionTravel),
               "Total suspension travel from full rebound to full jounce", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Spring Rate", "Spring Rate",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetSuspensionSpringCoef),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetSuspensionSpringCoef),
               "Spring rate (force/distance) at wheel", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Damping Coefficient", "Suspension Damping Coefficient",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetSuspensionSpringDamper),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetSuspensionSpringDamper),
               "Coefficient of linear damping (force/velocity)", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Spring Target", "Suspension Spring Target",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetSuspensionSpringTarget),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetSuspensionSpringTarget),
               "Target value position of spring where the spring force is zero.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Steer Angle", "Max Steer Angle",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetMaxSteerAngle),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetMaxSteerAngle),
               "The maximum angle the wheel can steer (rotate about its vertical axis) in degrees.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Extreme Slip", "Tire Extreme Slip",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireExtremumSlip),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireExtremumSlip),
               "Extremal point of curve.  Values must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Extreme Value", "Tire Extreme Value",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireExtremumValue),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireExtremumValue),
               "Extremal point of curve.  Values must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Asymptote Slip", "Tire Asymptote Slip",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireAsymptoteSlip),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireAsymptoteSlip),
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Asymptote Value", "Tire Asymptote Value",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireAsymptoteValue),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireAsymptoteValue),
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Stiffness", "Tire Stiffness",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireStiffnessFactor),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireStiffnessFactor),
               "This is an additional overall positive scaling that gets applied to the tire forces before passing them to the solver.  Higher values make for better grip.  If you raise the values above, you may need to lower this. A setting of zero will disable all friction in this direction.",
               WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Restitution", "Tire Restitution",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireRestitution),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireRestitution),
               "Coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.  Note that values close to or above 1 may cause stability problems and/or increasing energy.",
               WHEELGROUP));
   }
} // end namespace

