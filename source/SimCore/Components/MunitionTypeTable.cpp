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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <iosfwd>
#include <sstream>
#include <dtUtil/log.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Components/MunitionsComponent.h>



using std::shared_ptr;

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Munition Type Table Code
      //////////////////////////////////////////////////////////////////////////
      MunitionTypeTable::MunitionTypeTable()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionTypeTable::~MunitionTypeTable()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionTypeTable::GetCount() const
      {
         return mNameToMunitionMap.size();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionTypeTable::AddMunitionType( const std::shared_ptr<SimCore::Actors::MunitionTypeActorProxy>& newType )
      {
         if( ! newType.valid() ) { return false; }

         const std::string& munitionName = newType->GetName();
         if( HasMunitionType( munitionName ) )
         {
            return false;
         }

         SimCore::Actors::MunitionTypeActor* actor = 
            dynamic_cast<SimCore::Actors::MunitionTypeActor*>(newType->GetDrawable());

         if( actor == nullptr )
         {
            std::ostringstream ss;
            ss << "MunitionType \""
               << munitionName.c_str()
               <<"\" does not have a valid actor."
               << std::endl;
            LOG_ERROR( ss.str() );
            return false;
         }

         // Ensure the actor has the same name
         actor->SetName( munitionName );

         bool success = mNameToMunitionMap.insert( 
            std::make_pair( munitionName, actor ) 
            ).second;

         if( !success )
         {
            std::ostringstream ss;
            ss << "Failure adding MunitionType "
               << munitionName.c_str()
               <<". MunitionType may have already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
         }
         else
         {
            InsertMunitionTypeToOrderedList( *actor );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionTypeTable::RemoveMunitionType( const std::string& name )
      {
         std::map< std::string, std::shared_ptr<SimCore::Actors::MunitionTypeActor> >::iterator iter = 
            mNameToMunitionMap.find( name );

         if( iter != mNameToMunitionMap.end() )
         {
            RemoveMunitionTypeFromOrderedList( *(iter->second) );
            mNameToMunitionMap.erase( iter );
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionTypeTable::HasMunitionType( const std::string& name ) const
      {
         return mNameToMunitionMap.find( name ) != mNameToMunitionMap.end();
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionType( const std::string& name )
      {
         std::map< std::string, std::shared_ptr<SimCore::Actors::MunitionTypeActor> >::const_iterator iter = 
            mNameToMunitionMap.find( name );

         return iter != mNameToMunitionMap.end() ? iter->second.get() : nullptr;
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionType( const std::string& name ) const
      {
         std::map< std::string, std::shared_ptr<SimCore::Actors::MunitionTypeActor> >::const_iterator iter = 
            mNameToMunitionMap.find( name );

         return iter != mNameToMunitionMap.end() ? iter->second.get() : nullptr;
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionTypeByDIS( 
         const std::string& dis, bool exactMatch ) const
      {
         // Convert the string to a comparable DIS object
         SimCore::Actors::DISIdentifier matchDis;
         matchDis.SetByString( dis );
         return GetMunitionTypeByDIS( matchDis, exactMatch );
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionTypeTable::GetMunitionTypeByDIS( 
         const SimCore::Actors::DISIdentifier& dis, bool exactMatch ) const
      {
         // Define variables to be used in the loop
         const SimCore::Actors::MunitionTypeActor* closestType = nullptr;
         unsigned int matchLevel = 0;
         unsigned int curMatchLevel = 0;
         std::vector<std::shared_ptr<SimCore::Actors::MunitionTypeActor> >::const_iterator iter, iterEnd;
         iter = mOrderedList.begin();
         iterEnd = mOrderedList.end();

         // Iterate through DIS levels
         for( ; iter != iterEnd; ++iter )
         {
            SimCore::Actors::MunitionTypeActor& mta = **iter;
            // Match current level
            curMatchLevel = dis.GetDegreeOfMatch( mta.GetDISIdentifier() );

            // Return immediately if this is a full match
            if( curMatchLevel == 7 ) { return &mta; }

            // Capture the first closest match
            if( matchLevel <= curMatchLevel ) // "=" has been put in for a just-in-case situation, though it should not be necessary
            {
               matchLevel = curMatchLevel; // "raise the bar" for matching

               // Only capture a close match if the current munition's DIS
               // has a zero for the next
               if( mta.GetDISIdentifier().GetNumber( matchLevel ) == 0 )
               {
                  closestType = &mta;
               }
            }
            // If match level is decreasing, no further checks will produce a higher match.
            // NOTE: This is under heavy assumption that the list is properly sorted.
            else if( matchLevel > curMatchLevel )
            {
               break;
            }
         }

         // If an exact match is requested and the match level is not 7
         // (there are 7 numbers in a DIS identifier), then nullptr should be returned.
         if( closestType == nullptr || ( exactMatch && matchLevel < 7 ) )
         {
            std::ostringstream ss;
            ss << "Could not find a munition that matches DIS " << dis.ToString();
            if (closestType != nullptr)
               ss << "closest is " << closestType->GetDISIdentifierString();
               
            LOG_WARNING( ss.str() );
            return nullptr;
         }

         return closestType;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeTable::Clear()
      {
         mNameToMunitionMap.clear();
         mOrderedList.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeTable::InsertMunitionTypeToOrderedList( SimCore::Actors::MunitionTypeActor& newType )
      {
         if( mOrderedList.empty() )
         {
            mOrderedList.push_back( &newType );
            return;
         }

         bool inserted = false;
         std::vector<std::shared_ptr<SimCore::Actors::MunitionTypeActor> >::iterator iter = mOrderedList.begin();
         const SimCore::Actors::DISIdentifier& dis = newType.GetDISIdentifier();
         for( ; iter != mOrderedList.end(); ++iter )
         {
            if( (*iter)->GetDISIdentifier() >= dis )
            {
               mOrderedList.insert(iter,&newType);
               inserted = true;
               return;
            }
         }

         if( ! inserted )
         {
            mOrderedList.push_back( &newType );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeTable::RemoveMunitionTypeFromOrderedList( const SimCore::Actors::MunitionTypeActor& oldType )
      {
         std::vector<std::shared_ptr<SimCore::Actors::MunitionTypeActor> >::iterator iter = mOrderedList.begin();
         const SimCore::Actors::DISIdentifier& dis = oldType.GetDISIdentifier();
         const std::string& name = oldType.GetName();
         for( ; iter != mOrderedList.end(); ++iter )
         {
            if( (*iter)->GetName() == name && (*iter)->GetDISIdentifier() == dis )
            {
               mOrderedList.erase(iter);
               return;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionTypeTable::GetOrderedListSize() const
      {
         return mOrderedList.size();
      }

   }
}
