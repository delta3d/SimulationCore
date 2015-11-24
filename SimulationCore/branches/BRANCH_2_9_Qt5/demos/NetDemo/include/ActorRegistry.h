/* -*-c++-*-
* Driver Demo - DriverActorRegistry (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @author Curtiss Murphy
*/
#ifndef _DRIVER_ACTOR_REGISTRY_H_
#define _DRIVER_ACTOR_REGISTRY_H_

#include <dtCore/actorpluginregistry.h>
#include <DemoExport.h>


namespace NetDemo
{
   /**
    * Class that exports the applicable actor proxies to a library
    */
   class NETDEMO_EXPORT NetDemoActorRegistry : public dtCore::ActorPluginRegistry
   {
      public:

         static dtCore::RefPtr<dtCore::ActorType> HOVER_VEHICLE_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> PLAYER_STATUS_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> SERVER_GAME_STATUS_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> FORT_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> TOWER_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> ENEMY_DESCRIPTION_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> ENEMY_MINE_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> ENEMY_MOTHERSHIP_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> ENEMY_HELIX_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> LIGHT_TOWER_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> FIREBALL_TOWER_ACTOR_TYPE;
         static dtCore::RefPtr<dtCore::ActorType> FIREBALL_ACTOR_TYPE;

         static dtCore::RefPtr<dtCore::ActorType> PROPELLED_VEHICLE_ACTOR_TYPE;
         /// Constructor
         NetDemoActorRegistry();

         /// Registers all of the actor proxies to be exported
         void RegisterActorTypes();
   };
}

#endif
