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
#include <dtUtil/getsetmacros.h>

namespace osgSim
{
   class DOFTransform;
}

namespace SimCore
{
   class SIMCORE_EXPORT FourWheelVehiclePhysicsActComp : public SimCore::BaseWheeledVehiclePhysicsActComp
   {
   public:
      typedef SimCore::BaseWheeledVehiclePhysicsActComp BaseClass;

   public:
      FourWheelVehiclePhysicsActComp();


      // ///////////////////////////////////////////////////////////////////////////////////
      //                               Utility Calculations                               //
      // ///////////////////////////////////////////////////////////////////////////////////
      float GetWheelRotation(unsigned index) const;
      float GetWheelJounce(unsigned index) const;


      // ///////////////////////////////////////////////////////////////////////////////////
      //                                  Vehicle methods                                 //
      // ///////////////////////////////////////////////////////////////////////////////////

      void UpdateVehicle(float deltaTime);
      void ApplyAccelerator(float pedal);
      void Steer(float normalize_wheel_angle);
      void ApplyBrake(float normalized_brake);

      /**
       * Creates the vehicle body and dynamics rig.  you can pass in a scale
       */
      bool CreateVehicle(const dtCore::Transform& transformForRot,
               osg::Node& bodyNode, const osg::Vec3& scale);


      //////////////////////////////////////////////////////////////////
      // Build the property list for the actor
      virtual void BuildPropertyMap();

      virtual void OnEnteredWorld();

      DT_DECLARE_ACCESSOR(bool, IsVehicleFourWheelDrive);

      /**
       * Track is the distance along the axle of a wheel from the centerline of the vehicle.
       * Setting this to a positive number moves the wheel farther away than defined in the model.
       * Setting it negative moves it closer.
       */
      DT_DECLARE_ACCESSOR(float, FrontTrackAdjustment);
      /**
       * Track is the distance along the axle of a wheel from the centerline of the vehicle.
       * Setting this to a positive number moves the wheel farther away than defined in the model.
       * Setting it negative moves it closer.
       */
      DT_DECLARE_ACCESSOR(float, RearTrackAdjustment);

      DT_DECLARE_ACCESSOR(float, FrontWheelMass);
      DT_DECLARE_ACCESSOR(float, FrontWheelRadius);
      DT_DECLARE_ACCESSOR(float, FrontWheelWidth);
      DT_DECLARE_ACCESSOR(float, FrontSuspensionTravel);
      DT_DECLARE_ACCESSOR(float, FrontSuspensionRestLength);
      DT_DECLARE_ACCESSOR(float, FrontSuspensionSpringFreq);
      DT_DECLARE_ACCESSOR(float, FrontSuspensionDamperFactor);

      /// extremal point of curve.  Values must be positive.
      DT_DECLARE_ACCESSOR(float, FrontTireSlip);
      // extremal point of curve.  Values must be positive.
      DT_DECLARE_ACCESSOR(float, FrontTireValue);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DT_DECLARE_ACCESSOR(float, FrontTireAsymptoteSlip);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DT_DECLARE_ACCESSOR(float, FrontTireAsymptoteValue);

      /**
      *  This is an additional overall positive scaling that gets applied to the tire forces before passing
      *  them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may
      *  need to lower this. A setting of zero will disable all friction in this direction.
      */
      DT_DECLARE_ACCESSOR(float, FrontTireStiffness);
      /**
      * coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.
      * Note that values close to or above 1 may cause stability problems and/or increasing energy.
      */
      DT_DECLARE_ACCESSOR(float, FrontTireRestitution);

      DT_DECLARE_ACCESSOR(float, RearWheelMass);
      DT_DECLARE_ACCESSOR(float, RearWheelRadius);
      DT_DECLARE_ACCESSOR(float, RearWheelWidth);
      DT_DECLARE_ACCESSOR(float, RearSuspensionTravel);
      DT_DECLARE_ACCESSOR(float, RearSuspensionRestLength);
      DT_DECLARE_ACCESSOR(float, RearSuspensionSpringFreq);
      DT_DECLARE_ACCESSOR(float, RearSuspensionDamperFactor);

      /// extremal point of curve.  Values must be positive.
      DT_DECLARE_ACCESSOR(float, RearTireSlip);
      /// extremal point of curve.  Values must be positive.
      DT_DECLARE_ACCESSOR(float, RearTireValue);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DT_DECLARE_ACCESSOR(float, RearTireAsymptoteSlip);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DT_DECLARE_ACCESSOR(float, RearTireAsymptoteValue);

      /**
      *  This is an additional overall positive scaling that gets applied to the tire forces before passing
      *  them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may
      *  need to lower this. A setting of zero will disable all friction in this direction.
      */
      DT_DECLARE_ACCESSOR(float, RearTireStiffness);
      /**
      * coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.
      * Note that values close to or above 1 may cause stability problems and/or increasing energy.
      */
      DT_DECLARE_ACCESSOR(float, RearTireRestitution);


      virtual void CleanUp();

   protected:
      virtual ~FourWheelVehiclePhysicsActComp();

   private:

      std::vector<WheelType>   mWheels;                //!< All of the vehicle's wheels

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
