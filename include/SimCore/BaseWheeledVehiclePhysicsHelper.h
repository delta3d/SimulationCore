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

#ifdef AGEIA_PHYSICS
      NxWheelShape* mWheel;
#else
      palWheel* mWheel;
#endif
   };

   class SIMCORE_EXPORT BaseVehiclePhysicsHelper : public dtPhysics::PhysicsHelper
   {
   public:
      BaseVehiclePhysicsHelper(dtGame::GameActorProxy &proxy);

      //////////////////////////////////////////////////////////////////////////////////////
      //                               Vehicle Initialization                             //
      //////////////////////////////////////////////////////////////////////////////////////

      /**
      * /brief Purpose : Create Wheels onto the main physics vehicle
      */
      virtual WheelType AddWheel(const osg::Vec3& position, bool powered = true, bool steered = true, bool braked = true);

      /**
      * /brief Purpose : To create a 4 wheeled vehicle
      */
      virtual bool CreateChassis(const dtCore::Transform& transformForRot, osgSim::DOFTransform* bodyNode);


      //////////////////////////////////////////////////////////////////////////////////////
      //                                    Properties                                    //
      //////////////////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////
      // Build the property list for the actor
      virtual void   BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn);

      float       GetEngineTorque() const                {return mEngineTorque;}
      float       GetMaxBrakeTorque() const              {return mMaxBrakeTorque;}
      float       GetVehicleTopSpeed() const             {return mVehicleTopSpeed;}
      float       GetVehicleTopSpeedReverse() const      {return mVehicleTopSpeedReverse;}
      int         GetVehicleHorsePower() const           {return mHorsePower;}
      float       GetVehicleMass() const                 {return mVehicleMass;}
      float       GetVehicleTurnRadiusPerUpdate() const  {return mWheelTurnRadiusPerUpdate;}
      float       GetWheelInverseMass() const            {return mWheelInverseMass;}
      float       GetWheelRadius() const                 {return mWheelRadius;}
      float       GetWheelWidth() const                  {return mWheelWidth; }
      float       GetWheelSuspensionTravel() const       {return mWheelSuspensionTravel;}
      float       GetSuspensionSpringCoef() const        {return mSuspensionSpringCoef;}
      float       GetSuspensionSpringDamper()const       {return mSuspensionSpringDamper;}
      float       GetSuspensionSpringTarget() const      {return mSuspensionSpringTarget;}
      float       GetMaxSteerAngle() const               {return mMaxSteerAngle;}
      float       GetTireExtremumSlip() const            {return mTireExtremumSlip;}
      float       GetTireExtremumValue() const           {return mTireExtremumValue;}
      float       GetTireAsymptoteSlip() const           {return mTireAsymptoteSlip;}
      float       GetTireAsymptoteValue() const          {return mTireAsymptoteValue;}
      float       GetTireStiffnessFactor() const         {return mTireStiffnessFactor;}
      float       GetTireRestitution() const             {return mTireRestitution;}

      void SetEngineTorque(float value)               {mEngineTorque = value;}
      void SetMaxBrakeTorque(float value)             {mMaxBrakeTorque =  value;}
      void SetVehicleTopSpeed(float value)            {mVehicleTopSpeed = value;}
      void SetVehicleTopSpeedReverse(float value)     {mVehicleTopSpeedReverse = value;}
      void SetVehicleHorsePower(int value)            {mHorsePower = value;}
      void SetVehicleMass(float value)                {mVehicleMass = value;}
      void SetVehicleTurnRadiusPerUpdate(float value) {mWheelTurnRadiusPerUpdate = value;}
      void SetWheelInverseMass(float value)           {mWheelInverseMass = value;}
      void SetWheelRadius(float value)                {mWheelRadius = value;}
      void SetWheelWidth(float width)                 { mWheelWidth = width; }
      void SetWheelSuspensionTravel(float value)      {mWheelSuspensionTravel = value;}
      void SetSuspensionSpringCoef(float value)       {mSuspensionSpringCoef = value;}
      void SetSuspensionSpringDamper(float value)     {mSuspensionSpringDamper = value;}
      void SetSuspensionSpringTarget(float value)     {mSuspensionSpringTarget = value;}
      void SetMaxSteerAngle(float value)              {mMaxSteerAngle = value;}
      void SetTireExtremumSlip(float value)           {mTireExtremumSlip = value;}
      void SetTireExtremumValue(float value)          {mTireExtremumValue = value;}
      void SetTireAsymptoteSlip(float value)          {mTireAsymptoteSlip = value;}
      void SetTireAsymptoteValue(float value)         {mTireAsymptoteValue = value;}
      void SetTireStiffnessFactor(float value)        {mTireStiffnessFactor = value;}
      void SetTireRestitution(float value)            {mTireRestitution = value;}

      ////////// Vehicle Control///////////////////
      void Control(float acceleration, float normalizedWheelAngle, float normalizedBrakes);

      /// Call this once all initialization is done, but before driving.
      virtual void FinalizeInitialization();

   protected:
      virtual ~BaseVehiclePhysicsHelper();

      /// This currently assumes this dof is placed in the model using the 3DSMax OSG Helper
      /// because the local to world matrix is placed right above the DOF in the export process
      /// since the dof transform does not actually have a matrix
      void GetLocalMatrix(osgSim::DOFTransform* node, osg::Matrix& wcMatrix);

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
      int               mHorsePower;               //!< @todo Used for brake torque, not implemented currently

      float             mVehicleMass;            //!< Mass of chassis / wheels without people in it
      float             mWheelTurnRadiusPerUpdate; //!< How much do the tires move when you press the movement key
      float             mWheelInverseMass;         //!< Affects acceleration of the vehicle with MOVEMENT_AMOUNT
      float             mWheelRadius;              //!< how big our rims our, cant get exact between meters / inches / etc, have to play with
      float             mWheelWidth;              //!< width of the wheel
      float             mMaxSteerAngle;            //!< Maximum steer angle at wheel
      float             mWheelSuspensionTravel;    //!< Total suspension travel from full rebound to full jounce
      float             mSuspensionSpringCoef;     //!< Spring constant
      float             mSuspensionSpringDamper;   //!< Coefficient for linear damping
      float             mSuspensionSpringTarget;   //!< Set to 0 usually

      //////////////////////////////////////////////////////////////////////
      // Parameters to tire model
      //////////////////////////////////////////////////////////////////////

      float             mTireExtremumSlip;         //!< extremal point of curve.  Values must be positive.
      float             mTireExtremumValue;        //!< extremal point of curve.  Values must be positive.
      float             mTireAsymptoteSlip;        //!< point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      float             mTireAsymptoteValue;       //!< point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      /**
      *  This is an additional overall positive scaling that gets applied to the tire forces before passing
      *  them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may
      *  need to lower this. A setting of zero will disable all friction in this direction.
      */
      float             mTireStiffnessFactor;

      /**
      * coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.
      * Note that values close to or above 1 may cause stability problems and/or increasing energy.
      */
      float             mTireRestitution;

      std::vector<WheelType> mWheels;
   };
}

#endif //_BASE_VEHICLE_PHYSICS_HELPER_
