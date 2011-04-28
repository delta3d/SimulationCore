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
#include <dtUtil/refstring.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Components/MunitionDamageTable.h>
#include <SimCore/Components/MunitionTypeTable.h>
#include <SimCore/Components/WeaponEffectsManager.h>
#include <dtGame/gmcomponent.h>
#include <deque>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
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
   class BaseWeaponEventMessage;
   class DetonationMessage;
   class ShotFiredMessage;

   namespace Actors
   {
      class MunitionEffectsInfoActor;
      class DetonationActorProxy;
   }

   namespace Components
   {
      class DamageType;
      class DamageHelper;



      //////////////////////////////////////////////////////////////////////////
      // Munitions Component Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionsComponent : public dtGame::GMComponent
      {
         public:

            static const dtUtil::RefString DEFAULT_NAME;

            static const std::string CONFIG_PROP_MUNITION_DEFAULT;
            static const std::string CONFIG_PROP_MUNITION_KINETIC_ROUND_DEFAULT;

            /// Constructor
            MunitionsComponent( const std::string& name = DEFAULT_NAME.Get() );

            /** 
             * Register a local entity to automatically take damage from direct & indirect munitions
             * Override the ValidateIncomingDamage() or RespondToHit() methods if you want to do 
             * something custom when you get hit from a munition. 
             * @param autoNotifyNetwork Defaults to true. Sends actor update when the entity takes damage.
             * @param maxDamageAmount Lets an entity take more than the default 1.0 damage before dying
             */
            bool Register( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork = true, 
               float maxDamageAmount = 1.0f);

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

            MunitionTypeTable* GetMunitionTypeTable() { return mMunitionTypeTable.get(); }
            const MunitionTypeTable* GetMunitionTypeTable() const { return mMunitionTypeTable.get(); }

            void ClearTables();

            virtual void ProcessMessage( const dtGame::Message& msg );
            
            DamageHelper* GetHelperByEntityId( const dtCore::UniqueId& id );

            void SetDamage( SimCore::Actors::BaseEntity& entity, DamageType& damage );
            
            void ApplyShotfiredEffects( const ShotFiredMessage& message,
               const SimCore::Actors::MunitionTypeActor& munitionType );

            void ApplyDetonationEffects( const DetonationMessage& message,
               const SimCore::Actors::MunitionTypeActor& munitionType );

            virtual void OnAddedToGM();

            /**
             * This function retrieves the name of the MunitionDamage object that
             * is mapped to the MunitionTypeActor of name munitionTypeName
             *  @param munitionTypeName The name of the MunitionTypeActor
             * @return The name of the MunitionDamage that the MunitionTypeActor references.
             */
            const std::string& GetMunitionDamageTypeName( const std::string& munitionTypeName ) const;

            /**
             * Set the default munition that will be used in place of any detonations
             * for which the associated munition is not found in the munition table.
             * @param munitionName Name of the munition to use for the default detonation effect.
             */
            DT_DECLARE_ACCESSOR(std::string, DefaultMunitionName);

            /**
             * Set the default munition that will be used in place of any detonations
             * for which the associated munition is not found in the munition table.
             * @param munitionName Name of the munition to use for the default detonation effect.
             */
            DT_DECLARE_ACCESSOR(std::string, DefaultKineticRoundMunitionName);

            /**
             * Set the munition config file that will be loaded whenever the system is restarted.
             * It defaults to "Configs:MunitionsConfig.xml". If you want to change it, you MUST
             * call this method BEFORE the RESTART message is sent.
             * @param munitionConfigFileName The config file name that is loaded/reloaded on a restart.
             */
            DT_DECLARE_ACCESSOR(std::string, MunitionConfigFileName);

            /**
             * Encapsulates the complexity of picking a munition based on the message sent in.
             * It will allow using one of the defaults set if no exact match is found, and it can use
             * any data on the weapon message to figure out what the munition is.  Also, the method may be overridden
             * so more complicated work can be done with it.
             */
            virtual const SimCore::Actors::MunitionTypeActor* FindMunitionForMessage(const SimCore::BaseWeaponEventMessage& messageData) const;

            /**
             * Gets a munition from the contained munition table.
             * @param munitionName Name of the munition to be retrieved from the munition type table.
             * @param defaultMunitionName Name of a default munition to be returned if the first munition could not be found.
             * @return the specified munition OR a default munition if default munition name has been specified
             *         OR NULL if neither a match or a default could be found.
             */
            const SimCore::Actors::MunitionTypeActor* GetMunition(
               const std::string& munitionName, const std::string& defaultMunitionName = "" ) const;

            /**
             * Convenience method for directly retrieving munition effects info from the contained munition table.
             * @param munition Munition with effects to be verified and returned.
             * @param defaultMunitionName Name of a default munition to be returned if the first munition could not be found.
             * @return effects from the specified munition OR default munition effects if default munition name has been specified
             *         OR NULL if neither munition has a valid effect nor a default could be found.
             */
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

            /**
             * Returns the maximum number of munitions that are allowed to be active. This is a performance
             * setting. The Munitions Component will track created munitions. When it needs to create a new
             * munition, it will first check that it is not over this count. If there are too many, then it
             * will remove the oldest munitions.
             */
            DT_DECLARE_ACCESSOR(unsigned, MaximumActiveMunitions);

            void ConvertMunitionInfoActorsToDetonationActors(const std::string& mapName);

         protected:

            // Destructor
            virtual ~MunitionsComponent();

            void InitMunitionTypeTable();
            void SetMunitionTypeTable( dtCore::RefPtr<MunitionTypeTable>& table ) { mMunitionTypeTable = table.get(); }

            virtual DamageHelper* CreateDamageHelper(SimCore::Actors::BaseEntity& entity, 
               bool autoNotifyNetwork, float maxDamageAmount);

            void OnDetonation(const dtGame::Message& msg);
            void OnShotFired(const dtGame::Message& msg);

            void RunIsectorForFIDCodes(bool hitEntity, const DetonationMessage& message, int fidID);
            dtCore::RefPtr<SimCore::Actors::DetonationActorProxy> CreateDetonationPrototype(const DetonationMessage& message);

            void ConvertSingleMunitionInfo(SimCore::Actors::MunitionEffectsInfoActorProxy& infoProxy, SimCore::Actors::DetonationActorProxy& detonationProxy);

         private:

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
            //float mLastDetonationTime;

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

      };

   }
}

#endif
