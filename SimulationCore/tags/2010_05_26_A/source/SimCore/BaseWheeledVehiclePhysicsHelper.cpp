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
#include <osg/ComputeBoundsVisitor>
#include <osgSim/DOFTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtCore/transform.h>

#include <dtUtil/mathdefines.h>

#ifndef AGEIA_PHYSICS
#include <dtPhysics/bodywrapper.h>
#include <dtPhysics/palutil.h>
#include <pal/palBodies.h>
#include <pal/palVehicle.h>
#include <pal/palFactory.h>
#endif

namespace SimCore
{
   static const float AIR_DENSITY = 1.204f;    // density of air in kg/m^3 at 20C

   /// Constructor
   BaseWheeledVehiclePhysicsHelper::BaseWheeledVehiclePhysicsHelper(dtGame::GameActorProxy &proxy)
   : dtPhysics::PhysicsHelper(proxy)
#ifdef AGEIA_PHYSICS
   , mEngineTorque(300.0f)
   , mMaxBrakeTorque(100.0f)
   , mVehicleTopSpeed(120.0f)
   , mVehicleTopSpeedReverse(40.0f)
   , mHorsePower(150)
   , mWheelInverseMass(0.15)
   , mWheelRadius(-1.0f)
   , mWheelWidth(-1.0f)
   , mMaxSteerAngle(45.0f)
   , mWheelSuspensionTravel(0.35f)
   , mSuspensionSpringFreq(1.1f)
   , mSuspensionSpringDamper(0.4f)
   , mSuspensionSpringTarget(0.0f)
   , mTireExtremumSlip(1.0f)
   , mTireExtremumValue(2.0f)
   , mTireAsymptoteSlip(2.0f)
   , mTireAsymptoteValue(1.0f)
   , mTireStiffnessFactor(1000.0f)
   , mTireRestitution(0.5f)
#else
   , mVehicle(NULL)
   , mEngineTorque(1000.0f)
   , mMaxBrakeTorque(100.0f)
   , mVehicleTopSpeed(120.0f)
   , mVehicleTopSpeedReverse(40.0f)
   , mWheelInverseMass(0.15)
   , mWheelRadius(-1.0f)
   , mWheelWidth(-1.0f)
   , mMaxSteerAngle(45.0f)
   , mWheelSuspensionTravel(0.3f)
   , mSuspensionSpringFreq(1.1f)
   , mSuspensionSpringDamper(0.4f)
   , mSuspensionSpringTarget(0.35f)
   , mTireExtremumSlip(10.5f)
   , mTireExtremumValue(2.0f)
   , mTireAsymptoteSlip(2.0f)
   , mTireAsymptoteValue(1.0f)
   , mTireStiffnessFactor(1000.0f)
   , mTireRestitution(1000.0f)
   , mAeroDynDragCoefficient(0.80)
   , mAeroDynDragArea(8.0)

#endif
   {
#ifndef AGEIA_PHYSICS
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("chassis");
      AddPhysicsObject(*physicsObject);
#endif
   }

   /// Destructor
   BaseWheeledVehiclePhysicsHelper::~BaseWheeledVehiclePhysicsHelper()
   {
#ifndef AGEIA_PHYSICS
      delete mVehicle;
#endif
   }

