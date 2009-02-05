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

#include <SimCore/BaseVehiclePhysicsHelper.h>
#include <osg/Matrix>
#include <dtDAL/enginepropertytypes.h>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
#endif

namespace SimCore
{
   ///a workaround for the transform inefficiency
   void GetLocalMatrix(osgSim::DOFTransform* node, osg::Matrix& wcMatrix);

   /// Constructor
   BaseVehiclePhysicsHelper::BaseVehiclePhysicsHelper(dtGame::GameActorProxy &proxy)
      : dtPhysics::PhysicsHelper(proxy)
         , mMotorTorque(300.0f)      
         , mVehicleMaxMPH(120.0f)            
         , mVehicleMaxReverseMPH(40.0f)     
         , mHorsePower(150)        
         , mAllWheelDrive(false)    
         , mVehicleWeight(3500.0f)        
         , mWheelTurnRadiusPerUpdate(0.03f) 
         , mWheelInverseMass(0.15)         
         , mWheelRadius(0.5f)          
         , mWheelSuspensionTravel(0.35f)    
         , mSuspensionSpringCoef(600.0f)   
         , mSuspensionSpringDamper(15.0f)       
         , mSuspensionSpringTarget(0.0f)          
         , mMaxWheelRotation(45.0f)          
         , mTireExtremumSlip(1.0f)  
         , mTireExtremumValue(0.02f) 
         , mTireAsymptoteSlip(2.0f) 
         , mTireAsymptoteValue(0.01f)
         , mTireStiffnessFactor(1000000.0f)
         , mTireRestitution(0.5f)
         , mTireDynamicFriction(0.5f)
         , mTireStaticFriction(0.2f)
      {

      }

      /// Destructor (currently does nothing)
      BaseVehiclePhysicsHelper::~BaseVehiclePhysicsHelper(){}

      /// A workaround for the transform inefficiency
      void GetLocalMatrix( osgSim::DOFTransform* node, osg::Matrix& wcMatrix )
      {
         if(node->getNumParents() > 0)
         {
            osg::MatrixTransform* parentNode = dynamic_cast<osg::MatrixTransform*>(node->getParent(0));
            if(parentNode != NULL)
            {
               wcMatrix = parentNode->getMatrix();
            }
         }
      }


      // ///////////////////////////////////////////////////////////////////////////////////
      //                               Vehicle Initialization                             //
      // ///////////////////////////////////////////////////////////////////////////////////

