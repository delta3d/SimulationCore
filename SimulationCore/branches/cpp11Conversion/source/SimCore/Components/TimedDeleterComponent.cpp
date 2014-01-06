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
 * @author Chris Rodgers
 */

#include <prefix/SimCorePrefix.h>
#include <dtCore/uniqueid.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>

#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

namespace SimCore
{
   namespace Components
   {
      const std::string TimedDeleterComponent::DEFAULT_NAME = "TimedDeleterComponent";

      TimedDeleterComponent::TimedDeleterComponent( const std::string& name )
         : dtGame::GMComponent(name)
      {
      }

      TimedDeleterComponent::~TimedDeleterComponent()
      {
      }

      void TimedDeleterComponent::Clear()
      {
         //std::cout << "Resetting and deleting all actors:\n";

         // Delete the objects sharing the found index
         std::shared_ptr<dtDAL::ActorProxy> curProxy;

         std::map<std::string, dtCore::UniqueId>::iterator i = mTimerToIDMap.begin();
         for( ; i != mTimerToIDMap.end(); ++i )
         {
            curProxy = GetGameManager()->FindActorById( i->second );

            if( curProxy.valid() )
            {
               //std::cout << "\tDeleting associated actor" << i->second << "\n";
               GetGameManager()->DeleteActor( *curProxy );
            }
         }

         // Clear out all generated timers
         std::map<dtCore::UniqueId, std::string>::iterator i2 = mIDToTimerMap.begin();
         for( ; i2 != mIDToTimerMap.end(); ++i2 )
         {
            GetGameManager()->ClearTimer( i2->second, nullptr );
         }

         // Clear the lists
         mIDToTimerMap.clear();
         mTimerToIDMap.clear();
      }

      void TimedDeleterComponent::Reset()
      {
         Clear();

         // Re-initialize here...
      }

      void TimedDeleterComponent::GetIds( std::vector<dtCore::UniqueId>& listToFill )
      {
         listToFill.clear();
         listToFill.reserve( mTimerToIDMap.size() );

         std::map<std::string, dtCore::UniqueId>::iterator i = mTimerToIDMap.begin();
         for( ; i != mTimerToIDMap.end(); ++i )
         {
            listToFill.push_back( i->second );
         }
      }

      unsigned int TimedDeleterComponent::GetIdCount() const
      {
         return mIDToTimerMap.size();
      }

      const std::string TimedDeleterComponent::GetAssociatedTimerName( const dtCore::UniqueId& id ) const
      {
         std::map<dtCore::UniqueId, std::string>::const_iterator i = mIDToTimerMap.find(id);
         if (i != mIDToTimerMap.end())
         {
            return i->second;
         }
         return "";
      }

      bool TimedDeleterComponent::HasId( const dtCore::UniqueId& id ) const
      {
         return mIDToTimerMap.find(id) != mIDToTimerMap.end();
      }

      bool TimedDeleterComponent::HasTimer( const std::string& timerName )
      {
         return mTimerToIDMap.find(timerName) != mTimerToIDMap.end();
      }

      void TimedDeleterComponent::AddId( const dtCore::UniqueId& id, double simWaitTime )
      {
         if( !HasId(id) )
         {
            //std::cout << "\nAdding delete schedule: " << id.ToString().c_str() << "\n";

            // Create the associated timer
            const std::string &timerName = CreateTimerNameFromId( id );
            GetGameManager()->SetTimer( timerName, nullptr, simWaitTime );

            // Register the ID and timer name
            bool ok = mTimerToIDMap.insert(std::make_pair(timerName, id)).second;
            if (ok)
               ok = mIDToTimerMap.insert(std::make_pair(id, timerName)).second;

            //std::cout << "\t" << GetIdCount() << " deletes remain\n\n";
         }
      }

      void TimedDeleterComponent::RemoveId( const dtCore::UniqueId& id )
      {
         std::map<dtCore::UniqueId, std::string>::iterator i = mIDToTimerMap.find(id);
         if (i != mIDToTimerMap.end())
         {
            std::string timerName = i->second;
            mIDToTimerMap.erase(i);

            std::map<std::string, dtCore::UniqueId>::iterator i2 = mTimerToIDMap.find(timerName);
            if(i2 != mTimerToIDMap.end())
            {
               mTimerToIDMap.erase(i2);
            }
         }
      }

      void TimedDeleterComponent::RemoveIdByTimerName( const std::string& timerName )
      {

         std::map<std::string, dtCore::UniqueId>::iterator i = mTimerToIDMap.find(timerName);
         if(i != mTimerToIDMap.end())
         {
            dtCore::UniqueId id = i->second;
            //std::cout << "Deleting " << id.ToString().c_str() << "\n";
            mTimerToIDMap.erase(i);

            std::map<dtCore::UniqueId, std::string>::iterator i2 = mIDToTimerMap.find(id);
            if (i2 != mIDToTimerMap.end())
            {
               mIDToTimerMap.erase(i2);
            }
         }
      }

      void TimedDeleterComponent::ProcessMessage( const dtGame::Message& message )
      {
         // Check for an external delete.
         if( message.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED )
         {
            // Remove the ID from this component.
            RemoveId(message.GetAboutActorId());
         }
         // Check if a delete deadline has been met.
         else if( message.GetMessageType() == dtGame::MessageType::INFO_TIMER_ELAPSED )
         {
            // Check that an ID is not in the message;
            // nullptr proxys are registered with the timers,
            // thus the elapsed timer will not be able to access
            // the associated ID from within the GameManager.
            if( message.GetAboutActorId().ToString().empty() )
            {
               // Find the timer referred to by the message
               const dtGame::TimerElapsedMessage& timeElapseMessage = 
                  static_cast<const dtGame::TimerElapsedMessage&> (message);
               const std::string &timerName = timeElapseMessage.GetTimerName();

               // Delete the objects sharing the found index
               std::shared_ptr<dtDAL::ActorProxy> proxy = nullptr;
               std::map<std::string, dtCore::UniqueId>::iterator i = mTimerToIDMap.find(timerName);
               if( i != mTimerToIDMap.end() )
               {
                  proxy = GetGameManager()->FindActorById( i->second );
               }
               
               if( proxy.valid() )
               {
                  //std::cout << "\tDeleting associated actor" << mIds[index] << "\n";
                  GetGameManager()->DeleteActor( *proxy );
               }

               // Unregister timer name and id
               RemoveIdByTimerName( timerName );
               //std::cout << "\t" << GetIdCount() << " deletes remain\n\n";
            }
         }
         // Is this a global application state change?
         else if( message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED
            || message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED )
         {
            // Deletes will not need to be called from this component
            // since the change in map states will cause the GameManager
            // to delete all actors anyway.
            Clear();
         }
         // Restarts will have accelerate the deletion
         // of registered deletable actors. Otherwise, actors might not
         // be deleted depending on the implementation of the custom application.
         // If we are in a playback, then we want to delete these temporary objects. This is necessary
         // to prevent long-lingering smoke effects to just hang around if we jump back and forth.
         else if( message.GetMessageType() == dtGame::MessageType::INFO_RESTARTED
            || message.GetMessageType() == dtGame::MessageType::LOG_COMMAND_BEGIN_LOADKEYFRAME_TRANS )
         {
            Reset();
         }

      }

      const std::string TimedDeleterComponent::CreateTimerNameFromId( const dtCore::UniqueId& id ) const
      {
         return GetName() + id.ToString();
      }

   }
}