   /// A workaround for the transform inefficiency
   void BaseWheeledVehiclePhysicsHelper::GetLocalMatrix(const osg::Node& node, osg::Matrix& wcMatrix)
   {
      if(node.getNumParents() > 0)
      {
         const osg::MatrixTransform* parentNode = dynamic_cast<const osg::MatrixTransform*>(node.getParent(0));
         if (parentNode != NULL)
         {
            wcMatrix = parentNode->getMatrix();
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////////////////
   osg::Vec3 BaseWheeledVehiclePhysicsHelper::ComputeAeroDynDrag(const osg::Vec3& linearVelocity)
   {
      osg::Vec3 workVec = linearVelocity;
      float magnitude = workVec.normalize();
      float dragMag = -mAeroDynDragCoefficient * mAeroDynDragArea * magnitude * magnitude * 0.5f * AIR_DENSITY;
      workVec *= dragMag;
      return workVec;
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
   WheelType BaseWheeledVehiclePhysicsHelper::AddWheel(const osg::Vec3& position, osg::Transform& node,
            float springRate, float damperCoef, bool powered, bool steered, bool braked)
   {
      osg::ComputeBoundsVisitor bb;

      node.accept(bb);

      float wheelRadius = (bb.getBoundingBox().zMax() - bb.getBoundingBox().zMin()) / 2.0f;
      float wheelWidth = (bb.getBoundingBox().xMax() - bb.getBoundingBox().xMin()) / 2.0f;

      if (mWheelRadius > FLT_EPSILON)
      {
         wheelRadius = mWheelRadius;
      }

      if (mWheelWidth > FLT_EPSILON)
      {
         wheelWidth = mWheelWidth;
      }

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

      wheelShapeDesc.suspension.spring       = springRate;
      wheelShapeDesc.suspension.damper       = damperCoef;
      wheelShapeDesc.suspension.targetValue  = mSuspensionSpringTarget;
      wheelShapeDesc.radius                  = wheelRadius;
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
      wheel.mWheel = wheelShape;
      wheel.mTransform = &node;

      mWheels.push_back(wheel);

      return wheel;
   }
#else
   WheelType BaseWheeledVehiclePhysicsHelper::AddWheel(const osg::Vec3& position, osg::Transform& node, float springRate, float damperCoef, bool powered, bool steered, bool braked)
   {
      WheelType wheel;
      //pal keeps track of this, but I set the anyway for completeness.
      wheel.mPowered = powered;
      wheel.mSteered = steered;
      wheel.mBraked = braked;

      wheel.mWheel = mVehicle->AddWheel();
      osg::ComputeBoundsVisitor bb;

      node.accept(bb);

      float wheelRadius = (bb.getBoundingBox().zMax() - bb.getBoundingBox().zMin()) / 2.0f;
      float wheelWidth = (bb.getBoundingBox().xMax() - bb.getBoundingBox().xMin()) / 2.0f;

      if (mWheelRadius > FLT_EPSILON)
      {
         wheelRadius = mWheelRadius;
      }

      if (mWheelWidth > FLT_EPSILON)
      {
         wheelWidth = mWheelWidth;
      }

      wheel.mWheel->Init(position.x(), position.y(), position.z(),
               wheelRadius, wheelWidth, mSuspensionSpringTarget,
               springRate, // in newtons per meter
               damperCoef,
               powered, steered, braked,
               mWheelSuspensionTravel * 100.0f,  // in centimeters
               mTireRestitution);

      wheel.mTransform = &node;

      mWheels.push_back(wheel);

      return wheel;
   }
#endif

   /////////////////////////////////////////////////////////
   bool BaseWheeledVehiclePhysicsHelper::CreateChassis(const dtCore::Transform& transformForRot, const osg::Node& bodyNode)
   {
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

      dtPhysics::PhysicsObject* physicsObject = SetCollisionConvexMesh(const_cast<osg::Node*>(&bodyNode), sendInMatrix, 0, GetChassisMass(), false, "", "Default", "Default", 0);

      dtPhysics::VectorType massPos = physicsObject->getCMassLocalPosition();
      massPos.x += GetVehiclesCenterOfMass()[0];
      massPos.y += GetVehiclesCenterOfMass()[1];
      massPos.z += GetVehiclesCenterOfMass()[2];
      physicsObject->setCMassOffsetLocalPosition(massPos);
#else
      if (mVehicle != NULL)
      {
         delete mVehicle;
         mVehicle = NULL;
      }

      //Create the vehicle here so we can add wheels any time.
      mVehicle = dynamic_cast<palVehicle*>(dtPhysics::PhysicsWorld::GetInstance().GetPalFactory()->CreateObject("palVehicle"));

      GetMainPhysicsObject()->SetTransform(transformForRot);
      GetMainPhysicsObject()->CreateFromProperties(&bodyNode);
      mVehicle->Init(&GetMainPhysicsObject()->GetBodyWrapper()->GetPalBody(), GetEngineTorque(),
                GetMaxBrakeTorque());

#endif

      return true;

   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsHelper::GetChassisMass() const
   {
#ifdef AGEIA_PHYSICS
      return GetMainPhysicsObject()->getMass();
#else
      return GetMainPhysicsObject()->GetMass();
#endif
   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsHelper::GetAeroDynDragCoefficient() const
   {
      return mAeroDynDragCoefficient;
   }
   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsHelper::SetAeroDynDragCoefficient(float drag)
   {
      mAeroDynDragCoefficient = drag;
   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsHelper::GetAeroDynDragArea() const
   {
      return mAeroDynDragArea;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsHelper::SetAeroDynDragArea(float area)
   {
      mAeroDynDragArea = area;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsHelper::Control(float acceleration, float normalizedWheelAngle, float normalizedBrakes)
   {

      float maxWheelAngle = (GetMaxSteerAngle() * osg::PI / 180.0f);   // convert from deg to rad
#ifdef AGEIA_PHYSICS
      float steeringAngle = maxWheelAngle * normalizedWheelAngle;

      dtUtil::Clamp(normalizedBrakes, 0.0f, 1.0f);

      float brakeTorque = normalizedBrakes * GetMaxBrakeTorque();  //!< @todo Need to use max brake pressure value that includes air pressure for trucks or malfunctions

      float wheelTorque = -acceleration * GetEngineTorque();

      for(unsigned i = 0 ; i < mWheels.size(); ++i)
      {
         WheelType& curWheel = mWheels[i];
         if (curWheel.mBraked)
         {
            curWheel.mWheel->setBrakeTorque(brakeTorque);
         }

         if (curWheel.mPowered)
         {
            curWheel.mWheel->setMotorTorque(wheelTorque);
         }

         if (curWheel.mSteered)
         {
            curWheel.mWheel->setSteerAngle(-steeringAngle);
         }
      }
#else
      float turningFraction = (normalizedWheelAngle * maxWheelAngle) / osg::PI_2;
      //std::ostringstream ss;
      //ss << "vehicle stats: accel " << acceleration * mEngineTorque << " steering " << turningFraction  << " brakes " << normalizedBrakes * mMaxBrakeTorque;
      //LOG_ALWAYS(ss.str());
      mVehicle->ForceControl(turningFraction, acceleration * mEngineTorque, normalizedBrakes * mMaxBrakeTorque);
#endif

   }

   void BaseWheeledVehiclePhysicsHelper::FinalizeInitialization()
   {
#ifndef AGEIA_PHYSICS
      mVehicle->Finalize();
#endif
   }

   //////////////////////////////////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsHelper::UpdateWheelTransforms()
   {
#ifndef AGEIA_PHYSICS
      std::vector<WheelType>::iterator i, iend;
      i = mWheels.begin();
      iend = mWheels.end();
      for (; i != iend; ++i)
      {
         WheelType& wheel = *i;
         palMatrix4x4& palmat = wheel.mWheel->GetLocationMatrix();
         osg::Matrix m;
         dtPhysics::PalMatrixToTransform(m, palmat);

         osg::Matrix worldMat;
         dtCore::Transformable::GetAbsoluteMatrix(wheel.mTransform->getParent(0), worldMat);
         osg::Matrix relMat = m * osg::Matrix::inverse(worldMat);


         osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>(wheel.mTransform.get());
         if (dof != NULL)
         {
            dtCore::Transform xform;
            xform.Set(relMat);
            osg::Vec3 hpr;
            xform.GetRotation(hpr);

            dof->setHPRMultOrder(osgSim::DOFTransform::HPR);
            dof->setCurrentHPR(hpr);
            dof->setCurrentTranslate(xform.GetTranslation());
         }
         else
         {
            osg::MatrixTransform* matTrans = dynamic_cast<osg::MatrixTransform*>(wheel.mTransform.get());
            matTrans->setMatrix(relMat);
         }


      }
#endif
   }

   //////////////////////////////////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsHelper::CalcSpringRate(float freq, float vehMass, float wheelbase, float leverArm) const
   {
      float lumpedMass = leverArm / wheelbase * 0.5f * vehMass;
      float temp = osg::PI * 2.0f * freq;
      return lumpedMass * temp * temp;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsHelper::CalcDamperCoeficient(float dampingFactor, float vehMass, float springRate, float wheelbase, float leverArm) const
   {
      float lumpedMass = leverArm / wheelbase * 0.5f * vehMass;
      return dampingFactor * 2.0f * std::sqrt ( springRate * lumpedMass );
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   /// Builds the property map for this vehicle.
   ///
   /// @param toFillIn    dtDAL::ActorProperty for this vehicle

   void BaseWheeledVehiclePhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
   {
      dtPhysics::PhysicsHelper::BuildPropertyMap(toFillIn);

      static const dtUtil::RefString VEHICLEGROUP("Vehicle Physics");
      static const dtUtil::RefString WHEELGROUP("Wheel Physics");

      toFillIn.push_back(new dtDAL::FloatActorProperty("Engine Torque", "Engine Torque",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetEngineTorque),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetEngineTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Break Torque", "Max Break Torque",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetMaxBrakeTorque),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetMaxBrakeTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Top Speed (MPH)", "Top Speed (MPH)",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetVehicleTopSpeed),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetVehicleTopSpeed),
               "Top speed of vehicle", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("mVehicleTopSpeedReverse", "mVehicleTopSpeedReverse",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetVehicleTopSpeedReverse),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetVehicleTopSpeedReverse),
               "Top speed in reverse", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("WheelMass", "Wheel Mass",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetWheelInverseMass),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetWheelInverseMass),
               "This is not used for dtPhysics.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Wheel Radius", "Wheel Radius Override",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetWheelRadius),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetWheelRadius),
               "Rolling radius of wheel", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Wheel Width", "Wheel Width Override",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetWheelWidth),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetWheelWidth),
               "Width of the wheel.  PhysX doesn't use this.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Travel", "Suspension Travel",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetWheelSuspensionTravel),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetWheelSuspensionTravel),
               "Total suspension travel from full rebound to full jounce", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Spring Frequency", "Spring Frequency",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetSuspensionSpringFreq),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetSuspensionSpringFreq),
               "The oscillation frequency of the spring in Hz, 1.0-2.0 is usual", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Damping Factor", "Suspension Damping Factor",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetSuspensionSpringDamper),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetSuspensionSpringDamper),
               "Ratio this damping to the critical damping for the suspsension, usually 0-0.5", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Spring Target", "Suspension Spring Target",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetSuspensionSpringTarget),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetSuspensionSpringTarget),
               "Target value position of spring where the spring force is zero.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Steer Angle", "Max Steer Angle",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetMaxSteerAngle),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetMaxSteerAngle),
               "The maximum angle the wheel can steer (rotate about its vertical axis) in degrees.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Extreme Slip", "Tire Extreme Slip",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetTireExtremumSlip),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetTireExtremumSlip),
               "Extremal point of curve.  Values must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Extreme Value", "Tire Extreme Value",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetTireExtremumValue),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetTireExtremumValue),
               "Extremal point of curve.  Values must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Asymptote Slip", "Tire Asymptote Slip",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetTireAsymptoteSlip),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetTireAsymptoteSlip),
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Asymptote Value", "Tire Asymptote Value",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetTireAsymptoteValue),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetTireAsymptoteValue),
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.", WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Stiffness", "Tire Stiffness",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetTireStiffnessFactor),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetTireStiffnessFactor),
               "This is an additional overall positive scaling that gets applied to the tire forces before passing them to the solver.  Higher values make for better grip.  If you raise the values above, you may need to lower this. A setting of zero will disable all friction in this direction.",
               WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Restitution", "Tire Restitution",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetTireRestitution),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetTireRestitution),
               "Coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.  Note that values close to or above 1 may cause stability problems and/or increasing energy.",
               WHEELGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("AeroDynDragCoefficient", "Aero Dynamic Drag Coefficient",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetAeroDynDragCoefficient),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetAeroDynDragCoefficient),
               "The Coefficient of friction to use for aerodynamic friction.  Anything from just over 0.0 to a  bit over 1.0 will work.",
               VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("AeroDynDragArea", "Aerodynamic Drag Area",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetAeroDynDragArea),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetAeroDynDragArea),
               "The area in square meters to use when computing aerodynamic drag, i.e. the surface area on the front/back of the vehicle.",
               VEHICLEGROUP));
   }
} // end namespace

