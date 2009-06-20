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

#ifndef DELTA_FOUR_WHEEL_VEHICLE_PHYSICS_HELPER
#define DELTA_FOUR_WHEEL_VEHICLE_PHYSICS_HELPER
#include <SimCore/Export.h>
#include <SimCore/BaseWheeledVehiclePhysicsHelper.h>

namespace osgSim
{
   class DOFTransform;
}

namespace SimCore
{
   class SIMCORE_EXPORT FourWheelVehiclePhysicsHelper : public SimCore::BaseWheeledVehiclePhysicsHelper
   {
   public:
      typedef SimCore::BaseWheeledVehiclePhysicsHelper BaseClass;
      enum WheelLocation {FRONT_LEFT = 0, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT};

   public:
      FourWheelVehiclePhysicsHelper(dtGame::GameActorProxy &proxy);


      // ///////////////////////////////////////////////////////////////////////////////////
      //                               Utility Calculations                               //
      // ///////////////////////////////////////////////////////////////////////////////////
      float GetMPH();
      float GetWheelRotation(WheelLocation index) const;
      float GetWheelJounce(WheelLocation index) const;


      // ///////////////////////////////////////////////////////////////////////////////////
      //                                  Vehicle methods                                 //
      // ///////////////////////////////////////////////////////////////////////////////////

      void UpdateVehicle(float deltaTime);
      void ApplyAccelerator(float pedal);
      void Steer(float normalize_wheel_angle);
      void ApplyBrake(float normalized_brake);

      bool CreateVehicle(const dtCore::Transform& transformForRot, const osg::Node& bodyNode, osgSim::DOFTransform* wheels[4]);


      //////////////////////////////////////////////////////////////////
      // Build the property list for the actor
      virtual void BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn);

      bool        GetIsVehicleFourWheelDrive() const     {return mFourWheelDrive;}
      void SetIsVehicleFourWheelDrive(bool value)     {mFourWheelDrive = value;}

   protected:
      virtual ~FourWheelVehiclePhysicsHelper();
      static const float ACC_GRAVITY;


   private:

      WheelType        mWheels[4];                //!< All of the vehicle's wheels
      float            mCurrentNormalizedSteering;     //!< Current steering from -1.0 to 1.0
      float            mCurrentEngineTorque;      //!< Current torque from engine: depends on mAccelerator and mEngineTorque
      float            mCurrentNormalizedBrakes;  //!< Current brakes from 0-1
      float            mAxleRotation[2];          //!< The current rotational velocity of each axle
      float            mAccelerator;              //!< current normalized pedal position
      float            mFrontMaxJounce;           //!< Vertical translation of front wheel (relative to body) with suspension fully compressed.
      float            mRearMaxJounce;            //!< Vertical translation of rear wheel (relative to body) with suspension fully compressed.
      bool             mFourWheelDrive;           //!< Is this vehicle using 4 wheel drive or not? affects speed
   };
}
#endif //DELTA_FOUR_WHEEL_VEHICLE_PHYSICS_HELPER
