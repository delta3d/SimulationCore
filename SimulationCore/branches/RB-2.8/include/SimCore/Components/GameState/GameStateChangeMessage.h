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
 * Bradley Anderegg
 */

#ifndef SIMCORE_GAME_STATE_CHANGED_MESSAGE_H
#define SIMCORE_GAME_STATE_CHANGED_MESSAGE_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtGame/message.h>
#include <SimCore/Export.h>
#include <SimCore/Components/GameState/GameState.h>



namespace SimCore
{
   namespace Components
   {
      /////////////////////////////////////////////////////////////////////////////
      // GAME STATE CHANGED MESSAGE
      /////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT GameStateChangedMessage : public dtGame::Message
      {
         public:
            static const std::string PARAMETER_STATE_OLD;
            static const std::string PARAMETER_STATE_NEW;

            GameStateChangedMessage();

            /// Gets the outgoing game state
            GameState::Type& GetOldState() const;

            /// Sets the outgoing game state
            void SetOldState(const GameState::Type &oldState);

            /// Gets the incoming state
            GameState::Type& GetNewState() const;

            /// Sets the incoming state
            void SetNewState(const GameState::Type &newState);

         protected:
            virtual ~GameStateChangedMessage();

         private:
            dtGame::EnumMessageParameter& mOldParam;
            dtGame::EnumMessageParameter& mNewParam;
      };

   }
}

#endif
