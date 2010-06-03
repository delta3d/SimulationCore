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
   , mVehicle(NULL)
   , mEngineTorque(1000.0f)
   , mMaxBrakeTorque(100.0f)
   , mVehicleTopSpeed(120.0f)
   , mVehicleTopSpeedReverse(40.0f)
   , mMaxSteerAngle(45.0f)
   , mAeroDynDragCoefficient(0.80)
   , mAeroDynDragArea(8.0)

   {
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("chassis");
      AddPhysicsObject(*physicsObject);
   }

   /// Destructor
   BaseWheeledVehiclePhysicsHelper::~BaseWheeledVehiclePhysicsHelper()
   {
      delete mVehicle;
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

   WheelType BaseWheeledVehiclePhysicsHelper::AddWheel(const osg::Vec3& position, osg::Transform& node,
            TireParameters& tireParams, SuspensionParameters& suspensionParams, bool powered, bool steered, bool braked)
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

      if (tireParams.mRadius > FLT_EPSILON)
      {
         wheelRadius = tireParams.mRadius;
      }

      if (tireParams.mWidth > FLT_EPSILON)
      {
         wheelWidth = tireParams.mWidth;
      }

      wheel.mWheel->Init(position.x(), position.y(), position.z(),
               wheelRadius, wheelWidth, suspensionParams.mRestLength,
               suspensionParams.mSpringRate, // in newtons per meter
               suspensionParams.mDamperCoef,
               powered, steered, braked,
               suspensionParams.mTravel * 100.0f,  // in centimeters
               tireParams.mExtremeSlip);

      wheel.mTransform = &node;

      mWheels.push_back(wheel);

      return wheel;
   }

   /////////////////////////////////////////////////////////
   bool BaseWheeledVehiclePhysicsHelper::CreateChassis(const dtCore::Transform& transformForRot, const osg::Node& bodyNode)
   {
      osg::Matrix BodyMatrix;
      GetLocalMatrix(bodyNode, BodyMatrix);


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


      return true;

   }

   ////////////////////////////////////////////////////////
   float BaseWheeledVehiclePhysicsHelper::GetChassisMass() const
   {
      return GetMainPhysicsObject()->GetMass();
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
      float turningFraction = (normalizedWheelAngle * maxWheelAngle) / osg::PI_2;
      //std::ostringstream ss;
      //ss << "vehicle stats: accel " << acceleration * mEngineTorque << " steering " << turningFraction  << " brakes " << normalizedBrakes * mMaxBrakeTorque;
      //LOG_ALWAYS(ss.str());
      mVehicle->ForceControl(turningFraction, acceleration * mEngineTorque, normalizedBrakes * mMaxBrakeTorque);

   }

   void BaseWheeledVehiclePhysicsHelper::FinalizeInitialization()
   {
      mVehicle->Finalize();
   }

   //////////////////////////////////////////////////////////////////////////////////////
   void BaseWheeledVehiclePhysicsHelper::UpdateWheelTransforms()
   {
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

      toFillIn.push_back(new dtDAL::FloatActorProperty("Engine Torque", "Engine Torque",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetEngineTorque),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetEngineTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Break Torque", "Max Break Torque",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetMaxBrakeTorque),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetMaxBrakeTorque),
               "Maximum torque developed by engine", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Max Steer Angle", "Max Steer Angle",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetMaxSteerAngle),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetMaxSteerAngle),
               "The maximum angle the wheel can steer (rotate about its vertical axis) in degrees.", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("Top Speed (MPH)", "Top Speed (MPH)",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetVehicleTopSpeed),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetVehicleTopSpeed),
               "Top speed of vehicle", VEHICLEGROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("mVehicleTopSpeedReverse", "mVehicleTopSpeedReverse",
               dtDAL::FloatActorProperty::SetFuncType(this, &BaseWheeledVehiclePhysicsHelper::SetVehicleTopSpeedReverse),
               dtDAL::FloatActorProperty::GetFuncType(this, &BaseWheeledVehiclePhysicsHelper::GetVehicleTopSpeedReverse),
               "Top speed in reverse", VEHICLEGROUP));

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

