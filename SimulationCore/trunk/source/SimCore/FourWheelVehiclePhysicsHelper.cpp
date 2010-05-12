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
#include <SimCore/PhysicsTypes.h>

//#else
//#include <dtPhysics/physicscomponent.h>
//#endif
#include <osg/Matrix>
#include <osgSim/DOFTransform>
#include <osg/MatrixTransform>
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
   , mMaxMPH(200.0f)
   , mIsVehicleFourWheelDrive(false)
   , mFrontTrackAdjustment(0.0f)
   , mRearTrackAdjustment(0.0f)
   , mCurrentNormalizedSteering(0.0f)
   , mCurrentEngineTorque(0.0f)
   , mCurrentNormalizedBrakes(0.0f)
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
#ifdef AGEIA_PHYSICS
      // ( X * 10 * 60) / (5280* 12) * 100 = X * 0.9469696
      //return (-((10 * GetWheelSizeWidth() * NxPi * mWheels[BACK_LEFT]->getAxleSpeed() * 60) / (5280 * 12)) * 100);
      return -0.9469696 * 2.0 * GetWheelRadius() * osg::PI * mWheels[BACK_LEFT].mWheel->getAxleSpeed();
#endif
      return 0.0;
   }

   float FourWheelVehiclePhysicsHelper::GetWheelRotation( WheelLocation index ) const
   {
      return mAxleRotation[(int(index) < 2) ? 0 : 1];
   }

   /// Returns the current vertical displacement of the chosen wheel.
   float FourWheelVehiclePhysicsHelper::GetWheelJounce( WheelLocation index ) const
   {
#ifdef AGEIA_PHYSICS
      NxWheelContactData wheelPatchData;
      NxShape*           contactObject;
      float              maxJounce = (index < 2 ? mFrontMaxJounce : mRearMaxJounce);

      contactObject =  mWheels[index].mWheel->getContact ( wheelPatchData );
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
#endif
      return 0.0;
   }

   IMPLEMENT_PROPERTY(FourWheelVehiclePhysicsHelper, float, MaxMPH);
   IMPLEMENT_PROPERTY(FourWheelVehiclePhysicsHelper, bool, IsVehicleFourWheelDrive);
   IMPLEMENT_PROPERTY(FourWheelVehiclePhysicsHelper, float, FrontTrackAdjustment);
   IMPLEMENT_PROPERTY(FourWheelVehiclePhysicsHelper, float, RearTrackAdjustment);

   // ///////////////////////////////////////////////////////////////////////////////////
   //                                  Vehicle Methods                                 //
   // ///////////////////////////////////////////////////////////////////////////////////

   /// Updates vehicle position and rotation.
   void FourWheelVehiclePhysicsHelper::UpdateVehicle(float deltaTime)
   {
      float mph = GetMPH();

#ifdef AGEIA_PHYSICS
      static const double TWO_PI = osg::PI * 2;
      for(int i = 0 ; i< 2; i++)
      {
         float RotationTemp = mAxleRotation[i];
         RotationTemp += (mWheels[2*i].mWheel->getAxleSpeed()+mWheels[2*i+1].mWheel->getAxleSpeed()) / 2.0f * deltaTime;
         float dum;
         RotationTemp = TWO_PI * std::modf( RotationTemp / TWO_PI, &dum );
         if ( RotationTemp < 0.0f )
            RotationTemp += TWO_PI;
         mAxleRotation[i] = RotationTemp;
      }
#endif
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
   void FourWheelVehiclePhysicsHelper::Steer(float normalizedWheelAngle)
   {
      mCurrentNormalizedSteering = normalizedWheelAngle;
   }

   /////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsHelper::ApplyBrake(float normalizedBrakes)
   {
      mCurrentNormalizedBrakes = normalizedBrakes;
   }

   /////////////////////////////////////////////////////////
   void FourWheelVehiclePhysicsHelper::ApplyAccelerator(float pedal)
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

   bool FourWheelVehiclePhysicsHelper::CreateVehicle(const dtCore::Transform& transformForRot,
            const osg::Node& bodyNode, osgSim::DOFTransform* wheels[4])
   {
      // Make sure we have valid nodes for geometry of all four wheels.
      for(int i = 0 ; i < 4; ++i)
      {
         if(wheels[i] == NULL)
            return false;
      }

      osg::Matrix WheelMatrix[4];
      osg::Vec3   WheelVec[4];
      osg::Matrix bodyOffset;
      GetLocalMatrix(bodyNode, bodyOffset);
      //This is not needed for physX
#ifndef AGEIA_PHYSICS
      //To allow the developer to shift the center of mass.
      bodyOffset.setTrans(bodyOffset.getTrans() - GetMainPhysicsObject()->GetOriginOffset());
#endif
      for(int i = 0; i < 4; i++)
      {
         GetLocalMatrix(*(wheels[i]), WheelMatrix[i]);
         WheelVec[i] = WheelMatrix[i].getTrans() - bodyOffset.getTrans();
      }

      float frontLeverArm   =  WheelVec[FRONT_LEFT][1]; // Y distance from front wheels to center of gravity
      float rearLeverArm    = -WheelVec[BACK_LEFT][1];  // Y distance from rear wheels to center of gravity
      float wheelbase       = frontLeverArm + rearLeverArm;
      if (wheelbase <= 0.0)
      {
         LOGN_ERROR("FourWheelVehiclePhysicsHelper.cpp", "Wheelbase must be greater than zero. "
                  "A zero wheel base can be a result of wheels being configured in the wrong order in the array.");
         //Prevent NAN and INF
         wheelbase = 1.0;
      }

      float frontDamping, rearDamping, frontSpring, rearSpring;

      frontSpring = CalcSpringRate(GetSuspensionSpringFreq(), GetChassisMass(), wheelbase, rearLeverArm);
      rearSpring = CalcSpringRate(GetSuspensionSpringFreq(), GetChassisMass(), wheelbase, frontLeverArm);
      frontDamping = CalcDamperCoeficient(GetSuspensionSpringDamper(), GetChassisMass(), frontSpring, wheelbase, rearLeverArm);
      rearDamping = CalcDamperCoeficient(GetSuspensionSpringDamper(), GetChassisMass(), rearSpring, wheelbase, frontLeverArm);

      float frontWheelLoad  = 0.5f * ( GetChassisMass() * ACC_GRAVITY * rearLeverArm / wheelbase );
      float rearWheelLoad   = 0.5f * ( GetChassisMass() * ACC_GRAVITY * frontLeverArm / wheelbase );
      float frontDeflection = (frontWheelLoad / frontSpring);
      float rearDeflection  = (rearWheelLoad / rearSpring);
      mFrontMaxJounce       = dtUtil::Max(0.0f, GetWheelSuspensionTravel() - frontDeflection);
      mRearMaxJounce        = dtUtil::Max(0.0f, GetWheelSuspensionTravel() - rearDeflection);

      WheelVec[FRONT_LEFT][2] += mFrontMaxJounce;
      WheelVec[FRONT_RIGHT][2] += mFrontMaxJounce;
      WheelVec[BACK_LEFT][2] += mRearMaxJounce;
      WheelVec[BACK_RIGHT][2] += mRearMaxJounce;

      WheelVec[FRONT_LEFT][0] -= mFrontTrackAdjustment;
      WheelVec[FRONT_RIGHT][0] += mFrontTrackAdjustment;
      WheelVec[BACK_LEFT][0] -= mRearTrackAdjustment;
      WheelVec[BACK_RIGHT][0] += mRearTrackAdjustment;

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

      mWheels[FRONT_LEFT]   = AddWheel(WheelVec[FRONT_LEFT], *static_cast<osg::Transform*>(wheels[FRONT_LEFT]->getParent(0)), frontSpring, frontDamping, GetIsVehicleFourWheelDrive(), true, true);
      mWheels[FRONT_RIGHT]  = AddWheel(WheelVec[FRONT_RIGHT], *static_cast<osg::Transform*>(wheels[FRONT_RIGHT]->getParent(0)), frontSpring, frontDamping, GetIsVehicleFourWheelDrive(), true, true);
      mWheels[BACK_LEFT]    = AddWheel(WheelVec[BACK_LEFT], *static_cast<osg::Transform*>(wheels[BACK_LEFT]->getParent(0)), rearSpring, rearDamping, true, false, true);
      mWheels[BACK_RIGHT]   = AddWheel(WheelVec[BACK_RIGHT], *static_cast<osg::Transform*>(wheels[BACK_RIGHT]->getParent(0)), rearSpring, rearDamping, true, false, true);


#ifdef AGEIA_PHYSICS
      NxMat33 orient;
      orient.setRow(0, NxVec3(1,0,0));
      orient.setRow(1, NxVec3(0,0,-1));
      orient.setRow(2, NxVec3(0,1,0));
      SwitchCoordinateSystem(orient);
#endif

      FinalizeInitialization();

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

      static const dtUtil::RefString FOUR_WHEEL_GROUP("Four Wheel Vehicle");

      toFillIn.push_back(new dtDAL::BooleanActorProperty("Four Wheel Drive", "Four Wheel Drive",
               dtDAL::BooleanActorProperty::SetFuncType(this, &FourWheelVehiclePhysicsHelper::SetIsVehicleFourWheelDrive),
               dtDAL::BooleanActorProperty::GetFuncType(this, &FourWheelVehiclePhysicsHelper::GetIsVehicleFourWheelDrive),
               "", FOUR_WHEEL_GROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("FrontTrackAdjustment", "Front Wheel Track Adjustment",
               dtDAL::FloatActorProperty::SetFuncType(this, &FourWheelVehiclePhysicsHelper::SetFrontTrackAdjustment),
               dtDAL::FloatActorProperty::GetFuncType(this, &FourWheelVehiclePhysicsHelper::GetFrontTrackAdjustment),
               "Track is the distance along the axle of a wheel from the centerline of a vehicle."
               "Setting this moves the front wheels closer or farther from the centerline.",
               FOUR_WHEEL_GROUP));

      toFillIn.push_back(new dtDAL::FloatActorProperty("RearTrackAdjustment", "Rear Wheel Track Adjustment",
               dtDAL::FloatActorProperty::SetFuncType(this, &FourWheelVehiclePhysicsHelper::SetRearTrackAdjustment),
               dtDAL::FloatActorProperty::GetFuncType(this, &FourWheelVehiclePhysicsHelper::GetRearTrackAdjustment),
               "Track is the distance along the axle of a wheel from the centerline of a vehicle."
               "Setting this moves the rear wheels closer or farther from the centerline.",
               FOUR_WHEEL_GROUP));
   }
} // end namespace
