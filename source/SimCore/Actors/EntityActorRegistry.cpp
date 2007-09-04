/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of th e License, or (at your option)
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
 * @author Eddie Johnson 
 */
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/EntityActorRegistry.h>

#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/FireActor.h>
#include <SimCore/Actors/LocalEffectActor.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/MissileActor.h>
#include <SimCore/Actors/InteriorActor.h>
#include <SimCore/Actors/TextureProjectorActor.h>
#include <SimCore/Actors/IGEnvironmentActor.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Actors/VehicleAttachingConfigActor.h>
#include <SimCore/Actors/PortalActor.h>
#include <SimCore/Actors/OpenFlightToIVETerrain.h>

#ifdef AGEIA_PHYSICS
   #include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
   #include <SimCore/Actors/NxAgeiaParticleSystemActor.h>
   #include <SimCore/Actors/NxAgeiaMunitionsPSysActor.h>
#endif

#include <SimCore/Actors/NxAgeiaRemoteKinematicActor.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/NxAgeiaPlayerActor.h>

#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Actors
   {
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PLATFORM_ACTOR_TYPE(new dtDAL::ActorType("Platform", "Entity", "Represents a entity in the game world"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::HUMAN_ACTOR_TYPE(new dtDAL::ActorType("Human", "Entity", "Represents a Human"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::STEALTH_ACTOR_TYPE(new dtDAL::ActorType("Stealth Actor", "Stealth Actor", "This actor is a stealth actor"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PLAYER_ACTOR_TYPE(new dtDAL::ActorType("Player Actor", "Player Actor", "This actor represents a player"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::DETONATION_ACTOR_TYPE(new dtDAL::ActorType("Detonation Actor", "Effects", "This actor represents a detonation"));
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
      RefPtr<dtDAL::ActorType> EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE(new dtDAL::ActorType("ControlState", "ControlState", "The base control state actor."));
      
#ifdef AGEIA_PHYSICS
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_PARTICLE_SYSTEM_TYPE(new dtDAL::ActorType("NxAgeiaParticleSystemActor", "NxAgeiaPhysicsModels"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE(new dtDAL::ActorType("NxAgeiaMunitionsPSysActor", "NxAgeiaPhysicsModels"));
#endif

      // needs to be implemented whether or not physics is on.
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_TLAND_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaTerraPageLandActor", "NxAgeiaPhysicsModels"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_VEHICLE_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaFourWheelVehicle", "NxAgeiaPhysicsModels"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_REMOTE_KINEMATIC_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaRemoteKinematicActor", "NxAgeiaPhysicsModels"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_CHARACTER_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaPlayerActor", "NxAgeiaPhysicsModels"));
      //RefPtr<dtDAL::ActorType> EntityActorRegistry::AGEIA_EMBARKABLE_ACTOR_TYPE(new dtDAL::ActorType("NxAgeiaEmbarkableVehicleActor", "NxAgeiaPhysicsModels"));
      
      RefPtr<dtDAL::ActorType> EntityActorRegistry::PORTAL_ACTOR_TYPE(new dtDAL::ActorType("Portal", "PortalModels"));

      RefPtr<dtDAL::ActorType> EntityActorRegistry::VEHICLE_CONFIG_ACTOR_TYPE(new dtDAL::ActorType("VehicleConfigActorType", "ViSiTToolkit"));
      RefPtr<dtDAL::ActorType> EntityActorRegistry::LM_OPENFLIGHT_TERRAIN_ACTORTYPE(new dtDAL::ActorType("LM_OpenFlightTerrain", "DVTETerrain"));

      ///////////////////////////////////////////////////////////////////////////
      extern "C" SIMCORE_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
      {
          return new EntityActorRegistry;
      }

      ///////////////////////////////////////////////////////////////////////////
      extern "C" SIMCORE_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
      {
          delete registry;
      }

      ///////////////////////////////////////////////////////////////////////////
      EntityActorRegistry::EntityActorRegistry() :
         dtDAL::ActorPluginRegistry("This library will store some entity actors")
      {
         dtCore::ShaderManager::GetInstance().LoadShaderDefinitions("Shaders/Base/ShaderDefs.xml", true);
      }

      ///////////////////////////////////////////////////////////////////////////
      void EntityActorRegistry::RegisterActorTypes()
      {
         mActorFactory->RegisterType<PlatformActorProxy>(PLATFORM_ACTOR_TYPE.get());
         mActorFactory->RegisterType<HumanActorProxy>(HUMAN_ACTOR_TYPE.get());
         
         mActorFactory->RegisterType<StealthActorProxy>(STEALTH_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PlayerActorProxy>(PLAYER_ACTOR_TYPE.get());
         mActorFactory->RegisterType<DetonationActorProxy>(DETONATION_ACTOR_TYPE.get());
         mActorFactory->RegisterType<TerrainActorProxy>(TERRAIN_ACTOR_TYPE.get());
         mActorFactory->RegisterType<InteriorActorProxy>(INTERIOR_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MissileActorProxy>(MISSILE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<TextureProjectorActorProxy>(TEXTURE_PROJECTION_ACTOR_TYPE.get());
         mActorFactory->RegisterType<IGEnvironmentActorProxy>(ENVIRONMENT_ACTOR_TYPE.get());
         mActorFactory->RegisterType<DayTimeActorProxy>(DAYTIME_ACTOR_TYPE.get());
         mActorFactory->RegisterType<UniformAtmosphereActorProxy>(UNIFORM_ATMOSPHERE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<ViewerMaterialActorProxy>(MATERIAL_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MunitionTypeActorProxy>(MUNITION_TYPE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<MunitionEffectsInfoActorProxy>(MUNITION_EFFECTS_INFO_ACTOR_TYPE.get());
         mActorFactory->RegisterType<WeaponActorProxy>(WEAPON_ACTOR_TYPE.get());
         mActorFactory->RegisterType<WeaponFlashActorProxy>(WEAPON_FLASH_ACTOR_TYPE.get());
         mActorFactory->RegisterType<ControlStateProxy>(CONTROL_STATE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<PortalProxy>(PORTAL_ACTOR_TYPE.get());

#ifdef AGEIA_PHYSICS
         mActorFactory->RegisterType<NxAgeiaFourWheelVehicleActorProxy>(AGEIA_VEHICLE_ACTOR_TYPE.get());
         mActorFactory->RegisterType<NxAgeiaParticleSystemActorProxy>(AGEIA_PARTICLE_SYSTEM_TYPE.get());
         mActorFactory->RegisterType<NxAgeiaMunitionsPSysActorProxy>(AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE.get());
#endif
         mActorFactory->RegisterType<NxAgeiaPlayerActorProxy>(AGEIA_CHARACTER_ACTOR_TYPE.get());
         mActorFactory->RegisterType<NxAgeiaTerraPageLandActorProxy>(AGEIA_TLAND_ACTOR_TYPE.get());
         mActorFactory->RegisterType<NxAgeiaRemoteKinematicActorProxy>(AGEIA_REMOTE_KINEMATIC_ACTOR_TYPE.get());

         mActorFactory->RegisterType<VehicleAttachingConfigActorProxy>(VEHICLE_CONFIG_ACTOR_TYPE.get());

         mActorFactory->RegisterType<OpenFlightToIVETerrainActorProxy>(LM_OPENFLIGHT_TERRAIN_ACTORTYPE.get());
      }
   }
}
