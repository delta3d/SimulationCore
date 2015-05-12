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

#include <prefix/SimCorePrefix.h>
#include <SimCore/BaseWheeledVehiclePhysicsHelper.h>
#include <SimCore/ActComps/WheelActComp.h>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/transform.h>

#include <dtUtil/mathdefines.h>
#include <SimCore/CollisionGroupEnum.h>

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
   BaseWheeledVehiclePhysicsActComp::BaseWheeledVehiclePhysicsActComp()
   : dtPhysics::PhysicsActComp()
   , mVehicle(NULL)
   , mEngineTorque(1000.0f)
   , mMaxBrakeTorque(100.0f)
   , mVehicleTopSpeed(120.0f)
   , mVehicleTopSpeedReverse(40.0f)
   , mMaxSteerAngle(45.0f)
   , mAeroDynDragCoefficient(0.80)
   , mAeroDynDragArea(8.0)
   , mLastMPH(0.0f)
   {
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = dtPhysics::PhysicsObject::CreateNew("chassis");
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetCollisionGroup(SimCore::CollisionGroup::GROUP_VEHICLE_GROUND);
      AddPhysicsObject(*physicsObject);
   }

   /// Destructor
   BaseWheeledVehiclePhysicsActComp::~BaseWheeledVehiclePhysicsActComp()
   {
      delete mVehicle;
   }

