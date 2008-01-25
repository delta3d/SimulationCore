/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#ifndef _VIEWER_MESSAGE_PROCESSOR_H_
#define _VIEWER_MESSAGE_PROCESSOR_H_

#include <SimCore/Export.h>

#include <SimCore/Actors/StealthActor.h>

#include <dtGame/defaultmessageprocessor.h>

namespace dtUtil
{
   class Log;
}

namespace dtCore
{
   class Isector;
}

namespace dtGame
{
   class ActorUpdateMessage;
   class ActorDeletedMessage;
}

namespace SimCore
{  
   class DetonationMessage;
   class TimeValueMessage;

   namespace Components
   {
      class SIMCORE_EXPORT ViewerMessageProcessor : public dtGame::DefaultMessageProcessor
      {
         public:

            /// Constructor
            ViewerMessageProcessor();

            /// Destructor
            virtual ~ViewerMessageProcessor();

            /**
             * Process the local update actor messages
             * @param msg The message to process
             */
            void ProcessLocalUpdateActor(const dtGame::ActorUpdateMessage &msg);

            virtual dtCore::RefPtr<dtGame::GameActorProxy> ProcessRemoteCreateActor(const dtGame::ActorUpdateMessage &msg);

            /**
             * Cleans up the player ref ptr on delete.
             * @param msg The message
             */
            virtual void ProcessLocalDeleteActor(const dtGame::ActorDeletedMessage &msg);

            /**
             * Process unhandled messages from the DefaultMessageProcessor
             * @param msg The message to process
             * @see class dtGame::DefaultMessageProcessor
             */
            virtual void ProcessUnhandledLocalMessage(const dtGame::Message &msg);

            /**
             * Handles the on player entered world message
             * @param msg The message
             */
            virtual void ProcessPlayerEnteredWorldMessage(const dtGame::Message &msg);

            /**
             * Sets the player actor on the VMP
             * @param pa The player actor to set, or NULL
             */
            void SetPlayerActor(SimCore::Actors::StealthActor *pa) { mPlayer = pa; }

            /**
             * Gets the player actor on the VMP
             * @return mPlayer
             */
            const SimCore::Actors::StealthActor* GetPlayerActor() const { return mPlayer.get(); }

            /**
             * Gets the player actor on the VMP
             * @return mPlayer
             */
            SimCore::Actors::StealthActor* GetPlayerActor() { return mPlayer.get(); }
            
            virtual void OnAddedToGM();
            
            /**
             * Method used to determine if the actor referenced by a player entered world message
             * could be considered the "main" player.  The default implementation makes sure
             * the actor is not remote.
             * @return true if the player could be accepted, false if not.
             */
            virtual bool AcceptPlayer(dtGame::GameActorProxy& playerProxy);

            virtual void ProcessMessage(const dtGame::Message &msg);
         
            /// @return the name of the time master as was retrieved from a time value message.
            const std::string& GetTimeMasterName() const { return mTimeMasterName; }
            
            const std::string& GetTimeSyncSenderName() const { return mTimeSyncSenderName.ToString(); }
            
            unsigned long GetTimeSyncLatency() const { return mTimeSyncLatency; } 
            
         protected:
            /// updates the GM time based on the time 
            void UpdateSyncTime(const SimCore::TimeValueMessage& tvMsg);

         private:

            /// updates the magnification of entities based on the current magnification value set via input.
            void UpdateMagnification();
            
            dtUtil::Log* mLogger;
            
            dtCore::ObserverPtr<SimCore::Actors::StealthActor> mPlayer;
            float mMagnification;
            
            std::string mTimeMasterName;
            dtCore::UniqueId mTimeSyncSenderName;
            unsigned long mTimeSyncLatency;
      };
   }
}
#endif