         // Adds a single wheel to the vehicle.
         WheelType* BaseVehiclePhysicsHelper::AddWheel(const osg::Vec3& position, bool steerable)
         {
            if(GetPhysXObject() == NULL)
               return NULL;

            NxWheelShapeDesc wheelShapeDesc;

            NxTireFunctionDesc lngTFD;
            lngTFD.extremumSlip = mTireExtremumSlip;
            lngTFD.extremumValue = mTireExtremumValue;
            lngTFD.asymptoteSlip = mTireAsymptoteSlip;
            lngTFD.asymptoteValue = mTireAsymptoteValue;
            lngTFD.stiffnessFactor = mTireStiffnessFactor; // effects acceleration

            std::string NameofMaterial = "WheelNoFrictionMaterial";
            dtPhysics::PhysicsComponent* worldComponent = dynamic_cast<dtPhysics::PhysicsComponent*>(mGameProxy->GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"));
            if(worldComponent != NULL)
            {
               worldComponent->RegisterMaterial(NameofMaterial, mTireRestitution, mTireDynamicFriction, mTireStaticFriction);
               unsigned flags = worldComponent->GetMaterial(NameofMaterial, "Default", true).getFlags();
               if(flags & NX_MF_DISABLE_FRICTION)
               {}
               else
               {
                  flags |= NX_MF_DISABLE_FRICTION;
                  worldComponent->GetMaterial(NameofMaterial, "Default", true).setFlags(flags);
               }
               wheelShapeDesc.materialIndex = worldComponent->GetMaterialIndex(NameofMaterial, "Default", true);
            }
            else
               wheelShapeDesc.materialIndex = 0;

            osg::Vec3 newPosition = position + GetLocalOffSet();
            wheelShapeDesc.localPose.t = dtPhysics::VectorType(newPosition[0], newPosition[1], newPosition[2]);

            wheelShapeDesc.suspension.spring       = mSuspensionSpringCoef;
            wheelShapeDesc.suspension.damper       = mSuspensionSpringDamper;
            wheelShapeDesc.suspension.targetValue  = mSuspensionSpringTarget;
            wheelShapeDesc.radius                  = mWheelRadius;
            wheelShapeDesc.suspensionTravel        = mWheelSuspensionTravel;
            wheelShapeDesc.inverseWheelMass        = mWheelInverseMass;
            
            //TODO- do we need to different versions of the properties to handle this?
            wheelShapeDesc.longitudalTireForceFunction = lngTFD;
            wheelShapeDesc.lateralTireForceFunction = lngTFD;

            WheelType* wheelShape = static_cast<WheelType*>(GetPhysXObject()->createShape(wheelShapeDesc));
            if(!steerable)
               wheelShape->setWheelFlags(NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED);
            else
               wheelShape->setWheelFlags(NX_WF_USE_WHEELSHAPE | NX_WF_BUILD_LOWER_HALF | NX_WF_ACCELERATED | NX_WF_STEERABLE_INPUT);

            return wheelShape;
         }

         bool BaseVehiclePhysicsHelper::CreateVehicle(const dtCore::Transform& transformForRot, osgSim::DOFTransform* bodyNode)
         {
            // make sure bad data wasn't passed in
            if(bodyNode == NULL)
            {
               return false;
            }

            dtPhysics::PhysicsObject* physicsObject = GetPhysXObject();
            if(physicsObject == NULL)
            {
               return false;
            }

            osg::Matrix BodyMatrix;
            GetLocalMatrix(bodyNode, BodyMatrix);

            osg::Matrix rot;
            transformForRot.GetRotation(rot);

            // we set the actor's relative position to the position of the dof_chassis
            // by setting the translation of this matrix
            NxMat34 sendInMatrix(NxMat33( dtPhysics::VectorType(rot(0,0), rot(0,1), rot(0,2)),
                                          dtPhysics::VectorType(rot(1,0), rot(1,1), rot(1,2)),
                                          dtPhysics::VectorType(rot(2,0), rot(2,1), rot(2,2))),
                                          dtPhysics::VectorType(BodyMatrix.getTrans()[0],
                                                                BodyMatrix.getTrans()[1],
                                                                BodyMatrix.getTrans()[2]));

            //we set the local offset to zero so the convex mesh below will be created relative to
            //the position of the actor which is set to be position of the dof_chassis above
            SetLocalOffSet(osg::Vec3(0.0f, 0.0f, 0.0f));

            SetCollisionConvexMesh(bodyNode, sendInMatrix, 0, mVehicleWeight, false, "", "Default", "Default", 0);

            dtPhysics::VectorType massPos = physicsObject->getCMassLocalPosition();
            massPos.x += GetVehiclesCenterOfMass()[0];
            massPos.y += GetVehiclesCenterOfMass()[1];
            massPos.z += GetVehiclesCenterOfMass()[2];
            physicsObject->setCMassOffsetLocalPosition(massPos);
            return true;
         
         }

      //////////////////////////////////////////////////////////////////////////////////////
      //                                    Properties                                    //
      //////////////////////////////////////////////////////////////////////////////////////

         /// Builds the property map for this vehicle.
         ///
         /// @param toFillIn    dtDAL::ActorProperty for this vehicle

         void BaseVehiclePhysicsHelper::BuildPropertyMap(std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >& toFillIn)
         {
            dtPhysics::PhysicsHelper::BuildPropertyMap(toFillIn);

            const std::string& VEHICLEGROUP = "Vehicle Physics";
            const std::string& WHEELGROUP = "Wheel Physics";

            toFillIn.push_back(new dtDAL::FloatActorProperty("Motor Torque", "Motor Torque",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetMotorTorque),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetMotorTorque),
               "", VEHICLEGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Max Velocity in MPH", "Max Velocity in MPH",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleMaxMPH),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleMaxMPH),
               "", VEHICLEGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("mVehicleMaxReverseMPH", "mVehicleMaxReverseMPH",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleMaxReverseMPH),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleMaxReverseMPH),
               "", VEHICLEGROUP));

            toFillIn.push_back(new dtDAL::IntActorProperty("Horse Power", "Horse Power",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleHorsePower),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleHorsePower),
               "", VEHICLEGROUP));

            toFillIn.push_back(new dtDAL::BooleanActorProperty("Is All Wheel Drive", "Is All Wheel Drive",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetIsAllWheelDrive),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetIsAllWheelDrive),
               "", VEHICLEGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Vehicle Weight", "Vehicle Weight",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleWeight),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleWeight),
               "", VEHICLEGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("mWheelTurnRadiusPerUpdate", "mWheelTurnRadiusPerUpdate",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetVehicleTurnRadiusPerUpdate),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetVehicleTurnRadiusPerUpdate),
               "", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Inverse Mass of Wheel", "Inverse Mass of Wheel",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelInverseMass),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelInverseMass),
               "", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Wheel Radius", "Wheel Radius",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelRadius),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelRadius),
               "", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Travel", "Suspension Travel",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetWheelSuspensionTravel),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetWheelSuspensionTravel),
               "", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Spring Coefficient", "Suspension Spring Coefficient",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetSuspensionSpringCoef),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetSuspensionSpringCoef),
               "", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Spring Damper", "Suspension Spring Damper",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetSuspensionSpringDamper),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetSuspensionSpringDamper),
               "", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Suspension Spring Target", "Suspension Spring Target",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetSuspensionSpringTarget),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetSuspensionSpringTarget),
               "Target value position of spring where the spring force is zero.", WHEELGROUP));           

            toFillIn.push_back(new dtDAL::FloatActorProperty("Max Wheel Rotation", "Max Wheel Rotation",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetMaxWheelRotation),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetMaxWheelRotation),
               "The maximum amount the wheel can rotate when turning, in degrees.", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Extreme Slip", "Tire Extreme Slip",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireExtremumSlip),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireExtremumSlip),
               "Extremal point of curve.  Values must be positive.", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Extreme Value", "Tire Extreme Value",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireExtremumValue),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireExtremumValue),
               "Extremal point of curve.  Values must be positive.", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Asymptote Slip", "Tire Asymptote Slip",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireAsymptoteSlip),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireAsymptoteSlip),
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Asymptote Value", "Tire Asymptote Value",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireAsymptoteValue),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireAsymptoteValue),
               "Point on curve at which for all x > minumumX, function equals minimumY.  Must be positive.", WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Stiffness", "Tire Stiffness",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireStiffnessFactor),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireStiffnessFactor),
               "This is an additional overall positive scaling that gets applied to the tire forces before passing them to the solver.  Higher values make for better grip.  If you raise the values above, you may need to lower this. A setting of zero will disable all friction in this direction.",
               WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Restitution", "Tire Restitution",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireRestitution),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireRestitution),
               "Coefficient of restitution --  0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce.  Note that values close to or above 1 may cause stability problems and/or increasing energy.",
               WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Dynamic Friction", "Tire Dynamic Friction",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireDynamicFriction),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireDynamicFriction),
               "Coefficient of dynamic friction -- should be in [0, +inf]. If set to greater than staticFriction, the effective value of staticFriction will be increased to match.  If flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)",
               WHEELGROUP));

            toFillIn.push_back(new dtDAL::FloatActorProperty("Tire Static Friction", "Tire Static Friction",
               dtDAL::MakeFunctor(*this, &BaseVehiclePhysicsHelper::SetTireStaticFriction),
               dtDAL::MakeFunctorRet(*this, &BaseVehiclePhysicsHelper::GetTireStaticFriction),
               "Coefficient of static friction -- should be in [0, +inf] if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)",
               WHEELGROUP));
         }
} // end namespace

