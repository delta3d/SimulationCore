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

#ifndef _BASE_VEHICLE_PHYSICS_HELPER_
#define _BASE_VEHICLE_PHYSICS_HELPER_

#ifdef AGEIA_PHYSICS
#include <NxWheelShapeDesc.h>
#endif
#include <SimCore/Export.h>
#include <SimCore/PhysicsTypes.h>
#include <dtGame/gameactor.h>

#include <dtCore/observerptr.h>

#include <osg/Transform>

class palVehicle;
class palWheel;

namespace osgSim
{
   class DOFTransform;
}

namespace SimCore
{
   struct WheelType
   {
      bool mPowered;
      bool mSteered;
      bool mBraked;

      palWheel* mWheel;
   };

   class SIMCORE_EXPORT BaseWheeledVehiclePhysicsActComp : public dtPhysics::PhysicsActComp
   {
   public:
      typedef dtPhysics::PhysicsActComp BaseClass;

      BaseWheeledVehiclePhysicsActComp();

      //////////////////////////////////////////////////////////////////////////////////////
      //                               Vehicle Initialization                             //
      //////////////////////////////////////////////////////////////////////////////////////

      struct TireParameters
      {
         float mExtremeSlip, mExtremeValue, mAsymptoteSlip, mAsypmtoteValue;
         float mStiffness, mRestitution;
         float mRadius, mWidth;
      };

      struct SuspensionParameters
      {
         float mTravel, mRestLength;
         float mSpringRate, mDamperCoef;
         float mRollInfluence;
      };

      /**
      * /brief Purpose : Create Wheels onto the main physics vehicle
      */
      virtual WheelType AddWheel(const osg::Vec3& position, TireParameters& tireParams,
               SuspensionParameters& suspensionParams,
               bool powered = true, bool steered = true, bool braked = true);

      /**
      * /brief Purpose : To create the chassis for the physics.
      */
      virtual bool CreateChassis(const dtCore::Transform& transformForRot,
               osg::Node& bodyNode, const osg::Vec3& scale);


      //////////////////////////////////////////////////////////////////////////////////////
      //                                    Properties                                    //
      //////////////////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////
      // Build the property list for the actor
      virtual void   BuildPropertyMap();

      float       GetEngineTorque() const                {return mEngineTorque;}
      float       GetMaxBrakeTorque() const              {return mMaxBrakeTorque;}
      float       GetVehicleTopSpeed() const             {return mVehicleTopSpeed;}
      float       GetVehicleTopSpeedReverse() const      {return mVehicleTopSpeedReverse;}

      void SetEngineTorque(float value)               {mEngineTorque = value;}
      void SetMaxBrakeTorque(float value)             {mMaxBrakeTorque =  value;}
      void SetVehicleTopSpeed(float value)            {mVehicleTopSpeed = value;}
      void SetVehicleTopSpeedReverse(float value)     {mVehicleTopSpeedReverse = value;}

      void SetMaxSteerAngle(float value)              {mMaxSteerAngle = value;}

      float       GetMaxSteerAngle() const               {return mMaxSteerAngle;}

      float GetChassisMass() const;

      float GetAeroDynDragCoefficient() const;
      void SetAeroDynDragCoefficient(float drag);

      /// This is the frontal area to use for wind drag.
      float GetAeroDynDragArea() const;
      /// This is the frontal area to use for wind drag.
      void SetAeroDynDragArea(float area);

      /// @return the last calculated MPH.  You should call CalcMPH every frame or so to update the value.
      float GetMPH() const;

      /**
       * Made virtual so it can be calculated in a way more appropriate for the subclass simulated vehicle
       * this function should call SetMPH on this object.
       */
      virtual void CalcMPH();

      ////////// Vehicle Control///////////////////
      void Control(float acceleration, float normalizedWheelAngle, float normalizedBrakes);

      /// Call this once all initialization is done, but before driving.
      virtual void FinalizeInitialization();

      void UpdateWheelTransforms();

      /// This currently assumes this dof is placed in the model using the 3DSMax OSG Helper
      /// because the local to world matrix is placed right above the DOF in the export process
      /// since the dof transform does not actually have a matrix
      static void GetLocalMatrix(const osg::Node& node, osg::Matrix& wcMatrix);

      /// Computes the aero dynamic drag on the vehicle.  It will be positive, you have to figure out the direction.
      osg::Vec3 ComputeAeroDynDrag(const osg::Vec3& linearVelocity);

      virtual void CleanUp();
   protected:

      virtual ~BaseWheeledVehiclePhysicsActComp();

      float CalcSpringRate(float freq, float vehMass, float wheelbase, float leverArm) const;

      float CalcDamperCoeficient(float dampingFactor, float vehMass, float springRate, float wheelbase, float leverArm) const;

      void SetMPH(float newMPH);

#ifndef AGEIA_PHYSICS
      palVehicle* GetPalVehicle() { return mVehicle; };
   private:
      palVehicle* mVehicle;
#endif

   private:
      //////////////////////////////////////////////////////////////////////
      // Powertrain properties
      //////////////////////////////////////////////////////////////////////
      float             mEngineTorque;              //!< Maximum torque capacity of engine
      float             mMaxBrakeTorque;              //!< Maximum braking torque
      float             mVehicleTopSpeed;            //!< Top speed of vehicle
      float             mVehicleTopSpeedReverse;     //!< Top speed in reverse

      float             mMaxSteerAngle;

      float mAeroDynDragCoefficient;
      float mAeroDynDragArea;

      std::vector<WheelType> mWheels;

      float mLastMPH;
   };
}

#endif //_BASE_VEHICLE_PHYSICS_HELPER_
