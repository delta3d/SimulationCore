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
#ifndef _ENTITY_ACTOR_REGISTRY_H_
#define _ENTITY_ACTOR_REGISTRY_H_

#include <dtCore/actorpluginregistry.h>
#include <SimCore/Export.h>

namespace SimCore
{
   namespace Actors
   {
      /**
       * Class that exports the applicable actor proxies to a library
       */
      class SIMCORE_EXPORT EntityActorRegistry : public dtCore::ActorPluginRegistry
      {
         public:

            static dtCore::RefPtr<dtCore::ActorType> PLATFORM_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> HUMAN_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> GROUND_PLATFORM_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> AIR_PLATFORM_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> HELO_PLATFORM_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> MILITARY_GROUND_PLATFORM_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> MILITARY_AIR_PLATFORM_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> MILITARY_HELO_PLATFORM_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> PLATFORM_WITH_PHYSICS_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> STEALTH_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> PLAYER_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> DETONATION_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> MULTIPLE_DETONATION_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> POSITION_MARKER_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> TERRAIN_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> INTERIOR_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> MISSILE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> TEXTURE_PROJECTION_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> ENVIRONMENT_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> DAYTIME_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> UNIFORM_ATMOSPHERE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> MATERIAL_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> MUNITION_TYPE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> MUNITION_EFFECTS_INFO_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> WEAPON_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> WEAPON_FLASH_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> FLARE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> CONTROL_STATE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> BASE_WATER_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> PHYSICS_PARTICLE_SYSTEM_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> PHYSICS_MUNITIONS_PARTICLE_SYSTEM_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> PAGED_TERRAIN_PHYSICS_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> AGEIA_VEHICLE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> FOUR_WHEEL_VEHICLE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> FOUR_WHEEL_VEHICLE_CIV_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> FOUR_WHEEL_VEHICLE_MIL_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> HUMAN_PHYSICS_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> WARFIGHTER_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> PORTAL_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> VEHICLE_CONFIG_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> LM_OPENFLIGHT_TERRAIN_ACTORTYPE;

            static dtCore::RefPtr<dtCore::ActorType> DYNAMIC_LIGHT_PROTOTYPE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> SPOT_LIGHT_PROTOTYPE_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> OCEAN_WATER_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> BLIP_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> LAT_LONG_DATA_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> OCEAN_DATA_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> SURFACE_HAZE_DATA_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> WATER_GRID_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> DR_GHOST_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> SURFACE_VESSEL_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> SOUND_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> DYNAMIC_PARTICLE_SYSTEM_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> LOGIC_CONDITIONAL_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> LOGIC_ON_EVENT_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> CAMO_CONFIG_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> ENVIRONMENT_PROCESS_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE;

            static dtCore::RefPtr<dtCore::ActorType> BATTLEFIELD_GRAPHICS_ACTOR_TYPE;


            /// Constructor
            EntityActorRegistry();

            /// Destructor
            virtual ~EntityActorRegistry();

            /// Registers all of the actor proxies to be exported
            void RegisterActorTypes();
         private:

            //Private so it won't be used in code
            static dtCore::RefPtr<dtCore::ActorType> OLD_REMOTE_PHYSX_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> AGEIA_TLAND_ACTOR_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> AGEIA_PARTICLE_SYSTEM_TYPE;
            static dtCore::RefPtr<dtCore::ActorType> AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE;

      };
   }
}

#endif
