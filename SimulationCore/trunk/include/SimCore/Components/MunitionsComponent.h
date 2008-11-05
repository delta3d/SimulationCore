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

#ifndef MUNITIONS_COMPONENT_H
#define MUNITIONS_COMPONENT_H

// High Explosives (HE) use: Carleton Damage Model
// Improved Conventional Munition (ICM) use: Cookie Cutter Model

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtCore/base.h>
#include <dtCore/batchisector.h>
#include <dtCore/observerptr.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Components/MunitionDamage.h>
#include <SimCore/Components/WeaponEffectsManager.h>
#include <dtGame/gmcomponent.h>
#include <deque>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtAudio
{
   class Sound;
}

namespace dtCore
{
   class UniqueId;
   class ParticleSystem;
}
namespace dtGame
{
   class Message;
}

namespace SimCore
{
   class DetonationMessage;
   class ShotFiredMessage;

   namespace Actors
   {
      class DISIdentifier;
      class MunitionTypeActorProxy;
      class MunitionEffectsInfoActor;
   }

   namespace Components
   {
      class DamageType;
      class DamageHelper;

      //////////////////////////////////////////////////////////////////////////
      // Munition Type Table Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionTypeTable : public dtCore::Base
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
         bool AddMunitionType( const dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy>& newType );

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
         //         NULL if no MunitionTypeActor was found with the name.
         SimCore::Actors::MunitionTypeActor* GetMunitionType( const std::string& name );
         const SimCore::Actors::MunitionTypeActor* GetMunitionType( const std::string& name ) const;

         // This function makes a closest match. DIS is not guaranteed to be exact.
         // @param dis The DIS identifier used to find a MunitionTypeActor with
         //        the closest matching DIS identifier
         // @param exactMatch Determine if this function should perform an exact match.
         // @return The MunitionTypeActor with the closest or exactly matching DIS
         //         identifier as that specified by the parameter dis.
         //         NULL if there was no match.
         const SimCore::Actors::MunitionTypeActor* GetMunitionTypeByDIS( const SimCore::Actors::DISIdentifier& dis, bool exactMatch = false ) const;
         const SimCore::Actors::MunitionTypeActor* GetMunitionTypeByDIS( const std::string& dis, bool exactMatch = false ) const;

         // Access this table's sorted list directly.
         // @return The list of MunitionTypeActors sorted by DIS identifiers,
         //         from low to high numbers (identifier numbers are read from left to right).
         const std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> >& GetOrderedList() const { return mOrderedList; }

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
         std::map<std::string, dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> > mNameToMunitionMap;
         std::vector<dtCore::RefPtr<SimCore::Actors::MunitionTypeActor> > mOrderedList; // ordered list
      };



      //////////////////////////////////////////////////////////////////////////
      // Munition Damage Table Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionDamageTable : public dtCore::Base
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
            MunitionDamageTable( const std::string& entityClassName );

            unsigned int GetCount() const;

            bool AddMunitionDamage( const dtCore::RefPtr<MunitionDamage>& newInfo );

            bool RemoveMunitionDamage( const std::string& name );

            bool HasMunitionDamage( const std::string& name ) const;

            const MunitionDamage* GetMunitionDamage( const std::string& name ) const;

            void Clear();

         protected:

            // Destructor
            virtual ~MunitionDamageTable();

