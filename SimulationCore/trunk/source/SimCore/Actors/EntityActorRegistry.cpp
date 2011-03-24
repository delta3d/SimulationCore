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
#include <SimCore/Actors/SoundActorProxy.h>
#include <SimCore/Actors/DynamicParticleSystem.h>
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

#include <dtActors/engineactorregistry.h>

#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Actors
   {
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PLATFORM_ACTOR_TYPE(new dtDAL::ActorType("Platform", "Entity", "Represents a entity in the game world"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::HUMAN_ACTOR_TYPE(new dtDAL::ActorType("Human", "Entity", "Represents a Human"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::PLATFORM_WITH_PHYSICS_ACTOR_TYPE(
               new dtDAL::ActorType("PlatformWithPhysics", "Entity", "A platform with a basic convex hull for collision detection",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE(
               new dtDAL::ActorType("PlatformSwitchablePhysics", "Entity", "A platform that can have physics enabled or disabled globally via an app flag.",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE(
               new dtDAL::ActorType("Platform", "Entity.Ground.Civilian", "A ground vehicle platform",
                        EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE(
               new dtDAL::ActorType("Platform", "Entity.Air.Civilian", "A air vehicle platform",
                        EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE(
               new dtDAL::ActorType("Platform", "Entity.Ground.Military", "A military ground platform",
                        EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::MILITARY_AIR_PLATFORM_ACTOR_TYPE(
               new dtDAL::ActorType("Platform", "Entity.Air.Military", "A military air platform",
                        EntityActorRegistry::AIR_PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::STEALTH_ACTOR_TYPE(
               new dtDAL::ActorType("Stealth Actor", "Stealth Actor", "This actor is a stealth actor",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PLAYER_ACTOR_TYPE(
               new dtDAL::ActorType("Player Actor", "Player Actor", "This actor represents a player",
                        EntityActorRegistry::STEALTH_ACTOR_TYPE.get()));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::DETONATION_ACTOR_TYPE(new dtDAL::ActorType("Detonation Actor", "Effects", "This actor represents a detonation"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::MULTIPLE_DETONATION_ACTOR_TYPE(new dtDAL::ActorType("Multiple Detonation Actor", "Effects", "This actor is used to represent multiple detonations."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::POSITION_MARKER_ACTOR_TYPE(new dtDAL::ActorType("Position Marker", "Entity", "This represents a position report or a blip."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::TERRAIN_ACTOR_TYPE(new dtDAL::ActorType("Terrain", "DVTETerrain", "This actor is the terrain used in DVTE."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::INTERIOR_ACTOR_TYPE(new dtDAL::ActorType("Interior", "Vehicles", "This is an actor for generic vehicle interiors"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::MISSILE_ACTOR_TYPE(new dtDAL::ActorType("Missile", "Munitions", "This is a generic actor for many possible missile-like objects"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::TEXTURE_PROJECTION_ACTOR_TYPE(new dtDAL::ActorType("TextureProjector", "Effects", "This actor represents an image projected onto a model"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::ENVIRONMENT_ACTOR_TYPE(new dtDAL::ActorType("Environment", "IG.Environment", "An extended environment actor based on dtActors::BasicEnvironmentActor"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::DAYTIME_ACTOR_TYPE(new dtDAL::ActorType("DayTime", "Environment", "This is a generic actor for capturing time of day information"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE(new dtDAL::ActorType("UniformAtmosphere", "Environment", "This is a generic actor for many possible missile-like objects"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::MATERIAL_ACTOR_TYPE(new dtDAL::ActorType("ViewerMaterialActor", "Environment", "Holds onto material properties, multiple materials can reference this."));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE(new dtDAL::ActorType(
         "MunitionTypeActor", "Munitions",
         "Contains the name, DIS ID, and an effects info reference specific to a munition."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::MUNITION_EFFECTS_INFO_ACTOR_TYPE(new dtDAL::ActorType(
         "MunitionEffectsInfoActor", "Munitions",
         "Contains resource paths to various content related to munitions, such as sound, geometry and particles."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::WEAPON_ACTOR_TYPE(new dtDAL::ActorType("WeaponActor", "Munitions", "The actor type that fires a munition."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::WEAPON_FLASH_ACTOR_TYPE(new dtDAL::ActorType("WeaponFlashActor", "Munitions", "The actor that represents a weapon's flash effect."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::FLARE_ACTOR_TYPE(new dtDAL::ActorType("FlareActor", "Munitions", "The actor that represents a flare (illumination round)."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE(new dtDAL::ActorType("ControlState", "ControlState", "The base control state actor."));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::BASE_WATER_ACTOR_TYPE(new dtDAL::ActorType("BaseWaterActor", "SimCore.Actors", "The base class for water actors."));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_PARTICLE_SYSTEM_TYPE(new dtDAL::ActorType("NxAgeiaParticleSystemActor", "NxAgeiaPhysicsModels"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE(new dtDAL::ActorType("NxAgeiaMunitionsPSysActor", "NxAgeiaPhysicsModels"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::PHYSICS_PARTICLE_SYSTEM_TYPE(new dtDAL::ActorType("PhysicsParticleSystem", "Effects"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PHYSICS_MUNITIONS_PARTICLE_SYSTEM_TYPE(new dtDAL::ActorType("MunitionParticles", "Effects"));

      // needs to be implemented whether or not physics is on.
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PAGED_TERRAIN_PHYSICS_ACTOR_TYPE(new dtDAL::ActorType("PagedTerrainPhysicsActor", "Terrain"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_VEHICLE_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaFourWheelVehicle", "NxAgeiaPhysicsModels", "",
               EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::FOUR_WHEEL_VEHICLE_ACTOR_TYPE(new dtDAL::ActorType("FourWheelVehicle", "Entity", "",
               EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::FOUR_WHEEL_VEHICLE_MIL_ACTOR_TYPE(new dtDAL::ActorType("FourWheelVehicle", "Entity.Ground.Military", "Multi-wheeled vehicle with physics configured for a military vehicle with camo and weopons",
               EntityActorRegistry::MILITARY_GROUND_PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::FOUR_WHEEL_VEHICLE_CIV_ACTOR_TYPE(new dtDAL::ActorType("FourWheelVehicle", "Entity.Ground.Civilian", "Multi-wheeled vehicle with physics configured more simply than a military vehicle.",
               EntityActorRegistry::GROUND_PLATFORM_ACTOR_TYPE.get()));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::HUMAN_PHYSICS_ACTOR_TYPE(
               new dtDAL::ActorType("HumanWithPhysicsActor", "Entity", "Human with a physics collision mesh",
                        EntityActorRegistry::HUMAN_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::PORTAL_ACTOR_TYPE(new dtDAL::ActorType("Portal", "PortalModels"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::VEHICLE_CONFIG_ACTOR_TYPE(new dtDAL::ActorType("VehicleConfigActorType", "ViSiTToolkit"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::LM_OPENFLIGHT_TERRAIN_ACTORTYPE(new dtDAL::ActorType("LM_OpenFlightTerrain", "DVTETerrain"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::DYNAMIC_LIGHT_PROTOTYPE_ACTOR_TYPE(new dtDAL::ActorType("DynamicLightPrototypeActorType", "Effects"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::SPOT_LIGHT_PROTOTYPE_ACTOR_TYPE(new dtDAL::ActorType("SpotLightPrototypeActorType", "Effects"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::BLIP_ACTOR_TYPE(
               new dtDAL::ActorType("Blip", "Entity", ""));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::OLD_REMOTE_PHYSX_ACTOR_TYPE(
               new dtDAL::ActorType("NxAgeiaRemoteKinematicActor", "NxAgeiaPhysicsModels", "",
                        EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_TLAND_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaTerraPageLandActor", "NxAgeiaPhysicsModels"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::LAT_LONG_DATA_ACTOR_TYPE
         = new dtDAL::ActorType("LatLongDataActor","SimCore",
         "Actor for capturing remote environmental data.",NULL);

      RefPtr<dtDAL::ActorType> EntityActorRegistry::OCEAN_DATA_ACTOR_TYPE
         = new dtDAL::ActorType("OceanDataActor","SimCore",
         "Actor for capturing remote environmental data.", LAT_LONG_DATA_ACTOR_TYPE.get());

      RefPtr<dtDAL::ActorType> EntityActorRegistry::SURFACE_HAZE_DATA_ACTOR_TYPE
         = new dtDAL::ActorType("SurfaceHazeDataActor","SimCore",
         "Actor for capturing remote environmental data.", LAT_LONG_DATA_ACTOR_TYPE.get());

      dtCore::RefPtr<dtDAL::ActorType> EntityActorRegistry::WATER_GRID_ACTOR_TYPE(
         new dtDAL::ActorType("WaterGridActor", "SimCore", "This is the base water actor.",
         SimCore::Actors::EntityActorRegistry::BASE_WATER_ACTOR_TYPE.get() ));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::DR_GHOST_ACTOR_TYPE(
         new dtDAL::ActorType("DRGhostActor", "SimCore", "Shows how dead reckoning works by following a platform",
         dtActors::EngineActorRegistry::GAME_MESH_ACTOR_TYPE.get()));

      dtCore::RefPtr<dtDAL::ActorType> EntityActorRegistry::SURFACE_VESSEL_ACTOR_TYPE(
         new dtDAL::ActorType("SurfaceVesselActor", "SimCore", "This is the actor used for any type of water vehicle.",
         EntityActorRegistry::PLATFORM_ACTOR_TYPE.get()));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::SOUND_ACTOR_TYPE(
               new dtDAL::ActorType("SoundActor", "SimCore", "A sound actor that only takes up a source when it's not playing."));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::DYNAMIC_PARTICLE_SYSTEM_ACTOR_TYPE(
         new dtDAL::ActorType("DynamicParticleSystemActor", "SimCore", "A particle system actor that can dynamically adjust various attributes of a particle system over time.."));

      // Game Event Logic pieces
      RefPtr<dtDAL::ActorType> EntityActorRegistry::LOGIC_CONDITIONAL_ACTOR_TYPE(
         new dtDAL::ActorType("LogicConditionalActor", "SimCore.Logic", "A true & false data class that is used by one of the Logic behavior classes such as LogicOnEventActor."));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::LOGIC_ON_EVENT_ACTOR_TYPE(
         new dtDAL::ActorType("LogicOnEventActor", "SimCore.Logic", "A logic actor that can fire events when some set of conditions for its children have been met."));
      
      // Config Actors
      RefPtr<dtDAL::ActorType> EntityActorRegistry::CAMO_CONFIG_ACTOR_TYPE(
         new dtDAL::ActorType("CamoConfigActor", "SimCore.Config", "An actor that loads information about camo patterns and colors from a specified config file."));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::ENVIRONMENT_PROCESS_ACTOR_TYPE(
               new dtDAL::ActorType("EnvironmentProcess", "SimCore", "Represents an environmental process such as smoke plumes, clouds, mist, etc.")
               );

      RefPtr<dtDAL::ActorType> EntityActorRegistry::ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE(
               new dtDAL::ActorType("EnvironmentProcessMovingShape", "SimCore", "Represents an shape or other deadreckoned area that is part of an environment process.")
               );

      RefPtr<dtDAL::ActorType> EntityActorRegistry::BATTLEFIELD_GRAPHICS_ACTOR_TYPE( new dtDAL::ActorType("BattlefieldGraphics", "SimCore", "Represents a shape or area that is extruded in 2D."));


      ///////////////////////////////////////////////////////////////////////////
      extern "C" SIMCORE_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
      {
          return new EntityActorRegistry;
      }

      ///////////////////////////////////////////////////////////////////////////
      extern "C" SIMCORE_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry* registry)
      {
          delete registry;
      }

      ///////////////////////////////////////////////////////////////////////////
      EntityActorRegistry::EntityActorRegistry() :
         dtDAL::ActorPluginRegistry("This library will store some entity actors")
      {
         dtCore::ShaderManager::GetInstance().LoadShaderDefinitions("Shaders/ShaderDefs.xml", false);
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

         mActorFactory->RegisterType<PlatformActorProxy>(MILITARY_GROUND_PLATFORM_ACTOR_TYPE);

         mActorFactory->RegisterType<PlatformActorProxy>(MILITARY_AIR_PLATFORM_ACTOR_TYPE);

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
         mActorFactory->RegisterType<UniformAtmosphereActorProxy>(UNIFORM_ATMOSPHERE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<ViewerMaterialActorProxy>(MATERIAL_ACTOR_TYPE.get());
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
         dtDAL::ActorType* oldEntityType = new dtDAL::ActorType("Entity", "Entity",
               "OBSOLETE ENTITY TYPE - IS NOW PLATFORM - BACKWARD COMPATIBLE FOR OLDER LOG FILES");
         mActorFactory->RegisterType<PlatformActorProxy>(oldEntityType);

         mActorFactory->RegisterType<HumanWithPhysicsActorProxy>(new dtDAL::ActorType("HumanWithPhysicsActor", "NxAgeiaPhysicsModels", "Human with a physics collision mesh",
                  EntityActorRegistry::HUMAN_ACTOR_TYPE.get()));

         mActorFactory->RegisterType<EnvironmentProcessActorProxy>(ENVIRONMENT_PROCESS_ACTOR_TYPE);
         mActorFactory->RegisterType<SimpleMovingShapeActorProxy>(ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE);

         mActorFactory->RegisterType<WaterGridActorProxy>(WATER_GRID_ACTOR_TYPE.get());
         mActorFactory->RegisterType<DRGhostActorProxy>(DR_GHOST_ACTOR_TYPE.get());
         mActorFactory->RegisterType<SurfaceVesselActorProxy>(SURFACE_VESSEL_ACTOR_TYPE.get());
         mActorFactory->RegisterType<SoundActorProxy>(SOUND_ACTOR_TYPE.get());

         mActorFactory->RegisterType<DynamicParticleSystemActorProxy>(DYNAMIC_PARTICLE_SYSTEM_ACTOR_TYPE.get());

         // Game Event Logic pieces
         mActorFactory->RegisterType<LogicConditionalActorProxy>(LOGIC_CONDITIONAL_ACTOR_TYPE.get());
         mActorFactory->RegisterType<LogicOnEventActorProxy>(LOGIC_ON_EVENT_ACTOR_TYPE.get());
         
         // Config Actors
         mActorFactory->RegisterType<CamoConfigActorProxy>(CAMO_CONFIG_ACTOR_TYPE.get());

         mActorFactory->RegisterType<BattlefieldGraphicsActorProxy>(BATTLEFIELD_GRAPHICS_ACTOR_TYPE.get());

      }
   }
}