//   /////////////////////////////////////////////////////////////////////////////////////
//   void BaseWheeledVehiclePhysicsActComp::GetLocalMatrix(const osg::Node& node, osg::Matrix& wcMatrix)
//   {
//      if(node.getNumParents() > 0)
//      {
//         const osg::Transform* parentNode = node.getParent(0)->asTransform();
//         if (parentNode != NULL)
//         {
//            parentNode->computeLocalToWorldMatrix(wcMatrix,NULL);
//         }
//      }
//   }

   /////////////////////////////////////////////////////////////////////////////////////
   osg::Vec3 BaseWheeledVehiclePhysicsActComp::ComputeAeroDynDrag(const osg::Vec3& linearVelocity)
   {
      osg::Vec3 workVec = linearVelocity;
      float magnitude = workVec.normalize();
      float dragMag = -mAeroDynDragCoefficient * mAeroDynDragArea * magnitude * magnitude * 0.5f * AIR_DENSITY;
      workVec *= dragMag;
      return workVec;
   }

   /////////////////////////////////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::CleanUp()
   {
      delete mVehicle;
      mVehicle = NULL;
      mWheels.clear();
      BaseClass::CleanUp();
   }
   // ///////////////////////////////////////////////////////////////////////////////////
   //                               Vehicle Initialization                             //
   // ///////////////////////////////////////////////////////////////////////////////////

   WheelType BaseWheeledVehiclePhysicsActComp::AddWheel(const osg::Vec3& position,
            TireParameters& tireParams, SuspensionParameters& suspensionParams,
            bool powered, bool steered, bool braked)
   {
      WheelType wheel;
      //pal keeps track of this, but I set the anyway for completeness.
      wheel.mPowered = powered;
      wheel.mSteered = steered;
      wheel.mBraked = braked;
      wheel.mWheel = NULL;

      if (mVehicle != NULL)
      {
         wheel.mWheel = mVehicle->AddWheel();

         palWheelInfo wheelInfo;
         wheelInfo.m_fPosX = position.x();
         wheelInfo.m_fPosY = position.y();
         wheelInfo.m_fPosZ = position.z();
         wheelInfo.m_fRadius = tireParams.mRadius;
         wheelInfo.m_fWidth = tireParams.mWidth;
         wheelInfo.m_fSuspension_Travel = suspensionParams.mTravel;
         wheelInfo.m_fSuspension_Rest_Length = suspensionParams.mRestLength;
         wheelInfo.m_fSuspension_Ks = suspensionParams.mSpringRate;
         wheelInfo.m_fSuspension_Kd = suspensionParams.mDamperCoef;
         wheelInfo.m_fRoll_Influence = suspensionParams.mRollInfluence;
         wheelInfo.m_fFriction_Slip = tireParams.mExtremeSlip;
         wheelInfo.m_bBrake = braked;
         wheelInfo.m_bSteer = steered;
         wheelInfo.m_bDrive = powered;

         wheel.mWheel->Init(wheelInfo);
      }
      mWheels.push_back(wheel);

      return wheel;
   }

   /////////////////////////////////////////////////////////
   bool BaseWheeledVehiclePhysicsActComp::CreateChassis(const dtCore::Transform& transformForRot,
            osg::Node& bodyNode, const osg::Vec3& scale)
   {
      dtCore::Transform xform;
      xform.Rescale(scale);

      osg::Matrix m;
      xform.Get(m);

      osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform(m);
      mt->addChild(&bodyNode);

      GetMainPhysicsObject()->SetTransform(transformForRot);
      GetMainPhysicsObject()->Create(mt);

      dtGame::GameActorProxy* ga = NULL;
      GetOwner(ga);

      if (ga == NULL)
      {
         return false;
      }

      if (!ga->IsRemote())
      {
         //Create the vehicle here so we can add wheels any time.
         mVehicle = dynamic_cast<palVehicle*>(dtPhysics::PhysicsWorld::GetInstance().GetPalFactory()->CreateObject("palVehicle"));
         mVehicle->Init(&GetMainPhysicsObject()->GetBodyWrapper()->GetPalBody(), GetEngineTorque(),
                   GetMaxBrakeTorque());
      }

      return true;

   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsActComp::GetChassisMass() const
   {
      return GetMainPhysicsObject()->GetMass();
   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsActComp::GetAeroDynDragCoefficient() const
   {
      return mAeroDynDragCoefficient;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::SetAeroDynDragCoefficient(float drag)
   {
      mAeroDynDragCoefficient = drag;
   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsActComp::GetAeroDynDragArea() const
   {
      return mAeroDynDragArea;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::SetAeroDynDragArea(float area)
   {
      mAeroDynDragArea = area;
   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsActComp::GetMPH() const
   {
      return mLastMPH;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::SetMPH(float newMPH)
   {
      mLastMPH = newMPH;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::CalcMPH()
   {
      static const float METERSPS_TO_MILESPH = 2.236936291;
      const dtPhysics::PhysicsObject* po = GetMainPhysicsObject();
      if (po == NULL)
      {
         SetMPH(0.0f);
         return;
      }

      dtPhysics::VectorType vec = po->GetLinearVelocity();
      dtCore::Transform xform;
      po->GetTransform(xform);
      osg::Vec3 fwd;
      xform.GetRow(1, fwd);

      dtPhysics::VectorType vecNormal = vec;
      vecNormal.normalize();

      float dot = fwd * vecNormal;

      float mph = dot * vec.length() * METERSPS_TO_MILESPH;

      mLastMPH = mph;
   }

   ////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::Control(float acceleration, float normalizedWheelAngle, float normalizedBrakes)
   {
      if (mVehicle == NULL)
      {
         return;
      }

      float maxWheelAngle = (GetMaxSteerAngle() * osg::PI / 180.0f);   // convert from deg to rad
      float turningFraction = (normalizedWheelAngle * maxWheelAngle) / osg::PI_2;
      //std::ostringstream ss;
      //ss << "vehicle stats: accel " << acceleration * mEngineTorque << " steering " << turningFraction  << " brakes " << normalizedBrakes * mMaxBrakeTorque;
      //LOG_ALWAYS(ss.str());

      if (acceleration < 0.0f)
      {
         if (GetVehicleTopSpeedReverse() < -GetMPH())
         {
            acceleration = 0.0f;
         }
      }
      else if (acceleration > 0.0f)
      {
         if (GetVehicleTopSpeed() < GetMPH())
         {
            acceleration = 0.0f;
         }
      }

      mVehicle->ForceControl(turningFraction, acceleration * mEngineTorque, normalizedBrakes * mMaxBrakeTorque);

   }

   void BaseWheeledVehiclePhysicsActComp::FinalizeInitialization()
   {
      if (mVehicle != NULL)
      {
         mVehicle->Finalize();
      }
   }

   //////////////////////////////////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsActComp::UpdateWheelTransforms()
   {
      SimCore::ActComps::WheelActComp* wheelAC = NULL;
      GetOwner()->GetComponent(wheelAC);
      if (wheelAC != NULL)
      {
         unsigned wheelIndex = 0;
         for (unsigned i = 0; i < wheelAC->GetNumAxles() && wheelIndex < mWheels.size(); ++i)
         {
            SimCore::ActComps::Axle* axle = wheelAC->GetAxle(i);
            for (unsigned j = 0; j < axle->GetNumWheels() && wheelIndex < mWheels.size(); ++j)
            {
               WheelType& wheel = mWheels[wheelIndex];
               ++wheelIndex;

               palMatrix4x4& palmat = wheel.mWheel->GetLocationMatrix();
               osg::Matrix m;
               dtPhysics::PalMatrixToTransform(m, palmat);

               axle->SetWheelBaseTransform(j, m);
            }
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsActComp::CalcSpringRate(float freq, float vehMass, float wheelbase, float leverArm) const
   {
      float lumpedMass = leverArm / wheelbase * 0.5f * vehMass;
      float temp = osg::PI * 2.0f * freq;
      return lumpedMass * temp * temp;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsActComp::CalcDamperCoeficient(float dampingFactor, float vehMass, float springRate, float wheelbase, float leverArm) const
   {
      float lumpedMass = leverArm / wheelbase * 0.5f * vehMass;
      return dampingFactor * 2.0f * std::sqrt ( springRate * lumpedMass );
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   /// Builds the property map for this vehicle.
   ///
   /// @param toFillIn    dtCore::ActorProperty for this vehicle

   void BaseWheeledVehiclePhysicsActComp::BuildPropertyMap()
   {
      dtPhysics::PhysicsActComp::BuildPropertyMap();

      static const dtUtil::RefString VEHICLEGROUP("Vehicle Physics");

      AddProperty(new dtCore::FloatActorProperty("Engine Torque", "Engine Torque",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetEngineTorque),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetEngineTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      AddProperty(new dtCore::FloatActorProperty("Max Break Torque", "Max Break Torque",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetMaxBrakeTorque),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetMaxBrakeTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      AddProperty(new dtCore::FloatActorProperty("Max Steer Angle", "Max Steer Angle",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetMaxSteerAngle),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetMaxSteerAngle),
               "The maximum angle the wheel can steer (rotate about its vertical axis) in degrees.", VEHICLEGROUP));

      AddProperty(new dtCore::FloatActorProperty("Top Speed (MPH)", "Top Speed (MPH)",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetVehicleTopSpeed),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetVehicleTopSpeed),
               "Top speed of vehicle", VEHICLEGROUP));

      AddProperty(new dtCore::FloatActorProperty("mVehicleTopSpeedReverse", "mVehicleTopSpeedReverse",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetVehicleTopSpeedReverse),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetVehicleTopSpeedReverse),
               "Top speed in reverse", VEHICLEGROUP));

      AddProperty(new dtCore::FloatActorProperty("AeroDynDragCoefficient", "Aero Dynamic Drag Coefficient",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetAeroDynDragCoefficient),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetAeroDynDragCoefficient),
               "The Coefficient of friction to use for aerodynamic friction.  Anything from just over 0.0 to a  bit over 1.0 will work.",
               VEHICLEGROUP));

      AddProperty(new dtCore::FloatActorProperty("AeroDynDragArea", "Aerodynamic Drag Area",
               dtCore::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsActComp::SetAeroDynDragArea),
               dtCore::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsActComp::GetAeroDynDragArea),
               "The area in square meters to use when computing aerodynamic drag, i.e. the surface area on the front/back of the vehicle.",
               VEHICLEGROUP));
   }
} // end namespace

