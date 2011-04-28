/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
 * Chris Rodgers
 */

#ifndef NETDEMO_STATES_H
#define NETDEMO_STATES_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Components/GameState/GameState.h>



namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////
   // STATE TYPE CODE
   //////////////////////////////////////////////////////////////////////////
   class NetDemoState : public SimCore::Components::StateType
   {
      public:
         typedef SimCore::Components::StateType BaseClass;

         /* Base States
         static StateType STATE_UNKNOWN;
         static StateType STATE_SPLASH;
         static StateType STATE_MENU;
         static StateType STATE_LOGIN;
         static StateType STATE_INTRO;
         static StateType STATE_CUTSCENE;
         static StateType STATE_LOADING;
         static StateType STATE_RUNNING;
         static StateType STATE_SHUTDOWN;*/
         static NetDemoState STATE_CONNECTING;
         static NetDemoState STATE_LOBBY;
         static NetDemoState STATE_LOBBY_CONNECT_FAIL;
         static NetDemoState STATE_UNLOAD;
         static NetDemoState STATE_GAME_GARAGE;
         static NetDemoState STATE_GAME_RUNNING;
         static NetDemoState STATE_GAME_READYROOM;
         static NetDemoState STATE_GAME_DEAD;
         static NetDemoState STATE_GAME_OPTIONS;
         static NetDemoState STATE_GAME_QUIT;
         static NetDemoState STATE_GAME_UNKNOWN;
         static NetDemoState STATE_SCORE_SCREEN;

         NetDemoState(const std::string& name)
            : StateType(name,false)
         {
         }

      protected:
         virtual ~NetDemoState()
         {
         }
   };




   //////////////////////////////////////////////////////////////////////////
   // TRANSITION CODE
   //////////////////////////////////////////////////////////////////////////
   class Transition : public SimCore::Components::EventType
   {
      public:
         typedef SimCore::Components::EventType BaseClass;

         /* Base Transition Types
         static EventType TRANSITION_OCCURRED; //used for testing
         static EventType TRANSITION_FORWARD;
         static EventType TRANSITION_BACK;
         static EventType TRANSITION_QUIT;
         static EventType TRANSITION_GAME_OVER;*/
         static Transition TRANSITION_CONNECTION_FAIL; // Use when connection fails anywhere during a connected state.
         static Transition TRANSITION_LOBBY;
         static Transition TRANSITION_LOADING;
         static Transition TRANSITION_GAME;
         static Transition TRANSITION_GAME_DEAD;
         static Transition TRANSITION_OPTIONS;
         static Transition TRANSITION_SCORE_SCREEN;

         Transition( const std::string& name )
            : BaseClass(name)
         {
         }

      protected:
         virtual ~Transition()
         {
         }
   };

}

#endif
