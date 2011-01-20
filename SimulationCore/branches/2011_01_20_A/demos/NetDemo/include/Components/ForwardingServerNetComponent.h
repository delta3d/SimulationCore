/* -*-c++-*-
 * SimulationCore
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
 *
 * David Guthrie
 */

#ifndef FORWARDINGSERVERNETCOMPONENT_H_
#define FORWARDINGSERVERNETCOMPONENT_H_

#include <DemoExport.h>
#include <dtNetGM/servernetworkcomponent.h>
#include <dtNetGM/networkbridge.h>
#include <dtNetGM/serverconnectionlistener.h>
#include <dtUtil/macros.h>

namespace NetDemo
{
   class NETDEMO_EXPORT ForwardingServerNetComponent: public dtNetGM::ServerNetworkComponent
   {
   public:
      ForwardingServerNetComponent(const std::string& gameName, const int gameVersion, const std::string& logFile = "")
      : dtNetGM::ServerNetworkComponent(gameName, gameVersion, logFile)
      {
         SetName(DEFAULT_NAME);
      }

      ////////////////////////////////////////////////////////////////////////////////
      dtNetGM::MessageActionCode& OnBeforeSendMessage(const dtGame::Message& message, std::string& rejectReason)
      {
         dtNetGM::MessageActionCode& code = ServerNetworkComponent::OnBeforeSendMessage(message, rejectReason);
         if (code == dtNetGM::MessageActionCode::SEND)
         {
            // Create the MessageDataStream
            dtUtil::DataStream dataStream = CreateDataStream(message);
            for (std::vector<dtNetGM::NetworkBridge*>::iterator iter = mConnections.begin(); iter != mConnections.end(); iter++)
            {
               dtNetGM::NetworkBridge* bridge = *iter;
               if (bridge->IsConnectedClient() && bridge->GetMachineInfo() != message.GetSource())
               {
                  bridge->SendDataStream(dataStream);
               }
            }
         }
         return code;
      }
      ////////////////////////////////////////////////////////////////////////////////
      void ProcessNetClientRequestConnection(const dtGame::MachineInfoMessage& msg)
      {
         // Workaround for a bug where an not-connected server network component assumes it still should handle
         // the messages.  Needs to be Put into Delta3D
         if (GetConnection(msg.GetSource()) != NULL)
         {
            ServerNetworkComponent::ProcessNetClientRequestConnection(msg);
         }
      }

      // This disconnect won't work on windows because it accesses a static method in GNE, but GNE is
      // static on windows.
#ifndef DELTA_WIN32
      ////////////////////////////////////////////////////////////////////////////////
      void Disconnect()
      {
         NetworkComponent::Disconnect();
         dtNetGM::ServerConnectionListener::closeAllListeners();
      }
#endif

   };
}
#endif /* FORWARDINGSERVERNETCOMPONENT_H_ */