         private:
            std::map<std::string, dtCore::RefPtr<MunitionDamage> > mNameToMunitionMap;
      };



      //////////////////////////////////////////////////////////////////////////
      // Munitions Component Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionsComponent : public dtGame::GMComponent
      {
         public:

            static const std::string DEFAULT_NAME;

            /// Constructor
            MunitionsComponent( const std::string& name = DEFAULT_NAME );

            bool Register( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork = true );

            bool Unregister( const dtCore::UniqueId& entityId );

            bool HasRegistered( const dtCore::UniqueId& entityId ) const;

            void ClearRegisteredEntities();

            // Calls upon a MunitionConfig parser to pull in data about munitions
            // per entity specified in entityClassName.
            // @param munitionConfigPath The file path to the MunitionsConfig.xml
            // @return number of successfully loaded tables.
            unsigned int LoadMunitionDamageTables( const std::string& munitionConfigPath );

            // Searches for a table by name of an entity class.
            // @param entityClassName The name of the entity class that should have
            //        a table of munition data that was loaded from the MunitionsConfig.xml
            // @return a pointer to the munition table; NULL if it does not exist.
            MunitionDamageTable* GetMunitionDamageTable( const std::string& entityClassName );
            const MunitionDamageTable* GetMunitionDamageTable( const std::string& entityClassName ) const;

            // NOTE: Takes a ref pointer to ensure that there is an object
            // tracking the table's memory, in case addition of the table fails.
            bool AddMunitionDamageTable( dtCore::RefPtr<MunitionDamageTable>& table );

            // Remove a damage table associated with an entity type.
            // @param entityClassName The name of the damage table to be removed.
            //        The damage table shares the same name as the entity class
            //        declared within the MunitionsConfig.xml
            bool RemoveMunitionDamageTable( const std::string& entityClassName );

            // This function loads the table that contains all munition type data.
            // @param mapName The name of the map within the MunitionTypesMap.xml
            // @return the number of munition types successfully loaded to the table.
            unsigned int LoadMunitionTypeTable( const std::string& mapName );

            MunitionTypeTable* GetMunitionTypeTable() { return mMunitionTypeTable.get(); }
            const MunitionTypeTable* GetMunitionTypeTable() const { return mMunitionTypeTable.get(); }

            void ClearTables();

            virtual void ProcessMessage( const dtGame::Message& msg );
            
            DamageHelper* GetHelperByEntityId( const dtCore::UniqueId id );

            void SetDamage( SimCore::Actors::BaseEntity& entity, DamageType& damage );
            
            void ApplyShotfiredEffects( const ShotFiredMessage& message,
               const SimCore::Actors::MunitionTypeActor& munitionType );

            void ApplyDetonationEffects( const DetonationMessage& message,
               const SimCore::Actors::MunitionTypeActor& munitionType );

            virtual void OnAddedToGM();

            // This function retrieves the name of the MunitionDamage object that
            // is mapped to the MunitionTypeActor of name munitionTypeName
            // @param munitionTypeName The name of the MunitionTypeActor
            // @return The name of the MunitionDamage that the MunitionTypeActor references.
            const std::string& GetMunitionDamageTypeName( const std::string& munitionTypeName ) const;

            // Set the default munition that will be used in place of any detonations
            // for which the associated munition is not found in the munition table.
            // @param munitionName Name of the munition to use for the default detonation effect.
            void SetDefaultMunitionName( const std::string& munitionName );
            const std::string& GetDefaultMunitionName() const;

            // Set the munition config file that will be loaded whenever the system is restarted. 
            // It defaults to "Configs:MunitionsConfig.xml". If you want to change it, you MUST 
            // call this method BEFORE the RESTART message is sent.
            // @param munitionConfigFileName The config file name that is loaded/reloaded on a restart.
            void SetMunitionConfigFileName( const std::string& munitionConfigFileName ) { mMunitionConfigPath = munitionConfigFileName; }
            const std::string& GetMunitionConfigFileName() const { return mMunitionConfigPath; }

            // Convenience method for directly retrieving a munition from the contained munition table.
            // @param munitionName Name of the munition to be retrieved from the munition type table.
            // @param defaultMunitionName Name of a default munition to be returned if the first munition could not be found.
            // @return the specified munition OR a default munition if default munition name has been specified
            //         OR NULL if neither a match or a default could be found.
            const SimCore::Actors::MunitionTypeActor* GetMunition(
               const std::string& munitionName, const std::string& defaultMunitionName = "" ) const;

            // Convenience method for directly retrieving munition effects info from the contained munition table.
            // @param munition Munition with effects to be verified and returned.
            // @param defaultMunitionName Name of a default munition to be returned if the first munition could not be found.
            // @return effects from the specified munition OR default munition effects if default munition name has been specified
            //         OR NULL if neither munition has a valid effect nor a default could be found.
            const SimCore::Actors::MunitionEffectsInfoActor* GetMunitionEffectsInfo(
               const SimCore::Actors::MunitionTypeActor& munition, const std::string& defaultMunitionName = "" ) const;

            // Removes all entries from the queue of created munitions. Usually happens on map unload or restart.
            void ClearCreatedMunitionsQueue();

            // Walks through the munitions queue and cleans up unique id's that are no longer valid. Called 
            // periodically so that our count is more accurate. This method will also make sure 
            // that there are number of current active munitions is less than the Maximum allowed
            // (can be above if the maximum was changed recently).
            void CleanupCreatedMunitionsQueue();

            // Goes through the created munitions queue and removes the old munition from the 
            // list. This is called when we get a new munition and we are already at our cap.
            // Note, as a byproduct of this call. A REAL munition actor will get deleted from
            // the game manager. This is a huge cost, but is necessary to keep from running out of memory.
            void RemoveOldestMunitionFromQueue();

            // Adds a newly created munition id to the end of our created munitions queue. If there 
            // are too many in our queue, it calls CleanupCreatedMunitionsQueue().
            void AddMunitionToCreatedMunitionsQueue(const dtCore::UniqueId &uniqueId);

            // Returns the maximum number of munitions that are allowed to be active. This is a performance 
            // setting. The Munitions Component will track created munitions. When it needs to create a new 
            // munition, it will first check that it is not over this count. If there are too many, then it 
            // will remove the oldest munitions. 
            int GetMaximumActiveMunitions() {return mMaximumActiveMunitions; }

            // Returns the maximum number of munitions that are allowed to be active. This is a performance 
            // setting. The Munitions Component will track created munitions. When it needs to create a new 
            // munition, it will first check that it is not over this count. If there are too many, then it 
            // will remove the oldest munitions. Note. If you set a new max that is lower than the current 
            // number of munitions, it should immediately scale smaller.  
            void SetMaximumActiveMunitions(int newMax);

         protected:

            // Destructor
            virtual ~MunitionsComponent();

            void SetMunitionTypeTable( dtCore::RefPtr<MunitionTypeTable>& table ) { mMunitionTypeTable = table.get(); }

            virtual DamageHelper* CreateDamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork );

         private:

            // The path to the munition config file that contains all the damage
            // tables to be loaded.
            std::string mMunitionConfigPath;

            // The name of the default munition to be used in place of munition detonations
            // that have not been defined in the munition table. This also points to the 
            // default munition effects info object used to setup the detonation effect.
            std::string mDefaultMunitionName;

            // This map holds onto all damages helpers. Each damage helper is mapped
            // to the entity's ID of the entity that is needing the damage help.
            std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> > mIdToHelperMap;

            // This map is responsible for holding onto all munition tables.
            // Damage helpers will reference these tables only by observer pointers
            // so that they are not left holding the table and its memory.
            // This helps munitions tables stay "alive" and not fall out of scope.
            std::map<std::string, dtCore::RefPtr<MunitionDamageTable> > mNameToMunitionDamageTableMap;

            // This table is responsible for holding onto all data related to
            // a munition, such as DIS identifier, name and resource file paths
            // for visual and sound effects.
            //
            // NOTE: This table does NOT hold damage information, since damage
            // information is specific to the interaction between a certain entity
            // type and the munition. Such data is contained in MunitionDamageTables,
            // per entity type.
            //
            // A MunitionDamage and a MunitionTypeActor should share the same name
            // of the munition to which they refer. With this, one can obtain
            // all the data pertaining to a certain munition
            dtCore::RefPtr<MunitionTypeTable> mMunitionTypeTable;

            // This isector is used for ground clamping detonation particle effects.
            dtCore::RefPtr<dtCore::BatchIsector> mIsector;
            float mLastDetonationTime;

            // A reference to the player is needed so that the sound from detonations
            // can be offset properly; there is a sound delay for which to account.
            dtCore::ObserverPtr<SimCore::Actors::StealthActor> mPlayer;

            // The effects manager is used for placing weapon effects on remote
            // entities that identify themselves as firing objects.
            // Effects include gun flash, fire sounds and tracers.
            dtCore::RefPtr<WeaponEffectsManager> mEffectsManager;

            // A queue of all the munitions that the component has created. This is 
            // useful for when we get in trouble with particles and such. If we have too 
            // many munitions, then we can simply kill off the oldest. 
            std::deque<dtCore::UniqueId> mCreatedMunitionsQueue;
            int mMaximumActiveMunitions;

      };

   }
}

#endif
