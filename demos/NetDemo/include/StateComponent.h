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

#ifndef NETDEMO_STATE_COMPONENT_H
#define NETDEMO_STATE_COMPONENT_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Components/GameState/GameStateComponent.h>



////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
namespace dtGame
{
   class Message;
}

namespace SimCore
{
   namespace Components
   {
      class GameStateChangedMessage;
   }
}



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // COMPONENT CODE
   /////////////////////////////////////////////////////////////////////////////
   class StateComponent : public SimCore::Components::GameStateComponent
   {
      public:
         typedef SimCore::Components::GameStateComponent BaseClass;

         static const dtUtil::RefString DEFAULT_NAME;

         StateComponent(const std::string& name = DEFAULT_NAME.Get() );

         /**
          * Handles incoming messages
          */
         virtual void ProcessMessage(const dtGame::Message& message);

         /**
          * Handles logic dependent on certain states.
          */
         virtual void ProcessStateChange(
            const SimCore::Components::GameStateChangedMessage& stateChange );

      protected:
         virtual ~StateComponent();

      private:

    };

}

#endif
