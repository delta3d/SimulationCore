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
* @author Chris Rodgers, Curtiss Murphy
*/

#ifndef _DAMAGE_HELPER_H_
#define _DAMAGE_HELPER_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtCore/observerptr.h>

namespace dtGame
{
   class DeadReckoningAlgorithm;
}

namespace SimCore
{
   class ShotFiredMessage;
   class DetonationMessage;

   namespace Actors
   {
      class BaseEntity;
      class MunitionTypeActor;
   }

   namespace Components
   {
      class MunitionDamageTable;
      class DamageType;
      class DamageProbability;

      //////////////////////////////////////////////////////////////////////////
      // DAMAGE HELPER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DamageHelper : public dtCore::Base
      {
         public:

            /** 
             * Constructor
             * @param autoNotifyNetwork Determines if this helper has permission to
             *        automatically send a network update when the observed entity's
             *        damage state has changed.
             * @param maxDamageAmount Allows other than default 1.0 damage value before being killed.
             */            
            explicit DamageHelper( bool autoNotifyNetwork = true, float maxDamageAmount = 1.0f);

            /** 
             * Constructor
             * @param entity The entity that this helper must observe
             * @param autoNotifyNetwork Determines if this helper has permission to
             *        automatically send a network update when the observed entity's
             *        damage state has changed.
             * @param maxDamageAmount Allows other than default 1.0 damage value before being killed.
             */            
            DamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork = true, 
               float maxDamageAmount = 1.0f);

            void SetEntity( SimCore::Actors::BaseEntity& entity ) { mEntity = &entity; }
            // @return The observed entity to which this DamageHelper is devoted.
            const SimCore::Actors::BaseEntity* GetEntity() const { return mEntity.get(); }

            // @param dimensions The dimensions of the entity to which impact points of fire
            //        will be compared.
            void SetEntityDimensions( const osg::Vec3& dimensions ) { mEntityDimensions = dimensions; }
            // @return The dimensions of the observed entity that was set on this helper
            const osg::Vec3& GetEntityDimensions() const { return mEntityDimensions; }

            // @param outPos The argument to capture the observed entity's position in world space
            // @return FALSE if this function failed to retrieve the observed entity's position; 
            //         fails if the observed entity is NULL
            bool GetEntityPosition( osg::Vec3& outPos );

            // @param autoNotify Determines if this helper has permission to
            //        automatically send a network update when the observed entity's
            //        damage state has changed.
            void SetAutoNotifyNetwork( bool autoNotify ) { mAutoNotifyNet = autoNotify; }
            bool GetAutoNotifyNetwork() const { return mAutoNotifyNet; }

            // A munition table must be set in order for this helper to determine the
            // observed entity's damage. The table contains information on each munition
            // or munition type that affects the entity's damage state. Each MunitionDamage
            // holds the damage probabilities that are specific to it and the entity;
            // in other words, the probabilities may not be the same for another type of
            // entity that is hit by the same munition.
            //
            // NOTE: This object merely observes the table and is NOT responsible for
            //       handling it memory. Tables can be shared and may need to exist
            //       outside the scope of helpers.
            //
            // @param table The collection of MunitionDamage that pertain to the observed entity.
            //              This is a ref pointer to ensure that the table memory is being
            //              tracked rather than this function receiving a new raw table pointer.
            void SetMunitionDamageTable( dtCore::RefPtr<MunitionDamageTable>& table ) { mTable = table.get(); }
            MunitionDamageTable* GetMunitionDamageTable() { return mTable.get(); }
            const MunitionDamageTable* GetMunitionDamageTable() const { return mTable.get(); }

            /// This function sets the damage state property on the observed entity. Causes an actor update if AutoPublish is true
            void SetDamage( DamageType& damage );

            // @param damageToCompare The damage type to be compared with this 
            //        helper's current damage state/type
            // @return The greater damage type between the current damage type
            //         and the specified damage type
            DamageType& GetGreaterDamageState( DamageType& damageToCompare ) const;

