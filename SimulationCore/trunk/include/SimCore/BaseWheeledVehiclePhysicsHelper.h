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
* @author Allen Danklefsen, Bradley Anderegg
*/

#ifndef _BASE_VEHICLE_PHYSICS_HELPER_
#define _BASE_VEHICLE_PHYSICS_HELPER_

#ifdef AGEIA_PHYSICS
#include <NxWheelShapeDesc.h>
#else
#include <dtPhysics/physicshelper.h>
#include <pal/palVehicle.h>
#endif
#include <osgSim/DOFTransform>
#include <SimCore/Export.h>
#include <SimCore/PhysicsTypes.h>
#include <dtGame/gameactor.h>

namespace SimCore
{
#ifdef AGEIA_PHYSICS
   typedef NxWheelShape WheelType;
#else
   typedef palWheel WheelType;
#endif

   class SIMCORE_EXPORT BaseVehiclePhysicsHelper : public dtPhysics::PhysicsHelper
   {
      public:
         BaseVehiclePhysicsHelper(dtGame::GameActorProxy &proxy);

         //////////////////////////////////////////////////////////////////////////////////////
         //                               Vehicle Initialization                             //
         //////////////////////////////////////////////////////////////////////////////////////

            /**
            * /brief Purpose : Create Wheels onto the main NxActor from the base class
            */
            WheelType* AddWheel(const osg::Vec3& position, bool steerable);

            /**
            * /brief Purpose : To create a 4 wheeled vehicle
            */
            bool CreateVehicle(const dtCore::Transform& transformForRot, osgSim::DOFTransform* bodyNode);


         //////////////////////////////////////////////////////////////////////////////////////
         //                                    Properties                                    //
         //////////////////////////////////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////////////
            // Build the property list for the actor
            virtual void   BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn);

            float GetMotorTorque()                 {return mMotorTorque;}
            float GetVehicleMaxMPH()               {return mVehicleMaxMPH;}
            float GetVehicleMaxReverseMPH()        {return mVehicleMaxReverseMPH;}
            int   GetVehicleHorsePower()           {return mHorsePower;}
            bool  GetIsAllWheelDrive()             {return mAllWheelDrive;}
            float GetVehicleWeight()               {return mVehicleWeight;}
            float GetVehicleTurnRadiusPerUpdate()  {return mWheelTurnRadiusPerUpdate;}
            float GetWheelInverseMass()            {return mWheelInverseMass;}
            float GetWheelRadius()                 {return mWheelRadius;}
            float GetWheelSuspensionTravel()       {return mWheelSuspensionTravel;}
            float GetSuspensionSpringCoef()        {return mSuspensionSpringCoef;}
            float GetSuspensionSpringDamper()      {return mSuspensionSpringDamper;}
            float GetSuspensionSpringTarget()      {return mSuspensionSpringTarget;}
            float GetMaxWheelRotation()            {return mMaxWheelRotation;}
            float GetTireExtremumSlip()            {return mTireExtremumSlip;}
            float GetTireExtremumValue()           {return mTireExtremumValue;}
            float GetTireAsymptoteSlip()           {return mTireAsymptoteSlip;}
            float GetTireAsymptoteValue()          {return mTireAsymptoteValue;}
            float GetTireStiffnessFactor()         {return mTireStiffnessFactor;}
            float GetTireRestitution()             {return mTireRestitution;}
            float GetTireDynamicFriction()         {return mTireDynamicFriction;}
            float GetTireStaticFriction()          {return mTireStaticFriction;}

