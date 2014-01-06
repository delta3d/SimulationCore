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

#ifndef MUNITION_DAMAGE_TABLE_H
#define MUNITION_DAMAGE_TABLE_H

// High Explosives (HE) use: Carleton Damage Model
// Improved Conventional Munition (ICM) use: Cookie Cutter Model

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Components/MunitionDamage.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Components
   {
      class MunitionDamage;



      //////////////////////////////////////////////////////////////////////////
      // Munition Damage Table Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionDamageTable : public std::enable_shared_from_this
      {
         public:

            // Constructor 
            // Note that munition data is specific to the interaction of certain
            // munitions to a specific entity type; in other words, the same
            // munition can be specified for multiple entity classes but the
            // munition damage probabilities may not be the same, and usually
            // are not.
            // @param entityClassName The name of the table ought to be the name
            //        of the entity class that has munition damage probability data.
            //        NOTE: this name will be used for mapping in a MunitionsComponent.
            MunitionDamageTable( const std::string& entityClassName, bool isDefault = false );

            void SetName( const std::string& name );
            const std::string& GetName() const;

            unsigned int GetCount() const;

            // @return true if this is the default muntition table for things that don't declare a table.
            bool IsDefault() const;

            bool AddMunitionDamage( const std::shared_ptr<MunitionDamage>& newInfo );

            bool RemoveMunitionDamage( const std::string& name );

            bool HasMunitionDamage( const std::string& name ) const;

            const MunitionDamage* GetMunitionDamage( const std::string& name ) const;

            void Clear();

         protected:

            // Destructor
            virtual ~MunitionDamageTable();

         private:
            std::string mName;
            std::map<std::string, std::shared_ptr<MunitionDamage> > mNameToMunitionMap;
            bool mDefault;
      };

   }
}

#endif
