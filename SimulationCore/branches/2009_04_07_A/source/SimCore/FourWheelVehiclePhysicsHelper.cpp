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

#include <SimCore/FourWheelVehiclePhysicsHelper.h>
#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
//#else
//#include <dtPhysics/physicscomponent.h>
//#endif
#include <osg/Matrix>
#include <osgSim/DOFTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>

namespace SimCore
{
   const float FourWheelVehiclePhysicsHelper::ACC_GRAVITY = 9.80665;  // @fixme We assume SI metric: meter, kilogram, second.

   // forward declaration
   void GetLocalMatrix(osgSim::DOFTransform* node, osg::Matrix& wcMatrix);

   /// Constructor that provides default values for properties and initial values for state variables.
   FourWheelVehiclePhysicsHelper::FourWheelVehiclePhysicsHelper(dtGame::GameActorProxy &proxy)
      : BaseClass(proxy)
      , mCurrentSteeringAngle(0.0f)
      , mCurrentEngineTorque(0.0f)
      , mCurrentBrakeTorque(0.0f)
      , mFrontMaxJounce(0.0f)
      , mRearMaxJounce(0.0f)
      {
         mAxleRotation[0] = 0.0f;
         mAxleRotation[1] = 0.0f;
      }

      /// Destructor (currently does nothing)
      FourWheelVehiclePhysicsHelper::~FourWheelVehiclePhysicsHelper()
      {

      }

      // ///////////////////////////////////////////////////////////////////////////////////
      //                               Utility Calculations                               //
      // ///////////////////////////////////////////////////////////////////////////////////

         /// Returns the current speed in miles/hour
         float FourWheelVehiclePhysicsHelper::GetMPH()
         {
            // ( X * 10 * 60) / (5280* 12) * 100 = X * 0.9469696
            //return (-((10 * GetWheelSizeWidth() * NxPi * mWheels[BACK_LEFT]->getAxleSpeed() * 60) / (5280 * 12)) * 100);
            return -0.9469696 * 2.0 * GetWheelRadius() * osg::PI * mWheels[BACK_LEFT]->getAxleSpeed();
         }

         float FourWheelVehiclePhysicsHelper::GetWheelRotation( WheelLocation index ) const
         {
            return mAxleRotation[(int(index) < 2) ? 0 : 1];
         }

         /// Returns the current vertical displacement of the chosen wheel.
         float FourWheelVehiclePhysicsHelper::GetWheelJounce( WheelLocation index ) const
         {
            NxWheelContactData wheelPatchData;
            NxShape*           contactObject;
            float              maxJounce = (index < 2 ? mFrontMaxJounce : mRearMaxJounce);

            contactObject =  mWheels[index]->getContact ( wheelPatchData );
            if ( contactObject == NULL )
            {
               return maxJounce - GetWheelSuspensionTravel();
            }
            else
            {
               float displacement = wheelPatchData.contactPosition; // Vertical displacement of wheel
               displacement = maxJounce - displacement + GetWheelRadius();
               return displacement;
            }
         }


      // ///////////////////////////////////////////////////////////////////////////////////
      //                                  Vehicle Methods                                 //
      // ///////////////////////////////////////////////////////////////////////////////////

         /// Updates vehicle position and rotation.
         void FourWheelVehiclePhysicsHelper::UpdateVehicle(float deltaTime)
         {
            float mph = GetMPH();

            for(int i = 0 ; i< 2; i++)
            {
               float RotationTemp = mAxleRotation[i];
               RotationTemp += (mWheels[2*i]->getAxleSpeed()+mWheels[2*i+1]->getAxleSpeed()) / 2.0f * deltaTime;
               float dum;
               RotationTemp = NxTwoPi * modff( RotationTemp / NxTwoPi, &dum );
               if ( RotationTemp < 0.0f )
                  RotationTemp += NxTwoPi;
               mAxleRotation[i] = RotationTemp;
            }

            if ( mph >= 0.0f )
            {
               // Forward motion: limited by specified top speed.
               //!< @todo this needs to be a function of RPM and transmission
               if ( mph < GetVehicleTopSpeed() ) {
                  mCurrentEngineTorque = -mAccelerator * GetEngineTorque();
                  mCurrentBrakeTorque = 0;
               }
               else {
                  mCurrentEngineTorque = 0;
               }
            }
            else
            {
               // Rearward motion: limited by specified reverse top speed
               //!< @todo this needs to be a function of RPM and transmission
               if ( -mph < GetVehicleTopSpeedReverse() ) {
                  mCurrentEngineTorque = mAccelerator * GetEngineTorque();
                  mCurrentBrakeTorque = 0;
               }
               else {
                  mCurrentEngineTorque = 0;
               }
            }

            // Apply the torque to the wheels
            if(GetIsVehicleFourWheelDrive())
            {
               for(int i = 0 ; i < 4; ++i)
               {
                  mWheels[i]->setMotorTorque(mCurrentEngineTorque/4.0f);
               }
            }
            else
            {
               mWheels[BACK_LEFT]->setMotorTorque(mCurrentEngineTorque/2.0f);
               mWheels[BACK_RIGHT]->setMotorTorque(mCurrentEngineTorque/2.0f);
            }
         }

