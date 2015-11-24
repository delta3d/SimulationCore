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
#include <SimCore/Components/MunitionDamage.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <dtUtil/log.h>
#include <sstream>


using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Munition Damage Table Code
      //////////////////////////////////////////////////////////////////////////
      MunitionDamageTable::MunitionDamageTable( const std::string& name, bool isDefault )
         : mName(name)
         , mDefault(isDefault)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionDamageTable::~MunitionDamageTable()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionDamageTable::SetName( const std::string& name )
      {
         mName = name;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& MunitionDamageTable::GetName() const
      {
         return mName;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionDamageTable::GetCount() const
      {
         return mNameToMunitionMap.size();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::AddMunitionDamage( const dtCore::RefPtr<MunitionDamage>& newInfo )
      {
         if( ! newInfo.valid() ) { return false; }
         if( HasMunitionDamage( newInfo->GetName() ) ) { return false; }

         bool success = mNameToMunitionMap.insert( 
            std::make_pair( newInfo->GetName(), newInfo.get() ) 
            ).second;

         if( !success )
         {
            std::stringstream ss;
            ss << "Failure adding MunitionDamage "
               << newInfo->GetName().c_str()
               <<". MunitionDamage may have already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::RemoveMunitionDamage( const std::string& name )
      {
         std::map< std::string, dtCore::RefPtr<MunitionDamage> >::iterator iter = 
            mNameToMunitionMap.find( name );

         if( iter != mNameToMunitionMap.end() )
         {
            mNameToMunitionMap.erase( iter );
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::HasMunitionDamage( const std::string& name ) const
      {
         return mNameToMunitionMap.find( name ) != mNameToMunitionMap.end();
      }

      //////////////////////////////////////////////////////////////////////////
      const MunitionDamage* MunitionDamageTable::GetMunitionDamage( const std::string& name ) const
      {
         std::map< std::string, dtCore::RefPtr<MunitionDamage> >::const_iterator iter = 
            mNameToMunitionMap.find( name );

         return iter != mNameToMunitionMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionDamageTable::Clear()
      {
         mNameToMunitionMap.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionDamageTable::IsDefault() const
      {
         return mDefault;
      }

   }
}
