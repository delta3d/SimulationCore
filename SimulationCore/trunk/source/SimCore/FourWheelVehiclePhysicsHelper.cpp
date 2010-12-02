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
#include <SimCore/FourWheelVehiclePhysicsHelper.h>
#include <SimCore/PhysicsTypes.h>

//#else
//#include <dtPhysics/physicscomponent.h>
//#endif
#include <osg/Matrix>
#include <osgSim/DOFTransform>
#include <osg/MatrixTransform>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>
#include <dtDAL/propertymacros.h>

namespace SimCore
{
   const float FourWheelVehiclePhysicsActComp::ACC_GRAVITY = 9.80665;  // @fixme We assume SI metric: meter, kilogram, second.

   // forward declaration
   void GetLocalMatrix(osgSim::DOFTransform* node, osg::Matrix& wcMatrix);

   /// Constructor that provides default values for properties and initial values for state variables.
   FourWheelVehiclePhysicsActComp::FourWheelVehiclePhysicsActComp(dtGame::GameActorProxy& proxy)
   : BaseClass(proxy)
   , mIsVehicleFourWheelDrive(false)
   , mFrontTrackAdjustment(0.0f)
   , mRearTrackAdjustment(0.0f)
   , mFrontWheelMass(25.0f)
   , mFrontWheelRadius(-1.0f)
   , mFrontWheelWidth(-1.0f)
   , mFrontSuspensionTravel(0.3f)
   , mFrontSuspensionRestLength(0.35f)
   , mFrontSuspensionSpringFreq(1.1f)
   , mFrontSuspensionDamperFactor(0.4f)
   , mFrontTireSlip(0.05)
   , mFrontTireValue(0.1)
   , mFrontTireAsymptoteSlip(0.1)
   , mFrontTireAsymptoteValue(1.01)
   , mFrontTireStiffness(100000.0f)
   , mFrontTireRestitution(0.1f)
   , mRearWheelMass(25.0f)
   , mRearWheelRadius(-1.0f)
   , mRearWheelWidth(-1.0f)
   , mRearSuspensionTravel(0.3f)
   , mRearSuspensionRestLength(0.35f)
   , mRearSuspensionSpringFreq(1.1f)
   , mRearSuspensionDamperFactor(0.4f)
   , mRearTireSlip(0.1f)
   , mRearTireValue(0.1f)
   , mRearTireAsymptoteSlip(0.1f)
   , mRearTireAsymptoteValue(0.1f)
   , mRearTireStiffness(100000.0f)
   , mRearTireRestitution(0.1f)
   , mCurrentNormalizedSteering(0.0f)
   , mCurrentEngineTorque(0.0f)
   , mCurrentNormalizedBrakes(0.0f)
   , mAccelerator(0.0f)
   , mFrontMaxJounce(0.0f)
   , mRearMaxJounce(0.0f)
   , mFourWheelDrive(false)
   , mDoubleBackWheels(false)
   {
      mAxleRotation[0] = 0.0f;
      mAxleRotation[1] = 0.0f;
   }

   /// Destructor (currently does nothing)
   FourWheelVehiclePhysicsActComp::~FourWheelVehiclePhysicsActComp()
   {

   }

   // ///////////////////////////////////////////////////////////////////////////////////
   //                               Utility Calculations                               //
   // ///////////////////////////////////////////////////////////////////////////////////

   float FourWheelVehiclePhysicsActComp::GetWheelRotation( WheelLocation index ) const
   {
      return mAxleRotation[(int(index) < 2) ? 0 : 1];
   }

