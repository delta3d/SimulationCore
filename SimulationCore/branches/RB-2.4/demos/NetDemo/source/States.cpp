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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <States.h>



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // STATE TYPE CODE
   /////////////////////////////////////////////////////////////////////////////
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
   NetDemoState NetDemoState::STATE_CONNECTING("STATE_CONNECTING");
   NetDemoState NetDemoState::STATE_LOBBY("STATE_LOBBY");
   NetDemoState NetDemoState::STATE_LOBBY_CONNECT_FAIL("STATE_LOBBY_CONNECT_FAIL");
   NetDemoState NetDemoState::STATE_UNLOAD("STATE_UNLOAD");
   NetDemoState NetDemoState::STATE_GAME_GARAGE("STATE_GAME_GARAGE");
   NetDemoState NetDemoState::STATE_GAME_RUNNING("STATE_GAME_RUNNING");
   NetDemoState NetDemoState::STATE_GAME_READYROOM("STATE_GAME_READYROOM");
   NetDemoState NetDemoState::STATE_GAME_DEAD("STATE_GAME_DEAD");
   NetDemoState NetDemoState::STATE_GAME_OPTIONS("STATE_GAME_OPTIONS");
   NetDemoState NetDemoState::STATE_GAME_QUIT("STATE_GAME_QUIT");
   NetDemoState NetDemoState::STATE_GAME_UNKNOWN("STATE_UNKNOWN");
   NetDemoState NetDemoState::STATE_SCORE_SCREEN("STATE_SCORE_SCREEN");




   /////////////////////////////////////////////////////////////////////////////
   // TRANSITION CODE
   /////////////////////////////////////////////////////////////////////////////
   /* Base Transition Types
   static EventType TRANSITION_OCCURRED; //used for testing
   static EventType TRANSITION_FORWARD;
   static EventType TRANSITION_BACK;
   static EventType TRANSITION_QUIT;
   static EventType TRANSITION_GAME_OVER;*/
   Transition Transition::TRANSITION_CONNECTION_FAIL("TRANSITION_CONNECTION_FAIL");
   Transition Transition::TRANSITION_LOBBY("TRANSITION_LOBBY");
   Transition Transition::TRANSITION_LOADING("TRANSITION_LOADING");
   Transition Transition::TRANSITION_GAME("TRANSITION_GAME");
   Transition Transition::TRANSITION_GAME_DEAD("TRANSITION_GAME_DEAD");
   Transition Transition::TRANSITION_OPTIONS("TRANSITION_OPTIONS");
   Transition Transition::TRANSITION_SCORE_SCREEN("TRANSITION_SCORE_SCREEN");
}
