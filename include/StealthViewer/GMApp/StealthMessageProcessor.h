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
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
* @author David Guthrie
*/
#ifndef STEALTH_MESSAGE_PROCESSOR_H_
#define STEALTH_MESSAGE_PROCESSOR_H_

#include <SimCore/Components/ViewerMessageProcessor.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/exception.h>

namespace dtGame
{
   class GameActorProxy;
   class ActorUpdateMessage;
}

namespace StealthGM
{

   class StealthMessageProcessor: public SimCore::Components::ViewerMessageProcessor
   {
      public:

         StealthMessageProcessor();

         virtual ~StealthMessageProcessor();

      protected:
         // HACK - To work around environment being created twice in playback.  AAR hack.  Curt.
         virtual std::shared_ptr<dtGame::GameActorProxy> ProcessRemoteCreateActor(const dtGame::ActorUpdateMessage &msg);

      private:

   };
}
#endif
