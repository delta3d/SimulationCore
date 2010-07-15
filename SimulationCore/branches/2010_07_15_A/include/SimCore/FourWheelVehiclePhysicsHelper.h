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
   class SIMCORE_EXPORT FourWheelVehiclePhysicsHelper : public SimCore::BaseWheeledVehiclePhysicsHelper
   {
   public:
      typedef SimCore::BaseWheeledVehiclePhysicsHelper BaseClass;
      enum WheelLocation {FRONT_LEFT = 0, FRONT_RIGHT, BACK_LEFT, BACK_RIGHT};

   public:
      FourWheelVehiclePhysicsHelper(dtGame::GameActorProxy& proxy);


      // ///////////////////////////////////////////////////////////////////////////////////
      //                               Utility Calculations                               //
      // ///////////////////////////////////////////////////////////////////////////////////
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


      DECLARE_PROPERTY(bool, IsVehicleFourWheelDrive);

      /**
       * Track is the distance along the axle of a wheel from the centerline of the vehicle.
       * Setting this to a positive number moves the wheel farther away than defined in the model.
       * Setting it negative moves it closer.
       */
      DECLARE_PROPERTY(float, FrontTrackAdjustment);
      /**
       * Track is the distance along the axle of a wheel from the centerline of the vehicle.
       * Setting this to a positive number moves the wheel farther away than defined in the model.
       * Setting it negative moves it closer.
       */
      DECLARE_PROPERTY(float, RearTrackAdjustment);

      DECLARE_PROPERTY(float, FrontWheelMass);
      DECLARE_PROPERTY(float, FrontWheelRadius);
      DECLARE_PROPERTY(float, FrontWheelWidth);
      DECLARE_PROPERTY(float, FrontSuspensionTravel);
      DECLARE_PROPERTY(float, FrontSuspensionRestLength);
      DECLARE_PROPERTY(float, FrontSuspensionSpringFreq);
      DECLARE_PROPERTY(float, FrontSuspensionDamperFactor);

      /// extremal point of curve.  Values must be positive.
      DECLARE_PROPERTY(float, FrontTireSlip);
      // extremal point of curve.  Values must be positive.
      DECLARE_PROPERTY(float, FrontTireValue);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DECLARE_PROPERTY(float, FrontTireAsymptoteSlip);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DECLARE_PROPERTY(float, FrontTireAsymptoteValue);

      /**
      *  This is an additional overall positive scaling that gets applied to the tire forces before passing
      *  them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may
      *  need to lower this. A setting of zero will disable all friction in this direction.
      */
      DECLARE_PROPERTY(float, FrontTireStiffness);
      /**
      * coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.
      * Note that values close to or above 1 may cause stability problems and/or increasing energy.
      */
      DECLARE_PROPERTY(float, FrontTireRestitution);

      DECLARE_PROPERTY(float, RearWheelMass);
      DECLARE_PROPERTY(float, RearWheelRadius);
      DECLARE_PROPERTY(float, RearWheelWidth);
      DECLARE_PROPERTY(float, RearSuspensionTravel);
      DECLARE_PROPERTY(float, RearSuspensionRestLength);
      DECLARE_PROPERTY(float, RearSuspensionSpringFreq);
      DECLARE_PROPERTY(float, RearSuspensionDamperFactor);

      /// extremal point of curve.  Values must be positive.
      DECLARE_PROPERTY(float, RearTireSlip);
      /// extremal point of curve.  Values must be positive.
      DECLARE_PROPERTY(float, RearTireValue);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DECLARE_PROPERTY(float, RearTireAsymptoteSlip);
      /// point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.
      DECLARE_PROPERTY(float, RearTireAsymptoteValue);

      /**
      *  This is an additional overall positive scaling that gets applied to the tire forces before passing
      *  them to the solver.  Higher values make for better grip.  If you raise the *Values above, you may
      *  need to lower this. A setting of zero will disable all friction in this direction.
      */
      DECLARE_PROPERTY(float, RearTireStiffness);
      /**
      * coefficient of restitution --  0 makes the tire bounce as little as possible, higher values up to 1.0 result in more bounce.
      * Note that values close to or above 1 may cause stability problems and/or increasing energy.
      */
      DECLARE_PROPERTY(float, RearTireRestitution);


      virtual void CleanUp();

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
