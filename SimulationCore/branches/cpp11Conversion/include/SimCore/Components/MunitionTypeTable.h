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

#ifndef MUNITION_TYPE_TABLE_H
#define MUNITION_TYPE_TABLE_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Actors
   {
      class DISIdentifier;
      class MunitionTypeActor;
      class MunitionTypeActorProxy;
   }

   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Munition Type Table Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionTypeTable : public std::enable_shared_from_this
      {
         public:

            // Constructor
            MunitionTypeTable();

            // Access the number of MunitionTypeActors contained in this table.
            // @return Total MunitionTypeActors contained in this table
            unsigned int GetCount() const;

            // Add a new MunitionTypeActor to this table.
            // @param newType The new MunitionTypeActor to be added to this table.
            // @return TRUE if addition was successful. FALSE usually means another
            //         entry with the same name exists in this table.
            bool AddMunitionType( const std::shared_ptr<SimCore::Actors::MunitionTypeActorProxy>& newType );

            // Remove a MunitionTypeActor by name.
            // @param name The name of the MunitionTypeActor to be removed.
            // @return TRUE if the entry existed and was successfully removed.
            bool RemoveMunitionType( const std::string& name );

            // Determine if a MunitionTypeActor exists within this table.
            // @param name The name of the MunitionTypeActor in question.
            // @return TRUE if this table contains a MunitionTypeActor with the specified name.
            bool HasMunitionType( const std::string& name ) const;

            // Access a MunitionTypeActor that is identified by the specified name.
            // @param name The name of the MunitionTypeActor to be found.
            // @return The MunitionTypeActor that has the specified name.
            //         nullptr if no MunitionTypeActor was found with the name.
            SimCore::Actors::MunitionTypeActor* GetMunitionType( const std::string& name );
            const SimCore::Actors::MunitionTypeActor* GetMunitionType( const std::string& name ) const;

            // This function makes a closest match. DIS is not guaranteed to be exact.
            // @param dis The DIS identifier used to find a MunitionTypeActor with
            //        the closest matching DIS identifier
            // @param exactMatch Determine if this function should perform an exact match.
            // @return The MunitionTypeActor with the closest or exactly matching DIS
            //         identifier as that specified by the parameter dis.
            //         nullptr if there was no match.
            const SimCore::Actors::MunitionTypeActor* GetMunitionTypeByDIS( const SimCore::Actors::DISIdentifier& dis, bool exactMatch = false ) const;
            const SimCore::Actors::MunitionTypeActor* GetMunitionTypeByDIS( const std::string& dis, bool exactMatch = false ) const;

            // Access this table's sorted list directly.
            // @return The list of MunitionTypeActors sorted by DIS identifiers,
            //         from low to high numbers (identifier numbers are read from left to right).
            const std::vector<std::shared_ptr<SimCore::Actors::MunitionTypeActor> >& GetOrderedList() const { return mOrderedList; }

            // Query the number of entries in this table with in a separate sorted list.
            // The list is sorted by DIS identifier.
            // @return Total entries in the sorted list.
            //
            // NOTE: If the table is valid, the value returned should match the total
            //       returned by GetCount.
            unsigned int GetOrderedListSize() const;

            // Remove all entries from this table.
            void Clear();

         protected:

            // Destructor
            virtual ~MunitionTypeTable();

            // This function inserts the Munition Type Actor in the ordered list,
            // sorted by DIS identifier.
            // @param newType The new MunitionTypeActor that was recently added to this
            //        table and that needs to be entered in a sorted list.
            //
            // NOTE: The sorted list is important in the quick comparison of DIS identifiers.
            void InsertMunitionTypeToOrderedList( SimCore::Actors::MunitionTypeActor& newType );

            // Remove a MunitionTypeActor from this table's sorted list before removal
            // from the table itself.
            // @param oldType The MunitionTypeActor to be removed from the sorted list.
            void RemoveMunitionTypeFromOrderedList( const SimCore::Actors::MunitionTypeActor& oldType );

         private:
            std::map<std::string, std::shared_ptr<SimCore::Actors::MunitionTypeActor> > mNameToMunitionMap;
            std::vector<std::shared_ptr<SimCore::Actors::MunitionTypeActor> > mOrderedList; // ordered list
      };

   }
}

#endif
