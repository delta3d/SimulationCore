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

//////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
//////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>

#include <dtGame/deadreckoningcomponent.h>

#include <SimCore/Components/DamageHelper.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Messages.h>

#include <SimCore/Actors/BaseEntity.h>

#include <dtUtil/mathdefines.h>

#include <dtCore/scene.h>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // DAMAGE HELPER CODE
      //////////////////////////////////////////////////////////////////////////
      DamageHelper::DamageHelper( bool autoNotifyNetwork )
         : mAutoNotifyNet(autoNotifyNetwork),
         mVulnerability(1.0f),
         mDamageModifier(0.0f),
         mLastNotifiedDamageState(&DamageType::DAMAGE_NONE),
         mCurrentDamageState(&DamageType::DAMAGE_NONE),
         mLastDRAlgorithm(NULL),
         mScratchProbs(new DamageProbability("")),
         mDamageLevels(new DamageProbability("DamageLevels"))
      {
         mDamageLevels->Set(0.85f,0.14f,0.0f,0.0f,0.01f);
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper::DamageHelper( SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork )
         : mAutoNotifyNet(autoNotifyNetwork),
         mVulnerability(1.0f),
         mDamageModifier(0.0f),
         mLastNotifiedDamageState(&DamageType::DAMAGE_NONE),
         mCurrentDamageState(&DamageType::DAMAGE_NONE),
         mLastDRAlgorithm(NULL),
         mEntity(&entity),
         mScratchProbs(new DamageProbability("")),
         mDamageLevels(new DamageProbability("DamageLevels"))
      {
         mDamageLevels->Set(0.85f,0.14f,0.0f,0.0f,0.01f);
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper::~DamageHelper()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      float DamageHelper::GetDamageProbability( bool directFire ) const
      {
         return directFire ? dtUtil::RandFloat(0.2,1.2) : dtUtil::RandFloat(0.5,1.2);
      }

      //////////////////////////////////////////////////////////////////////////
      DamageType& DamageHelper::GetGreaterDamageState( DamageType& damageToCompare ) const
      {
         if( mCurrentDamageState == NULL // NULL damage state should not happen
            || *mCurrentDamageState == DamageType::DAMAGE_NONE ) { return damageToCompare; }

         if( *mCurrentDamageState == DamageType::DAMAGE_KILL
            || damageToCompare == DamageType::DAMAGE_KILL )
         {
            return DamageType::DAMAGE_KILL;
         }

         if( *mCurrentDamageState == DamageType::DAMAGE_MOBILITY 
            && damageToCompare == DamageType::DAMAGE_FIREPOWER 
            || *mCurrentDamageState == DamageType::DAMAGE_FIREPOWER 
            && damageToCompare == DamageType::DAMAGE_MOBILITY )
         {
            return DamageType::DAMAGE_MOBILITY_FIREPOWER;
         }

         return *mCurrentDamageState;
      }

      //////////////////////////////////////////////////////////////////////////
      void DamageHelper::ProcessShotMessage( const ShotFiredMessage& message,
         const SimCore::Actors::MunitionTypeActor& munition, bool directFire )
      {
         if( ! mEntity.valid() || ! mTable.valid() ) { return; }

         // TODO: ShotFired processing. Currently efforts have been focused on detonations.
         // NOTE: This feature will involve more complex damage accumulation.

//         const MunitionDamage* munition = 
//            mTable->GetMunitionDamage( message.GetMunitionType().GetName() );

//         if( munition == NULL ) { return; }

         // TODO: Get quantity of bullets that hit
         // Run through each and modify integrity of vehicle

//         osg::Vec3 entityPos;
//         GetEntityPosition( entityPos );

//         munition->GetDamageProbabilities( *mScratchProbs, 
//            message.GetFinalVelocityVector(),
//            message.GetFiringLocation(),
//            entityPos );
      }

      //////////////////////////////////////////////////////////////////////////
      void DamageHelper::ProcessDetonationMessage( const DetonationMessage& message, 
         const SimCore::Actors::MunitionTypeActor& munition, bool directFire )
      {
         if( ! mEntity.valid() || ! mTable.valid() ) { return; }

         // Accumulate damage if the munition allows for damage accumulation.
         // NOTE: DO NOT accumulate on none explosive rounds nor unknown munitions.
         if( ! munition.GetFamily().IsExplosive() && ! directFire )
         {
            return;
         }

         const MunitionDamage* munitionDamage = mTable->GetMunitionDamage( munition.GetDamageType() );

         if( munitionDamage == NULL )
         {
            std::ostringstream ss;
            ss << "DamageHelper.ProcessDetonationMessage: Unable to locate munition \"" 
               << message.GetMunitionType().c_str() << "\"" << std::endl;
            LOG_DEBUG( ss.str() );
            return;
         }

         osg::Vec3 entityPos;
         GetEntityPosition( entityPos );

         // Notify the entity of the force
         osg::Vec3 force = munitionDamage->GetForce( entityPos, 
            message.GetDetonationLocation(), 
            message.GetFinalVelocityVector() );

         // Compute the probability of damage
         float distanceFromImpact = 0.0f;
         munitionDamage->GetDamageProbabilities( *mScratchProbs,
            distanceFromImpact,
            directFire,
            message.GetFinalVelocityVector(),
            message.GetDetonationLocation(),
            entityPos );

         // Apply damage and force only if the entity is within the range of effect.
         if( distanceFromImpact <= munitionDamage->GetCutoffRange() )
         {
            // Accumulate damage based on the probability numbers specified for the munition.
            // NOTE: Probabilities are treated as "actual damage" values rather than "chance damage" values.
            if( mDamageModifier < 1.0f )
            {
               mDamageModifier += (mScratchProbs->GetMobilityDamage()*0.5f + mScratchProbs->GetKillDamage()) 
                  * mVulnerability * this->GetDamageProbability(directFire)
                  * (message.GetQuantityFired()>0?message.GetQuantityFired():1.0f);
               if( mDamageModifier > 1.0f ) { mDamageModifier = 1.0f; }
            }

            // Update the entity's damage state
            // NOTE: Direct Fire will need greater damage returned from GetDamageType.
            SetDamage( GetGreaterDamageState( mDamageLevels->GetDamageType( mDamageModifier, false ) ) );
            
            // Notify the entity of an explosive force if one exists
            if( force.length2() > 0.0 )
            {
               std::stringstream ss;
               ss << "DamageHelper.ProcessDetonationMessage: Applying force of \"" 
                  << force.length() << "\"" << std::endl;
               LOG_DEBUG( ss.str() );
               mEntity->ApplyForce( force, message.GetDetonationLocation() );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool DamageHelper::NotifyNetwork()
      {
         if( mAutoNotifyNet
            && mLastNotifiedDamageState != mCurrentDamageState 
            && mEntity.valid() )
         {
            std::vector<std::string> propNames;
            propNames.push_back("Last Known Translation");
            propNames.push_back("Last Known Rotation");
            propNames.push_back("Velocity Vector");
            propNames.push_back("Acceleration Vector");
            propNames.push_back("Angular Velocity Vector");
            propNames.push_back("Dead Reckoning Algorithm");
            propNames.push_back("Damage State");
            mEntity->GetGameActorProxy().NotifyPartialActorUpdate(propNames);
            mLastNotifiedDamageState = mCurrentDamageState;
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void DamageHelper::SetDamage( DamageType& damage )
      {
         if( *mCurrentDamageState == damage || ! mEntity.valid() ) { return; }

         mCurrentDamageState = &damage;

         // Store dead reckoning algorithm
         if( dtGame::DeadReckoningAlgorithm::STATIC != 
            mEntity->GetDeadReckoningAlgorithm()
            && damage == DamageType::DAMAGE_KILL )
         {
            mLastDRAlgorithm = &mEntity->GetDeadReckoningAlgorithm();
         }

         if( damage == DamageType::DAMAGE_NONE )
         {
            mDamageModifier = 0.0f;
            mEntity->SetMobilityDisabled(false);
            mEntity->SetFirepowerDisabled(false);
            mEntity->SetDamageState( SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE );
            if( mLastDRAlgorithm != NULL )
            {
               mEntity->SetDeadReckoningAlgorithm( *mLastDRAlgorithm );
            }
            mEntity->SetFlamesPresent( false );
         }
         else if( damage == DamageType::DAMAGE_MOBILITY || damage == DamageType::DAMAGE_FIREPOWER )
         {
            if( damage == DamageType::DAMAGE_MOBILITY )
            {
               mEntity->SetMobilityDisabled(true);
            }
            else
            {
               mEntity->SetFirepowerDisabled(true);
            }
            mEntity->SetDamageState( SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE );
         }
         else if( damage == DamageType::DAMAGE_MOBILITY_FIREPOWER )
         {
            mEntity->SetMobilityDisabled(true);
            mEntity->SetFirepowerDisabled(true);
            mEntity->SetDamageState( SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE );
         }
         else if( damage == DamageType::DAMAGE_KILL )
         {
            mEntity->SetMobilityDisabled(true);
            mEntity->SetFirepowerDisabled(true);
            mEntity->SetDamageState( SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED );
            mEntity->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::STATIC );
            mEntity->SetFlamesPresent( true );
         }

         NotifyNetwork();
      }

      //////////////////////////////////////////////////////////////////////////
      bool DamageHelper::GetEntityPosition( osg::Vec3& outPos )
      {
         if( ! mEntity.valid() ) { return false; }

         dtCore::Transform trans;
         mEntity->GetTransform(trans);
         trans.GetTranslation(outPos);
         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      void DamageHelper::SetVulnerability( float vulnerability )
      {
         mVulnerability = vulnerability < 0.0 ? 0.0 : vulnerability > 1.0 ? 1.0 : vulnerability;
      }

   }
}
