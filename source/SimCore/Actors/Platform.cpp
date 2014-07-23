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
* @author David Guthrie
* @author Eddie Johnson
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>
#include <string>
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <SimCore/ActComps/WeaponInventoryActComp.h>
#include <SimCore/ActComps/WheelActComp.h>
#include <SimCore/ActComps/PlatformDefaultPhysicsActComp.h>
#include <SimCore/ActComps/TrailEffectActComp.h>
#include <SimCore/Actors/Platform.h>

// Included to allow code to select which actor components to add.
#include <SimCore/Actors/EntityActorRegistry.h>

#include <SimCore/CollisionGroupEnum.h>

#include <dtPhysics/physicsactcomp.h>

#include <dtGame/drpublishingactcomp.h>
#include <SimCore/Components/DefaultArticulationHelper.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/VisibilityOptions.h>

#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/invokable.h>
#include <dtGame/gameactor.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messageparameter.h>

#include <dtUtil/tangentspacevisitor.h>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderparamfloat.h>
#include <dtCore/shadermanager.h>
#include <dtCore/particlesystem.h>
#include <dtUtil/nodecollector.h>
#include <dtCore/camera.h>

#include <dtCore/enginepropertytypes.h>
#include <dtCore/groupactorproperty.h>
#include <dtCore/actorproxyicon.h>
#include <dtCore/actortype.h>
#include <dtCore/namedparameter.h>
#include <dtCore/functor.h> // deprecated
#include <dtCore/propertymacros.h>
#include <dtCore/resourcedescriptor.h>
#include <dtCore/project.h> // to lookup resource.

#include <dtUtil/boundingshapeutils.h>
#include <dtUtil/log.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/configproperties.h>

#include <dtAudio/sound.h>
#include <dtAudio/audiomanager.h>

