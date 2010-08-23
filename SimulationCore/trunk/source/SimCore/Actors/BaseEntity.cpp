
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
* circumstances in which the U.a S. Government may have rights in the software.
*
* @author David Guthrie, Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>

#include <dtUtil/mswin.h>
#include <dtUtil/mathdefines.h>

#include <SimCore/Actors/BaseEntity.h>

#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/functor.h>
#include <dtDAL/propertymacros.h>
#include <dtUtil/templateutility.h>

#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/ComputeBoundsVisitor>

#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/DRPublishingActComp.h>
#include <SimCore/Messages.h>
#include <SimCore/VisibilityOptions.h>
#include <SimCore/Components/ParticleManagerComponent.h>

#include <iostream>

namespace SimCore
{

   namespace Actors
   {

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::DomainEnum);
      BaseEntityActorProxy::DomainEnum::DomainEnum(const std::string& name, const std::string& displayName)
      : dtUtil::Enumeration(name)
      , mDisplayName(displayName)
      {
         AddInstance(this);
      }
      const std::string& BaseEntityActorProxy::DomainEnum::GetDisplayName() { return mDisplayName; }
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::AIR("AIR", "Air");
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::AMPHIBIOUS("AMPHIBIOUS", "Amphibious");
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::GROUND("GROUND", "Ground");
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::SPACE("SPACE", "Space");
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::SUBMARINE("SUBMARINE", "Submersible");
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::SURFACE("SURFACE", "Surface");
      BaseEntityActorProxy::DomainEnum BaseEntityActorProxy::DomainEnum::MULTI("MULTI", "Multi-Domain");

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::DamageStateEnum);
      BaseEntityActorProxy::DamageStateEnum::DamageStateEnum(const std::string& name)
      : dtUtil::Enumeration(name)
      {
         AddInstance(this);
      }
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE("No Damage");
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE("Slight Damage");
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE("Moderate Damage");
      BaseEntityActorProxy::DamageStateEnum BaseEntityActorProxy::DamageStateEnum::DESTROYED("Destroyed");

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::ForceEnum);
      BaseEntityActorProxy::ForceEnum::ForceEnum(const std::string& name, const std::string displayName)
      : dtUtil::Enumeration(name)
      , mDisplayName(displayName)
      {
         AddInstance(this);
      }
      const std::string& BaseEntityActorProxy::ForceEnum::GetDisplayName() { return mDisplayName; }
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::OTHER("OTHER", "Other");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::FRIENDLY("FRIENDLY", "Friendly");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::OPPOSING("OPPOSING", "Opposing");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::NEUTRAL("NEUTRAL", "Neutral");
      BaseEntityActorProxy::ForceEnum BaseEntityActorProxy::ForceEnum::INSURGENT("INSURGENT", "Insurgent");

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BaseEntityActorProxy::ServiceEnum);
      BaseEntityActorProxy::ServiceEnum::ServiceEnum(const std::string& name)
      : dtUtil::Enumeration(name)
      {
         AddInstance(this);
      }
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


      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_FROZEN("Frozen");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_FLAMES_PRESENT("FlamesPresent");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_SMOKE_PLUME_PRESENT("SmokePlumePresent");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_DAMAGE_STATE("Damage State");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MAX_DAMAGE_AMOUNT("Max Damage Amount");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_CUR_DAMAGE_RATIO("Cur Damage Ratio");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_DEFAULT_SCALE("Default Scale");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_DOMAIN("Domain");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_SCALE_MAGNIFICATION_FACTOR("Scale Magnification Factor");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MODEL_SCALE("Model Scale");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MODEL_ROTATION("Model Rotation");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_ENTITY_TYPE_ID("Entity Type Id");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_MAPPING_NAME("Object Mapping Name");
      const dtUtil::RefString BaseEntityActorProxy::PROPERTY_FORCE("Force Affiliation");


      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::GetPartialUpdateProperties(std::vector<dtUtil::RefString>& propNamesToFill)
      {
         // Add the properties for dead reckoning such as last known translation, etc...
         BaseEntity& e = static_cast<BaseEntity&>(GetGameActor());
         e.GetDeadReckoningHelper().GetPartialUpdateProperties(propNamesToFill);

         // Add your own properties that you want to publish with EVERY partial update (ie very often)
      }


      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::BuildPropertyMap()
      {
         BaseEntity& e = static_cast<BaseEntity&>(GetGameActor());

         BaseClass::BuildPropertyMap();

         static const dtUtil::RefString BASE_ENTITY_GROUP("Base Entity");
         static const dtUtil::RefString DEAD_RECKONING_GROUP("Dead Reckoning");

         typedef dtDAL::PropertyRegHelper<BaseEntityActorProxy&, BaseEntity> PropRegType;
         PropRegType propRegHelper(*this, &e, "Base Entity");


         REGISTER_PROPERTY_WITH_NAME(Frozen, PROPERTY_FROZEN, "Whether or not the simulation of the entity is frozen.", PropRegType, propRegHelper);

         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(FlamesPresent, PROPERTY_FLAMES_PRESENT, "Flames Present",
                  "Should the actor be burning", PropRegType, propRegHelper);

         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(SmokePlumePresent, PROPERTY_SMOKE_PLUME_PRESENT, "Smoke Plume Present",
                  "Enables full entity smoking", PropRegType, propRegHelper);

         dtCore::RefPtr<dtDAL::ResourceActorProperty>  rp = new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Smoke plume particles", "Smoke plume particles",
            dtDAL::ResourceActorProperty::SetFuncType(&e, &BaseEntity::SetSmokePlumesFile),
            "This is the file for the smoke particles", BASE_ENTITY_GROUP);

         dtDAL::ResourceDescriptor rdSmoke("Particles:smoke.osg");
         rp->SetValue(rdSmoke);
         AddProperty(rp.get());

         rp = new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Fire particles", "Fire particles",
            dtDAL::ResourceActorProperty::SetFuncType(&e, &BaseEntity::SetFlamesPresentFile),
            "This is the file for vehicle fire particles", BASE_ENTITY_GROUP);

         dtDAL::ResourceDescriptor rdFire("Particles:fire.osg");
         rp->SetValue(rdFire);
         AddProperty(rp.get());

         static const dtUtil::RefString PROPERTY_DRAWING_MODEL_DESC
            ("Flags if this entity should draw it's model.  This is typically turned off if the entity has the player attached to it.");
         AddProperty(new dtDAL::BooleanActorProperty("DrawingModel", "Draw Model",
            dtDAL::BooleanActorProperty::SetFuncType(&e, &BaseEntity::SetDrawingModel),
            dtDAL::BooleanActorProperty::GetFuncType(&e, &BaseEntity::GetDrawingModel),
            PROPERTY_DRAWING_MODEL_DESC, DEAD_RECKONING_GROUP));

         static const dtUtil::RefString PROPERTY_DAMAGE_STATE_DESC
            ("Changes which model to show based on the level of damage.");
         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::DamageStateEnum>(PROPERTY_DAMAGE_STATE, PROPERTY_DAMAGE_STATE,
            dtDAL::EnumActorProperty<BaseEntityActorProxy::DamageStateEnum>::SetFuncType(&e, &BaseEntity::SetDamageState),
            dtDAL::EnumActorProperty<BaseEntityActorProxy::DamageStateEnum>::GetFuncType(&e, &BaseEntity::GetDamageState),
            PROPERTY_DAMAGE_STATE_DESC, BASE_ENTITY_GROUP));


         static const dtUtil::RefString PROPERTY_MAX_DAMAGE_AMOUNT_DESC
            ("The max damage a local entity can take before dying. Default is 1.0, but is based on damage in the munition config tables. ");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_MAX_DAMAGE_AMOUNT, PROPERTY_MAX_DAMAGE_AMOUNT,
            dtDAL::FloatActorProperty::SetFuncType(&e, &BaseEntity::SetMaxDamageAmount),
            dtDAL::FloatActorProperty::GetFuncType(&e, &BaseEntity::GetMaxDamageAmount),
            PROPERTY_MAX_DAMAGE_AMOUNT_DESC, BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_CUR_DAMAGE_RATIO_DESC
            ("The current damage ratio - used for display purposes. Setting this manually has no effect - it is controlled by the MunitionsComponent.");
         AddProperty(new dtDAL::FloatActorProperty(PROPERTY_CUR_DAMAGE_RATIO, PROPERTY_CUR_DAMAGE_RATIO,
            dtDAL::FloatActorProperty::SetFuncType(&e, &BaseEntity::SetCurDamageRatio),
            dtDAL::FloatActorProperty::GetFuncType(&e, &BaseEntity::GetCurDamageRatio),
            PROPERTY_CUR_DAMAGE_RATIO_DESC, BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::DomainEnum>(
            PROPERTY_DOMAIN, PROPERTY_DOMAIN,
            dtDAL::EnumActorProperty<BaseEntityActorProxy::DomainEnum>::SetFuncType(&e, &BaseEntity::SetDomain),
            dtDAL::EnumActorProperty<BaseEntityActorProxy::DomainEnum>::GetFuncType(&e, &BaseEntity::GetDomain),
            "Specifies the type of environment an entity is specialized in navigating.",
            BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_FORCE_DESC("The force for which the entity is fighting.");
         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>(PROPERTY_FORCE, PROPERTY_FORCE,
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>::SetFuncType(&e, &BaseEntity::SetForceAffiliation),
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ForceEnum>::GetFuncType(&e, &BaseEntity::GetForceAffiliation),
            PROPERTY_FORCE_DESC, BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>("Service", "Service",
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>::SetFuncType(&e, &BaseEntity::SetService),
            dtDAL::EnumActorProperty<BaseEntityActorProxy::ServiceEnum>::GetFuncType(&e, &BaseEntity::GetService),
            "Sets the service of this entity", BASE_ENTITY_GROUP));


         REGISTER_PROPERTY_WITH_NAME(MunitionDamageTableName, "Munition Damage Table",
                  "The name of the munition damage table name found in Configs/MunitionsConfig.xml",
                  PropRegType, propRegHelper);


         static const dtUtil::RefString PROPERTY_DEFAULT_SCALE_DESC
            ("Changes the desired base scale to make the model/geometry "
                  "of this model correct for the rendering.  Model Scale = Default * Magnification");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_DEFAULT_SCALE, PROPERTY_DEFAULT_SCALE,
                  dtDAL::Vec3ActorProperty::SetFuncType(&e, &BaseEntity::SetDefaultScale),
                  dtDAL::Vec3ActorProperty::GetFuncType(&e, &BaseEntity::GetDefaultScale),
                  PROPERTY_DEFAULT_SCALE_DESC,
                  BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_SCALE_MAGNIFICATION_FACTOR_DESC
            ("Changes the amount the geometry of the entity is magnified.  Model Scale = Default * Magnification");
         AddProperty(new dtDAL::Vec3ActorProperty(PROPERTY_SCALE_MAGNIFICATION_FACTOR, PROPERTY_SCALE_MAGNIFICATION_FACTOR,
                  dtDAL::Vec3ActorProperty::SetFuncType(&e, &BaseEntity::SetScaleMagnification),
                  dtDAL::Vec3ActorProperty::GetFuncType(&e, &BaseEntity::GetScaleMagnification),
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
            dtDAL::Vec3ActorProperty::SetFuncType(&e, &BaseEntity::SetModelRotation),
            dtDAL::Vec3ActorProperty::GetFuncType(&e, &BaseEntity::GetModelRotation),
            PROPERTY_MODEL_ROTATION_DESC,
            BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_FIREPOWER_DISABLED("Firepower Disabled");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_FIREPOWER_DISABLED,
            PROPERTY_FIREPOWER_DISABLED,
            dtDAL::BooleanActorProperty::SetFuncType(&e, &BaseEntity::SetFirepowerDisabled),
            dtDAL::BooleanActorProperty::GetFuncType(&e, &BaseEntity::GetFirepowerDisabled),
            "Determines if this entity has had its fire power disabled.", BASE_ENTITY_GROUP));

         static const dtUtil::RefString PROPERTY_MOBILITY_DISABLED("Mobility Disabled");
         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_MOBILITY_DISABLED,
            PROPERTY_MOBILITY_DISABLED,
            dtDAL::BooleanActorProperty::SetFuncType(&e, &BaseEntity::SetMobilityDisabled),
            dtDAL::BooleanActorProperty::GetFuncType(&e, &BaseEntity::GetMobilityDisabled),
            "Determines if this entity has had its mobility disabled.", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_ENTITY_TYPE_ID,
            PROPERTY_ENTITY_TYPE_ID,
            dtDAL::StringActorProperty::SetFuncType(&e, &BaseEntity::SetEntityTypeId),
            dtDAL::StringActorProperty::GetFuncType(&e, &BaseEntity::GetEntityTypeId),
            "String property into which the Entity Type is captured", BASE_ENTITY_GROUP));

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_MAPPING_NAME,
            PROPERTY_MAPPING_NAME,
            dtDAL::StringActorProperty::SetFuncType(&e, &BaseEntity::SetMappingName),
            dtDAL::StringActorProperty::GetFuncType(&e, &BaseEntity::GetMappingName),
            "String property into which the Object Mapping Name is captured", BASE_ENTITY_GROUP));
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntityActorProxy::Init(const dtDAL::ActorType& actorType)
      {
         BaseClass::Init(actorType);
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
         // We don't use remote ticks
         //else
         //{
             //RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
             // Turn this on to print out debug info in ProcessMessage();
             //RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_ACTOR_UPDATED, dtGame::GameActorProxy::PROCESS_MSG_INVOKABLE);
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
      void BaseEntityActorProxy::NotifyFullActorUpdate()
      {
         // Remove the rot and trans from the full actor update.
         // If we send pos & rot out in an update, then that sometimes causes problems
         // on remote items. Network components usually pick up their data on tick local, which
         // sends a message to the DefaultmessageProcessorComponent. However, before that message
         // gets processed, the tick-remote gets picked up by the DeadReckoningComponent. Causes jumpiness.
         std::vector<dtDAL::ActorProperty* > allProperties;
         GetPropertyList(allProperties);

         std::vector<dtUtil::RefString> finalPropNameList;
         finalPropNameList.reserve(allProperties.size());

         for (size_t i = 0; i < allProperties.size(); ++i)
         {
            if (allProperties[i]->GetName() != dtDAL::TransformableActorProxy::PROPERTY_ROTATION &&
               allProperties[i]->GetName() != dtDAL::TransformableActorProxy::PROPERTY_TRANSLATION)
            {
               finalPropNameList.push_back(allProperties[i]->GetName());
            }
         }

         NotifyPartialActorUpdate(finalPropNameList);
      }

      /////////////////////////////////////////////////////////////////////
      ///////////////   BaseEntity                     ////////////////////
      /////////////////////////////////////////////////////////////////////

      BaseEntity::BaseEntity(dtGame::GameActorProxy& proxy)
         : IGActor(proxy)
         , mMaxDamageAmount(1.0f)
         , mCurDamageRatio(0.0f)
         , mDomain(&BaseEntityActorProxy::DomainEnum::GROUND)
         , mForceAffiliation(&BaseEntityActorProxy::ForceEnum::NEUTRAL)
         , mService(&BaseEntityActorProxy::ServiceEnum::MARINES)
         , mSmokePlumePresent(false)
         , mFlamesPresent(false)
         , mDrawingModel(true)
         , mPlayerAttached(false)
         , mMobilityDisabled(false)
         , mFirepowerDisabled(false)
         , mFrozen(false)
         , mAutoRegisterWithMunitionsComponent(true)
         , mAutoRegisterWithDeadReckoningComponent(true)
         , mScaleMatrixNode(new osg::MatrixTransform)
         , mDeadReckoningHelper(NULL)
         , mDamageState(&BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
         , mDefaultScale(1.0f, 1.0f, 1.0f)
         , mScaleMagnification(1.0f, 1.0f, 1.0f)
         , mFireLightID(0)
      {
         mLogger = &dtUtil::Log::GetInstance("BaseEntity.cpp");
         osg::Group* g = GetOSGNode()->asGroup();
         g->addChild(mScaleMatrixNode.get());
         mScaleMatrixNode->setName("mScaleMatrixNode");

         // temp turned off to test performance.
         SetCollisionDetection(false);
      }

      /////////////////////////////////////////////////////////////////////
      BaseEntity::~BaseEntity()
      {
         SetFlamesPresent(false);
      }

      IMPLEMENT_PROPERTY(BaseEntity, float, MaxDamageAmount);
      IMPLEMENT_PROPERTY(BaseEntity, float, CurDamageRatio);
      IMPLEMENT_PROPERTY(BaseEntity, std::string, MappingName);
      IMPLEMENT_PROPERTY(BaseEntity, std::string, EntityTypeId);
      IMPLEMENT_PROPERTY(BaseEntity, dtUtil::EnumerationPointer<BaseEntityActorProxy::DomainEnum>, Domain);
      IMPLEMENT_PROPERTY(BaseEntity, dtUtil::EnumerationPointer<BaseEntityActorProxy::ForceEnum>, ForceAffiliation);
      IMPLEMENT_PROPERTY(BaseEntity, dtUtil::EnumerationPointer<BaseEntityActorProxy::ServiceEnum>, Service);
      IMPLEMENT_PROPERTY_GETTER(BaseEntity, bool, SmokePlumePresent);
      IMPLEMENT_PROPERTY_GETTER(BaseEntity, bool, FlamesPresent);
      IMPLEMENT_PROPERTY_GETTER(BaseEntity, bool, DrawingModel);
      IMPLEMENT_PROPERTY(BaseEntity, bool, PlayerAttached);
      IMPLEMENT_PROPERTY(BaseEntity, bool, MobilityDisabled);
      IMPLEMENT_PROPERTY(BaseEntity, bool, FirepowerDisabled);
      IMPLEMENT_PROPERTY(BaseEntity, bool, Frozen);
      IMPLEMENT_PROPERTY(BaseEntity, bool, AutoRegisterWithMunitionsComponent);
      IMPLEMENT_PROPERTY(BaseEntity, bool, AutoRegisterWithDeadReckoningComponent);
      IMPLEMENT_PROPERTY(BaseEntity, std::string, MunitionDamageTableName);

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDrawingModel(bool newDrawing)
      {
         mDrawingModel = newDrawing;
         SetNodeVisible(mDrawingModel, GetScaleMatrixTransform());
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();

         // DEAD RECKONING - ACT COMPONENT
         if (!HasComponent(dtGame::DeadReckoningHelper::TYPE)) // not added by a subclass
         {
            mDeadReckoningHelper = new dtGame::DeadReckoningHelper();

            // Flying was replaced with GroundClampType, and the default is already 'KeepAbove'
            //////mDeadReckoningHelper->SetFlying(false); // Causes ground clamping by default
            //mDeadReckoningHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::KEEP_ABOVE);

            // attempt to fix the z-fighting on treads and wheels that are
            // very close to the ground. We move the vehicle up about 3-4 inches...
            mDeadReckoningHelper->SetGroundOffset(0.09);

            AddComponent(*mDeadReckoningHelper);
         }
         //else
         //{
            //GetComponent(mDeadReckoningHelper->GetType());
         //}


         // DEAD RECKONING - PUBLISHING ACTOR COMPONENT
         if (!HasComponent(DRPublishingActComp::TYPE)) // not added by a subclass
         {
            mDRPublishingActComp = new DRPublishingActComp();
            AddComponent(*mDRPublishingActComp);  // Add AFTER the DRhelper.
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::OnEnteredWorld()
      {
         GetOSGNode()->setName(GetName());

         if (!IsRemote())
         {
            dtCore::Transform xform;
            GetTransform(xform);
            osg::Vec3 pos;
            xform.GetTranslation(pos);
            GetDeadReckoningHelper().SetLastKnownTranslation(pos);
            osg::Vec3 rot;
            xform.GetRotation(rot);
            GetDeadReckoningHelper().SetLastKnownRotation(rot);

            // Local entities usually need the ability to take damage. So, register with the munitions component.
            if (mAutoRegisterWithMunitionsComponent)
            {
               SimCore::Components::MunitionsComponent* munitionsComp;
               GetGameActorProxy().GetGameManager()->GetComponentByName
                  (SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);
               if (munitionsComp != NULL && !munitionsComp->HasRegistered(GetUniqueId()))
               {
                  // Changed to the second parameter to false because the entity does the update itself now.
                  munitionsComp->Register(*this, false, GetMaxDamageAmount());
               }
            }
         }

         if (mAutoRegisterWithDeadReckoningComponent)
         {
            RegisterWithDeadReckoningComponent();
         }

         //////////////////////////////
         // DR CONFIGURATION OPTIONS
         dtUtil::ConfigProperties& configParams = GetGameActorProxy().GetGameManager()->GetConfiguration();

         // Use Cubic Splines (vs the older Linear Blend) - If not specified, don't override default
         std::string useCubicSplines = configParams.GetConfigPropertyValue("SimCore.DR.UseCubicSpline", "");
         if (useCubicSplines == "true" || useCubicSplines == "TRUE" || useCubicSplines == "1")
         {
            GetDeadReckoningHelper().SetUseCubicSplineTransBlend(true);
         }
         else if (useCubicSplines == "false" || useCubicSplines == "FALSE" || useCubicSplines == "0")
         {
            GetDeadReckoningHelper().SetUseCubicSplineTransBlend(false);
         }

         // Always Use Max Smoothing Time (as opposed to averaged update rate)
         // Some systems publish regularly, and some don't. If a system doesn't
         // publish updates like clockwork, then we use the average publish rate to blend. 
         std::string useFixedTimeBlends = configParams.GetConfigPropertyValue("SimCore.DR.UseFixedTimeBlends", "");
         if (useFixedTimeBlends == "true" || useFixedTimeBlends == "TRUE" || useFixedTimeBlends == "1")
         {
            GetDeadReckoningHelper().SetUseFixedSmoothingTime(true);
         }
         else if (useFixedTimeBlends == "false" || useFixedTimeBlends == "FALSE" || useFixedTimeBlends == "0")
         {
            GetDeadReckoningHelper().SetUseFixedSmoothingTime(false);
         }

         // The MaxTransSmoothingTime is usually set, but there are very obscure cases where it might
         // not have been set or not published for some reason. In that case, we need a non-zero value.
         // In practice, a vehicle that publishes will typically set these directly (for example, see 
         // BasePhysicsVehicleActor.SetMaxUpdateSendRate()). 
         // Previously, it set the smoothing time to 0.0 so that local actors would not smooth
         // their DR pos & rot to potentially make a cleaner comparison with less publishes.
         // Turning local smoothing on allows better vis & debugging of DR values (ex the DRGhostActor).
         if (GetDeadReckoningHelper().GetMaxTranslationSmoothingTime() == 0.0f)
            GetDeadReckoningHelper().SetMaxTranslationSmoothingTime(0.5f);
         if (GetDeadReckoningHelper().GetMaxRotationSmoothingTime() == 0.0f)
            GetDeadReckoningHelper().SetMaxRotationSmoothingTime(0.5f);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::OnRemovedFromWorld()
      {
         SetFlamesPresent(false);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      BaseEntityActorProxy::DamageStateEnum& BaseEntity::GetDamageState() const
      {
         return *mDamageState;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetDamageState(BaseEntityActorProxy::DamageStateEnum& damageState)
      {
         if (mDamageState == &damageState)
            return;

         mDamageState = &damageState;

         CauseFullUpdate();
      }


      ////////////////////////////////////////////////////////////////////////////////////
      DRPublishingActComp* BaseEntity::GetDRPublishingActComp()
      {
         return mDRPublishingActComp.get();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::GetBoundingSphere(osg::Vec3& center, float& radius)
      {
         const osg::BoundingSphere& boundingSphere = GetScaleMatrixTransform().getBound();
         radius = boundingSphere._radius;

         center = boundingSphere._center * GetMatrix();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::BoundingBox BaseEntity::GetBoundingBox()
      {
         osg::Node& topNode = GetScaleMatrixTransform();

         osg::ComputeBoundsVisitor cbv;
         topNode.accept(cbv);

         const osg::Matrix& mat = GetMatrix();

         osg::BoundingBox bb = cbv.getBoundingBox();
         osg::BoundingBox newBB;
         for (unsigned i = 0; i < 8U; ++i)
         {
            newBB.expandBy(bb.corner(i) * mat);
         }
         return newBB;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetFlamesPresent(bool enable)
      {
         if (mFlamesPresent == enable)
            return;

         if (enable)
         {
            if (!mFlamesSystem.valid())
               mFlamesSystem = new dtCore::ParticleSystem;

            mFlamesSystem->LoadFile(mFlamesSystemFile, true);
            mFlamesSystem->SetEnabled(enable);
            AddChild(mFlamesSystem.get());

            Components::ParticleInfoAttributeFlags attrs = {true,true};
            RegisterParticleSystem(*mFlamesSystem,&attrs);

            if (mFireLightID == 0 && GetGameActorProxy().GetGameManager() != NULL)
            {
               // HACK: Add lights with copied code
               SimCore::Components::RenderingSupportComponent* renderComp;
               GetGameActorProxy().GetGameManager()->GetComponentByName(
                  SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

               if (renderComp != NULL)
               {
                  SimCore::Components::RenderingSupportComponent::DynamicLight* dl =
                     renderComp->AddDynamicLightByPrototypeName("Light-Entity-Flames");
                  dl->mTarget = this;
                  dl->mAutoDeleteLightOnTargetNull = true;
                  mFireLightID = dl->GetId();
               }
            }
         }
         else
         {
            if (mFlamesSystem.get())
            {
               UnregisterParticleSystem(*mFlamesSystem);
               RemoveChild(mFlamesSystem.get());
               mFlamesSystem = NULL;
            }
            if (mFireLightID != 0 && GetGameActorProxy().GetGameManager() != NULL)
            {
               // HACK: Remove the fire light since a NULL target does not remove it.
               SimCore::Components::RenderingSupportComponent* renderComp;
               GetGameActorProxy().GetGameManager()->GetComponentByName(
                  SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

               if (renderComp != NULL)
               {
                  renderComp->RemoveDynamicLight(mFireLightID);
               }
               mFireLightID = 0;
            }
         }
         mFlamesPresent = enable;

         CauseFullUpdate();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::SetSmokePlumePresent(bool enable)
      {
         if (mSmokePlumePresent == enable)
            return;

         if (enable)
         {
            if (!mSmokePlumesSystem.valid())
               mSmokePlumesSystem = new dtCore::ParticleSystem;

            mSmokePlumesSystem->LoadFile(mSmokePlumesSystemFile, true);
            mSmokePlumesSystem->SetEnabled(enable);
            AddChild(mSmokePlumesSystem.get());

            Components::ParticleInfoAttributeFlags attrs = {true,true};
            RegisterParticleSystem(*mSmokePlumesSystem,&attrs);
         }
         else
         {
            if (mSmokePlumesSystem.valid())
            {
               UnregisterParticleSystem(*mSmokePlumesSystem);
               RemoveChild(mSmokePlumesSystem.get());
               mSmokePlumesSystem = NULL;
            }
         }
         mSmokePlumePresent = enable;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::RegisterWithDeadReckoningComponent()
      {
         dtGame::DeadReckoningComponent* drc = NULL;

         GetGameActorProxy().GetGameManager()->
            GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drc);

         // We have to have a DR helper to add the component
         if (!IsDeadReckoningHelperValid())
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Actor \"%s\"\"%s\" does not have a DeadReckoningHelper.",
               GetName().c_str(), GetUniqueId().ToString().c_str());

            return;
         }

         if (drc != NULL)
         {
            drc->RegisterActor(GetGameActorProxy(), GetDeadReckoningHelper());
         }
         else
         {
            mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,
               "Actor \"%s\"\"%s\" unable to find DeadReckoningComponent.",
               GetName().c_str(), GetUniqueId().ToString().c_str());
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::ProcessMessage(const dtGame::Message& message)
      {
      }


      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {

         GameActor::OnTickLocal(tickMessage);

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

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::RespondToHit(const DetonationMessage& message,
         const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force,
         const osg::Vec3& location)
      {
         // An opportunity to respond to damage. Only called on local entities that have been
         // damaged by a munition hit of some sort. Damage has already been
         // applied and published by the time this method is called. We still need to
         // apply physics forces though in case it's a physically modeled entity.

         // check mCurDamageRatio if you need to know how damaged you are.

         if (force.length2() > 0.0f)
         {
            // Apply an instantaneous impulse force to the entity
            ApplyForce(force, location, true);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool BaseEntity::ShouldBeVisible(const SimCore::VisibilityOptions& options)
      {
         const BasicVisibilityOptions& basicOptions = options.GetBasicOptions();

         bool forceIsVisible = basicOptions.IsEnumVisible(GetForceAffiliation());

         bool domainIsVisible = basicOptions.IsEnumVisible(GetDomain());

         return forceIsVisible && domainIsVisible;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void BaseEntity::CauseFullUpdate()
      {
         if (!IsRemote() && GetDRPublishingActComp() != NULL && GetGameActorProxy().IsInGM())
         {
            GetDRPublishingActComp()->ForceFullUpdateAtNextOpportunity();
         }
      }

   }
}