            void SetMotorTorque(float value)                {mMotorTorque = value;}
            void SetVehicleMaxMPH(float value)              {mVehicleMaxMPH = value;}
            void SetVehicleMaxReverseMPH(float value)       {mVehicleMaxReverseMPH = value;}
            void SetVehicleHorsePower(int value)            {mHorsePower = value;}
            void SetIsAllWheelDrive(bool value)             {mAllWheelDrive = value;}
            void SetVehicleWeight(float value)              {mVehicleWeight = value;}
            void SetVehicleTurnRadiusPerUpdate(float value) {mWheelTurnRadiusPerUpdate = value;}
            void SetWheelInverseMass(float value)           {mWheelInverseMass = value;}
            void SetWheelRadius(float value)                {mWheelRadius = value;}
            void SetWheelSuspensionTravel(float value)      {mWheelSuspensionTravel = value;}
            void SetSuspensionSpringCoef(float value)       {mSuspensionSpringCoef = value;}
            void SetSuspensionSpringDamper(float value)     {mSuspensionSpringDamper = value;}
            void SetSuspensionSpringTarget(float value)     {mSuspensionSpringTarget = value;}
            void SetMaxWheelRotation(float value)           {mMaxWheelRotation = value;}
            void SetTireExtremumSlip(float value)           {mTireExtremumSlip = value;}
            void SetTireExtremumValue(float value)          {mTireExtremumValue = value;}
            void SetTireAsymptoteSlip(float value)          {mTireAsymptoteSlip = value;}
            void SetTireAsymptoteValue(float value)         {mTireAsymptoteValue = value;}
            void SetTireStiffnessFactor(float value)        {mTireStiffnessFactor = value;}
            void SetTireRestitution(float value)            {mTireRestitution = value;}
            void SetTireDynamicFriction(float value)        {mTireDynamicFriction = value;}
            void SetTireStaticFriction(float value)         {mTireStaticFriction = value;}


      protected:
         virtual ~BaseVehiclePhysicsHelper();


         private:

            //////////////////////////////////
            // Properties
            float             mMotorTorque;              /// Effects total acceleration, not only modifier however
            float             mVehicleMaxMPH;            /// The max mph you want your vehicle to have
            float             mVehicleMaxReverseMPH;     /// The max reverse mph you can go
            int               mHorsePower;               /// Used for brake torque, not implemented currently TODO
            bool              mAllWheelDrive;           /// Is this vehicle using all wheel drive or not? effects speed
            float             mVehicleWeight;            /// How much is just the chassis / wheels without people in it
            float             mWheelTurnRadiusPerUpdate; /// How much do the tires move when u press the movement key
            float             mWheelInverseMass;         /// Effects acceleration of the vehicle with MOVEMENT_AMOUNT
            float             mWheelRadius;              /// how big our rims our, cant get exact between meters / inches / etc, have to play with
            float             mWheelSuspensionTravel;    /// Bumpiness
            float             mSuspensionSpringCoef;     /// Correctness and bounciness
            float             mSuspensionSpringDamper;   /// Damping amount used with restitution
            float             mSuspensionSpringTarget;   /// Set to 0 usually
            float             mMaxWheelRotation;         /// How it can go left and right , 45 degrees etc.
            float             mTireExtremumSlip;         ///extremal point of curve.  Values must be positive.
            float             mTireExtremumValue;        ///extremal point of curve.  Values must be positive.
            float             mTireAsymptoteSlip;        ///point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
            float             mTireAsymptoteValue;       ///point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
            float             mReverse;

            /**
            *  This is an additional overall positive scaling that gets applied to the tire forces before passing
            *  them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may
            *  need to lower this. A setting of zero will disable all friction in this direction.
            */
            float             mTireStiffnessFactor;

            /**
            * coefficient of restitution --  0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce.
            * Note that values close to or above 1 may cause stability problems and/or increasing energy.
            */
            float             mTireRestitution;

            /**
            * Coefficient of dynamic friction -- should be in [0, +inf]. If set to greater than staticFriction, the effective value of staticFriction will be increased to match.
            * if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)
            */
            float             mTireDynamicFriction;

            /**
            * Coefficient of static friction -- should be in [0, +inf]
            * if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)
            */
            float             mTireStaticFriction;

#ifndef AGEIA_PHYSICS
            palVehicle* vehicle;
#endif

   };
}

#endif //_BASE_VEHICLE_PHYSICS_HELPER_