            // @return The current damage state of the associated entity
            DamageType& GetDamageState() { return *mCurrentDamageState; }
            // @return The last damage state reported to the network by this helper
            DamageType& GetLastNotifiedDamageState() { return *mLastNotifiedDamageState; }

            // NOTE: Overrides of this function should call this version internally.
            //
            // @param message ShotFired message to be processed
            // @param munition The munition that closely matches the DIS held in the 
            //        message's MunitionType parameter.
            // @param directHit Determines if the shots are to be interpreted and handled as a direct hit
            virtual void ProcessShotMessage( const ShotFiredMessage& message,
               const SimCore::Actors::MunitionTypeActor& munition, bool directHit = false );

            // NOTE: Overrides of this function should call this version internally.
            //
            // @param message Detonation message to be processed
            // @param munition The munition that closely matches the DIS held in the 
            //        message's MunitionType parameter.
            // @param directHit Determines if the detonation is to be interpreted and handled as direct hit
            virtual void ProcessDetonationMessage( const DetonationMessage& message,
               const SimCore::Actors::MunitionTypeActor& munition, bool directHit = false );

            // Adjust the vulnerability of the entity taking damage.
            // @param vulnerability The vulnerability of the entity ranging from 0 to 1.
            //        0 means normal
            //
            // NOTE: The value is auto clamped to 0 or 1 for values outside these ranges.
            void SetVulnerability( float vulnerability );
            float GetVulnerability() const { return mVulnerability; }

            /** 
            * Access the current amount of damage - as a normalized ratio. 
            * Starts at 0 - hits add to this until the entity is dead at 1.0. 
            * Entity damage is added as a scaled value based on GetMaxDamageAmount().
            */
            float GetCurrentDamageRatio() const { return mCurDamageRatio; }
            void SetCurrentDamageRatio(float newValue) { mCurDamageRatio = newValue; }
            //void SetDamageProbabilityModifier( float modifier ) { mDamageModifier = modifier; }
            //float GetDamageProbabilityModifier() const { return mDamageModifier; }

            /// Max damage amount is the non-normalized maximum damage an entity can take. Default is 1.0.
            void SetMaxDamageAmount(float newValue) { mMaxDamageAmount = newValue; }
            float GetMaxDamageAmount() const { return mMaxDamageAmount; }
            

            // Get the damage levels used by this helper.
            // Damage levels determine the type of damage to assign when damage
            // accumulates from 0.0 up to 1.0.
            DamageProbability& GetDamageLevels() { return *mDamageLevels; }
            const DamageProbability& GetDamageLevels() const { return *mDamageLevels; }

            // NOTE: This function's default behavior is generating a random number.
            //       This function should be overridden if specific probabilities
            //       are required in such cases as damage accumulation testing.
            //
            // @param directFire Determines which one of two preset ranges
            //        to be used for generating a random number.
            // @return random float value between numbers in preset ranges.
            virtual float GetDamageProbability( bool directFire ) const;

            // Determine if the entity is capable of taking any type of damage.
            bool IsVulnerable() const { return mVulnerability > 0.0f; }

            // @return TRUE if the message was sent to the network
            bool NotifyNetwork();

         protected:

            // Destructor
            virtual ~DamageHelper();

         private:

            bool mAutoNotifyNet;
            float mVulnerability;
            float mCurDamageRatio; //mDamageModifier;
            float mLastNotifiedDamageRatio; 
            float mMaxDamageAmount; // Default is 1.0
            DamageType* mLastNotifiedDamageState;
            DamageType* mCurrentDamageState;
            dtGame::DeadReckoningAlgorithm* mLastDRAlgorithm;
            dtCore::ObserverPtr<MunitionDamageTable> mTable;
            dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mEntity; // the observed entity
            dtCore::RefPtr<DamageProbability> mScratchProbs;
            dtCore::RefPtr<DamageProbability> mDamageLevels; // the damage probabilities for individual levels of damage of the entity
            osg::Vec3 mEntityDimensions;

      };

   }
}

#endif
