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
*/
#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Actors/BaseEntity.h>

#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/templateutility.h>

#include <osg/MatrixTransform>
#include <osg/Group>

#include <SimCore/Components/RenderingSupportComponent.h>

namespace SimCore
{

   namespace Actors
   {
      
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::DamageStateEnum);
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE("No Damage");
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE("Slight Damage");
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE("Moderate Damage");
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::DESTROYED("Destroyed");

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::ForceEnum);
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::OTHER("OTHER");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::FRIENDLY("FRIENDLY");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::OPPOSING("OPPOSING");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::NEUTRAL("NEUTRAL");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::INSURGENT("INSURGENT");

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::ServiceEnum);
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::OTHER("OTHER");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::ARMY("ARMY");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::AIR_FORCE("AIR FORCE");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::COAST_GUARD("COAST GUARD");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::MARINES("MARINES");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::NAVY("NAVY");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::JOINT("JOINT");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::CIVILIAN("CIVILIAN");
      BaseEntityActorProxy::ServiceEnum BaseEntityActorProxy::ServiceEnum::REFUGEE("REFUGEE");

      /////////////////////////////////////////////////////////////////////
      ///////////////   BaseEntityActorProxy           ////////////////////
      /////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::BaseEntityActorProxy()
      {
         SetClassName("SimCore::Actors::BaseEntity");
      }
      /////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::~BaseEntityActorProxy()
      {
      }


      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_LAST_KNOWN_TRANSLATION("Last Known Translation");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_LAST_KNOWN_ROTATION("Last Known Rotation");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_VELOCITY_VECTOR("Velocity Vector");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ACCELERATION_VECTOR("Acceleration Vector");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ANGULAR_VELOCITY_VECTOR("Angular Velocity Vector");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ENGINE_SMOKE_POSITION("EngineSmokePosition");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ENGINE_SMOKE_ON("EngineSmokeOn");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_FROZEN("Frozen");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_FLAMES_PRESENT("FlamesPresent");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_SMOKE_PLUME_PRESENT("SmokePlumePresent");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ENGINE_POSITION("Engine Position");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_FLYING("Flying");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_DAMAGE_STATE("Damage State");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_DEFAULT_SCALE("Default Scale");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_SCALE_MAGNIFICATION_FACTOR("Scale Magnification Factor");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MODEL_SCALE("Model Scale");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MODEL_ROTATION("Model Rotation");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ENTITY_TYPE("Entity Type As String");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MAPPING_NAME("Object Mapping Name");

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::BuildPropertyMap()
      {
         BaseEntity &e = static_cast<BaseEntity&>(GetGameActor());

         BaseClass::BuildPropertyMap();

         static const dtUtil::RefString BASE_ENTITY_GROUP("Base Entity");

         static const dtUtil::RefString PROPERTY_LAST_KNOWN_TRANSLATION_DESC("Sets the last know position of this BaseEntity");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_LAST_KNOWN_TRANSLATION, PROPERTY_LAST_KNOWN_TRANSLATION,
            dtDAL::MakeFunctor(e, &BaseEntity::SetLastKnownTranslation),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetLastKnownTranslation),
            PROPERTY_LAST_KNOWN_TRANSLATION_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_LAST_KNOWN_ROTATION_DESC("Sets the last know rotation of this BaseEntity");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_LAST_KNOWN_ROTATION, PROPERTY_LAST_KNOWN_ROTATION,
            dtDAL::MakeFunctor(*this, &BaseEntityActorProxy::SetLastKnownRotation),
            dtDAL::MakeFunctorRet(*this, &BaseEntityActorProxy::GetLastKnownRotation),
            PROPERTY_LAST_KNOWN_ROTATION_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_VELOCITY_VECTOR_DESC("Sets the velocity vector of this BaseEntity");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_VELOCITY_VECTOR, PROPERTY_VELOCITY_VECTOR,
            dtDAL::MakeFunctor(e, &BaseEntity::SetVelocityVector),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetVelocityVector),
            PROPERTY_VELOCITY_VECTOR_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_ACCELERATION_VECTOR_DESC("Sets the acceleration vector of this BaseEntity");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_ACCELERATION_VECTOR, PROPERTY_ACCELERATION_VECTOR,
            dtDAL::MakeFunctor(e, &BaseEntity::SetAccelerationVector),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetAccelerationVector),
            PROPERTY_ACCELERATION_VECTOR_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_ANGULAR_VELOCITY_VECTOR_DESC("Sets the angular velocity vector of this BaseEntity");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_ANGULAR_VELOCITY_VECTOR, PROPERTY_ANGULAR_VELOCITY_VECTOR,
            dtDAL::MakeFunctor(e, &BaseEntity::SetAngularVelocityVector),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetAngularVelocityVector),
            PROPERTY_ANGULAR_VELOCITY_VECTOR_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_FROZEN_DESC("Whether or not the simulation of the entity is frozen.");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_FROZEN, PROPERTY_FROZEN,
            dtDAL::MakeFunctor(e, &BaseEntity::SetFrozen),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetFrozen),
            PROPERTY_FROZEN_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_POSITION_LABEL("Engine Smoke Position");
         static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_POSITION_DESC("Sets the engine smoke position of this BaseEntity");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_ENGINE_SMOKE_POSITION, PROPERTY_ENGINE_SMOKE_POSITION_LABEL,
            dtDAL::MakeFunctor(e, &BaseEntity::SetEngineSmokePos),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetEngineSmokePos),
            PROPERTY_ENGINE_SMOKE_POSITION_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_ON_LABEL("Engine Smoke On");
         static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_ON_DESC("Enables engine smoke");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_ENGINE_SMOKE_ON, PROPERTY_ENGINE_SMOKE_ON_LABEL,
            dtDAL::MakeFunctor(e, &BaseEntity::SetEngineSmokeOn),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsEngineSmokeOn),
            PROPERTY_ENGINE_SMOKE_ON_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_FLAMES_PRESENT_LABEL("Flames Present");
         static const dtUtil::RefString PROPERTY_FLAMES_PRESENT_DESC("Should the actor be burning");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_FLAMES_PRESENT, PROPERTY_FLAMES_PRESENT_LABEL,
            dtDAL::MakeFunctor(e, &BaseEntity::SetFlamesPresent),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsFlamesPresent),
            PROPERTY_FLAMES_PRESENT_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_SMOKE_PLUME_PRESENT_LABEL("Flames Present");
         static const dtUtil::RefString PROPERTY_SMOKE_PLUME_PRESENT_DESC("Enables engine smoke");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_SMOKE_PLUME_PRESENT, PROPERTY_SMOKE_PLUME_PRESENT_LABEL,
            dtDAL::MakeFunctor(e, &BaseEntity::SetSmokePlumePresent),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsSmokePlumePresent),
            PROPERTY_SMOKE_PLUME_PRESENT_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_ENGINE_POSITION_DESC("Position of the engine in the vehicle");
         dtDAL::Vec3ActorProperty *prop = new dtDAL::Vec3ActorProperty(PROPERTY_ENGINE_POSITION, PROPERTY_ENGINE_POSITION,
                  dtDAL::Vec3ActorProperty::SetFuncType(),
                  dtDAL::Vec3ActorProperty::GetFuncType(&e, &BaseEntity::GetEngineSmokePos),
                  PROPERTY_ENGINE_POSITION_DESC, BASE_ENTITY_GROUP);

         prop->SetReadOnly(true);
         AddProperty(prop);

         dtCore::RefPtr<dtDAL::ResourceActorProperty>  rp = new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Smoke plume particles", "Smoke plume particles",
            dtDAL::MakeFunctor(e, &BaseEntity::SetSmokePlumesFile),
            "This is the file for the smoke particles", BASE_ENTITY_GROUP);

         dtDAL::ResourceDescriptor rdSmoke("Particles:smoke.osg");
         rp->SetValue(&rdSmoke);
         AddProperty(rp.get());

         rp = new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Fire particles", "Fire particles",
            dtDAL::MakeFunctor(e, &BaseEntity::SetFlamesPresentFile),
            "This is the file for vehicle fire particles", BASE_ENTITY_GROUP);

         dtDAL::ResourceDescriptor rdFire("Particles:fire.osg");
         rp->SetValue(&rdFire);
         AddProperty(rp.get());
         
         rp = new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Engine smoke particles", "Engine smoke particles",
            dtDAL::MakeFunctor(e, &BaseEntity::SetEngineSmokeFile),
            "This is the file for engine smoke particles", BASE_ENTITY_GROUP);

         dtDAL::ResourceDescriptor rdEngine("Particles:smoke.osg");
         rp->SetValue(&rdEngine);
         AddProperty(rp.get());

         static const dtUtil::RefString PROPERTY_FLYING_DESC
            ("Flags if the dead-reckoning code should not make this actor follow the ground as it moves.");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_FLYING, PROPERTY_FLYING,
            dtDAL::MakeFunctor(e, &BaseEntity::SetFlying),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsFlying),
            PROPERTY_FLYING_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_DRAWING_MODEL_DESC
            ("Flags if this entity should draw it's model.  This is typically turned off if the entity has the player attached to it.");
         AddProperty(new dtDAL::BooleanActorProperty("DrawingModel", "Draw Model",
            dtDAL::MakeFunctor(e, &BaseEntity::SetDrawingModel),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsDrawingModel),
            PROPERTY_DRAWING_MODEL_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_DAMAGE_STATE_DESC
            ("Changes which model to show based on the level of damage.");
         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::DamageStateEnum>(PROPERTY_DAMAGE_STATE, PROPERTY_DAMAGE_STATE,
            dtDAL::MakeFunctor(e, &BaseEntity::SetDamageState),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetDamageState),
            PROPERTY_DAMAGE_STATE_DESC, BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::EnumActorProperty<dtGame::DeadReckoningAlgorithm>("Dead Reckoning Algorithm", "Dead Reckoning Algorithm",
            dtDAL::MakeFunctor(e, &BaseEntity::SetDeadReckoningAlgorithm),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetDeadReckoningAlgorithm),
            "Sets the enumerated dead reckoning algorithm to use.", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>("Force Affiliation", "Force Affiliation", 
            dtDAL::MakeFunctor(e, &BaseEntity::SetForceAffiliation), 
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetForceAffiliation), 
            "The force for which the entity is fighting.", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("Ground Offset", "Ground Offset", 
            dtDAL::MakeFunctor(e, &BaseEntity::SetGroundOffset), 
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetGroundOffset), 
            "Sets the offset from the ground this entity should have.  This only matters if it is not flying.", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>("Service", "Service", 
            dtDAL::MakeFunctor(e, &BaseEntity::SetService), 
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetService), 
            "Sets the service of this entity", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::StringActorProperty("Munition Damage Table","Munition Damage Table",
            dtDAL::MakeFunctor(e, &BaseEntity::SetMunitionDamageTableName),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetMunitionDamageTableName),
            "The name of the munition damage table name found in Configs/MunitionsConfig.xml", BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_DEFAULT_SCALE_DESC
            ("Changes the desired base scale to make the model/geometry "
                  "of this model correct for the rendering.  Model Scale = Default * Magnification");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_DEFAULT_SCALE, PROPERTY_DEFAULT_SCALE, 
            dtDAL::MakeFunctor(e, &BaseEntity::SetDefaultScale), 
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetDefaultScale),
            PROPERTY_DEFAULT_SCALE_DESC,
            BASE_ENTITY_GROUP));
         
         static const dtUtil::RefString PROPERTY_SCALE_MAGNIFICATION_FACTOR_DESC
            ("Changes the amount the geometry of the entity is magnified.  Model Scale = Default * Magnification");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_SCALE_MAGNIFICATION_FACTOR, PROPERTY_SCALE_MAGNIFICATION_FACTOR,
            dtDAL::MakeFunctor(e, &BaseEntity::SetScaleMagnification), 
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetScaleMagnification), 
            PROPERTY_SCALE_MAGNIFICATION_FACTOR_DESC,
            BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_MODEL_SCALE_DESC
            ("Returns the current scale of the model.  Model Scale = Default * Magnification");
         dtDAL::Vec3ActorProperty* modelScaleProp = new dtDAL::Vec3ActorProperty(
            PROPERTY_MODEL_SCALE, PROPERTY_MODEL_SCALE, 
            dtDAL::Vec3ActorProperty::SetFuncType(), 
            dtDAL::Vec3ActorProperty::GetFuncType(&e, &BaseEntity::GetModelScale), 
            PROPERTY_MODEL_SCALE_DESC,
            BASE_ENTITY_GROUP);

         modelScaleProp->SetReadOnly(true);

         AddProperty(modelScaleProp);

         static const dtUtil::RefString PROPERTY_MODEL_ROTATION_DESC
            ("Model offset rotation HPR");
         AddProperty(new dtDAL::Vec3ActorProperty(
            PROPERTY_MODEL_ROTATION, PROPERTY_MODEL_ROTATION, 
            dtDAL::MakeFunctor(e, &BaseEntity::SetModelRotation), 
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetModelRotation), 
            PROPERTY_MODEL_ROTATION_DESC,
            BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_FIREPOWER_DISABLED("Firepower Disabled");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_FIREPOWER_DISABLED,
            PROPERTY_FIREPOWER_DISABLED,
            dtDAL::MakeFunctor(e, &BaseEntity::SetFirepowerDisabled),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsFirepowerDisabled),
            "Determines if this entity has had its fire power disabled."));

         static const dtUtil::RefString PROPERTY_MOBILITY_DISABLED("Mobility Disabled");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_MOBILITY_DISABLED,
            PROPERTY_MOBILITY_DISABLED,
            dtDAL::MakeFunctor(e, &BaseEntity::SetMobilityDisabled),
            dtDAL::MakeFunctorRet(e, &BaseEntity::IsMobilityDisabled),
            "Determines if this entity has had its mobility disabled."));

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_ENTITY_TYPE,
            PROPERTY_ENTITY_TYPE,
            dtDAL::MakeFunctor(e, &BaseEntity::SetEntityType),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetEntityType),
            "String property into which the Entity Type is captured", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_MAPPING_NAME,
            PROPERTY_MAPPING_NAME,
            dtDAL::MakeFunctor(e, &BaseEntity::SetMappingName),
            dtDAL::MakeFunctorRet(e, &BaseEntity::GetMappingName),
            "String property into which the Object Mapping Name is captured", BASE_ENTITY_GROUP));
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         
         if (!IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         }
         // don't register for remote ticks!!!  
         //else
         //{
            //RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         //}
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::OnRemovedFromWorld()
      {
         // TODO: !!! Call both the actor and proxy functions with InvokeRemovedFromWorld on Game Actor.
         // Game Actor currently does not have this function nor is it being called by the game manager.
         static_cast<SimCore::Actors::BaseEntity*>(&GetGameActor())->OnRemovedFromWorld();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::SetLastKnownRotation(const osg::Vec3 &vec)
      {
         static_cast<BaseEntity&>(GetGameActor()).SetLastKnownRotation(osg::Vec3(vec[2], vec[0], vec[1]));
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntityActorProxy::GetLastKnownRotation() const
      {
         const BaseEntity &e = static_cast<const BaseEntity&>(GetGameActor());

         const osg::Vec3& result = e.GetLastKnownRotation();
         return osg::Vec3(result[1], result[2], result[0]);
      }

      /////////////////////////////////////////////////////////////////////
      ///////////////   BaseEntity                     ////////////////////
      /////////////////////////////////////////////////////////////////////
      const float BaseEntity::TIME_BETWEEN_UPDATES(10.0f);

      BaseEntity::BaseEntity(dtGame::GameActorProxy& proxy): IGActor(proxy),
         mTimeUntilNextUpdate(0.0f),
         mScaleMatrixNode(new osg::MatrixTransform),
         mDeadReckoningHelper(NULL),
         mDRAlgorithm(&dtGame::DeadReckoningAlgorithm::NONE),
         mForceAffiliation(&BaseEntityActorProxy::ForceEnum::NEUTRAL),
         mService(&BaseEntityActorProxy::ServiceEnum::MARINES),
         mDamageState(&BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE),
         mDefaultScale(1.0f, 1.0f, 1.0f),
         mScaleMagnification(1.0f, 1.0f, 1.0f),
         mMaxRotationError(10.0f),
         mMaxRotationError2(100.0f),
         mMaxTranslationError(1.0f),
         mMaxTranslationError2(1.0f),
         mEngineSmokeOn(false),
         mSmokePlumePresent(false),
         mFlamesPresent(false),
         mIsPlayerAttached(false),
         mDisabledFirepower(false),
         mDisabledMobility(false),
         mIsFrozen(false),
         mFireLightID(0)
      {
         mLogger = &dtUtil::Log::GetInstance("BaseEntity.cpp");
         osg::Group* g = GetOSGNode()->asGroup();
         g->addChild(mScaleMatrixNode.get());

         // temp turned off to test performance.
         SetCollisionDetection(false);
      }

      /////////////////////////////////////////////////////////////////////
      BaseEntity::~BaseEntity()
      {
         SetFlamesPresent(false);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetMappingName( const std::string& name )
      {
         mMappingName = name;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      std::string BaseEntity::GetMappingName() const
      {
         return mMappingName;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetEntityType( const std::string& entityType )
      {
         mEntityType = entityType;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      std::string BaseEntity::GetEntityType() const
      {
         return mEntityType;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::InitDeadReckoningHelper()
      {
         mDeadReckoningHelper = new dtGame::DeadReckoningHelper();
         
         SetFlying(false);
                  
         // attempt to fix the z-fighting on treads and wheels that are 
         // very close to the ground. We move the vehicle up about 3-4 inches...
         mDeadReckoningHelper->SetGroundOffset(0.09);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::OnEnteredWorld()
      {
         if (!IsRemote())
         {
            //for now. Set the time for update sending to 10 seconds.
            mTimeUntilNextUpdate = TIME_BETWEEN_UPDATES;

            dtCore::Transform xform;
            GetTransform(xform);
            osg::Vec3 pos;
            xform.GetTranslation(pos);
            SetLastKnownTranslation(pos);
            osg::Vec3 rot;
            xform.GetRotation(rot);
            SetLastKnownRotation(rot);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::OnRemovedFromWorld()
      {
         SetFlamesPresent(false);
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDeadReckoningHelper(dtGame::DeadReckoningHelper* pHelper)
      {
         mDeadReckoningHelper = pHelper;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::DamageStateEnum& BaseEntity::GetDamageState() const
      {
         return *mDamageState;
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDamageState(BaseEntityActorProxy::DamageStateEnum &damageState)
      {
         mDamageState = &damageState;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm& newAlgorithm)
      {
         mDRAlgorithm = &newAlgorithm;
         //if the entity is destroyed, it should be set to static unless DR is turned off altogether.
         if (GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED 
         &&  newAlgorithm != dtGame::DeadReckoningAlgorithm::NONE)
         {
            mDeadReckoningHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::STATIC);
         }
         else
            mDeadReckoningHelper->SetDeadReckoningAlgorithm(newAlgorithm);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtGame::DeadReckoningAlgorithm& BaseEntity::GetDeadReckoningAlgorithm() const
      {
         return *mDRAlgorithm;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetForceAffiliation(BaseEntityActorProxy::ForceEnum& newForceEnum)
      {
         mForceAffiliation = &newForceEnum;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::ForceEnum& BaseEntity::GetForceAffiliation() const
      {
         return *mForceAffiliation;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetService(BaseEntityActorProxy::ServiceEnum &service)
      {
         mService = &service;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::ServiceEnum& BaseEntity::GetService() const
      {
         return *mService;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetLastKnownTranslation(const osg::Vec3& vec)
      {
         mDeadReckoningHelper->SetLastKnownTranslation(vec);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetLastKnownRotation(const osg::Vec3& vec)
      {
         mDeadReckoningHelper->SetLastKnownRotation(vec);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetVelocityVector(const osg::Vec3& vec)
      {
         mDeadReckoningHelper->SetVelocityVector(vec);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetVelocityVector() const
      {
         return mDeadReckoningHelper->GetVelocityVector();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetAccelerationVector(const osg::Vec3& vec)
      {
         mDeadReckoningHelper->SetAccelerationVector(vec);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetAccelerationVector() const
      {
         return mDeadReckoningHelper->GetAccelerationVector();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetAngularVelocityVector(const osg::Vec3 &vec)
      {
         return mDeadReckoningHelper->SetAngularVelocityVector(vec);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetAngularVelocityVector() const
      {
         return mDeadReckoningHelper->GetAngularVelocityVector();
      }
      ////////////////////////////////////////////////////////////////////////////////////
      bool BaseEntity::IsFlying() const
      {
         return mDeadReckoningHelper->IsFlying();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetFlying(bool newFlying)
      {
         return mDeadReckoningHelper->SetFlying(newFlying);
//         if (mFlying)
//            mNode->asGroup()->removeChild(mPointsGeode.get());
//         else
//            mNode->asGroup()->addChild(mPointsGeode.get());
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool BaseEntity::IsDrawingModel() const
      {
         return mDrawing;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDrawingModel(bool newDrawing)
      {
         //don't mess with the node hierarchy if the values have not changed.
         if (mDrawing == newDrawing)
            return;

         mDrawing = newDrawing;
         HandleModelDrawToggle(mDrawing);
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      bool BaseEntity::IsPlayerAttached() const
      {
         return mIsPlayerAttached;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetIsPlayerAttached(bool attach)
      {
         mIsPlayerAttached = attach;
      }
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetFlamesPresent(bool enable)
      {
         if(mFlamesPresent == enable)
            return;

         if(enable)
         {
            if(!mFlamesSystem.valid())
               mFlamesSystem = new dtCore::ParticleSystem;

            mFlamesSystem->LoadFile(mFlamesSystemFile, true);
            mFlamesSystem->SetEnabled(enable);
            AddChild(mFlamesSystem.get());

            Components::ParticleInfo::AttributeFlags attrs = {true,true};
            RegisterParticleSystem(*mFlamesSystem,&attrs);

            if(mFireLightID == 0 && GetGameActorProxy().GetGameManager() != NULL )
            {
               // HACK: Add lights with copied code
               SimCore::Components::RenderingSupportComponent* renderComp;
               GetGameActorProxy().GetGameManager()->GetComponentByName(
                  SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

               if( renderComp != NULL )
               {
                  SimCore::Components::RenderingSupportComponent::DynamicLight* dl = 
                     renderComp->AddDynamicLightByPrototypeName("Light-Entity-Flames");
                  dl->mTarget = this;
                  dl->mAutoDeleteLightOnTargetNull = true;
                  mFireLightID = dl->mID;
               }
            }
         }
         else
         {
            if(mFlamesSystem.get())
            {
               UnregisterParticleSystem(*mFlamesSystem);
               RemoveChild(mFlamesSystem.get());
               mFlamesSystem = NULL;
            }
            if( mFireLightID != 0 && GetGameActorProxy().GetGameManager() != NULL )
            {
               // HACK: Remove the fire light since a NULL target does not remove it.
               SimCore::Components::RenderingSupportComponent* renderComp;
               GetGameActorProxy().GetGameManager()->GetComponentByName(
                  SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

               if( renderComp != NULL )
               {
                  renderComp->RemoveDynamicLight( mFireLightID );
               }
               mFireLightID = 0;
            }
         }
         mFlamesPresent = enable;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetEngineSmokeOn(bool enable)
      {
         if(mEngineSmokeOn == enable)
            return;

         if(enable)
         {
            if(!mEngineSmokeSystem.valid())
               mEngineSmokeSystem = new dtCore::ParticleSystem;

            mEngineSmokeSystem->LoadFile(mEngineSmokeSystemFile, true);
            dtCore::Transform xform;
            xform.MakeScale(osg::Vec3d(0.25, 0.24, 0.25));
            xform.SetTranslation(mEngineSmokePosition);

            mEngineSmokeSystem->SetTransform(xform, dtCore::Transformable::REL_CS);
            mEngineSmokeSystem->SetEnabled(enable);
            mEngineSmokeOn = enable;
         }
         else
         {
            if(mEngineSmokeSystem.valid())
            {
               RemoveChild(mEngineSmokeSystem.get());
               mEngineSmokeSystem = NULL;
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetSmokePlumePresent(bool enable)
      {
         if(mSmokePlumePresent == enable)
            return;

         if(enable)
         {
            if(!mSmokePlumesSystem.valid())
               mSmokePlumesSystem = new dtCore::ParticleSystem;

            mSmokePlumesSystem->LoadFile(mSmokePlumesSystemFile, true);
            mSmokePlumesSystem->SetEnabled(enable);
            AddChild(mSmokePlumesSystem.get());
            mSmokePlumePresent = enable;

            Components::ParticleInfo::AttributeFlags attrs = {true,true};
            RegisterParticleSystem(*mSmokePlumesSystem,&attrs);
         }
         else
         {
            if(mSmokePlumesSystem.valid())
            {
               UnregisterParticleSystem(*mSmokePlumesSystem);
               RemoveChild(mSmokePlumesSystem.get());
               mSmokePlumesSystem = NULL;
            }
         }
      }
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::TickRemote(const dtGame::Message& tickMessage) 
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool BaseEntity::ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate)
      {
         bool forceUpdate = false;

         //if it's not time for an update, check to see if it's moved or turned enough to warrant one.
         if (!forceUpdate)
         {

            osg::Vec3 distanceMoved = pos - GetDeadReckoningHelper().GetCurrentDeadReckonedTranslation();
            // Note the rotation check isn't perfect (ie, not a quaternion), so you might get 
            // an extra update, but it's close enough and is very cheap processor wise.
            osg::Vec3 distanceTurned = rot - GetDeadReckoningHelper().GetCurrentDeadReckonedRotation();

            if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "The change in the translation is \"%f\" for \"%s\" named \"%s\".  The max is \"%f\".",
                  distanceMoved.length(), GetGameActorProxy().GetName().c_str(),
                  GetName().c_str(),  mMaxTranslationError);

               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "The change in the rotation is \"%f\" for \"%s\" named \"%s\".  The max is \"%f\".",
                  distanceTurned.length(), GetGameActorProxy().GetName().c_str(),
                  GetName().c_str(),  mMaxRotationError);
            }

            if (distanceMoved.length2() > mMaxTranslationError2)
            {
               forceUpdate = true;
            }
            else if (distanceTurned.length2() > mMaxRotationError2)
            {
               forceUpdate = true;
            }
         }
         return forceUpdate;
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::RegisterWithDeadReckoningComponent()
      {
         dtGame::DeadReckoningComponent* drc = NULL;
         
         GetGameActorProxy().GetGameManager()->
            GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drc);

         if (drc != NULL)
         {
            drc->RegisterActor(GetGameActorProxy(), GetDeadReckoningHelper());
         } 
         else
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Actor \"%s\"\"%s\" unable to find DeadReckoningComponent.",
               GetName().c_str(), GetUniqueId().ToString().c_str());
         }
      }
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill)
      {
         propNamesToFill.push_back(BaseEntityActorProxy::PROPERTY_LAST_KNOWN_TRANSLATION);
         propNamesToFill.push_back(BaseEntityActorProxy::PROPERTY_LAST_KNOWN_ROTATION);
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::TickLocal(const dtGame::Message& tickMessage)
      {
         mTimeUntilNextUpdate -= static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();

         GameActor::TickLocal(tickMessage);
         
         bool forceUpdate = false;
         bool fullUpdate = false;

         dtCore::Transform xform;
         GetTransform(xform);
         osg::Vec3 rot;
         xform.GetRotation(rot);
         osg::Vec3 pos;
         xform.GetTranslation(pos);

         if (mTimeUntilNextUpdate <= 0.0f)
         {
            mTimeUntilNextUpdate = TIME_BETWEEN_UPDATES;
            fullUpdate = true;
            forceUpdate = true;
         }

         //if( ! fullUpdate )
         //{
         // Check for update (or child class). Call this even if fullUpdate, because they may set some 
         // properties that we need to publish
         forceUpdate = ShouldForceUpdate(pos, rot, fullUpdate);
         //}

         if (forceUpdate)
         {
            SetLastKnownTranslation(pos);
            SetLastKnownRotation(rot);

            dtCore::RefPtr<dtGame::Message> msg = GetGameActorProxy().GetGameManager()->
               GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED);

            if (fullUpdate)
            {
               GetGameActorProxy().PopulateActorUpdate(static_cast<dtGame::ActorUpdateMessage&>(*msg));
            }
            else
            {
               std::vector<std::string> propNames;
               FillPartialUpdatePropertyVector(propNames);
               GetGameActorProxy().PopulateActorUpdate(static_cast<dtGame::ActorUpdateMessage&>(*msg), propNames);
            }

            GetGameActorProxy().GetGameManager()->SendMessage(*msg);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::MatrixTransform& BaseEntity::GetScaleMatrixTransform()
      {
         return *mScaleMatrixNode;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      const osg::MatrixTransform& BaseEntity::GetScaleMatrixTransform() const
      {
         return *mScaleMatrixNode;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      float BaseEntity::GetGroundOffset() const
      {
         return mDeadReckoningHelper->GetGroundOffset();
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetGroundOffset(float newOffset)
      {
         mDeadReckoningHelper->SetGroundOffset(newOffset);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetMunitionDamageTableName( const std::string& tableName )
      {
         mMunitionTableName = tableName;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      std::string BaseEntity::GetMunitionDamageTableName() const
      {
         return mMunitionTableName;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetLastKnownTranslation() const
      { 
         return mDeadReckoningHelper->GetLastKnownTranslation(); 
      }
      
      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetLastKnownRotation() const 
      { 
         return mDeadReckoningHelper->GetLastKnownRotation(); 
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDefaultScale(const osg::Vec3& newDefScale) 
      { 
         mDefaultScale = newDefScale;
         UpdateModelScale();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetScaleMagnification(const osg::Vec3& newScaleMag) 
      { 
         mScaleMagnification = newScaleMag; 
         UpdateModelScale();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::UpdateModelScale()
      {
         dtCore::Transform xform;
         xform.Set(mScaleMatrixNode->getMatrix());
         osg::Vec3 newScale;
         for (int i = 0; i < 3; ++i)
         {
            newScale[i] = mDefaultScale[i] * mScaleMagnification[i]; 
         } 
         xform.Rescale(newScale);
         osg::Matrix m;
         xform.Get(m);
         mScaleMatrixNode->setMatrix(m);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetModelScale() const
      {
         dtCore::Transform xform;
         xform.Set(mScaleMatrixNode->getMatrix());
         osg::Vec3 currentScale;
         xform.CalcScale(currentScale);
         return currentScale;
      } 

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetModelRotation(const osg::Vec3& hpr)
      {
         dtCore::Transform xform;
         xform.Set(mScaleMatrixNode->getMatrix());
         xform.SetRotation(hpr);
         osg::Matrix m;
         xform.Get(m);
         mScaleMatrixNode->setMatrix(m);

         // Recompute the scale since the rotation probably just overwrote it
         UpdateModelScale();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 BaseEntity::GetModelRotation() const
      {
         osg::Vec3 result;
         dtCore::Transform xform;
         xform.Set(mScaleMatrixNode->getMatrix());
         xform.GetRotation(result);
         return result;
      }

      void BaseEntity::SetFrozen( bool frozen )
      {
         mIsFrozen = frozen;
      }

      bool BaseEntity::GetFrozen() const
      {
         return mIsFrozen;
      }
   }

}