   /// Returns the current vertical displacement of the chosen wheel.
   float FourWheelVehiclePhysicsActComp::GetWheelJounce( WheelLocation index ) const
   {
      return 0.0;
   }

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, bool, IsVehicleFourWheelDrive);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTrackAdjustment);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTrackAdjustment);

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontWheelMass);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontWheelRadius);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontWheelWidth);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontSuspensionTravel);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontSuspensionRestLength);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontSuspensionSpringFreq);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontSuspensionDamperFactor);

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTireSlip);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTireValue);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTireAsymptoteSlip);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTireAsymptoteValue);

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTireStiffness);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, FrontTireRestitution);

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearWheelMass);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearWheelRadius);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearWheelWidth);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearSuspensionTravel);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearSuspensionRestLength);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearSuspensionSpringFreq);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearSuspensionDamperFactor);

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTireSlip);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTireValue);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTireAsymptoteSlip);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTireAsymptoteValue);

   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTireStiffness);
   DT_IMPLEMENT_ACCESSOR(FourWheelVehiclePhysicsActComp, float, RearTireRestitution);

   // ///////////////////////////////////////////////////////////////////////////////////
   //                                  Vehicle Methods                                 //
   // ///////////////////////////////////////////////////////////////////////////////////

   /// Updates vehicle position and rotation.
   void FourWheelVehiclePhysicsActComp::UpdateVehicle(float deltaTime)
   {
      //float mph = GetMPH();

//      if ( mph >= 0.0f )
//      {
//         // Forward motion: limited by specified top speed.
//         //!< @todo this needs to be a function of RPM and transmission
//         if ( mph < GetVehicleTopSpeed() ) {
//            mCurrentEngineTorque = -mAccelerator * GetEngineTorque();
//            mCurrentNormalizedBrakes = 0;
//         }
//         else {
//            mCurrentEngineTorque = 0;
//         }
//      }
//      else
//      {
//         // Rearward motion: limited by specified reverse top speed
//         //!< @todo this needs to be a function of RPM and transmission
//         if ( -mph < GetVehicleTopSpeedReverse() ) {
//            mCurrentEngineTorque = mAccelerator * GetEngineTorque();
//            mCurrentNormalizedBrakes = 0;
//         }
//         else {
//            mCurrentEngineTorque = 0;
//         }
//      }

      //float perWheelEngineTorque = GetIsVehicleFourWheelDrive() ? mCurrentEngineTorque / 4.0: mCurrentEngineTorque / 2.0;
      Control(mAccelerator, mCurrentNormalizedSteering, mCurrentNormalizedBrakes);
   }

   /////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsActComp::Steer(float normalizedWheelAngle)
   {
      mCurrentNormalizedSteering = normalizedWheelAngle;
   }

   /////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsActComp::ApplyBrake(float normalizedBrakes)
   {
      mCurrentNormalizedBrakes = normalizedBrakes;
   }

   /////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsActComp::ApplyAccelerator(float pedal)
   {
      mAccelerator = pedal;
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

   bool FourWheelVehiclePhysicsActComp::CreateVehicle(const dtCore::Transform& transformForRot,
            const osg::Node& bodyNode, osgSim::DOFTransform* wheels[6])
   {
      // Make sure we have valid nodes for geometry of all four wheels.
      for (int i = 0 ; i < 4; ++i)
      {
         if (wheels[i] == NULL)
         {
            return false;
         }
      }

      if (wheels[4] != NULL && wheels[5] != NULL)
      {
          mDoubleBackWheels = true;
      }

      float frontDamping = 0.0f, rearDamping = 0.0f, frontSpring = 0.0f, rearSpring = 0.0f;
      osg::Matrix WheelMatrix[6];
      osg::Vec3   WheelVec[6];

      osg::Matrix bodyOffset;
      GetLocalMatrix(bodyNode, bodyOffset);
      //To allow the developer to shift the center of mass.
      bodyOffset.setTrans(bodyOffset.getTrans() - GetMainPhysicsObject()->GetOriginOffset());

      unsigned wheelCount = mDoubleBackWheels ? 6U : 4U;
      for(unsigned i = 0; i < wheelCount; i++)
      {
         GetLocalMatrix(*(wheels[i]), WheelMatrix[i]);
         WheelVec[i] = WheelMatrix[i].getTrans() - bodyOffset.getTrans();
      }

      if (!GetGameActorProxy()->IsRemote())
      {

         float frontLeverArm   =  WheelVec[FRONT_LEFT][1]; // Y distance from front wheels to center of gravity
         float rearLeverArm    = -WheelVec[BACK_LEFT][1];  // Y distance from rear wheels to center of gravity

         if (mDoubleBackWheels)
         {
            rearLeverArm =  (rearLeverArm + (-WheelVec[BACK_LEFT2][1])) / 2.0f;  // average distance to rear wheels.
         }

         float wheelbase       = frontLeverArm + rearLeverArm;
         if (wheelbase <= 0.0)
         {
            LOGN_ERROR("FourWheelVehiclePhysicsActComp.cpp", "Wheelbase must be greater than zero. "
                     "A zero wheel base can be a result of wheels being configured in the wrong order in the array.");
            //Prevent NAN and INF
            wheelbase = 1.0;
         }

         frontSpring = CalcSpringRate(GetFrontSuspensionSpringFreq(), GetChassisMass(), wheelbase, rearLeverArm);
         rearSpring = CalcSpringRate(GetRearSuspensionSpringFreq(), GetChassisMass(), wheelbase, frontLeverArm);
         frontDamping = CalcDamperCoeficient(GetFrontSuspensionDamperFactor(), GetChassisMass(), frontSpring, wheelbase, rearLeverArm);
         rearDamping = CalcDamperCoeficient(GetRearSuspensionDamperFactor(), GetChassisMass(), rearSpring, wheelbase, frontLeverArm);

         float frontWheelLoad  = 0.5f * ( GetChassisMass() * ACC_GRAVITY * rearLeverArm / wheelbase );
         float rearWheelLoad   = 0.5f * ( GetChassisMass() * ACC_GRAVITY * frontLeverArm / wheelbase );
         float frontDeflection = (frontWheelLoad / frontSpring);
         float rearDeflection  = (rearWheelLoad / rearSpring);
         mFrontMaxJounce       = dtUtil::Max(0.0f, GetFrontSuspensionRestLength() - frontDeflection);
         mRearMaxJounce        = dtUtil::Max(0.0f, GetRearSuspensionRestLength() - rearDeflection);

         WheelVec[FRONT_LEFT][2] += mFrontMaxJounce;
         WheelVec[FRONT_RIGHT][2] += mFrontMaxJounce;
         WheelVec[BACK_LEFT][2] += mRearMaxJounce;
         WheelVec[BACK_RIGHT][2] += mRearMaxJounce;

         if (mDoubleBackWheels)
         {
             WheelVec[BACK_LEFT2][2] += mRearMaxJounce;
             WheelVec[BACK_RIGHT2][2] += mRearMaxJounce;
         }

         WheelVec[FRONT_LEFT][0] -= mFrontTrackAdjustment;
         WheelVec[FRONT_RIGHT][0] += mFrontTrackAdjustment;
         WheelVec[BACK_LEFT][0] -= mRearTrackAdjustment;
         WheelVec[BACK_RIGHT][0] += mRearTrackAdjustment;

         if (mDoubleBackWheels)
         {
             WheelVec[BACK_LEFT2][0] -= mRearTrackAdjustment;
             WheelVec[BACK_RIGHT2][0] += mRearTrackAdjustment;
         }
      }

      CreateChassis(transformForRot, bodyNode);

//      osg::MatrixTransform* mtWheels[4];
//      for (unsigned i = 0; i < 4; ++i)
//      {
//         dtCore::RefPtr<osg::MatrixTransform> mat = new osg::MatrixTransform();
//         mtWheels[i] = mat.get();
//         mat->setName(wheels[i]->getName());
//         wheels[i]->getParent(0)->addChild(mat);
//         for (unsigned j = 0; j < wheels[i]->getNumChildren(); ++j)
//         {
//            mat->addChild(wheels[i]->getChild(j));
//         }
//
//         wheels[i]->getParent(0)->removeChild(wheels[i]);
//
//      }

      BaseWheeledVehiclePhysicsActComp::TireParameters tp;
      BaseWheeledVehiclePhysicsActComp::SuspensionParameters sp;

      tp.mWidth = GetFrontWheelWidth();
      tp.mRadius = GetFrontWheelRadius();
      tp.mStiffness = GetFrontTireStiffness();
      tp.mRestitution = GetFrontTireRestitution();
      tp.mExtremeSlip = GetFrontTireSlip();
      tp.mExtremeValue = GetFrontTireValue();
      tp.mAsymptoteSlip = GetFrontTireAsymptoteSlip();
      tp.mAsypmtoteValue = GetFrontTireAsymptoteValue();

      sp.mDamperCoef = frontDamping;
      sp.mSpringRate = frontSpring;
      sp.mRestLength = GetFrontSuspensionRestLength();
      sp.mTravel = GetFrontSuspensionTravel();
      sp.mRollInfluence = 0.3f;

      mWheels[FRONT_LEFT]   = AddWheel(WheelVec[FRONT_LEFT], *static_cast<osg::Transform*>(wheels[FRONT_LEFT]->getParent(0)), tp, sp, GetIsVehicleFourWheelDrive(), true, true);
      mWheels[FRONT_RIGHT]  = AddWheel(WheelVec[FRONT_RIGHT], *static_cast<osg::Transform*>(wheels[FRONT_RIGHT]->getParent(0)), tp, sp, GetIsVehicleFourWheelDrive(), true, true);

      tp.mWidth = GetRearWheelWidth();
      tp.mRadius = GetRearWheelRadius();
      tp.mStiffness = GetRearTireStiffness();
      tp.mRestitution = GetRearTireRestitution();
      tp.mExtremeSlip = GetRearTireSlip();
      tp.mExtremeValue = GetRearTireValue();
      tp.mAsymptoteSlip = GetRearTireAsymptoteSlip();
      tp.mAsypmtoteValue = GetRearTireAsymptoteValue();

      sp.mDamperCoef = rearDamping;
      sp.mSpringRate = rearSpring;

      if (mDoubleBackWheels)
      {
         sp.mDamperCoef /= 2.0f;
         sp.mSpringRate /= 2.0f;
      }

      sp.mRestLength = GetRearSuspensionRestLength();
      sp.mTravel = GetRearSuspensionTravel();
      sp.mRollInfluence = 0.1f;

      mWheels[BACK_LEFT]    = AddWheel(WheelVec[BACK_LEFT], *static_cast<osg::Transform*>(wheels[BACK_LEFT]->getParent(0)), tp, sp, true, false, true);
      mWheels[BACK_RIGHT]   = AddWheel(WheelVec[BACK_RIGHT], *static_cast<osg::Transform*>(wheels[BACK_RIGHT]->getParent(0)), tp, sp, true, false, true);

      if (mDoubleBackWheels)
      {
          mWheels[BACK_LEFT2]    = AddWheel(WheelVec[BACK_LEFT2], *static_cast<osg::Transform*>(wheels[BACK_LEFT2]->getParent(0)), tp, sp, true, false, true);
          mWheels[BACK_RIGHT2]   = AddWheel(WheelVec[BACK_RIGHT2], *static_cast<osg::Transform*>(wheels[BACK_RIGHT2]->getParent(0)), tp, sp, true, false, true);
      }

      FinalizeInitialization();

      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsActComp::CleanUp()
   {
      BaseClass::CleanUp();
      for (unsigned i = 0; i < 6; ++i)
      {
         mWheels[i].mTransform = NULL;
         // The wheel should be deleted by baseclass when it deletes the underlying vehicle.
         mWheels[i].mWheel = NULL;
      }
   }

   //////////////////////////////////////////////////////////////////////////////////////
   //                                    Properties                                    //
   //////////////////////////////////////////////////////////////////////////////////////

   /// Builds the property map for this vehicle.
   ///
   /// @param toFillIn    vector of dtDAL::ActorProperty for this vehicle

   void FourWheelVehiclePhysicsActComp::BuildPropertyMap()
   {

      static const dtUtil::RefString FOUR_WHEEL_GROUP("Four Wheel Vehicle");

      AddProperty(new dtDAL::BooleanActorProperty("Four Wheel Drive", "Four Wheel Drive",
               dtDAL::BooleanActorProperty::SetFuncType(this, &FourWheelVehiclePhysicsActComp::SetIsVehicleFourWheelDrive),
               dtDAL::BooleanActorProperty::GetFuncType(this, &FourWheelVehiclePhysicsActComp::GetIsVehicleFourWheelDrive),
               "", FOUR_WHEEL_GROUP));

      AddProperty(new dtDAL::FloatActorProperty("FrontTrackAdjustment", "Front Wheel Track Adjustment",
               dtDAL::FloatActorProperty::SetFuncType(this, &FourWheelVehiclePhysicsActComp::SetFrontTrackAdjustment),
               dtDAL::FloatActorProperty::GetFuncType(this, &FourWheelVehiclePhysicsActComp::GetFrontTrackAdjustment),
               "Track is the distance along the axle of a wheel from the centerline of a vehicle."
               "Setting this moves the front wheels closer or farther from the centerline.",
               FOUR_WHEEL_GROUP));

      AddProperty(new dtDAL::FloatActorProperty("RearTrackAdjustment", "Rear Wheel Track Adjustment",
               dtDAL::FloatActorProperty::SetFuncType(this, &FourWheelVehiclePhysicsActComp::SetRearTrackAdjustment),
               dtDAL::FloatActorProperty::GetFuncType(this, &FourWheelVehiclePhysicsActComp::GetRearTrackAdjustment),
               "Track is the distance along the axle of a wheel from the centerline of a vehicle."
               "Setting this moves the rear wheels closer or farther from the centerline.",
               FOUR_WHEEL_GROUP));

      static const dtUtil::RefString WHEELGROUP("Wheel Physics");
      typedef dtDAL::PropertyRegHelper<FourWheelVehiclePhysicsActComp&, FourWheelVehiclePhysicsActComp> PropRegType;
      PropRegType propRegHelper(*this, this, WHEELGROUP);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontWheelMass, "Front Wheel Mass","This is not used for dtPhysics."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontWheelRadius, "Front Wheel Radius","Rolling radius of wheel, -1 to auto calc."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontWheelWidth, "Front Wheel Width","Width of wheel, -1 to auto calc."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontSuspensionTravel, "Front Suspension Travel",
               "Total suspension travel from full rebound to full jounce"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontSuspensionRestLength, "Front Suspension Rest Length",
               "Target value position of spring where the spring force is zero.  This should > the suspension travel."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontSuspensionSpringFreq, "Front Spring Frequency",
               "The oscillation frequency of the spring in Hz, 1.0-2.0 is usual"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontSuspensionDamperFactor, "Front Suspension Damping Factor",
               "Ratio this damping to the critical damping for the suspension, usually 0-0.5"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontTireSlip, "Front Tire Slip",
               "The point on the tire curve where the tire stops applying any more lateral force"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontTireValue, "Front Tire Value",
               "The point on the tire curve where the tire stops responding with force near linearly vs. the lateral force"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontTireAsymptoteSlip, "Front Tire Asymptote Slip",
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontTireAsymptoteValue, "Front Tire Asymptote Value",
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontTireStiffness, "Front Tire Stiffness",
               "The spring coefficient in N/m for the tire.  Should be very large, 100 thousands range"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(FrontTireRestitution, "Front Tire Restitution",
               "Coefficient of restitution --  0 makes the tire bounce as little as possible, "
               "higher values up to 1.0 result in more bounce.  "
               "Note that values close to or above 1 may cause stability problems and/or increasing energy."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearWheelMass, "Rear Wheel Mass","This is not used for dtPhysics."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearWheelRadius, "Rear Wheel Radius","Rolling radius of wheel, -1 to auto calc."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearWheelWidth, "Rear Wheel Width","Width of wheel, -1 to auto calc."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearSuspensionTravel, "Rear Suspension Travel",
               "Total suspension travel from full rebound to full jounce"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearSuspensionRestLength, "Rear Suspension Rest Length",
               "Target value position of spring where the spring force is zero.  This should > the suspension travel."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearSuspensionSpringFreq, "Rear Spring Frequency",
               "The oscillation frequency of the spring in Hz, 1.0-2.0 is usual"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearSuspensionDamperFactor, "Rear Suspension Damping Factor",
               "Ratio this damping to the critical damping for the suspension, usually 0-0.5"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearTireSlip, "Rear Tire Slip",
               "The point on the tire curve where the tire stops applying any more lateral force"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearTireValue, "Rear Tire Value",
               "The point on the tire curve where the tire stops responding with force near linearly vs. the lateral force"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearTireAsymptoteSlip, "Rear Tire Asymptote Slip",
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearTireAsymptoteValue, "Rear Tire Asymptote Value",
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive."
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearTireStiffness, "Rear Tire Stiffness",
               "The spring coefficient in N/m for the tire.  Should be very large, 100 thousands range"
               , PropRegType, propRegHelper);

      DT_REGISTER_PROPERTY_WITH_LABEL(RearTireRestitution, "Rear Tire Restitution",
               "Coefficient of restitution --  0 makes the tire bounce as little as possible, "
               "higher values up to 1.0 result in more bounce.  "
               "Note that values close to or above 1 may cause stability problems and/or increasing energy."
               , PropRegType, propRegHelper);

      BaseClass::BuildPropertyMap();
   }
} // end namespace