         // /////////////////////////////////////////////////////////////////////////////////
         /// Applies a steering input.
         void FourWheelVehiclePhysicsHelper::Steer(float normalize_wheel_angle)
         {
             float max_wheel_angle = GetMaxSteerAngle() * osg::PI / 180.0f;   // convert from deg to rad
             mCurrentSteeringAngle = max_wheel_angle * normalize_wheel_angle;
             mWheels[FRONT_LEFT]->setSteerAngle(-mCurrentSteeringAngle);
             mWheels[FRONT_RIGHT]->setSteerAngle(-mCurrentSteeringAngle);
         }

         void SimCore::FourWheelVehiclePhysicsHelper::ApplyAccelerator(float pedal)
         {
            mAccelerator = pedal;
         }

         /// Applies a braking input.
         ///
         /// @Note For outside users:
         /// the braking system isn't the same as Ageia's doc
         /// and is kinda geared towards our own needs
         /// You will probably want to change this for your own needs.
         ///
         /// @param normalized_brake     Braking level: min 0, max 1

         void FourWheelVehiclePhysicsHelper::ApplyBrake( float normalized_brake )
         {
            dtUtil::Clamp(normalized_brake, 0.0f, 1.0f);

            mCurrentBrakeTorque = normalized_brake * 10000.0f;  //!< @todo Need to use max brake pressure value that includes air pressure for trucks or malfunctions

            for(int i = 0 ; i < 4; ++i)
            {
               mWheels[i]->setBrakeTorque(mCurrentBrakeTorque);
            }
         }

      // ///////////////////////////////////////////////////////////////////////////////////
      //                               Vehicle Initialization                             //
      // ///////////////////////////////////////////////////////////////////////////////////

   /// Creates the dynamics model of a four-wheeled vehicle.
   ///
   /// @param transformForRot
   /// @param bodyNode          OSG node for body geometry
   /// @param wheels            OSG nodes for geometry for wheels
   /// @param makeWheels        If true, create dynamics models for wheels and attach them
   ///
   /// @retval                  true if model was created
   /// @retval                  false if model wasn't created because of some error.

         bool FourWheelVehiclePhysicsHelper::CreateVehicle(const dtCore::Transform& transformForRot,
            osgSim::DOFTransform* bodyNode, osgSim::DOFTransform* wheels[4])
         {
            // Make sure we have a valid node for body geometry.
            if(bodyNode == NULL)
               return false;

            // Make sure we have valid nodes for geometry of all four wheels.
            for(int i = 0 ; i < 4; ++i)
            {
               if(wheels[i] == NULL)
                  return false;
            }

            osg::Matrix WheelMatrix[4];
            osg::Vec3   WheelVec[4];
            osg::Matrix BodyMatrix;
            GetLocalMatrix(bodyNode, BodyMatrix);

            for(int i = 0; i < 4; i++)
            {
               GetLocalMatrix((wheels[i]), WheelMatrix[i]);
               WheelVec[i] = WheelMatrix[i].getTrans() - BodyMatrix.getTrans();
            }

            float frontLeverArm   =  WheelVec[FRONT_LEFT][1]; // Y distance from front wheels to center of gravity
            float rearLeverArm    = -WheelVec[BACK_LEFT][1];  // Y distance from rear wheels to center of gravity
            float wheelbase       = frontLeverArm + rearLeverArm;
            float frontWheelLoad  = 0.5f * ( GetVehicleMass() * ACC_GRAVITY * rearLeverArm / wheelbase );
            float rearWheelLoad   = 0.5f * ( GetVehicleMass() * ACC_GRAVITY * frontLeverArm / wheelbase );
            float frontDeflection = frontWheelLoad / GetSuspensionSpringCoef();
            float rearDeflection  = rearWheelLoad / GetSuspensionSpringCoef();
            mFrontMaxJounce       = GetWheelSuspensionTravel() - frontDeflection;
            mRearMaxJounce        = GetWheelSuspensionTravel() - rearDeflection;

            WheelVec[FRONT_LEFT][2] += mFrontMaxJounce;
            WheelVec[FRONT_RIGHT][2] += mFrontMaxJounce;
            WheelVec[BACK_LEFT][2] += mRearMaxJounce;
            WheelVec[BACK_RIGHT][2] += mRearMaxJounce;

            mWheels[FRONT_LEFT]   = AddWheel(WheelVec[FRONT_LEFT]);
            mWheels[FRONT_RIGHT]  = AddWheel(WheelVec[FRONT_RIGHT]);
            mWheels[BACK_LEFT]    = AddWheel(WheelVec[BACK_LEFT]);
            mWheels[BACK_RIGHT]   = AddWheel(WheelVec[BACK_RIGHT]);

            if ( mWheels[FRONT_LEFT] == NULL
                || mWheels[FRONT_RIGHT] == NULL
                || mWheels[BACK_LEFT] == NULL
                || mWheels[BACK_RIGHT] == NULL )
                return false;


            CreateChassis(transformForRot, bodyNode);

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

         /// Builds the property map for this vehicle.
         ///
         /// @param toFillIn    vector of dtDAL::ActorProperty for this vehicle

         void FourWheelVehiclePhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
         {
            BaseClass::BuildPropertyMap(toFillIn);
         }
} // end namespace
#endif
