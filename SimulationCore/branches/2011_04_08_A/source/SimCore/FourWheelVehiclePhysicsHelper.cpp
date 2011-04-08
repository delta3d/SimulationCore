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

#include <SimCore/ActComps/WheelActComp.h>
#include <SimCore/Actors/IGActor.h>

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
   /// Constructor that provides default values for properties and initial values for state variables.
   FourWheelVehiclePhysicsActComp::FourWheelVehiclePhysicsActComp()
   : BaseClass()
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

   float FourWheelVehiclePhysicsActComp::GetWheelRotation( unsigned index ) const
   {
      return mAxleRotation[(int(index) < 2) ? 0 : 1];
   }

   /// Returns the current vertical displacement of the chosen wheel.
   float FourWheelVehiclePhysicsActComp::GetWheelJounce( unsigned index ) const
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
            const osg::Node& bodyNode)
   {
      dtGame::GameActor* ga = NULL;
      GetOwner(ga);

      if (ga == NULL)
      {
         return false;
      }

      // Wheel actor component finds the wheels...
      SimCore::ActComps::WheelActComp* wheelAC = NULL;
      ga->GetComponent(wheelAC);
      if (wheelAC == NULL)
      {
         LOG_ERROR("Unable to create vehicle physics with no wheel actor component. Aborting");
         return false;
      }

      if (wheelAC->GetNumAxles() == 0)
      {
         wheelAC->FindAxles();
         if (wheelAC->GetNumAxles() == 0)
         {
            LOG_ERROR("Unable to create vehicle physics with no wheels. Aborting");
            return false;
         }
      }

      osg::Matrix bodyOffset;
      GetLocalMatrix(bodyNode, bodyOffset);
      //To allow the developer to shift the center of mass.
      bodyOffset.setTrans(bodyOffset.getTrans() - GetMainPhysicsObject()->GetOriginOffset());

      float frontDamping = 0.0f, rearDamping = 0.0f, frontSpring = 0.0f, rearSpring = 0.0f;
      std::vector<osg::Matrix> WheelMatrix;
      std::vector<osg::Vec3>   WheelVec;
      std::vector<osg::Vec2>   WheelSizes;

      unsigned frontWheelCount = 0;
      unsigned rearWheelCount = 0;

      float frontLeverArm = 0.0f; // Y distance from front wheels to center of gravity
      float rearLeverArm  = 0.0f;  // Y distance from rear wheels to center of gravity

      for (unsigned i = 0; i < wheelAC->GetNumAxles(); ++i)
      {
         SimCore::ActComps::Axle* curAxle = wheelAC->GetAxle(i);

         for (unsigned j = 0; j < curAxle->GetNumWheels(); ++j)
         {
            osg::Matrix wheelModelRelative;
            osg::Vec2 widthRadius = curAxle->GetWheelWidthAndRadius();
            WheelSizes.push_back(widthRadius);
            curAxle->GetWheelBaseTransform(j, wheelModelRelative, false);
            WheelMatrix.push_back(wheelModelRelative);
            WheelVec.push_back(WheelMatrix.back().getTrans() - bodyOffset.getTrans());

            if (WheelVec.back()[1] > 0.0f)
            {
               ++frontWheelCount;
               frontLeverArm += WheelVec.back()[1];
            }
            else
            {
               ++rearWheelCount;
               rearLeverArm += -WheelVec.back()[1];
            }
         }
      }

      // Take the average distance from the center of mass.
      if (frontWheelCount > 0)
      {
         frontLeverArm /= float(frontWheelCount);
      }
      // Take the average distance from the center of mass.
      if (rearWheelCount > 0)
      {
         rearLeverArm /= float(rearWheelCount);
      }

      // This is a hack to handle no front or no back wheels.  It would be better to find
      // the hitch point since no wheels in one end would mean it has to be held up by something.
      if (frontWheelCount == 0)
      {
         frontLeverArm = rearLeverArm;
      }
      if (rearWheelCount == 0)
      {
         rearLeverArm = frontLeverArm;
      }

      if (!ga->GetGameActorProxy().IsRemote())
      {

         float wheelbase  = frontLeverArm + rearLeverArm;
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

         float gravity = dtPhysics::PhysicsWorld::GetInstance().GetGravity().length();

         float frontWheelLoad  = 0.5f * ( GetChassisMass() * gravity * rearLeverArm / wheelbase );
         float rearWheelLoad   = 0.5f * ( GetChassisMass() * gravity * frontLeverArm / wheelbase );
         float frontDeflection = (frontWheelLoad / frontSpring);
         float rearDeflection  = (rearWheelLoad / rearSpring);
         mFrontMaxJounce       = dtUtil::Max(0.0f, GetFrontSuspensionRestLength() - frontDeflection);
         mRearMaxJounce        = dtUtil::Max(0.0f, GetRearSuspensionRestLength() - rearDeflection);

         for (unsigned i = 0; i < frontWheelCount; ++i)
         {
            WheelVec[i][2] += mFrontMaxJounce;
            if (WheelVec[i][0] < 0.0)
            {
               WheelVec[i][0] -= mFrontTrackAdjustment;
            }
            else
            {
               WheelVec[i][0] += mFrontTrackAdjustment;
            }
         }

         for (unsigned i = frontWheelCount; i < frontWheelCount + rearWheelCount; ++i)
         {
            WheelVec[i][2] += mRearMaxJounce;
            if (WheelVec[i][0] < 0.0)
            {
               WheelVec[i][0] -= mRearTrackAdjustment;
            }
            else
            {
               WheelVec[i][0] += mRearTrackAdjustment;
            }
         }

      }

      CreateChassis(transformForRot, bodyNode);

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

      unsigned frontAxles = frontWheelCount / 2;

      sp.mDamperCoef /= float(frontAxles);
      sp.mSpringRate /= float(frontAxles);

      for (unsigned i = 0; i < frontWheelCount; ++i)
      {
         if (GetFrontWheelWidth() < FLT_EPSILON)
         {
            tp.mWidth = WheelSizes[i][0];
         }
         else
         {
            tp.mWidth = GetFrontWheelWidth();
         }

         if (GetFrontWheelRadius() < FLT_EPSILON)
         {
            tp.mRadius = WheelSizes[i][1];
         }
         else
         {
            tp.mRadius = GetFrontWheelRadius();
         }

         mWheels.push_back(AddWheel(WheelVec[i], tp, sp, GetIsVehicleFourWheelDrive(), true, true));
      }

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

      unsigned rearAxles = rearWheelCount / 2;

      sp.mDamperCoef /= float(rearAxles);
      sp.mSpringRate /= float(rearAxles);

      sp.mRestLength = GetRearSuspensionRestLength();
      sp.mTravel = GetRearSuspensionTravel();
      sp.mRollInfluence = 0.1f;

      for (unsigned i = frontWheelCount; i < frontWheelCount + rearWheelCount; ++i)
      {
         if (GetRearWheelWidth() < FLT_EPSILON)
         {
            tp.mWidth = WheelSizes[i][0];
         }
         else
         {
            tp.mWidth = GetRearWheelWidth();
         }

         if (GetRearWheelRadius() < FLT_EPSILON)
         {
            tp.mRadius = WheelSizes[i][1];
         }
         else
         {
            tp.mRadius = GetRearWheelRadius();
         }
         mWheels.push_back(AddWheel(WheelVec[i], tp, sp, true, false, true));
      }

      FinalizeInitialization();

      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsActComp::CleanUp()
   {
      BaseClass::CleanUp();
      for (unsigned i = 0; i < mWheels.size(); ++i)
      {
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

   void FourWheelVehiclePhysicsActComp::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      SimCore::Actors::IGActor* igActor = NULL;
      GetOwner(igActor);
      if (igActor == NULL)
      {
         LOG_ERROR("The four wheel vehicle physics helper only support IG Actors as owners currently.");
         return;
      }

      dtCore::Transform ourTransform;
      igActor->GetTransform(ourTransform);

      dtUtil::NodeCollector* nodeCollector = igActor->GetNodeCollector();

      osg::Node* chassis = NULL;
      if (nodeCollector != NULL)
      {
         chassis = nodeCollector->GetDOFTransform("dof_chassis");
         if (chassis == NULL)
         {
            chassis = nodeCollector->GetGroup("Body");
         }
      }

      if (chassis == NULL)
      {
         LOGN_ERROR("FourWheelVehicleActor.cpp",
                  "Unable to find either a \"dof_chassis\" node or a \"Body\" node.  Vehicle will not be created.");
      }
      else
      {
         osg::Matrix bodyOffset;
         bodyOffset.makeIdentity();
         GetLocalMatrix(*chassis, bodyOffset);
         bodyOffset.setTrans(bodyOffset.getTrans() - GetMainPhysicsObject()->GetOriginOffset());
         dtCore::Transform offsetXform;
         offsetXform.Set(bodyOffset);

         GetMainPhysicsObject()->SetVisualToBodyTransform(offsetXform);

         CreateVehicle(ourTransform, *chassis);
         GetMainPhysicsObject()->SetTransformAsVisual(ourTransform);
      }

   }
} // end namespace
