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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/Actors/PositionMarker.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/FireActor.h>
#include <SimCore/Actors/LocalEffectActor.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/MultipleDetonationActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/MissileActor.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/TextureProjectorActor.h>
#include <SimCore/Actors/EphemerisEnvironmentActor.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Actors/BaseWaterActor.h>
#include <SimCore/Actors/VehicleAttachingConfigActor.h>
#include <SimCore/Actors/PortalActor.h>
#include <SimCore/Actors/OpenFlightToIVETerrain.h>
#include <SimCore/Actors/FlareActor.h>
#include <SimCore/Actors/DynamicLightPrototypeActor.h>
#include <SimCore/Actors/WaterGridActor.h>
#include <SimCore/Actors/SurfaceVesselActor.h>
#include <dtAudio/soundactor.h>
#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/MunitionParticlesActor.h>
#include <SimCore/Actors/FourWheelVehicleActor.h>
#include <SimCore/Actors/EnvironmentProcessActor.h>
#include <SimCore/Actors/SimpleMovingShapeActor.h>
#include <SimCore/Actors/BattlefieldGraphicsActor.h>
#include <SimCore/Actors/PlatformWithPhysics.h>
#include <SimCore/Actors/PagedTerrainPhysicsActor.h>
#include <SimCore/Actors/HumanWithPhysicsActor.h>
#include <SimCore/Actors/LatLongDataActor.h>
#include <SimCore/Actors/OceanDataActor.h>
#include <SimCore/Actors/SurfaceHazeDataActor.h>
#include <SimCore/Actors/DRGhostActor.h>
#include <SimCore/Actors/LogicConditionalActor.h>
#include <SimCore/Actors/LogicOnEventActor.h>
#include <SimCore/Actors/CamoConfigActor.h>

#include <SimCore/Components/BaseGameAppComponent.h>
#include <SimCore/Components/ControlStateComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/ViewerMaterialComponent.h>
#include <SimCore/Components/PortalComponent.h>
#include <SimCore/Components/TextureProjectorComponent.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/Conversations/ConversationComponent.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/BaseInputComponent.h>

#include <SimCore/ActComps/AnimationClipActComp.h>
#include <SimCore/ActComps/BodyPaintActComp.h>

#include <dtActors/engineactorregistry.h>

