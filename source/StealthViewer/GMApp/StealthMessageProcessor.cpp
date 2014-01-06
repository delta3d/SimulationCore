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
* along with this librasry; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/StealthMessageProcessor.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/gamemanager.h>

#include <iostream>

namespace StealthGM
{

   StealthMessageProcessor::StealthMessageProcessor()
   {

   }

   StealthMessageProcessor::~StealthMessageProcessor()
   {

   }


   std::shared_ptr<dtGame::GameActorProxy> StealthMessageProcessor::ProcessRemoteCreateActor(const dtGame::ActorUpdateMessage &msg)
   {
      if (msg.GetSource() != GetGameManager()->GetMachineInfo() &&
            msg.GetActorTypeName() == "Environment" && msg.GetActorTypeCategory() == "dtcore.Environment")
      {
         // ignore the remote environment
         LOG_ALWAYS("HACK - Ignoring create of remote environment - this is a playback Hack");
         return nullptr;
      }

      std::shared_ptr<dtGame::GameActorProxy> gameActorProxy = SimCore::Components::ViewerMessageProcessor::ProcessRemoteCreateActor(msg);
      return gameActorProxy;
   }

}
