/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson 
 */
#ifndef _ENTITY_ACTOR_REGISTRY_H_
#define _ENTITY_ACTOR_REGISTRY_H_

#include <dtDAL/actorpluginregistry.h>
#include <SimCore/Export.h>

namespace SimCore 
{
   namespace Actors 
   {
      /**
       * Class that exports the applicable actor proxies to a library
       */
      class SIMCORE_EXPORT EntityActorRegistry : public dtDAL::ActorPluginRegistry
      {
         public:

            static dtCore::RefPtr<dtDAL::ActorType> PLATFORM_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> HUMAN_ACTOR_TYPE;

            static dtCore::RefPtr<dtDAL::ActorType> STEALTH_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> PLAYER_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> DETONATION_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> TERRAIN_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> INTERIOR_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> MISSILE_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> TEXTURE_PROJECTION_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> ENVIRONMENT_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> DAYTIME_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> UNIFORM_ATMOSPHERE_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> MATERIAL_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> MUNITION_TYPE_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> MUNITION_EFFECTS_INFO_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> WEAPON_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> WEAPON_FLASH_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> CONTROL_STATE_ACTOR_TYPE;

#ifdef AGEIA_PHYSICS
            static dtCore::RefPtr<dtDAL::ActorType> AGEIA_PARTICLE_SYSTEM_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> AGEIA_MUNITIONS_PARTICLE_SYSTEM_TYPE;
#endif

            static dtCore::RefPtr<dtDAL::ActorType> AGEIA_TLAND_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> AGEIA_REMOTE_KINEMATIC_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> AGEIA_VEHICLE_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> AGEIA_CHARACTER_ACTOR_TYPE;
            //static dtCore::RefPtr<dtDAL::ActorType> AGEIA_EMBARKABLE_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> PORTAL_ACTOR_TYPE;

            static dtCore::RefPtr<dtDAL::ActorType> VEHICLE_CONFIG_ACTOR_TYPE;
            static dtCore::RefPtr<dtDAL::ActorType> LM_OPENFLIGHT_TERRAIN_ACTORTYPE;

            /// Constructor
            EntityActorRegistry();

            /// Registers all of the actor proxies to be exported
            void RegisterActorTypes();
      };	
   }	
}

#endif 
