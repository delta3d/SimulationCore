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

//////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
//////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>

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
      DamageHelper::DamageHelper( bool autoNotifyNetwork, float maxDamageAmount)
         : mAutoNotifyNet(autoNotifyNetwork)
         , mVulnerability(1.0f)
         , mCurDamageRatio(0.0f) //mDamageModifier(0.0f),
         , mLastNotifiedDamageRatio(0.0f)
         , mMaxDamageAmount(maxDamageAmount)
         , mLastNotifiedDamageState(&DamageType::DAMAGE_NONE)
         , mCurrentDamageState(&DamageType::DAMAGE_NONE)
         //, mLastDRAlgorithm(NULL)
         , mScratchProbs(new DamageProbability(""))
         , mDamageLevels(new DamageProbability("DamageLevels"))
      {
         mDamageLevels->Set(0.85f,0.14f,0.0f,0.0f,0.01f);
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper::DamageHelper(SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork, float maxDamageAmount)
         : mAutoNotifyNet(autoNotifyNetwork)
         , mVulnerability(1.0f)
         , mCurDamageRatio(0.0f) //mDamageModifier(0.0f),
         , mLastNotifiedDamageRatio(0.0f)
         , mMaxDamageAmount(maxDamageAmount)
         , mLastNotifiedDamageState(&DamageType::DAMAGE_NONE)
         , mCurrentDamageState(&DamageType::DAMAGE_NONE)
         //, mLastDRAlgorithm(NULL)
         , mEntity(&entity)
         , mScratchProbs(new DamageProbability(""))
         , mDamageLevels(new DamageProbability("DamageLevels"))
      {
         mDamageLevels->Set(0.85f,0.14f,0.0f,0.0f,0.01f);
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper::~DamageHelper()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      float DamageHelper::GetDamageProbability( bool directHit ) const
      {
         return directHit ? dtUtil::RandFloat(0.2,1.2) : dtUtil::RandFloat(0.5,1.2);
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
         const SimCore::Actors::MunitionTypeActor& munition, bool directHit )
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
         const SimCore::Actors::MunitionTypeActor& munition, bool directHit )
      {
         if( ! mEntity.valid() || ! mTable.valid() ) { return; }

         // Accumulate damage if the munition allows for damage accumulation.
         // NOTE: DO NOT accumulate on none explosive rounds nor unknown munitions.
         if( ! munition.GetFamily().IsExplosive() && ! directHit )
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
            message.GetDetonationLocation(), message.GetFinalVelocityVector() );

         // Compute the probability of damage
         float distanceFromImpact = 0.0f;
         munitionDamage->GetDamageProbabilities( *mScratchProbs, distanceFromImpact, 
            mEntityDimensions, directHit, message.GetFinalVelocityVector(), 
            message.GetDetonationLocation(), entityPos );

         // Apply damage and force only if the entity is within the range of effect.
         if( distanceFromImpact <= munitionDamage->GetCutoffRange() )
         {
            // Accumulate damage based on the probability numbers specified for the munition.
            // NOTE: Probabilities are treated as "actual damage" values rather than "chance damage" values.
            if( mCurDamageRatio < 1.0f )
            {
               float quantityMultiplier = (message.GetQuantityFired() > 0) ? message.GetQuantityFired() : 1.0f;
               float damageFromMunition = (mScratchProbs->GetMobilityDamage()*0.5f + mScratchProbs->GetKillDamage()) 
                  * mVulnerability * GetDamageProbability(directHit) * quantityMultiplier;

               // The new damage is scaled based on the max damage possible for this entity
               // In the end, we only store the ratio.
               float scaledDamageFromMunition = damageFromMunition / mMaxDamageAmount;
               // Allow the entity to filter out or modify some types of damage
               mCurDamageRatio += mEntity->ValidateIncomingDamage(scaledDamageFromMunition, message, munition);
               if( mCurDamageRatio > 1.0f ) { mCurDamageRatio = 1.0f; }
            }

            // Update the entity's damage state
            // NOTE: Direct Fire will need greater damage returned from GetDamageType.
            SetDamage(GetGreaterDamageState(mDamageLevels->GetDamageType(mCurDamageRatio, false)));
            
            // Give the entity a chance to react and apply forces. Damage has already been applied & published.
            mEntity->RespondToHit(message, munition, force, message.GetDetonationLocation());
         } 
      }

      //////////////////////////////////////////////////////////////////////////
      bool DamageHelper::NotifyNetwork()
      {
         if(mAutoNotifyNet && mEntity.valid() && 
            (mLastNotifiedDamageState != mCurrentDamageState || mLastNotifiedDamageRatio != mCurDamageRatio))
         {
            // These old props were removed for efficiency. However, it is possible HLA requires them on an update
            // Once confirmed that this works well on all networks, this cruft can be deleted
            //propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_LAST_KNOWN_TRANSLATION);
            //propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_LAST_KNOWN_ROTATION);
            //propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_VELOCITY_VECTOR);
            //propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_ACCELERATION_VECTOR);
            //propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_ANGULAR_VELOCITY_VECTOR);
            //propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_DEAD_RECKONING_ALGORITHM);

            std::vector<dtUtil::RefString> propNames;
            propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_DAMAGE_STATE);
            propNames.push_back(SimCore::Actors::BaseEntityActorProxy::PROPERTY_CUR_DAMAGE_RATIO);
            mEntity->GetGameActorProxy().NotifyPartialActorUpdate(propNames);

            mLastNotifiedDamageState = mCurrentDamageState;
            mLastNotifiedDamageRatio = mCurDamageRatio;
            return true;
         }

         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void DamageHelper::SetDamage( DamageType& damage )
      {
         bool dataChanged = false;
         if(!mEntity.valid()) { return; }

         if (mEntity->GetCurDamageRatio() != mCurDamageRatio)
         {
            mEntity->SetCurDamageRatio(mCurDamageRatio);
            dataChanged = true;
         }

         if (*mCurrentDamageState != damage)
         {
            dataChanged = true;
            mCurrentDamageState = &damage;

            // We no longer deal with the LAST DR algorithm because even if the vehicle is DEAD
            // it could still be falling or being moved by other things (such as being hit or a munition)
            // If we stop DR'ing, then we mess up our publishing also.
            // Store dead reckoning algorithm
            //if( dtGame::DeadReckoningAlgorithm::STATIC != 
            //   mEntity->GetDeadReckoningAlgorithm()
            //   && damage == DamageType::DAMAGE_KILL )
            //{
            //   mLastDRAlgorithm = &mEntity->GetDeadReckoningAlgorithm();
            //}

            if( damage == DamageType::DAMAGE_NONE )
            {
               mCurDamageRatio = 0.0f;
               mEntity->SetMobilityDisabled(false);
               mEntity->SetFirepowerDisabled(false);
               mEntity->SetDamageState( SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE );
               //if( mLastDRAlgorithm != NULL )
               //{
               //   mEntity->GetDeadReckoningHelper().SetDeadReckoningAlgorithm( *mLastDRAlgorithm );
               //}
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
               mCurDamageRatio = 1.0;
               mEntity->SetMobilityDisabled(true);
               mEntity->SetFirepowerDisabled(true);
               mEntity->SetDamageState( SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED );
               // We no longer turn off the DR algorithm when we are dead. The reason is that we could 
               // be pushed around, or falling (if a plane gets shot, it falls down) even though we are dead
               //mEntity->GetDeadReckoningHelper().SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::STATIC );

               // This checks for the property.  The reason is that removing the property is the way we stopped external sims
               // from wantonly setting humans to be on fire.  Thus I check it here also to be consistent since this code would otherwise
               // have the same bug.  
               if (NULL != mEntity->GetGameActorProxy().GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_FLAMES_PRESENT))
               {
                  mEntity->SetFlamesPresent( true );
               }
            }
         }

         // Attempt to notify the network. It will also check for publish changes
         if (dataChanged) 
         {
            NotifyNetwork();
         }
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
