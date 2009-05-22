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
#include <StateComponent.h>
#include <SimCore/Components/GameState/GameStateChangeMessage.h>

using namespace SimCore::Components;



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // COMPONENT CODE
   /////////////////////////////////////////////////////////////////////////////
   const dtUtil::RefString StateComponent::DEFAULT_NAME("StateComponent");

   /////////////////////////////////////////////////////////////////////////////
   StateComponent::StateComponent(const std::string& name)
      : BaseClass(name)
   {
      // Register application-specific states.
      AddState( &NetDemoState::STATE_LOBBY );
      AddState( &NetDemoState::STATE_GAME );
      AddState( &NetDemoState::STATE_GAME_DEAD );
      AddState( &NetDemoState::STATE_GAME_UNKNOWN );
      AddState( &NetDemoState::STATE_SCORE_SCREEN );
   }

   /////////////////////////////////////////////////////////////////////////////
   StateComponent::~StateComponent()
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void StateComponent::ProcessMessage(const dtGame::Message &message)
   {
      BaseClass::ProcessMessage(message);

      const dtGame::MessageType& messageType = message.GetMessageType();

      // TODO: Any message specific stuff.
   }

   /////////////////////////////////////////////////////////////////////////////
   void StateComponent::ProcessStateChange( const GameStateChangedMessage& stateChange )
   {
      // TODO: Any state change specific stuff.
   }

}