#include <dtABC/application.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Switch>
#include <osg/MatrixTransform>
#include <osg/Point>
#include <osg/NodeVisitor>
#include <osg/Program>
#include <osgSim/DOFTransform>

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString Platform::INVOKABLE_TICK_CONTROL_STATE("TickControlState");

      ////////////////////////////////////////////////////////////////////////////////////
      // Actor Proxy code
      ////////////////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED("Head Lights Enabled");
      const dtUtil::RefString PlatformActorProxy::PROPERTY_MESH_NON_DAMAGED_ACTOR("Non-damaged actor");
      const dtUtil::RefString PlatformActorProxy::PROPERTY_MESH_DAMAGED_ACTOR("Damaged actor");
      const dtUtil::RefString PlatformActorProxy::PROPERTY_MESH_DESTROYED_ACTOR("Destroyed actor");
      const dtUtil::RefString PlatformActorProxy::PROPERTY_ENGINE_POSITION("Engine Position");
      const dtUtil::RefString PlatformActorProxy::PROPERTY_ENGINE_SMOKE_POSITION("EngineSmokePosition");
      const dtUtil::RefString PlatformActorProxy::PROPERTY_ENGINE_SMOKE_ON("EngineSmokeOn");

      bool PlatformActorProxy::mPhysicsCreationEnabled(true);

      ////////////////////////////////////////////////////////////////////////////////////
      PlatformActorProxy::PlatformActorProxy()
      : mHasLoadedResources(false)
      {
         SetClassName("SimCore::Actors::Platform");
      }

      ////////////////////////////////////////////////////////////////////////////////////
      PlatformActorProxy::~PlatformActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::SetPhysicsCreationEnabled(bool newValue)
      {
         mPhysicsCreationEnabled = newValue;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool PlatformActorProxy::GetPhysicsCreationEnabled()
      {
         return mPhysicsCreationEnabled;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::CreateDrawable()
      {
         Platform* pEntity = new Platform(*this);
         SetDrawable(*pEntity);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(PlatformActorProxy, dtCore::ResourceDescriptor, NonDamagedResource);
      void PlatformActorProxy::SetNonDamagedResource(const dtCore::ResourceDescriptor& rd)
      {
         mNonDamagedResource = rd;
         if (GetHasLoadedResources() || IsInSTAGE())
         {
            Platform* plat = NULL;
            GetDrawable(plat);
            plat->LoadDamageableFile(rd,  PlatformActorProxy::DamageStateEnum::NO_DAMAGE);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(PlatformActorProxy, dtCore::ResourceDescriptor, DamagedResource);
      void PlatformActorProxy::SetDamagedResource(const dtCore::ResourceDescriptor& rd)
      {
         mDamagedResource = rd;
         if (GetHasLoadedResources() || IsInSTAGE())
         {
            Platform* plat = NULL;
            GetDrawable(plat);
            plat->LoadDamageableFile(rd,  PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(PlatformActorProxy, dtCore::ResourceDescriptor, DestroyedResource);
      void PlatformActorProxy::SetDestroyedResource(const dtCore::ResourceDescriptor& rd)
      {
         mDestroyedResource = rd;
         if (GetHasLoadedResources() || IsInSTAGE())
         {
            Platform* plat = NULL;
            GetDrawable(plat);
            plat->LoadDamageableFile(rd,  PlatformActorProxy::DamageStateEnum::DESTROYED);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::GetPartialUpdateProperties(std::vector<dtUtil::RefString>& propNamesToFill)
      {
         BaseClass::GetPartialUpdateProperties(propNamesToFill);

         Platform* platform;
         GetDrawable(platform);

         if (platform->GetArticulationHelper() != NULL && platform->GetArticulationHelper()->IsDirty() )
         {
            propNamesToFill.push_back(platform->GetArticulationHelper()->GetArticulationArrayPropertyName());
            platform->GetArticulationHelper()->SetDirty(false);
         }
      }
      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::BuildPropertyMap()
      {
         Platform& plat = static_cast<Platform&>(GetGameActor());

         typedef dtCore::PropertyRegHelper<PlatformActorProxy&, Platform> PropRegType;
         PropRegType propRegHelper(*this, &plat, "Platform");

         BaseClass::BuildPropertyMap();

         AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::STATIC_MESH,
               PROPERTY_MESH_NON_DAMAGED_ACTOR,
               PROPERTY_MESH_NON_DAMAGED_ACTOR,
               dtCore::ResourceActorProperty::SetDescFuncType(this, &PlatformActorProxy::SetNonDamagedResource),
               dtCore::ResourceActorProperty::GetDescFuncType(this, &PlatformActorProxy::GetNonDamagedResource),
               "This is the model for a non damaged actor"));

         AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::STATIC_MESH,
               PROPERTY_MESH_DAMAGED_ACTOR,
               PROPERTY_MESH_DAMAGED_ACTOR,
               dtCore::ResourceActorProperty::SetDescFuncType(this, &PlatformActorProxy::SetDamagedResource),
               dtCore::ResourceActorProperty::GetDescFuncType(this, &PlatformActorProxy::GetDamagedResource),
               "This is the model for a damaged actor"));

         AddProperty(new dtCore::ResourceActorProperty(dtCore::DataType::STATIC_MESH,
               PROPERTY_MESH_DESTROYED_ACTOR,
               PROPERTY_MESH_DESTROYED_ACTOR,
               dtCore::ResourceActorProperty::SetDescFuncType(this, &PlatformActorProxy::SetDestroyedResource),
               dtCore::ResourceActorProperty::GetDescFuncType(this, &PlatformActorProxy::GetDestroyedResource),
               "This is the model for a destroyed actor"));

         static const dtUtil::RefString PROPERTY_MUZZLE_FLASH_POSITION("Muzzle Flash Position");
         AddProperty(new dtCore::Vec3ActorProperty(PROPERTY_MUZZLE_FLASH_POSITION,
            PROPERTY_MUZZLE_FLASH_POSITION,
            dtCore::Vec3ActorProperty::SetFuncType(&plat, &Platform::SetMuzzleFlashPosition),
            dtCore::Vec3ActorProperty::GetFuncType(&plat, &Platform::GetMuzzleFlashPosition),
            "Sets the muzzle flash position on an entity"));

         static const dtUtil::RefString PROPERTY_ARTICULATION_PARAM_ARRAY("Articulated Parameters Array");
         AddProperty(new dtCore::GroupActorProperty(PROPERTY_ARTICULATION_PARAM_ARRAY,
            PROPERTY_ARTICULATION_PARAM_ARRAY,
            dtCore::GroupActorProperty::SetFuncType(&plat, &Platform::SetArticulatedParametersArray),
            dtCore::GroupActorProperty::GetFuncType(&plat, &Platform::GetArticulatedParametersArray),
            "The list of articulated parameters for modifying DOF's", ""));

         AddProperty(new dtCore::BooleanActorProperty(PROPERTY_HEAD_LIGHTS_ENABLED,
            PROPERTY_HEAD_LIGHTS_ENABLED,
            dtCore::BooleanActorProperty::SetFuncType(&plat, &Platform::SetHeadLightsEnabled),
            dtCore::BooleanActorProperty::GetFuncType(&plat, &Platform::IsHeadLightsEnabled),
            "Determines if the entity has it head lights on or not."));

         static const dtUtil::RefString PROPERTY_SEAT_CONFIG_TABLE_NAME("VehiclesSeatConfigActorNameTable");
         AddProperty(new dtCore::StringActorProperty(PROPERTY_SEAT_CONFIG_TABLE_NAME,
            PROPERTY_SEAT_CONFIG_TABLE_NAME,
            dtCore::StringActorProperty::SetFuncType(&plat, &Platform::SetVehiclesSeatConfigActorName),
            dtCore::StringActorProperty::GetFuncType(&plat, &Platform::GetVehiclesSeatConfigActorName),
            "The Vehicle seat config option to coincide with the use of portals.",""));

         static const dtUtil::RefString PROPERTY_ENTITY_TYPE("EntityType");
         AddProperty(new dtCore::StringActorProperty(PROPERTY_ENTITY_TYPE,
            PROPERTY_ENTITY_TYPE,
            dtCore::StringActorProperty::SetFuncType(&plat, &Platform::SetEntityType),
            dtCore::StringActorProperty::GetFuncType(&plat, &Platform::GetEntityType),
            "The type of the entity, such as HMMWVDrivingSim. Used to determine what behaviors this entity can have at runtime, such as embark, gunner, commander, ...", ""));

         static const dtUtil::RefString SOUND_PROPERTY_TYPE("Sounds");

         AddProperty(new dtCore::FloatActorProperty("MaxDistanceIdleSound", "MaxDistanceIdleSound",
            dtCore::FloatActorProperty::SetFuncType(&plat, &Platform::SetMaxDistanceIdleSound),
            dtCore::FloatActorProperty::GetFuncType(&plat, &Platform::GetMaxDistanceIdleSound),
            "Distance for the sound", SOUND_PROPERTY_TYPE));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND,
            "mSFXSoundIdleEffect", "mSFXSoundIdleEffect",
            dtCore::ResourceActorProperty::SetFuncType(&plat, &Platform::SetSFXEngineIdleLoop),
            "What is the filepath / string of the sound effect", SOUND_PROPERTY_TYPE));

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(EngineSmokePos, PROPERTY_ENGINE_SMOKE_POSITION, "Engine Smoke Position",
                  "Sets the engine smoke position of this BaseEntity", PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(EngineSmokeOn, PROPERTY_ENGINE_SMOKE_ON, "Engine Smoke On",
                  "Enables engine smoke", PropRegType, propRegHelper);
         static const dtUtil::RefString PROPERTY_ENGINE_POSITION_DESC("Position of the engine in the vehicle");
         dtCore::Vec3ActorProperty *prop = new dtCore::Vec3ActorProperty(PROPERTY_ENGINE_POSITION, PROPERTY_ENGINE_POSITION,
                  dtCore::Vec3ActorProperty::SetFuncType(),
                  dtCore::Vec3ActorProperty::GetFuncType(&plat, &Platform::GetEngineSmokePos),
                  PROPERTY_ENGINE_POSITION_DESC, "Platform");

         prop->SetReadOnly(true);
         AddProperty(prop);

         dtCore::RefPtr<dtCore::ResourceActorProperty> rp = new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM,
            "Engine smoke particles", "Engine smoke particles",
            dtCore::ResourceActorProperty::SetFuncType(&plat, &Platform::SetEngineSmokeFile),
            "This is the file for engine smoke particles", "Platform");

         dtCore::ResourceDescriptor rdEngine("Particles:smoke.osg");
         rp->SetValue(rdEngine);
         AddProperty(rp.get());
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();

         Platform* actor = static_cast<Platform*>(GetDrawable());

         AddInvokable(*new dtGame::Invokable(Platform::INVOKABLE_TICK_CONTROL_STATE,
            dtUtil::MakeFunctor(&Platform::TickControlState, actor)));

         AddInvokable(*new dtGame::Invokable("TimeElapsedForSoundIdle",
            dtUtil::MakeFunctor(&Platform::TickTimerMessage, actor)));
      }

      /////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::BuildActorComponents()
      {
         const dtCore::ActorType& at = GetActorType();
         if (at.InstanceOf(*EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE))
         {
            if (!HasComponent(SimCore::ActComps::WheelActComp::TYPE))
            {
               AddComponent(*new SimCore::ActComps::WheelActComp());
            }
         }

         BaseClass::BuildActorComponents();

         if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
         {
            if (at.InstanceOf(*EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE) ||
                    ( at.InstanceOf(*EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE) && mPhysicsCreationEnabled ) )
            {
               dtCore::RefPtr<dtPhysics::PhysicsActComp> physActComp = new SimCore::ActComps::PlatformDefaultPhysicsActComp();
               AddComponent(*physActComp);
            }
         }

         SimCore::Actors::Platform* p = NULL;
         GetDrawable(p);

         if (at.InstanceOf(*EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE))
         {
            dtGame::DeadReckoningHelper* drHelper = NULL;
            GetComponent(drHelper);
            drHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::NONE);
            p->SetDomain(BaseEntityActorProxy::DomainEnum::AIR);
         }
         else if (at.InstanceOf(*EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE))
         {
            p->SetDomain(BaseEntityActorProxy::DomainEnum::GROUND);
         }

         if (at.InstanceOf(*EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE) ||
                  at.InstanceOf(*EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE))
         {
            // Setup the body paint component.
            AddComponent(*new SimCore::ActComps::CamoPaintStateActComp);
            if (!HasComponent(SimCore::ActComps::WeaponInventoryActComp::TYPE))
            {
               AddComponent(*new SimCore::ActComps::WeaponInventoryActComp());
            }
         }
         else
         {
            p->SetAutoRegisterWithMunitionsComponent(false);
         }

         // Add Trail Effect properties if none already exist.
         if (at.InstanceOf(*EntityActorRegistry::HELO_PLATFORM_ACTOR_TYPE)
            || at.InstanceOf(*EntityActorRegistry::MILITARY_HELO_PLATFORM_ACTOR_TYPE))
         {
            if (!HasComponent(SimCore::ActComps::TrailEffectActComp::TYPE))
            {
               AddComponent(*new SimCore::ActComps::TrailEffectActComp());
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtCore::ActorProxyIcon* PlatformActorProxy::GetBillboardIcon()
      {
         if (!mBillboardIcon.valid())
         {
            mBillboardIcon = new dtCore::ActorProxyIcon(dtCore::ActorProxyIcon::IMAGE_BILLBOARD_STATICMESH);
         }

         return mBillboardIcon.get();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::EnsureResourcesAreLoaded()
      {
         if (!mHasLoadedResources)
         {
            Platform* plat = NULL;
            GetDrawable(plat);

            plat->LoadDamageableFile(mNonDamagedResource,  PlatformActorProxy::DamageStateEnum::NO_DAMAGE);
            plat->LoadDamageableFile(mDamagedResource,  PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE);
            plat->LoadDamageableFile(mDestroyedResource,  PlatformActorProxy::DamageStateEnum::DESTROYED);
            mHasLoadedResources = true;
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool PlatformActorProxy::GetHasLoadedResources() const
      {
         return mHasLoadedResources;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void PlatformActorProxy::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();
         mHasLoadedResources = false;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      // Actor code
      ////////////////////////////////////////////////////////////////////////////////////
      const std::string Platform::DOF_NAME_HEAD_LIGHTS("headlight_01");

      ////////////////////////////////////////////////////////////////////////////////////
      Platform::Platform(dtGame::GameActorProxy& owner)
      : BaseEntity(owner)
      , mEngineSmokeOn(false)
      , mTimeBetweenControlStateUpdates(0.3333f)
      , mTimeUntilControlStateUpdate(0.0f)
      , mNonDamagedFileNode(new osg::Group())
      , mDamagedFileNode(new osg::Group())
      , mDestroyedFileNode(new osg::Group())
      , mSwitchNode(new osg::Switch)
      , mHeadLightsEnabled(false)
      , mHeadLightID(0)
      , mMinIdleSoundDistance(5.0f)
      , mMaxIdleSoundDistance(30.0f)
      {
         mSwitchNode->insertChild(0, mNonDamagedFileNode.get());
         mSwitchNode->insertChild(1, mDamagedFileNode.get());
         mSwitchNode->insertChild(2, mDestroyedFileNode.get());

         GetScaleMatrixTransform().addChild(mSwitchNode.get());

         SetDrawingModel(true);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      Platform::~Platform()
      {
         if(mSndIdleLoop.valid())
         {
            mSndIdleLoop->Stop();
            RemoveChild(mSndIdleLoop.get());
            mSndIdleLoop.release();
         }
      }

      DT_IMPLEMENT_ACCESSOR(Platform, osg::Vec3, EngineSmokePos);
      DT_IMPLEMENT_ACCESSOR_GETTER(Platform, bool, EngineSmokeOn);

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetEngineSmokeOn(bool enable)
      {
         if (mEngineSmokeOn == enable)
         {
            return;
         }

         if (enable)
         {
            if (!mEngineSmokeSystem.valid())
            {
               mEngineSmokeSystem = new dtCore::ParticleSystem;
            }

            mEngineSmokeSystem->LoadFile(mEngineSmokeSystemFile, true);
            dtCore::Transform xform;
            xform.MakeScale(osg::Vec3d(0.25, 0.24, 0.25));
            xform.SetTranslation(mEngineSmokePos);

            mEngineSmokeSystem->SetTransform(xform, dtCore::Transformable::REL_CS);
            mEngineSmokeSystem->SetEnabled(enable);
            AddChild(mEngineSmokeSystem);
         }
         else
         {
            if (mEngineSmokeSystem.valid())
            {
               RemoveChild(mEngineSmokeSystem);
               mEngineSmokeSystem = NULL;
            }
         }
         mEngineSmokeOn = enable;

         CauseFullUpdate();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetHeadLightsEnabled( bool enabled )
      {
         // We don't want to create the headlight if it's not necessary
         // Still want to make sure it forces a light turn on if you set it to true, so we only
         // quick exit for false
         if (!mHeadLightsEnabled && !enabled)
         {
            return;
         }
         // Attempt to capture the headlight transformable that is tracked by the
         // light effect for world position information.
         dtCore::RefPtr<dtCore::Transformable> headLightPoint;
         dtCore::DeltaDrawable* curChild = NULL;
         unsigned limit = GetNumChildren();
         for( unsigned i = 0; i < limit; ++i )
         {
            curChild = GetChild( i );
            if( curChild != NULL && curChild->GetName() == DOF_NAME_HEAD_LIGHTS )
            {
               headLightPoint = dynamic_cast<dtCore::Transformable*>(curChild);
               break;
            }
         }

         // If the head light point was not found then try to create it.
         if( ! headLightPoint.valid() )
         {
            // ...add a Delta transformable at the same point as the DOF so that the
            // dynamic light effect can track the position; it does not track OSG DOF transforms.
            headLightPoint = new dtCore::Transformable( DOF_NAME_HEAD_LIGHTS );
            bool foundLightNode = AddChild( headLightPoint.get(), DOF_NAME_HEAD_LIGHTS);
            // if you don't have a headlight node, then it will be added at the origin, so offset the light
            // with a fixed value.
            if (!foundLightNode)
            {
               dtCore::Transform xform;
               osg::Vec3 lightOffset(0.0f, 1.0f, 0.0f);
               xform.SetTranslation(lightOffset);
               headLightPoint->SetTransform( xform, dtCore::Transformable::REL_CS );
            }
         }

         // If the light point has been created or accessed...
         if( headLightPoint.valid() )
         {
            if (mHeadLightsEnabled != enabled)
            {
               CauseFullUpdate();
            }
            mHeadLightsEnabled = enabled;

            // ...get the game manager...
            dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
            if( gm == NULL )
            {
               return;
            }

            // ...so that the render support component can be accessed...
            SimCore::Components::RenderingSupportComponent* rsComp = NULL;

            if( enabled )
            {
               gm->GetComponentByName( SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsComp);

               SimCore::Components::RenderingSupportComponent::DynamicLight* dl = NULL;
               if (rsComp != NULL)
               {
                  // ...so that the head light effect can be accessed...
                  dl = rsComp->GetDynamicLight( mHeadLightID );
               }

               // ...and if the light effect does not exist...
               if( dl == NULL && rsComp != NULL)
               {
                  // ...create it and get its ID to be tracked.
                  //
                  // NOTE: The following string value for the light name should eventually be
                  //       property, but is not currently for the sake of time.
                  dl = rsComp->AddSpotLightByPrototypeName( "Light-Headlight" );
                  dl->mTarget = headLightPoint.get();
                  mHeadLightID = dl->GetId();
               }
            }
            else if (mHeadLightID != 0)
            {
               gm->GetComponentByName( SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsComp);

               if (rsComp != NULL)
               {
                  // ...turn it off by removing it.
                  rsComp->RemoveDynamicLight( mHeadLightID );
               }
               mHeadLightID = 0;
            }

         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool Platform::IsHeadLightsEnabled() const
      {
         bool result = mHeadLightsEnabled;
         if (mHeadLightsEnabled)
         {
            const dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
            if (gm != NULL)
            {
               if (mHeadLightID != 0)
               {
                  const SimCore::Components::RenderingSupportComponent* rsComp = NULL;
                  gm->GetComponentByName( SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, rsComp);

                  // Check to see if the light exists, in the world.
                  result = rsComp->HasLight( mHeadLightID );
               }
               else
               {
                  result = false;
               }
            }
         }
         return result;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      static osg::Vec3 ComputeDimensions( osg::Node& node )
      {
         dtUtil::BoundingBoxVisitor bbv;
         node.accept(bbv);

         osg::Vec3 modelDimensions;
         modelDimensions.x() = bbv.mBoundingBox.xMax() - bbv.mBoundingBox.xMin();
         modelDimensions.y() = bbv.mBoundingBox.yMax() - bbv.mBoundingBox.yMin();
         modelDimensions.z() = bbv.mBoundingBox.zMax() - bbv.mBoundingBox.zMin();
         return modelDimensions;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::InternalSetDamageState(PlatformActorProxy::DamageStateEnum& damageState)
      {
         bool setUseDimensions = true;
         float stateNum = 0.0f;
         osg::Node* modelToCalcDims = NULL;

         if (damageState == PlatformActorProxy::DamageStateEnum::NO_DAMAGE)
         {
            mSwitchNode->setSingleChildOn(0);
            modelToCalcDims = mNonDamagedFileNode.get();
         }
         else if (damageState == PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE || damageState == PlatformActorProxy::DamageStateEnum::MODERATE_DAMAGE)
         {
            stateNum = 1.0f;
            if (mDamagedFileNode->getUserData() == NULL)
            {
               mSwitchNode->setSingleChildOn(0);
               modelToCalcDims = mNonDamagedFileNode.get();
            }
            else
            {
               mSwitchNode->setSingleChildOn(1);
               modelToCalcDims = mDamagedFileNode.get();
            }
         }
         else if (damageState == PlatformActorProxy::DamageStateEnum::DESTROYED)
         {
            stateNum = 2.0f;
            if (mDestroyedFileNode->getUserData() == NULL)
            {
               if (mDamagedFileNode->getUserData() == NULL)
               {
                  mSwitchNode->setSingleChildOn(0);
                  modelToCalcDims = mNonDamagedFileNode.get();
               }
               else
               {
                  mSwitchNode->setSingleChildOn(1);
                  modelToCalcDims = mDamagedFileNode.get();
               }
            }
            else
            {
               mSwitchNode->setSingleChildOn(2);
               modelToCalcDims = mDestroyedFileNode.get();
            }
         }
         else
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
            "Damage state is not a valid state");

            setUseDimensions = false;
         }

         // Update the entity painting effect to show damage.
         using namespace SimCore::ActComps;
         CamoPaintStateActComp* paintActComp = GetComponent<CamoPaintStateActComp>();
         if (paintActComp != NULL)
         {
            paintActComp->SetPaintState(stateNum);
         }

         dtGame::DeadReckoningHelper* drHelper = NULL;
         GetComponent(drHelper);

         //compute the model dimensions for this damage state model, since the dimensions will differ from
         //the other damage states
         //NOTE: this fixes the flaming entity problem because the DeadReckoningComponent will
         //factor in any particle system attached to us with our bounding volume
         if (modelToCalcDims != NULL)
         {
            drHelper->SetModelDimensions(ComputeDimensions(*modelToCalcDims));
         }

         drHelper->SetUseModelDimensions(setUseDimensions);

         BaseClass::SetDamageState(damageState);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetDamageState(PlatformActorProxy::DamageStateEnum& damageState)
      {
         if (damageState != GetDamageState())
         {
            InternalSetDamageState(damageState);

            SimCore::ActComps::PlatformDefaultPhysicsActComp* physAC = NULL;
            // as if this writing, GetComponent doesn't do a dynamic cast, but the dynamic cast is required here
            // because I need to know not only if this has a physics component, but if that component is this specific type.
            physAC = GetComponent<SimCore::ActComps::PlatformDefaultPhysicsActComp>();
            if (physAC != NULL)
            {
               physAC->LoadCollision(false);
            }
         }
      }

      /// For the different physics models
      osg::Node* Platform::GetNonDamagedFileNode()
      {
         if (mNonDamagedFileNode->getNumChildren() > 0)
         {
            return mNonDamagedFileNode->getChild(0);
         }
         return NULL;
      }

      /// For the different physics models
      osg::Node* Platform::GetDamagedFileNode()
      {
         if (mDamagedFileNode->getNumChildren() > 0)
         {
            return mDamagedFileNode->getChild(0);
         }
         return NULL;
      }

      /// For the different physics models
      osg::Node* Platform::GetDestroyedFileNode()
      {
         if (mDestroyedFileNode->getNumChildren() > 0)
         {
            return mDestroyedFileNode->getChild(0);
         }
         return NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<osg::Switch> Platform::GetSwitchNode()
      {
         return mSwitchNode;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool Platform::LoadModelNodeInternal(osg::Group& modelNode,
               const std::string& fileName,
               const std::string& copiedNodeName)
      {
         if (fileName == modelNode.getName())
         {
            return false;
         }

         if (modelNode.getNumChildren() > 0)
         {
            std::ostringstream oss;
            oss << "Platform forced a reload of model files: File [" << fileName      <<
                   "], Actor Type[" << GetGameActorProxy().GetActorType().GetName() <<
                   "], Name[" << GetName() << "], Id[" << GetUniqueId().ToString()  <<
                   "].  Old model name ["<< modelNode.getName() << "]." ;

            mLogger->LogMessage(dtUtil::Log::LOG_WARNING,
                                __FUNCTION__, __LINE__, oss.str().c_str());

         }

         modelNode.removeChild(0, modelNode.getNumChildren());

         // Store the file name in the name of the node.
         modelNode.setName(fileName);

         dtCore::RefPtr<osg::Node> cachedOriginalNode;
         dtCore::RefPtr<osg::Node> copiedNode;
         if (!LoadFile(fileName, cachedOriginalNode, copiedNode, true))
         {
            throw dtGame::InvalidParameterException(
                     std::string("Model file could not be loaded: ") + fileName, __FILE__, __LINE__);
         }
         copiedNode->setName(copiedNodeName);
         modelNode.addChild(copiedNode.get());
         modelNode.setUserData(cachedOriginalNode.get());

         return true;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::LoadDamageableFile(const dtCore::ResourceDescriptor& rd, PlatformActorProxy::DamageStateEnum& state)
      {
         std::string fileName;

         if (!rd.IsEmpty())
         {
            try
            {
                fileName = dtCore::Project::GetInstance().GetResourcePath(rd);
            }
            catch (const dtCore::ProjectException& ex)
            {
                LOG_ERROR(ex.ToString());
                // Not returning, which will make it clear any models on the actor, then keep running.
                // It could return, but having no file is handled already, so I'm just going to let it be.
            }
         }

         if (!fileName.empty())
         {

            bool loadedNewModel = false;
            if (state == PlatformActorProxy::DamageStateEnum::NO_DAMAGE)
            {
               if (fileName != mNonDamagedFileNode->getName())
               {
                  loadedNewModel = LoadModelNodeInternal(*mNonDamagedFileNode, fileName, state.GetName());
                  LoadNodeCollector();
               }
            }
            else if (state == PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
            {
               loadedNewModel = LoadModelNodeInternal(*mDamagedFileNode, fileName, state.GetName());
            }
            else if (state == PlatformActorProxy::DamageStateEnum::MODERATE_DAMAGE)
            {
               loadedNewModel = LoadModelNodeInternal(*mDamagedFileNode, fileName, state.GetName());
            }
            else if (state == PlatformActorProxy::DamageStateEnum::DESTROYED)
            {
               loadedNewModel = LoadModelNodeInternal(*mDestroyedFileNode, fileName, state.GetName());
            }
            else
            {
               throw dtGame::InvalidParameterException(
                        "Damage state is not supported", __FILE__, __LINE__);
            }

            if (loadedNewModel)
            {
               /// this resets the model now that new model files have been loaded n' stuff.
               InternalSetDamageState(GetDamageState());
            }
         }
         else
         {
            if (state == PlatformActorProxy::DamageStateEnum::NO_DAMAGE)
            {
               mNonDamagedFileNode->removeChild(0,mNonDamagedFileNode->getNumChildren());
               mNonDamagedFileNode->setName("");
               mNonDamagedFileNode->setUserData(NULL);
               SetNodeCollector(NULL);
               if (mArticHelper.valid())
               {
                  mArticHelper->UpdateDOFReferences(NULL);
               }
            }
            else if (state == PlatformActorProxy::DamageStateEnum::SLIGHT_DAMAGE || state == PlatformActorProxy::DamageStateEnum::MODERATE_DAMAGE)
            {
               mDamagedFileNode->removeChild(0,mDamagedFileNode->getNumChildren());
               mDamagedFileNode->setName("");
               mDamagedFileNode->setUserData(NULL);
            }
            else if (state == PlatformActorProxy::DamageStateEnum::DESTROYED)
            {
               mDestroyedFileNode->removeChild(0,mDestroyedFileNode->getNumChildren());
               mDamagedFileNode->setName("");
               mDestroyedFileNode->setUserData(NULL);
            }
            else
            {
               throw dtGame::InvalidParameterException(
               "Damage state is not supported", __FILE__, __LINE__);
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::OnShaderGroupChanged()
      {
         if (GetShaderGroup().empty())
         {
            return;
         }

         const dtCore::ShaderGroup *shaderGroup =
            dtCore::ShaderManager::GetInstance().FindShaderGroupPrototype(GetShaderGroup());

         //First get the shader group assigned to this actor.
         if (shaderGroup == NULL)
         {
            //mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
            //   "Could not find shader group: [" + GetShaderGroup() + "].");
            return;
         }

         //All DVTE entities have at least three possible shaders, one for each damage
         //state.  These specific shaders are searched for first.  If one is not found,
         //then if a default shader is specified in the shader group it will be assigned
         //to that damage state.  If all three damage state shaders are undefined and
         //a default shader exists, that shader is applied to the root switch node of
         //the actor so it gets passed on to all damage states.
         const dtCore::ShaderProgram *defaultShader = shaderGroup->GetDefaultShader();
         const dtCore::ShaderProgram *noDamage = shaderGroup->FindShader("NoDamage");
         const dtCore::ShaderProgram *moderate = shaderGroup->FindShader("ModerateDamage");
         const dtCore::ShaderProgram *destroyed = shaderGroup->FindShader("Destroyed");

         try
         {
            //First if all three are not present check the default, assign it, or return.
            if (noDamage == NULL && moderate == NULL && destroyed == NULL)
            {
               if (defaultShader != NULL)
               {
                  dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *mSwitchNode);
               }
               else
               {
                  //mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,
                  //   "Could not find any shaders in shader group: [" + GetShaderGroup() + "].");
                  return;
               }
            }

            //Check the non damaged shader...
            if (noDamage != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*noDamage,*mNonDamagedFileNode);
            }
            else if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader,*mNonDamagedFileNode);
            }

            //Check the moderate damage shader...
            if (moderate != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*moderate,*mDamagedFileNode);
            }
            else if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader,*mDamagedFileNode);
            }

            //Check the destroyed shader...
            if (destroyed != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*destroyed,*mDestroyedFileNode);
            }
            else if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader,*mDestroyedFileNode);
            }
         }
         catch (const dtUtil::Exception &e)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,
               "Caught Exception while assigning shader: " + e.ToString());
            return;
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      osg::Vec3 Platform::GetEngineTransform()
      {
         dtCore::Transform xform;
         osg::Vec3 vec;

         GetTransform(xform, dtCore::Transformable::REL_CS);
         xform.GetTranslation(vec);

         return vec;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::EnsureResourcesAreLoaded()
      {
         // dynamic cast will throw an exception on the reference if it's not the right type.
         dynamic_cast<PlatformActorProxy&>(GetGameActorProxy()).EnsureResourcesAreLoaded();
      }


      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         EnsureResourcesAreLoaded();

         dtCore::RefPtr<SimCore::ActComps::CamoPaintStateActComp> camoPaintComp;
         GetComponent(camoPaintComp);

         if (camoPaintComp.valid() && camoPaintComp->GetParentNode() == NULL)
         {
            camoPaintComp->SetParentNode(&GetScaleMatrixTransform());
            camoPaintComp->SetHiderNode(mSwitchNode.get());
         }

         GetGameActorProxy().RegisterForMessagesAboutSelf(dtGame::MessageType::INFO_TIMER_ELAPSED, "TimeElapsedForSoundIdle");

         if (IsRemote())
         {
            dtUtil::NodeCollector* nodeCollector = GetNodeCollector();
            if (nodeCollector != NULL && !nodeCollector->GetTransformNodeMap().empty())
            {
               GetComponent<dtGame::DeadReckoningHelper>()->SetNodeCollector(*nodeCollector);
            }
         }
         else
         {
            GetComponent<dtGame::DeadReckoningHelper>()->SetUpdateMode(dtGame::DeadReckoningHelper::UpdateMode::CALCULATE_ONLY);
         }

         //// Curt - bump mapping
         dtCore::ShaderProgram* defaultShader = dtCore::ShaderManager::GetInstance().
            GetShaderInstanceForNode(GetOSGNode());
         dtCore::ShaderParamFloat* useBumpmappingParam = NULL;
         if (defaultShader != NULL)
         {
            useBumpmappingParam = dynamic_cast<dtCore::ShaderParamFloat*>
               (defaultShader->FindParameter("useBumpMap"));
         }

         // if bump mapping is turned on, generate the tangents to be passed to the shader
         if (useBumpmappingParam != NULL && useBumpmappingParam->GetValue() == 1.0f)
         {
            dtCore::RefPtr<dtUtil::TangentSpaceVisitor> visitor = new dtUtil::TangentSpaceVisitor
               ("vTangent", (osg::Program*)defaultShader->GetShaderProgram(), 6);
            mNonDamagedFileNode->accept(*visitor.get());
            mDamagedFileNode->accept(*visitor.get());
            mDestroyedFileNode->accept(*visitor.get());
         }

         if (!mSFXSoundIdleEffect.empty() && GetGameActorProxy().IsInGM())
         {
            LoadSFXEngineIdleLoop();
         }

         // Once it is added to the GM, it needs to actually create the headlight light, so we have to reset it.
         if (IsHeadLightsEnabled())
         {
            SetHeadLightsEnabled(true);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetSFXEngineIdleLoop(const std::string& soundFX)
      {
         mSFXSoundIdleEffect = soundFX;
         if (!mSFXSoundIdleEffect.empty() && GetGameActorProxy().IsInGM())
         {
            LoadSFXEngineIdleLoop();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::LoadSFXEngineIdleLoop()
      {
         if (mSndIdleLoop.valid())
         {
            mSndIdleLoop->Stop();
            RemoveChild(mSndIdleLoop.get());
            mSndIdleLoop.release();
         }

         if (!mSFXSoundIdleEffect.empty())
         {
            GetGameActorProxy().GetGameManager()->SetTimer("TimeElapsedForSoundIdle", &GetGameActorProxy(), 1, true);
            mSndIdleLoop = dtAudio::AudioManager::GetInstance().NewSound();
            mSndIdleLoop->LoadFile(mSFXSoundIdleEffect.c_str());
            mSndIdleLoop->SetLooping(true);
            mSndIdleLoop->SetMaxDistance(mMaxIdleSoundDistance);
            AddChild(mSndIdleLoop.get());
            dtCore::Transform xform;
            mSndIdleLoop->SetTransform(xform, dtCore::Transformable::REL_CS);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::TickTimerMessage(const dtGame::Message& tickMessage)
      {
         UpdateEngineIdleSoundEffect();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::UpdateEngineIdleSoundEffect()
      {
         if (mSndIdleLoop == NULL)
         {
            return;
         }

         if (GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {
            if (mSndIdleLoop->IsPlaying())
            {
               mSndIdleLoop->Stop();
            }
            return;
         }

         dtCore::Transform ourTransform;
         dtCore::Transform cameraTransform;

         GetTransform(ourTransform);
         GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->GetTransform(cameraTransform);

         osg::Vec3 ourXYZ, cameraXYZ;

         ourTransform.GetTranslation(ourXYZ);
         cameraTransform.GetTranslation(cameraXYZ);

         osg::Vec3 distanceVector = ourXYZ - cameraXYZ;
         float distanceFormula = distanceVector.length2();
         if ( (mMaxIdleSoundDistance * mMaxIdleSoundDistance ) <  distanceFormula)
         {
            mSndIdleLoop->Stop();
         }
         else
         {
            mSndIdleLoop->Play();
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetArticulatedParametersArray(const dtCore::NamedGroupParameter& newValue)
      {
         if ( mArticHelper.valid() && GetNodeCollector() != NULL)
         {
            mArticHelper->HandleArticulatedParametersArray(
               newValue, *GetNodeCollector(), *GetComponent<dtGame::DeadReckoningHelper>() );
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtCore::NamedGroupParameter> Platform::GetArticulatedParametersArray()
      {
         if ( mArticHelper.valid() )
         {
            return mArticHelper->BuildGroupProperty();
         }

         return new dtCore::NamedGroupParameter(Components::ArticulationHelper::PROPERTY_NAME_ARTICULATED_ARRAY);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::LoadNodeCollector()
      {
         dtCore::RefPtr<dtUtil::NodeCollector> nc =  new dtUtil::NodeCollector(mNonDamagedFileNode.get(), dtUtil::NodeCollector::AllNodeTypes);
         SetNodeCollector(nc);
         GetComponent<dtGame::DeadReckoningHelper>()->SetNodeCollector(*nc);
         // Update the articulation helper with DOFs of the current model.
         if (!mArticHelper.valid())
         {
            mArticHelper = new SimCore::Components::DefaultArticulationHelper;
         }

         mArticHelper->UpdateDOFReferences(nc.get());
         // TODO set node collector on weapon swapper.
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::SetArticulationHelper( SimCore::Components::ArticulationHelper* articHelper )
      {
         mArticHelper = articHelper;
         if (mArticHelper.valid())
         {
            mArticHelper->UpdateDOFReferences( GetNodeCollector() );

            // Check the config property for reversing the heading value. This is an issue because
            // in most HLA federations, the heading direction for articulations is reversed.
            // In Client-server, this is rarely true, so we default to false.
            dtUtil::ConfigProperties& configParams = GetGameActorProxy().GetGameManager()->GetConfiguration();
            std::string reverseStr = configParams.GetConfigPropertyValue("SimCore.Articulation.ReverseHeading", "false");
            bool reverseHeading = (reverseStr == "true" || reverseStr == "True" || reverseStr == "TRUE" || reverseStr == "1");
            mArticHelper->SetPublishReverseHeading(reverseHeading);
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::TickControlState( const dtGame::Message& tickMessage )
      {
         if ( mTimeUntilControlStateUpdate >= 0.0f )
         {
            mTimeUntilControlStateUpdate -=
               static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaRealTime();
         }

         // Send control state update if controlling articulations directly
         // on a remote entity.
         if ( mTimeUntilControlStateUpdate < 0.0f && mArticHelper.valid() && mArticHelper->IsDirty() )
         {
            if ( mArticHelper->GetControlState() != NULL )
            {
               mTimeUntilControlStateUpdate = mTimeBetweenControlStateUpdates;

               mArticHelper->NotifyControlStateUpdate();
               mArticHelper->SetDirty( false );
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void Platform::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         dtGame::DRPublishingActComp* drPublishingComp = NULL;
         GetComponent(drPublishingComp);
         if (drPublishingComp != NULL && mArticHelper.valid() && mArticHelper->IsDirty())
         {
            drPublishingComp->ForceUpdateAtNextOpportunity();
            //mArticHelper->SetDirty(false); //Dirty is cleared when we publish elsewhere
         }

         BaseClass::OnTickLocal(tickMessage);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool Platform::ShouldBeVisible(const SimCore::VisibilityOptions& options)
      {
         const BasicVisibilityOptions& basicOptions = options.GetBasicOptions();
         bool baseVal = BaseClass::ShouldBeVisible(options);
         return baseVal && basicOptions.mPlatforms;
      }

   }
}
