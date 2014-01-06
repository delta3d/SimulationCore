/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation.
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
 * @author David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#ifndef _TEST_COMPONENT_H_
#define _TEST_COMPONENT_H_
#include <dtUtil/refcountedbase.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>

namespace dtGame
{
   class Message;
   class MessageType;
}

class TestComponent: public dtGame::GMComponent
{
   public:

      TestComponent(const std::string &name = "TestComponent") : 
         dtGame::GMComponent(name)
      {

      }

      std::vector<std::shared_ptr<const dtGame::Message> >& GetReceivedProcessMessages() 
      { return mReceivedProcessMessages; }
      std::vector<std::shared_ptr<const dtGame::Message> >& GetReceivedDispatchNetworkMessages() 
      { return mReceivedDispatchNetworkMessages; }

      virtual void ProcessMessage(const dtGame::Message& msg)
      {
         mReceivedProcessMessages.push_back(&msg);
      }
      virtual void DispatchNetworkMessage(const dtGame::Message& msg)
      {
         mReceivedDispatchNetworkMessages.push_back(&msg);            
      }

      void reset() 
      {
         mReceivedDispatchNetworkMessages.clear();
         mReceivedProcessMessages.clear();
      }

      std::shared_ptr<const dtGame::Message> FindProcessMessageOfType(const dtGame::MessageType& type)
      {
         for (unsigned i = 0; i < mReceivedProcessMessages.size(); ++i)
         {
            if (mReceivedProcessMessages[i]->GetMessageType() == type)
               return mReceivedProcessMessages[i];
         }
         return nullptr;
      }
      std::shared_ptr<const dtGame::Message> FindDispatchNetworkMessageOfType(const dtGame::MessageType& type)
      {
         for (unsigned i = 0; i < mReceivedDispatchNetworkMessages.size(); ++i)
         {
            if (mReceivedDispatchNetworkMessages[i]->GetMessageType() == type)
               return mReceivedDispatchNetworkMessages[i];
         }
         return nullptr;
      }
   private:
      std::vector<std::shared_ptr<const dtGame::Message> > mReceivedProcessMessages;
      std::vector<std::shared_ptr<const dtGame::Message> > mReceivedDispatchNetworkMessages;
};

#endif

