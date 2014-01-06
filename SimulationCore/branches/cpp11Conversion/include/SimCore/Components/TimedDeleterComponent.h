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

#ifndef _TIMED_DELETER_COMPONENT_H_
#define _TIMED_DELETER_COMPONENT_H_

#include <string>
#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>

namespace dtCore
{
   class UniqueId;
}

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   namespace Components
   {

      /**
       * @class TimedDeleterComponent
       * @brief subclassed component used primarily to track local actors
       * spawned by remote actors. Example: particle systems from missiles
       * need to linger but need a timed delete.
       * All objects are referenced by unique ID.
       */
      class SIMCORE_EXPORT TimedDeleterComponent : public dtGame::GMComponent
      {
         public:
            // The default component name, used when looking it up on the GM. 
            static const std::string DEFAULT_NAME;

            // Constructor
            // @param name The name by which this component is called from the GameManager
            TimedDeleterComponent( const std::string& name = DEFAULT_NAME );

            // Clean all allocated memory
            void Clear();

            // Clear and re-initialize
            void Reset();

            // @param listToFill An empty list to which all IDs will be copied.
            void GetIds( std::vector<dtCore::UniqueId>& listToFill );

            // @return total ids registered with this component
            unsigned int GetIdCount() const;

            // @param id The ID of an object that need to find its associated timer.
            // @return name of the timer associated with the specified ID; nullptr if not found
            const std::string GetAssociatedTimerName( const dtCore::UniqueId& id ) const;

            // @param id The ID in question
            // @return true if the ID is registered with this component
            bool HasId( const dtCore::UniqueId& id ) const;

            // @param name of the timer in question
            // @return true if the timer is registered with this component
            bool HasTimer( const std::string& timerName );

            // This function will only add an ID once
            // @param id The ID of the object to be registered for a delete schedule
            // @param simWaitTime The time to wait before deleting the specified object
            void AddId( const dtCore::UniqueId& id, double simWaitTime );

            // @param id The ID to be unregistered with this component
            virtual void RemoveId( const dtCore::UniqueId& id );

            // Remove an ID associated with the specified timer.
            // @param timerName The name of the timer associated with a registered ID
            virtual void RemoveIdByTimerName( const std::string& timerName );

            // Process certain massages, primarily looking for INFO_TIME_ELAPSED messages.
            // @param message Normal message received via the GameManager
            virtual void ProcessMessage( const dtGame::Message& message );

         protected:

            // Destructor
            virtual ~TimedDeleterComponent();

            // Create a unique timer name that should be used only for a single
            // registered object ID.
            // @param id The ID of the object on which to base the generated timer name
            // @return name generated for a time based on the specified ID
            const std::string CreateTimerNameFromId( const dtCore::UniqueId& id ) const;

         private:

            // Map of IDs used to reference objects that are on a delete schedule.
            std::map<std::string, dtCore::UniqueId> mTimerToIDMap;

            // Map of timer names generated in parallel to the registered IDs.
            // Timers are created on a per-ID basis and are registered with
            // the GameManager upon creation. These timers should always be unique
            // to each registered ID.
            std::map<dtCore::UniqueId, std::string> mIDToTimerMap;

      };
   }
}

#endif