#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////
      //Components
      //////////////////////////////
      const dtCore::RefPtr<dtCore::SystemComponentType> BaseGameAppComponent::TYPE(new dtCore::SystemComponentType("BaseGameAppComponent","GMComponent.SimCore",
            "Deprecated.  This component has some simple functions used in legacy apps.", dtGame::GMComponent::BaseGMComponentType));
      const std::string BaseGameAppComponent::DEFAULT_NAME(BaseGameAppComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> ControlStateComponent::TYPE(new dtCore::SystemComponentType("ControlStateComponent","GMComponent.SimCore",
            "This is a custom component for dealing with attaching a detaching networked players to points in vehicle or other actor.", dtGame::GMComponent::BaseGMComponentType));
      const dtUtil::RefString ControlStateComponent::DEFAULT_NAME(ControlStateComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> MunitionsComponent::TYPE(new dtCore::SystemComponentType("MunitionsComponent","GMComponent.SimCore",
            "Munition mapping for effects and damage.", dtGame::GMComponent::BaseGMComponentType));
      const dtUtil::RefString MunitionsComponent::DEFAULT_NAME(MunitionsComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> RenderingSupportComponent::TYPE(new dtCore::SystemComponentType("RenderingSupportComponent","GMComponent.SimCore",
            "A grab-bag of rendering effects.  The main is dynamic lighting.", dtGame::GMComponent::BaseGMComponentType));
      const std::string RenderingSupportComponent::DEFAULT_NAME(RenderingSupportComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> ParticleManagerComponent::TYPE(new dtCore::SystemComponentType("ParticleManagerComponent","GMComponent.SimCore",
            "Manages shaders and wind on particle systems.",
            dtGame::GMComponent::BaseGMComponentType));
      const std::string ParticleManagerComponent::DEFAULT_NAME("ParticleManagerComponent");

      const dtCore::RefPtr<dtCore::SystemComponentType> ViewerMaterialComponent::TYPE(new dtCore::SystemComponentType("ViewerMaterialComponent","GMComponent.SimCore",
            "Loads a map of materials and keeps track of them.",
            dtGame::GMComponent::BaseGMComponentType));
      const dtUtil::RefString ViewerMaterialComponent::DEFAULT_NAME(ViewerMaterialComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> PortalComponent::TYPE(new dtCore::SystemComponentType("PortalComponent","GMComponent.SimCore",
            "Finds portals, tracks them, and can be queried for ones nearby.",
            dtGame::GMComponent::BaseGMComponentType));
      const dtUtil::RefString PortalComponent::DEFAULT_NAME(PortalComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> TextureProjectorComponent::TYPE(new dtCore::SystemComponentType("TextureProjectorComponent","GMComponent.SimCore",
            "Manages texture projectors.",
            dtGame::GMComponent::BaseGMComponentType));
      const dtUtil::RefString TextureProjectorComponent::DEFAULT_NAME(TextureProjectorComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> WeatherComponent::TYPE(new dtCore::SystemComponentType("WeatherComponent","GMComponents.SimCore",
            "Managers weather effects",
            dtGame::GMComponent::BaseGMComponentType));
      const std::string WeatherComponent::DEFAULT_NAME(WeatherComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> ConversationComponent::TYPE(new dtCore::SystemComponentType("ConversationComponent","GMComponents.SimCore",
            "Manages configurable complex conversations.",
            dtGame::GMComponent::BaseGMComponentType));
      const std::string ConversationComponent::DEFAULT_NAME(ConversationComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> TimedDeleterComponent::TYPE(new dtCore::SystemComponentType("TimedDeleterComponent","GMComponents.SimCore",
            "Deletes things after a certain period of time.",
            dtGame::GMComponent::BaseGMComponentType));
      const std::string TimedDeleterComponent::DEFAULT_NAME(TimedDeleterComponent::TYPE->GetName());


      const dtCore::RefPtr<dtCore::SystemComponentType> VolumeRenderingComponent::TYPE(new dtCore::SystemComponentType("VolumeRenderingComponent","GMComponents.SimCore",
            "Rendering volumetric effects.",
            dtGame::GMComponent::BaseGMComponentType));
      const std::string VolumeRenderingComponent::DEFAULT_NAME(VolumeRenderingComponent::TYPE->GetName());

      const dtCore::RefPtr<dtCore::SystemComponentType> BaseInputComponent::TYPE(new dtCore::SystemComponentType("BaseInputComponent","GMComponents.SimCore",
            "Base Input component subclass in Simcore.  It handles quitting and going to full screen.",
            dtGame::BaseInputComponent::DEFAULT_TYPE));
      const std::string BaseInputComponent::DEFAULT_NAME(BaseInputComponent::TYPE->GetName());

   }

   namespace ActComps
   {
      const dtGame::ActorComponent::ACType AnimationClipActComp::TYPE(new dtCore::ActorType("AnimationClipActComp","ActorComponents",
            "Plays animation clips stored in the geometry of a drawable.", dtGame::ActorComponent::BaseActorComponentType));
      const dtGame::ActorComponent::ACType BodyPaintActComp::TYPE( new dtCore::ActorType("BodyPaintActComp", "ActorComponents",
            "Repaints geometry in the shader given colors", dtGame::ActorComponent::BaseActorComponentType));
   }

   namespace Actors
   {
      RefPtr<dtCore::ActorType> EntityActorRegistry::PLATFORM_ACTOR_TYPE(new dtCore::ActorType("Platform", "Entity", "Represents a entity in the game world"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::HUMAN_ACTOR_TYPE(new dtCore::ActorType("Human", "Entity", "Represents a Human"));

      RefPtr<dtCore::ActorType> EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE(
               new dtCore::ActorType("PlatformWithPhysics", "Entity", "A platform with a basic convex hull for collision detection",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE(
               new dtCore::ActorType("PlatformSwitchablePhysics", "Entity", "A platform that can have physics enabled or disabled globally via an app flag.",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE(
               new dtCore::ActorType("Platform", "Entity.Ground.Civilian", "A ground vehicle platform",
                        EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE(
               new dtCore::ActorType("Platform", "Entity.Air.Civilian", "A air vehicle platform",
                        EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::HELO_PLATFORM_ACTOR_TYPE(
               new dtCore::ActorType("Platform", "Entity.Air.Civilian.Helo", "A helo vehicle platform",
                        EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE(
               new dtCore::ActorType("Platform", "Entity.Ground.Military", "A military ground platform",
                        EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE(
               new dtCore::ActorType("Platform", "Entity.Air.Military", "A military air platform",
                        EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::MILITARY_HELO_PLATFORM_ACTOR_TYPE(
               new dtCore::ActorType("Platform", "Entity.Air.Military.Helo", "A military helo platform",
                        EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::STEALTH_ACTOR_TYPE(
               new dtCore::ActorType("Stealth Actor", "Stealth Actor", "This actor is a stealth actor",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::PLAYER_ACTOR_TYPE(
               new dtCore::ActorType("Player Actor", "Player Actor", "This actor represents a player",
                        EntityActorRegistry::STEALTH_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::DETONATION_ACTOR_TYPE(new dtCore::ActorType("Detonation Actor", "Effects", "This actor represents a detonation"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::MULTIPLE_DETONATION_ACTOR_TYPE(new dtCore::ActorType("Multiple Detonation Actor", "Effects", "This actor is used to represent multiple detonations."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::POSITION_MARKER_ACTOR_TYPE(new dtCore::ActorType("Position Marker", "Entity", "This represents a position report or a blip."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::TERRAIN_ACTOR_TYPE(new dtCore::ActorType("Terrain", "DVTETerrain", "This actor is the terrain used in DVTE."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::INTERIOR_ACTOR_TYPE(new dtCore::ActorType("Interior", "Vehicles", "This is an actor for generic vehicle interiors"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::MISSILE_ACTOR_TYPE(new dtCore::ActorType("Missile", "Munitions", "This is a generic actor for many possible missile-like objects"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::TEXTURE_PROJECTION_ACTOR_TYPE(new dtCore::ActorType("TextureProjector", "Effects", "This actor represents an image projected onto a model"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::ENVIRONMENT_ACTOR_TYPE(new dtCore::ActorType("Environment", "IG.Environment", "An extended environment actor based on dtActors::BasicEnvironmentActor"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::DAYTIME_ACTOR_TYPE(new dtCore::ActorType("DayTime", "Environment", "This is a generic actor for capturing time of day information"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE(new dtCore::ActorType("UniformAtmosphere", "Environment", "This is a generic actor for many possible missile-like objects"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::MATERIAL_ACTOR_TYPE(new dtCore::ActorType("ViewerMaterialActor", "Environment", "Holds onto material properties, multiple materials can reference this."));

      RefPtr<dtCore::ActorType> EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE(new dtCore::ActorType(
         "MunitionTypeActor", "Munitions",
         "Contains the name, DIS ID, and an effects info reference specific to a munition."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::MUNITION_EFFECTS_INFO_ACTOR_TYPE(new dtCore::ActorType(
         "MunitionEffectsInfoActor", "Munitions",
         "Contains resource paths to various content related to munitions, such as sound, geometry and particles."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::WEAPON_ACTOR_TYPE(new dtCore::ActorType("WeaponActor", "Munitions", "The actor type that fires a munition."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::WEAPON_FLASH_ACTOR_TYPE(new dtCore::ActorType("WeaponFlashActor", "Munitions", "The actor that represents a weapon's flash effect."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::FLARE_ACTOR_TYPE(new dtCore::ActorType("FlareActor", "Munitions", "The actor that represents a flare (illumination round)."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE(new dtCore::ActorType("ControlState", "ControlState", "The base control state actor."));

      RefPtr<dtCore::ActorType> EntityActorRegistry::BASE_WATER_ACTOR_TYPE(new dtCore::ActorType("BaseWaterActor", "SimCore.Actors", "The base class for water actors."));

      RefPtr<dtCore::ActorType> EntityActorRegistry::AGEIA_PARTICLE_SYSTEM_TYPE(new dtCore::ActorType("NxAgeiaParticleSystemActor", "NxAgeiaPhysicsModels"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE(new dtCore::ActorType("NxAgeiaMunitionsPSysActor", "NxAgeiaPhysicsModels"));

      RefPtr<dtCore::ActorType> EntityActorRegistry::PHYSICS_PARTICLE_SYSTEM_TYPE(new dtCore::ActorType("PhysicsParticleSystem", "Effects"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::PHYSICS_MUNITIONS_PARTICLE_SYSTEM_TYPE(new dtCore::ActorType("MunitionParticles", "Effects"));

      // needs to be implemented whether or not physics is on.
      RefPtr<dtCore::ActorType> EntityActorRegistry::PAGED_TERRAIN_PHYSICS_ACTOR_TYPE(new dtCore::ActorType("PagedTerrainPhysicsActor", "Terrain"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::AGEIA_VEHICLE_ACTOR_TYPE(new dtCore::ActorType("NxAgeiaFourWheelVehicle", "NxAgeiaPhysicsModels", "",
               EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::FOUR_WHEEL_VEHICLE_ACTOR_TYPE(new dtCore::ActorType("FourWheelVehicle", "Entity", "",
               EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::FOUR_WHEEL_VEHICLE_MIL_ACTOR_TYPE(new dtCore::ActorType("FourWheelVehicle", "Entity.Ground.Military", "Multi-wheeled vehicle with physics configured for a military vehicle with camo and weopons",
               EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::FOUR_WHEEL_VEHICLE_CIV_ACTOR_TYPE(new dtCore::ActorType("FourWheelVehicle", "Entity.Ground.Civilian", "Multi-wheeled vehicle with physics configured more simply than a military vehicle.",
               EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::HUMAN_PHYSICS_ACTOR_TYPE(
               new dtCore::ActorType("HumanWithPhysicsActor", "Entity", "Human with a physics collision mesh",
                        EntityActorRegistry::HUMAN_ACTOR_TYPE.get()));
      RefPtr<dtCore::ActorType> EntityActorRegistry::WARFIGHTER_ACTOR_TYPE(
               new dtCore::ActorType("WarfighterActor", "Entity", "Human with a physics collision mesh and weapons",
                        EntityActorRegistry::HUMAN_PHYSICS_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::PORTAL_ACTOR_TYPE(new dtCore::ActorType("Portal", "PortalModels"));

      RefPtr<dtCore::ActorType> EntityActorRegistry::VEHICLE_CONFIG_ACTOR_TYPE(new dtCore::ActorType("VehicleConfigActorType", "ViSiTToolkit"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::LM_OPENFLIGHT_TERRAIN_ACTORTYPE(new dtCore::ActorType("LM_OpenFlightTerrain", "DVTETerrain"));

      RefPtr<dtCore::ActorType> EntityActorRegistry::DYNAMIC_LIGHT_PROTOTYPE_ACTOR_TYPE(new dtCore::ActorType("DynamicLightPrototypeActorType", "Effects"));
      RefPtr<dtCore::ActorType> EntityActorRegistry::SPOT_LIGHT_PROTOTYPE_ACTOR_TYPE(new dtCore::ActorType("SpotLightPrototypeActorType", "Effects"));

      RefPtr<dtCore::ActorType> EntityActorRegistry::BLIP_ACTOR_TYPE(
               new dtCore::ActorType("Blip", "Entity", ""));

      RefPtr<dtCore::ActorType> EntityActorRegistry::OLD_REMOTE_PHYSX_ACTOR_TYPE(
               new dtCore::ActorType("NxAgeiaRemoteKinematicActor", "NxAgeiaPhysicsModels", "",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::AGEIA_TLAND_ACTOR_TYPE(new dtCore::ActorType("NxAgeiaTerraPageLandActor", "NxAgeiaPhysicsModels"));

      RefPtr<dtCore::ActorType> EntityActorRegistry::LAT_LONG_DATA_ACTOR_TYPE
         = new dtCore::ActorType("LatLongDataActor","SimCore",
         "Actor for capturing remote environmental data.",NULL);

      RefPtr<dtCore::ActorType> EntityActorRegistry::OCEAN_DATA_ACTOR_TYPE
         = new dtCore::ActorType("OceanDataActor","SimCore",
         "Actor for capturing remote environmental data.", LAT_LONG_DATA_ACTOR_TYPE.get());

      RefPtr<dtCore::ActorType> EntityActorRegistry::SURFACE_HAZE_DATA_ACTOR_TYPE
         = new dtCore::ActorType("SurfaceHazeDataActor","SimCore",
         "Actor for capturing remote environmental data.", LAT_LONG_DATA_ACTOR_TYPE.get());

      dtCore::RefPtr<dtCore::ActorType> EntityActorRegistry::WATER_GRID_ACTOR_TYPE(
         new dtCore::ActorType("WaterGridActor", "SimCore", "This is the base water actor.",
         SimCore::Actors::EntityActorRegistry::BASE_WATER_ACTOR_TYPE.get() ));

      RefPtr<dtCore::ActorType> EntityActorRegistry::DR_GHOST_ACTOR_TYPE(
         new dtCore::ActorType("DRGhostActor", "SimCore", "Shows how dead reckoning works by following a platform",
         dtActors::EngineActorRegistry::GAME_MESH_ACTOR_TYPE.get()));

      dtCore::RefPtr<dtCore::ActorType> EntityActorRegistry::SURFACE_VESSEL_ACTOR_TYPE(
         new dtCore::ActorType("SurfaceVesselActor", "SimCore", "This is the actor used for any type of water vehicle.",
         EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtCore::ActorType> EntityActorRegistry::SOUND_ACTOR_TYPE(
               new dtCore::ActorType("SoundActor", "SimCore", "A sound actor that only takes up a source when it's not playing."));

      // Game Event Logic pieces
      RefPtr<dtCore::ActorType> EntityActorRegistry::LOGIC_CONDITIONAL_ACTOR_TYPE(
         new dtCore::ActorType("LogicConditionalActor", "SimCore.Logic", "A true & false data class that is used by one of the Logic behavior classes such as LogicOnEventActor."));
      RefPtr<dtCore::ActorType> EntityActorRegistry::LOGIC_ON_EVENT_ACTOR_TYPE(
         new dtCore::ActorType("LogicOnEventActor", "SimCore.Logic", "A logic actor that can fire events when some set of conditions for its children have been met."));
      
      // Config Actors
      RefPtr<dtCore::ActorType> EntityActorRegistry::CAMO_CONFIG_ACTOR_TYPE(
         new dtCore::ActorType("CamoConfigActor", "SimCore.Config", "An actor that loads information about camo patterns and colors from a specified config file."));

      RefPtr<dtCore::ActorType> EntityActorRegistry::ENVIRONMENT_PROCESS_ACTOR_TYPE(
               new dtCore::ActorType("EnvironmentProcess", "SimCore", "Represents an environmental process such as smoke plumes, clouds, mist, etc.")
               );

      RefPtr<dtCore::ActorType> EntityActorRegistry::ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE(
               new dtCore::ActorType("EnvironmentProcessMovingShape", "SimCore", "Represents an shape or other deadreckoned area that is part of an environment process.")
               );

      RefPtr<dtCore::ActorType> EntityActorRegistry::BATTLEFIELD_GRAPHICS_ACTOR_TYPE( new dtCore::ActorType("BattlefieldGraphics", "SimCore", "Represents a shape or area that is extruded in 2D."));


      ///////////////////////////////////////////////////////////////////////////
      extern "C" SIMCORE_EXPORT dtCore::ActorPluginRegistry* CreatePluginRegistry()
      {
          return new EntityActorRegistry;
      }

      ///////////////////////////////////////////////////////////////////////////
      extern "C" SIMCORE_EXPORT void DestroyPluginRegistry(dtCore::ActorPluginRegistry* registry)
      {
          delete registry;
      }

      ///////////////////////////////////////////////////////////////////////////
      EntityActorRegistry::EntityActorRegistry() :
         dtCore::ActorPluginRegistry("This library will store some entity actors")
      {
         dtCore::ShaderManager::GetInstance().LoadShaderDefinitions("Shaders/ShaderDefs.xml", true);
      }

      ///////////////////////////////////////////////////////////////////////////
      EntityActorRegistry::~EntityActorRegistry()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      void EntityActorRegistry::RegisterActorTypes()
      {
         mActorFactory->RegisterType<PlatformActorProxy>(PLATFORM_ACTOR_TYPE.get());
         mActorFactory->RegisterType<HumanActorProxy>(HUMAN_ACTOR_TYPE.get());

         mActorFactory->RegisterType<PlatformActorProxy>(PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE.get());

         mActorFactory->RegisterType<PlatformActorProxy>(PLATFORM_WITH_PHYSICS_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PlatformActorProxy>(OLD_REMOTE_PHYSX_ACTOR_TYPE.get());

         mActorFactory->RegisterType<PlatformActorProxy>(GROUND_PLATFORM_ACTOR_TYPE);

         mActorFactory->RegisterType<PlatformActorProxy>(AIR_PLATFORM_ACTOR_TYPE);
         mActorFactory->RegisterType<PlatformActorProxy>(HELO_PLATFORM_ACTOR_TYPE);

         mActorFactory->RegisterType<PlatformActorProxy>(MILITARY_GROUND_PLATFORM_ACTOR_TYPE);

         mActorFactory->RegisterType<PlatformActorProxy>(MILITARY_AIR_PLATFORM_ACTOR_TYPE);
         mActorFactory->RegisterType<PlatformActorProxy>(MILITARY_HELO_PLATFORM_ACTOR_TYPE);

         mActorFactory->RegisterType<StealthActorProxy>(STEALTH_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PlayerActorProxy>(PLAYER_ACTOR_TYPE.get());
         mActorFactory->RegisterType<DetonationActorProxy>(DETONATION_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MultipleDetonationActorProxy>(MULTIPLE_DETONATION_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PositionMarkerActorProxy>(POSITION_MARKER_ACTOR_TYPE.get());
         mActorFactory->RegisterType<TerrainActorProxy>(TERRAIN_ACTOR_TYPE.get());
         mActorFactory->RegisterType<InteriorActorProxy>(INTERIOR_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MissileActorProxy>(MISSILE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<TextureProjectorActorProxy>(TEXTURE_PROJECTION_ACTOR_TYPE.get());
         mActorFactory->RegisterType<EphemerisEnvironmentActorProxy>(ENVIRONMENT_ACTOR_TYPE.get());
         mActorFactory->RegisterType<DayTimeActorProxy>(DAYTIME_ACTOR_TYPE.get());
         mActorFactory->RegisterType<UniformAtmosphereActor>(UNIFORM_ATMOSPHERE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<ViewerMaterialActor>(MATERIAL_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MunitionTypeActorProxy>(MUNITION_TYPE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MunitionEffectsInfoActorProxy>(MUNITION_EFFECTS_INFO_ACTOR_TYPE.get());
         mActorFactory->RegisterType<WeaponActorProxy>(WEAPON_ACTOR_TYPE.get());
         mActorFactory->RegisterType<WeaponFlashActorProxy>(WEAPON_FLASH_ACTOR_TYPE.get());
         mActorFactory->RegisterType<FlareActorProxy>(FLARE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<ControlStateProxy>(CONTROL_STATE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<BaseWaterActorProxy>(BASE_WATER_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PortalProxy>(PORTAL_ACTOR_TYPE.get());

         mActorFactory->RegisterType<PhysicsParticleSystemActorProxy>(AGEIA_PARTICLE_SYSTEM_TYPE.get());
         mActorFactory->RegisterType<MunitionParticlesActorProxy>(AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE.get());

         mActorFactory->RegisterType<PhysicsParticleSystemActorProxy>(PHYSICS_PARTICLE_SYSTEM_TYPE.get());
         mActorFactory->RegisterType<MunitionParticlesActorProxy>(PHYSICS_MUNITIONS_PARTICLE_SYSTEM_TYPE.get());

         mActorFactory->RegisterType<PlatformWithPhysicsActorProxy>(AGEIA_VEHICLE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<FourWheelVehicleActorProxy>(FOUR_WHEEL_VEHICLE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<FourWheelVehicleActorProxy>(FOUR_WHEEL_VEHICLE_CIV_ACTOR_TYPE.get());
         mActorFactory->RegisterType<FourWheelVehicleActorProxy>(FOUR_WHEEL_VEHICLE_MIL_ACTOR_TYPE.get());

         mActorFactory->RegisterType<HumanWithPhysicsActorProxy>(HUMAN_PHYSICS_ACTOR_TYPE.get());
         mActorFactory->RegisterType<HumanWithPhysicsActorProxy>(WARFIGHTER_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PagedTerrainPhysicsActorProxy>(AGEIA_TLAND_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PagedTerrainPhysicsActorProxy>(PAGED_TERRAIN_PHYSICS_ACTOR_TYPE.get());

         mActorFactory->RegisterType<VehicleAttachingConfigActorProxy>(VEHICLE_CONFIG_ACTOR_TYPE.get());

         mActorFactory->RegisterType<OpenFlightToIVETerrainActorProxy>(LM_OPENFLIGHT_TERRAIN_ACTORTYPE.get());

         mActorFactory->RegisterType<DynamicLightPrototypeProxy>(DYNAMIC_LIGHT_PROTOTYPE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<SpotLightPrototypeProxy>(SPOT_LIGHT_PROTOTYPE_ACTOR_TYPE.get());

         mActorFactory->RegisterType<PositionMarkerActorProxy>(BLIP_ACTOR_TYPE.get());

         mActorFactory->RegisterType<LatLongDataActorProxy>(LAT_LONG_DATA_ACTOR_TYPE.get());
         mActorFactory->RegisterType<OceanDataActorProxy>(OCEAN_DATA_ACTOR_TYPE.get());
         mActorFactory->RegisterType<SurfaceHazeDataActorProxy>(SURFACE_HAZE_DATA_ACTOR_TYPE.get());

         // OBSOLETE ACTOR TYPES - FOR backward compatible playbacks back to IPT2 (summer 2007).
         dtCore::ActorType* oldEntityType = new dtCore::ActorType("Entity", "Entity",
               "OBSOLETE ENTITY TYPE - IS NOW PLATFORM - BACKWARD COMPATIBLE FOR OLDER LOG FILES");
         mActorFactory->RegisterType<PlatformActorProxy>(oldEntityType);

         mActorFactory->RegisterType<HumanWithPhysicsActorProxy>(new dtCore::ActorType("HumanWithPhysicsActor", "NxAgeiaPhysicsModels", "Human with a physics collision mesh",
                  EntityActorRegistry::HUMAN_ACTOR_TYPE.get()));

         mActorFactory->RegisterType<EnvironmentProcessActorProxy>(ENVIRONMENT_PROCESS_ACTOR_TYPE);
         mActorFactory->RegisterType<SimpleMovingShapeActorProxy>(ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE);

         mActorFactory->RegisterType<WaterGridActorProxy>(WATER_GRID_ACTOR_TYPE.get());
         mActorFactory->RegisterType<DRGhostActorProxy>(DR_GHOST_ACTOR_TYPE.get());
         mActorFactory->RegisterType<SurfaceVesselActorProxy>(SURFACE_VESSEL_ACTOR_TYPE.get());
         mActorFactory->RegisterType<dtAudio::SoundActor>(SOUND_ACTOR_TYPE.get());

         // Game Event Logic pieces
         mActorFactory->RegisterType<LogicConditionalActor>(LOGIC_CONDITIONAL_ACTOR_TYPE.get());
         mActorFactory->RegisterType<LogicOnEventActor>(LOGIC_ON_EVENT_ACTOR_TYPE.get());
         
         // Config Actors
         mActorFactory->RegisterType<CamoConfigActorProxy>(CAMO_CONFIG_ACTOR_TYPE.get());

         mActorFactory->RegisterType<BattlefieldGraphicsActorProxy>(BATTLEFIELD_GRAPHICS_ACTOR_TYPE.get());

         mActorFactory->RegisterType<SimCore::ActComps::AnimationClipActComp>();
         mActorFactory->RegisterType<SimCore::ActComps::BodyPaintActComp>();

      }
   }
}
