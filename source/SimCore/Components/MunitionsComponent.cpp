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
#include <dtUtil/mswin.h>
#include <algorithm>
// DELTA 3D
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtCore/camera.h>
#include <dtCore/transformable.h>
#include <dtCore/scene.h>
#include <dtCore/project.h>
#include <dtCore/map.h>
#include <dtGame/basemessages.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/gamemanager.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/configproperties.h>

// SIM Core
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
// Components
#include <SimCore/Components/DamageHelper.h>
#include <SimCore/Components/MunitionDamage.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/MunitionsConfig.h>
#include <SimCore/Components/ViewerMaterialComponent.h>
// Actors
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PagedTerrainPhysicsActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/Human.h>

using dtCore::RefPtr;

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Munitions Component Code
      //////////////////////////////////////////////////////////////////////////
      const std::string MunitionsComponent::CONFIG_PROP_MUNITION_DEFAULT("DefaultMunition");
      const std::string MunitionsComponent::CONFIG_PROP_MUNITION_KINETIC_ROUND_DEFAULT("DefaultKineticRoundMunition");

      //////////////////////////////////////////////////////////////////////////
      MunitionsComponent::MunitionsComponent( dtCore::SystemComponentType& type )
         : dtGame::GMComponent(type)
         , mMunitionConfigFileName("Configs:MunitionsConfig.xml")
         , mMaximumActiveMunitions(200U)
         , mMunitionTypeTable(new MunitionTypeTable())
         , mIsector(new dtCore::BatchIsector)
         , mEffectsManager(new WeaponEffectsManager)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionsComponent::~MunitionsComponent()
      {
         ClearCreatedMunitionsQueue();
         ClearRegisteredEntities();
         ClearTables();
         if( mMunitionTypeTable.valid() ) { mMunitionTypeTable->Clear(); }
      }

      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(MunitionsComponent, std::string, DefaultMunitionName);
      DT_IMPLEMENT_ACCESSOR(MunitionsComponent, std::string, DefaultKineticRoundMunitionName);
      DT_IMPLEMENT_ACCESSOR(MunitionsComponent, std::string, MunitionConfigFileName);
      DT_IMPLEMENT_ACCESSOR_GETTER(MunitionsComponent, unsigned, MaximumActiveMunitions);

      //////////////////////////////////////////////////////////////////////////
      DamageHelper* MunitionsComponent::CreateDamageHelper( SimCore::Actors::BaseEntity& entity,
         bool autoNotifyNetwork, float maxDamageAmount)
      {
         return new DamageHelper(entity, autoNotifyNetwork, maxDamageAmount);
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::Register(SimCore::Actors::BaseEntity& entity, bool autoNotifyNetwork, float maxDamageAmount)
      {
         if( HasRegistered( entity.GetUniqueId() ) )
         {
            std::stringstream ss;
            ss << "Entity "
               << entity.GetUniqueId().ToString().c_str()
               <<" has already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
            return false;
         }

         DamageHelper* newHelper = CreateDamageHelper(entity, autoNotifyNetwork, maxDamageAmount);

         if( newHelper == NULL ) { return false; }

         bool success = mIdToHelperMap.insert(
               std::make_pair( entity.GetUniqueId(), newHelper )
            ).second;

         if( !success )
         {
            std::stringstream ss;
            ss << "FAILURE: Munition Component registering entity \""
               << entity.GetUniqueId().ToString().c_str()
               << "\" of class type \"" << entity.GetGameActorProxy().GetClassName()
               <<"\" for damage tracking. Entity may have already been registered."
               << std::endl;
            LOG_WARNING( ss.str() );
         }
         else if (dtUtil::Log::GetInstance().IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            std::stringstream ss;
            ss << "Munition Component registered entity \""
               << entity.GetUniqueId().ToString().c_str()
               << "\" of class type \"" << entity.GetGameActorProxy().GetClassName()
               << "\"" << std::endl;
            LOG_DEBUG( ss.str() );
         }

         // Load the munition table that the helper will need to reference, if one exists
         std::string tableName(entity.GetMunitionDamageTableName());

         if (tableName.empty())
         {
            tableName = mDefaultDamageTableName;
         }

         if( ! tableName.empty() )
         {
            // If the table exists, link it to the newly created helper
            dtCore::RefPtr<MunitionDamageTable> table = GetMunitionDamageTable( tableName );
            newHelper->SetMunitionDamageTable( table );

            std::stringstream ss;
            if( table.valid() )
            {
               ss << "\tLoaded munition damage table \"" << tableName.c_str()
                  << "\"" << std::endl;
               LOG_DEBUG( ss.str() );
            }
            else
            {
               ss << "FAILURE: Munition damage table \"" << tableName.c_str()
                  << "\" could not be found." << std::endl;
               LOG_ERROR( ss.str() );
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::Unregister( const dtCore::UniqueId& entityId )
      {
         std::map< dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator itor =
            mIdToHelperMap.find( entityId );

         if( itor != mIdToHelperMap.end() )
         {
            mIdToHelperMap.erase( itor );
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::HasRegistered( const dtCore::UniqueId& entityId ) const
      {
         return mIdToHelperMap.find( entityId ) != mIdToHelperMap.end();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ClearRegisteredEntities()
      {
         mIdToHelperMap.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionsComponent::LoadMunitionDamageTables( const std::string& munitionConfigPath )
      {
         // Check just in case - no need to error out though. This happens sometimes in unit tests.
         if (munitionConfigPath.empty())
         {
            return 0 ;
         }

         // Find the specified file
         std::string resourcePath = dtCore::Project::GetInstance()
            .GetResourcePath(dtCore::ResourceDescriptor( munitionConfigPath ));
         if( resourcePath.empty() )
         {
            std::stringstream ss;
            ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could not locate \""
               << munitionConfigPath.c_str() << "\"" << std::endl;
            LOG_ERROR( ss.str() );
            return 0;
         }

         // Capture new tables in a vector
         std::vector<dtCore::RefPtr<MunitionDamageTable> > tables;

         // Parse the new table data
         dtCore::RefPtr<MunitionsConfig> mParser = new MunitionsConfig();
         unsigned int successes =
            mParser->LoadMunitionTables( resourcePath, tables );
         mParser = NULL;

         // Iterate through new tables and add/replace them into the table map
         MunitionDamageTable* existingTable = NULL;
         std::vector<dtCore::RefPtr<MunitionDamageTable> >::iterator iter = tables.begin();
         for( ; iter != tables.end(); ++iter )
         {
            MunitionDamageTable* curTable = iter->get();
            const std::string& curTableName = curTable->GetName();
            if (curTable->IsDefault())
            {
               mDefaultDamageTableName = curTableName;
            }
            // Check for existing table
            existingTable = GetMunitionDamageTable( curTableName );

            // Replace the existing table with the new one; reduce success if insert fails
            if( existingTable != NULL && ! RemoveMunitionDamageTable( existingTable->GetName() ) )
            {
               std::stringstream ss;
               ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could not remove existing munition table \""
                  << existingTable->GetName() << "\"" << std::endl;
               LOG_WARNING( ss.str() );
            }

            // Simply add the new table; reduce success if insert fails
            if( ! AddMunitionDamageTable( *curTable ) )
            {
               // Reduce successes because insert failed
               successes--;
               std::stringstream ss;
               ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could insert new munition table \""
                  << (*iter)->GetName() << "\"" << std::endl;
               LOG_WARNING( ss.str() );
            }
         }

         return successes;
      }

      /////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ProcessMessage( const dtGame::Message& message )
      {
         const dtGame::MessageType& type = message.GetMessageType();

         // Update the effects manager
         if( type == dtGame::MessageType::TICK_LOCAL )
         {
            const dtGame::TickMessage& tickMessage
               = static_cast<const dtGame::TickMessage&> (message);

            mEffectsManager->Update( tickMessage.GetDeltaSimTime() );

            return;
         }
         // Avoid the most common messages that will not need to be processed
         if( type == dtGame::MessageType::TICK_REMOTE )
         { return; }

         // Process special messages...

         if( type == SimCore::MessageType::DETONATION )
         {
            OnDetonation(message);
         }

         // HANDLE SHOT FIRED - this message is mostly just for visuals. Ie, it doesn't
         // do any damage. For direct & indirect, the damage occurs on the Detonation message.
         else if (type == SimCore::MessageType::SHOT_FIRED)
         {
            OnShotFired(message);
         }
         // Capture the player
         else if (message.GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD)
         {
            dtGame::GameActorProxy* proxy
               = GetGameManager()->FindGameActorById(message.GetAboutActorId());

            if ( proxy == NULL || proxy->IsRemote() ) { return; }

            mPlayer = dynamic_cast<SimCore::Actors::StealthActor*>(proxy->GetDrawable());

            if( ! mPlayer.valid() )
            {
               LOG_ERROR("Received a player entered world message from an actor that is not a player");
               return;
            }
         }
         else if( type == dtGame::MessageType::INFO_ACTOR_DELETED )
         {
            if( mPlayer.valid() && message.GetAboutActorId() == mPlayer->GetUniqueId() )
            {
               mPlayer = NULL;
            }

            Unregister( message.GetAboutActorId() );
         }
         else if( type == dtGame::MessageType::INFO_TIME_CHANGED )
         {
            //mLastDetonationTime = 0.0f;
         }
         else if( type == dtGame::MessageType::INFO_MAP_LOADED )
         {
            InitMunitionTypeTable();

            std::vector<dtCore::ActorProxy*> terrains;
            GetGameManager()->FindActorsByType( *SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, terrains );

            if( ! terrains.empty() )
            {
               mIsector->SetQueryRoot( terrains[0]->GetDrawable() );
            }

            CleanupCreatedMunitionsQueue();
         }
         else if (type == dtGame::MessageType::INFO_RESTARTED
            || type == dtGame::MessageType::INFO_MAP_UNLOAD_BEGIN)
         {
            mPlayer = NULL;
            ClearCreatedMunitionsQueue();
            ClearRegisteredEntities();
            ClearTables();
            mMunitionTypeTable->Clear();
            //mLastDetonationTime = 0.0f;

            if (type == dtGame::MessageType::INFO_RESTARTED)
            {
               LoadMunitionDamageTables( mMunitionConfigFileName );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::OnDetonation(const dtGame::Message& message)
      {
         const DetonationMessage& detMessage =
            dynamic_cast<const DetonationMessage&> (message);

         const SimCore::Actors::MunitionTypeActor* munitionType = FindMunitionForMessage(detMessage);

         if (munitionType != NULL)
         {
            // Is this Direct Fire?
            bool isDirect = !munitionType->GetFamily().IsExplosive();
            if (isDirect)
            {
               if (! message.GetAboutActorId().ToString().empty())
               {
                  // For indirect, only
                  DamageHelper* helper = GetHelperByEntityId( message.GetAboutActorId() );
                  if (helper != NULL)
                  {
                     helper->ProcessDetonationMessage( detMessage, *munitionType, true );
                  }
               }
            }
            else // this is Indirect Fire
            {
               // Since this is indirect fire, everything has to process the detonation in case
               // they have damage from the effect of the explosion
               std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator iter =
                  mIdToHelperMap.begin();

               for( ; iter != mIdToHelperMap.end(); ++iter )
               {
                  iter->second->ProcessDetonationMessage( detMessage, *munitionType, false );
               }
            }

            // Create the particle systems and sound effects
            ApplyDetonationEffects( detMessage, *munitionType );
         }
         else
         {
            std::ostringstream oss;
            oss << "Detonation munition \"" << detMessage.GetMunitionType()
               << "\" could not be found nor the default munition \""
               << GetDefaultMunitionName() << "\"" << std::endl;
            LOG_WARNING(oss.str());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::OnShotFired(const dtGame::Message& message)
      {
         const ShotFiredMessage& shotMessage =
            dynamic_cast<const ShotFiredMessage&> (message);

         const SimCore::Actors::MunitionTypeActor* munitionType = FindMunitionForMessage(shotMessage);

         if (munitionType != NULL)
         {
            DamageHelper* helper = NULL;
            // Is this Direct Fire?
            if (! message.GetAboutActorId().ToString().empty())
            {
               helper = GetHelperByEntityId( message.GetAboutActorId() );
               if( helper != NULL )
               {
                  helper->ProcessShotMessage( shotMessage, *munitionType, true );
               }
            }
            else // this is Indirect Fire
            {
               std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator iter =
                  mIdToHelperMap.begin();

               for( ; iter != mIdToHelperMap.end(); ++iter )
               {
                  iter->second->ProcessShotMessage( shotMessage, *munitionType, false );
               }
            }

            // Apply gun flash effects only to remote entities
            if (message.GetSource() != GetGameManager()->GetMachineInfo())
            {
               ApplyShotfiredEffects( shotMessage, *munitionType );
            }
         }
         else
         {
            std::ostringstream oss;
            oss << "Weapon fire munition \"" << shotMessage.GetMunitionType()
               << "\" could not be found nor the default munition \""
               << GetDefaultMunitionName() << "\"" << std::endl;
            LOG_WARNING(oss.str());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper* MunitionsComponent::GetHelperByEntityId( const dtCore::UniqueId& id )
      {
         std::map<dtCore::UniqueId, dtCore::RefPtr<DamageHelper> >::iterator iter =
            mIdToHelperMap.find( id );

         return iter != mIdToHelperMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionDamageTable* MunitionsComponent::GetMunitionDamageTable( const std::string& entityClassName )
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::iterator iter =
            mNameToMunitionDamageTableMap.find( entityClassName );

         return iter != mNameToMunitionDamageTableMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const MunitionDamageTable* MunitionsComponent::GetMunitionDamageTable( const std::string& entityClassName ) const
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::const_iterator iter =
            mNameToMunitionDamageTableMap.find( entityClassName );

         return iter != mNameToMunitionDamageTableMap.end() ? iter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::AddMunitionDamageTable( MunitionDamageTable& table )
      {
         return mNameToMunitionDamageTableMap.insert(
               std::make_pair( table.GetName(), &table )
            ).second;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsComponent::RemoveMunitionDamageTable( const std::string& entityClassName )
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::iterator iter =
            mNameToMunitionDamageTableMap.find( entityClassName );

         if( iter != mNameToMunitionDamageTableMap.end() )
         {
            mNameToMunitionDamageTableMap.erase(iter);
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ClearTables()
      {
         std::map<std::string, dtCore::RefPtr<MunitionDamageTable> >::iterator iter =
            mNameToMunitionDamageTableMap.begin();

         for( ; iter != mNameToMunitionDamageTableMap.end(); ++iter )
         {
            if( iter->second.valid() )
            {
               iter->second->Clear();
            }
         }

         mNameToMunitionDamageTableMap.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::SetDamage( SimCore::Actors::BaseEntity& entity, DamageType& damage )
      {
         DamageHelper* helper = GetHelperByEntityId( entity.GetUniqueId() );
         if( helper != NULL )
         {
            helper->SetDamage( damage );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ApplyShotfiredEffects( const ShotFiredMessage& message,
         const SimCore::Actors::MunitionTypeActor& munitionType )
      {
         const std::string& munitionName = message.GetMunitionType();

         // Get the munition effects info so that the detonation actor can be set
         // with relevant data.
         const SimCore::Actors::MunitionEffectsInfoActor* effects
            = GetMunitionEffectsInfo( munitionType, "" );// NO DEFAULT EFFECT FOR NOW: GetDefaultMunitionName() );

         if( effects == NULL )
         {
            std::ostringstream ss;
            ss << "Munition \"" << munitionName << "\" does not have effects defined for it."
               << std::endl;
            LOG_WARNING( ss.str() );
            return;
         }

         // Find the remote actor who sent the fire message so that flash and
         // sound effects can be applied to it.
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy
            = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>
            (GetGameManager()->FindActorById( message.GetSendingActorId() ));

         // Prevent further processing if no visible entity was found to have
         // fired the shot.
         if ( ! proxy.valid()) { return; }

         // Access the entity directly instead of though a proxy
         SimCore::Actors::BaseEntity* entity = NULL;
         proxy->GetDrawable(entity);

         if (entity == NULL) { return; }

         osg::Group* bestNode = entity->GetWeaponMuzzleForDirection(message.GetInitialVelocityVector());

         // Attach a flash effect if a DOF was found.
         if( bestNode != NULL )
         {
            // Get the player's location in the world so that sound effects can
            // be attenuated when created in 3D space.
            dtCore::Transform playerXform;
            GetGameManager()->GetApplication().GetCamera()->GetTransform(playerXform);
            osg::Vec3 playerPos;
            playerXform.GetTranslation(playerPos);
            // Initiate the weapon effect
            mEffectsManager->ApplyWeaponEffect(
               *entity, bestNode, *effects, playerPos );
         }

         // Place tracers into the scene
         {
            unsigned quantity = message.GetQuantityFired();

            // Determine if enough rounds have been shot to justify production
            // of a new tracer effect.
            int tracerFrequency = munitionType.GetTracerFrequency();

            // If tracers are allowable...
            if( tracerFrequency > 0 )
            {
               float probability = 0.0f;
               if( int(quantity) >= tracerFrequency ) // ...with 100% probability...
               {
                  probability = 1.0f;
               }
               else // ...determine a probability of a tracer effect...
               {
                  probability = float(quantity) / float(tracerFrequency);
               }

               // ...and if the probability of a tracer is within the probable area...
               if( probability == 1.0f || dtUtil::RandFloat( 0.0f, 1.0f ) < probability )
               {
                  // ...generate a tracer effect request...
                  dtCore::RefPtr<MunitionEffectRequest> effectRequest
                     = new MunitionEffectRequest( quantity, 0.05f, *effects );
                  effectRequest->SetVelocity( message.GetInitialVelocityVector() );
                  effectRequest->SetOwner( entity );
                  effectRequest->SetMunitionModelFile( munitionType.GetModel() );

                  if( entity != NULL && bestNode != NULL )
                  {
                     // ...from the fire point on the firing entity.
                     osg::Matrix mtx;
                     entity->GetAbsoluteMatrix( bestNode, mtx );

                     effectRequest->SetFirePoint( mtx.getTrans() );
                     mEffectsManager->AddMunitionEffectRequest( effectRequest );

                     //mEffectsManager->ApplyTracerEffect(
                     //   mtx.getTrans(), message.GetInitialVelocityVector(), *effects );
                  }
                  else
                  {
                     // ...from the firing location specified in the message.
                     effectRequest->SetFirePoint( message.GetFiringLocation() );
                     mEffectsManager->AddMunitionEffectRequest( effectRequest );

                     //mEffectsManager->ApplyTracerEffect(
                     //   message.GetFiringLocation(), message.GetInitialVelocityVector(), *effects );
                  }
               }
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ApplyDetonationEffects( const DetonationMessage& message,
         const SimCore::Actors::MunitionTypeActor& munitionType )
      {
         //brad- todo- make detonation queue
         //Sort of a hack.  Detonations cannot be drawn if they come so quickly.
         //double simTime = GetGameManager()->GetSimulationTime();
         //if ( simTime > mLastDetonationTime && simTime - mLastDetonationTime <= 0.05f )
         //{
         //   LOG_DEBUG("Skipping detonations that occur too close together.");
         //   return;
         //}
         //else
         //{
         //   mLastDetonationTime = simTime;
         //}

         //const std::string& munitionName = message.GetMunitionType();

         // Figure out if we hit an entity or if we hit the ground!
         bool hitEntity = ! message.GetAboutActorId().ToString().empty();
         bool entityIsHuman = false;
         if( hitEntity )
         {
            dtCore::ActorProxy * targetProxy = GetGameManager()->FindActorById(message.GetAboutActorId());
            // Note - the mIsector here is kinda wonky cause it holds the terrain Drawable.
            // Change hitEntity to false if this is found to be the terrain actor.
            if(targetProxy != NULL)
            {
               hitEntity = targetProxy != NULL && ( dynamic_cast<SimCore::Actors::BaseEntity*>(targetProxy->GetDrawable()) != NULL );

               // Check to see if we hit a person. Needs a different effect
               entityIsHuman = (hitEntity &&
                  dynamic_cast<SimCore::Actors::Human*>(targetProxy->GetDrawable()) != NULL);
            }
         }


         int fidID = 0;
         RunIsectorForFIDCodes(hitEntity, message, fidID);
         
         // Prepare a detonation actor to be placed into the scene
         dtCore::RefPtr<SimCore::Actors::DetonationActorProxy>  proxy = CreateDetonationPrototype(message);
         if(!proxy.valid())
         {
            LOG_ERROR("Unable to create detonation prototype, aborting ApplyDetonationEffects()");
            return;
         }

         SimCore::Actors::DetonationActor* da = dynamic_cast<SimCore::Actors::DetonationActor*>(proxy->GetDrawable());
         if(da == NULL)
         {
            LOG_ERROR("Received a detonation actor proxy that did not contain a detonation actor. Ignoring.");

            //set proxy to null if we do not have a valid detonation actor
            proxy = NULL;
            return;
         }

         //set the impact type
         if(entityIsHuman)
         {
            da->SetImpactType(Actors::DetonationActor::IMPACT_HUMAN);
         }
         else if(hitEntity)
         {
            da->SetImpactType(Actors::DetonationActor::IMPACT_ENTITY);
         }
         else
         {
            da->SetImpactType(Actors::DetonationActor::IMPACT_TERRAIN);
         }

         // Set the detonation's position
         osg::Vec3 pos = message.GetDetonationLocation();
         dtCore::Transform xform(pos[0], pos[1], pos[2]);
         da->SetTransform(xform, dtCore::Transformable::REL_CS);

         SimCore::Components::ViewerMaterialComponent* materialComponent;
         GetGameManager()->GetComponentByName(
               SimCore::Components::ViewerMaterialComponent::DEFAULT_NAME, materialComponent);

         if(materialComponent != NULL)
         {
            SimCore::Actors::ViewerMaterialActor& viewerMaterial
               = materialComponent->CreateOrChangeMaterialByFID(fidID);
            da->SetMaterialCollidedWith(viewerMaterial);
         }

         // Delay sound to ensure that it reaches the player at the proper time
         if( mPlayer.valid() )
         {
            dtCore::Transform xform;
            mPlayer->GetTransform(xform);
            osg::Vec3 playerPos;
            xform.GetTranslation(playerPos);
            da->CalculateDelayTime(playerPos);
         }

         // Determine if the detonation should have physics applied to its particles.
         bool avoidPhysics = munitionType.GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_ROUND
            || munitionType.GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_UNKNOWN;
         da->SetPhysicsEnabled( ! avoidPhysics );

         // Add the newly created detonation to the scene
         GetGameManager()->AddActor(da->GetGameActorProxy(), false, false);

         AddMunitionToCreatedMunitionsQueue(da->GetUniqueId());
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::OnAddedToGM()
      {
         dtGame::GMComponent::OnAddedToGM();

         dtUtil::ConfigProperties& config = GetGameManager()->GetConfiguration();

         if (GetDefaultMunitionName().empty())
         {
            std::string defaultMunition =
                     config.GetConfigPropertyValue(CONFIG_PROP_MUNITION_DEFAULT, "");
            defaultMunition =
                     config.GetConfigPropertyValue(GetName() + "." + CONFIG_PROP_MUNITION_DEFAULT, defaultMunition);
            // Set the default munition to be used for munitions that are not found
            // in the existing set of munition definitions in the munitions map.
            SetDefaultMunitionName(defaultMunition);
         }

         if (GetDefaultKineticRoundMunitionName().empty())
         {
            std::string defaultMunition =
                     config.GetConfigPropertyValue(GetName() + "." + CONFIG_PROP_MUNITION_KINETIC_ROUND_DEFAULT, "");
            // Set the default munition to be used for munitions that are not found
            // and have a kinetic, i.e. kinetic energy damage, usually a bullet or non-exploding shell.
            SetDefaultKineticRoundMunitionName(defaultMunition);
         }

         mIsector->SetScene( &GetGameManager()->GetScene() );
         mEffectsManager->SetGameManager( GetGameManager() );

         std::string stringValue = config.GetConfigPropertyValue(this->GetName() + ".MaximumActiveMunitions");
         if (!stringValue.empty())
         {
            unsigned newValue = dtUtil::ToType<unsigned>(stringValue);
            if (newValue > 0)
            {
               mMaximumActiveMunitions = newValue;
            }
            else
            {
               LOG_ERROR("Received bad number from configuration file for MaximumActiveMunitions on the Munitions Component.");
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& MunitionsComponent::GetMunitionDamageTypeName( const std::string& munitionTypeName ) const
      {
         // Static constant proposed by David so that this function can return a
         // string reference.
         static const std::string EMPTY("");

         if( ! mMunitionTypeTable.valid() ) { return EMPTY; }

         const SimCore::Actors::MunitionTypeActor* munitionType =
            GetMunitionTypeTable()->GetMunitionType( munitionTypeName );

         if( munitionType == NULL ) { return EMPTY; }

         return munitionType->GetDamageType();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionsComponent::FindMunitionForMessage(const BaseWeaponEventMessage& messageData) const
      {
         if (messageData.GetWarheadType() == 5000U)
         {
            return GetMunition(messageData.GetMunitionType(), GetDefaultKineticRoundMunitionName());
         }

         return GetMunition(messageData.GetMunitionType(), GetDefaultMunitionName());
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionTypeActor* MunitionsComponent::GetMunition(
         const std::string& munitionName, const std::string& defaultMunitionName ) const
      {
         if( ! mMunitionTypeTable.valid() )
         {
            LOG_ERROR("Cannot acquire munition, no MunitionTypeTable exists.");
            return NULL;
         }

         // Obtain the closest matching registered munition type.
         const SimCore::Actors::MunitionTypeActor* munitionType
            = mMunitionTypeTable->GetMunitionType( munitionName );

         if( munitionType == NULL )
         {
            // Attempt access to the specified default munition.
            munitionType = mMunitionTypeTable->GetMunitionType( defaultMunitionName );

            std::ostringstream oss;
            oss << "Received a detonation with an invalid munition \""
               << munitionName << "\". Attempting default munition \""
               << defaultMunitionName << "\".\n\tDefault munition "
               << (munitionType==NULL?"NOT found":"found") << std::endl;
            LOG_WARNING( oss.str() );
         }

         return munitionType;
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::MunitionEffectsInfoActor* MunitionsComponent::GetMunitionEffectsInfo(
         const SimCore::Actors::MunitionTypeActor& munition, const std::string& defaultMunitionName ) const
      {
         // Get the munition effects info so that the detonation actor can be set
         // with relevant data.
         const SimCore::Actors::MunitionEffectsInfoActor* effects =
            dynamic_cast<const SimCore::Actors::MunitionEffectsInfoActor*>
            (munition.GetEffectsInfoActor());

         if( effects == NULL )
         {
            std::ostringstream oss;
            oss << "Munition \"" << munition.GetName() << "\" has no effects assign to it."
               << "\nAttempting to use default effects from default munition \""
               << defaultMunitionName << "\"" << std::endl;

            const SimCore::Actors::MunitionTypeActor* defaultMunitionType
               = GetMunition( defaultMunitionName );
            if( defaultMunitionType != NULL )
            {
               effects = dynamic_cast<const SimCore::Actors::MunitionEffectsInfoActor*>
                  (defaultMunitionType->GetEffectsInfoActor());
            }
            else
            {
               oss << "Default munition not found." << std::endl;
            }

            LOG_WARNING( oss.str() );
         }

         return effects;
      }

      // This simple structure is a remove functor for the remove_if operator on the
      // active munitions deque. It basically checks to see if the unique id exists
      // in the GM or not so that we can delete old id's from our queue when we clean up.
      struct RemoveOldMunitionsChecker
      {
         RemoveOldMunitionsChecker(dtGame::GameManager &gm) : theGM(gm) { }

         // check to see if the unique id exists in the GM (return false) or not (return true).
         bool operator()(const dtCore::UniqueId idToCheck)
         {
            bool bFound = theGM.FindActorById(idToCheck) == NULL;
            return bFound;
         }

         dtGame::GameManager &theGM;
      };


      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::SetMaximumActiveMunitions(unsigned newMax)
      {
         if (newMax > 0) // a negative max would be bad.
            mMaximumActiveMunitions = newMax;

         // clean up in case our new max causes us to have too many
         CleanupCreatedMunitionsQueue();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ClearCreatedMunitionsQueue()
      {
         mCreatedMunitionsQueue.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::CleanupCreatedMunitionsQueue()
      {
         if (!mCreatedMunitionsQueue.empty())
         {
            // First, we remove all of entries in our queue that are no longer valid. Usually happens if the
            // Actors were deleted, probably from timing out.
               mCreatedMunitionsQueue.erase(std::remove_if(mCreatedMunitionsQueue.begin(),
                  mCreatedMunitionsQueue.end(), RemoveOldMunitionsChecker(*GetGameManager())), mCreatedMunitionsQueue.end());

            // if we still have too many valid munitions, remove them from the front until we have the right size.
            int curSize = mCreatedMunitionsQueue.size();
            for (int i = curSize - mMaximumActiveMunitions; i > 0; i --)
            {
               RemoveOldestMunitionFromQueue();
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::RemoveOldestMunitionFromQueue()
      {
         if (!mCreatedMunitionsQueue.empty())
         {
            const dtCore::UniqueId frontId = mCreatedMunitionsQueue.front();
            mCreatedMunitionsQueue.pop_front();

            dtCore::ActorProxy *frontProxy = GetGameManager()->FindActorById(frontId);
            if (frontProxy != NULL)
               GetGameManager()->DeleteActor(*frontProxy);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::AddMunitionToCreatedMunitionsQueue(const dtCore::UniqueId& uniqueId)
      {
         // Add it to the back, and then clean up if we have too many.
         mCreatedMunitionsQueue.push_back(uniqueId);

         unsigned curSize = mCreatedMunitionsQueue.size();
         if (curSize > mMaximumActiveMunitions)
         {
            CleanupCreatedMunitionsQueue();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::RunIsectorForFIDCodes(bool hitEntity, const DetonationMessage& message, int fidID)
      {
         //set to a default value
         fidID = 0;

         // Set the position and ground clamp
         osg::Vec3 pos( message.GetDetonationLocation() );
         osg::Vec3 endPos = pos;

         // If an entity was hit, use clamping along the impact velocity vector
         if( hitEntity )
         {
            osg::Vec3 normal( message.GetFinalVelocityVector() );
            if( normal.length2() >= 0.0f )
            {
               normal.normalize();
               normal *= 0.5f;
               pos -= normal;
               endPos += normal;
            }
         }
         else // use straight up and down clamping
         {
            pos.z() -= 500.0f;
            endPos.z() += 500.0f;
         }

         dtCore::BatchIsector::SingleISector& SingleISector = mIsector->EnableAndGetISector(0);
         SingleISector.SetSectorAsLineSegment(pos, endPos);

         if(mIsector->Update( osg::Vec3(0,0,0), true ) )
         {
            osg::Vec3 hp;

            // Make sure the isector actually has hit points on the terrain.
            // This component will not assume the isector to return an unsigned value
            // when its function return is a signed value; thus failing everything from
            // 0 and everything to the left of 0.
            if( SingleISector.GetNumberOfHits() <= 0 )
            {
               LOG_WARNING( "Munition Component could not place a detonation actor because the BatchIsector has an empty hit list." );
               return;
            }

            SingleISector.GetHitPoint(hp);
            const osg::Drawable* drawable = SingleISector.GetIntersectionHit(0)._drawable.get();
            if( drawable != NULL && drawable->getStateSet() != NULL)
            {
               RefPtr<const osg::IntArray> mOurList
                  = dynamic_cast<const osg::IntArray*>(drawable->getStateSet()->getUserData());
               if( mOurList.valid() )
               {
                  if( ! mOurList->empty() )
                  {
                     int value[4];
                     int iter = 0;
                     osg::IntArray::const_iterator listiter = mOurList->begin();
                     for(; listiter != mOurList->end(); listiter++)
                     {
                        value[iter] = *listiter;
                        ++iter;
                     }
                     fidID = value[0];
                  }
               }
            }

            if (dtUtil::Log::GetInstance().IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               std::ostringstream ss;
               ss << "Found a hit - old z " << pos.z() << " new z " << hp.z();
               LOG_DEBUG(ss.str());
            }

            mIsector->Reset();

            // --- DEBUG --- START --- //
            //std::cout << "MunComp. Det. (clamped): " << pos[0] << ", " << pos[1] << ", " << pos[2] << std::endl;
            // --- DEBUG --- END --- //
         }

      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<SimCore::Actors::DetonationActorProxy> MunitionsComponent::CreateDetonationPrototype(const DetonationMessage& message)
      {
         dtCore::RefPtr<SimCore::Actors::DetonationActorProxy> proxy;

         if(GetMunitionTypeTable() == NULL)
         {
            LOG_ERROR("Unable to create detonation, munition type table is NULL");
         }
         else
         {
            SimCore::Actors::MunitionTypeActor* munitionType = GetMunitionTypeTable()->GetMunitionType(message.GetMunitionType());

            if(munitionType != NULL)
            {
               GetGameManager()->CreateActorFromPrototype(munitionType->GetDetonationActor(), proxy);
            }

            if(!proxy.valid())
            {
               LOG_ERROR("Failed to create the detonation proxy");
            }
         }

         return proxy;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::InitMunitionTypeTable()
      {
         std::vector<dtCore::ActorProxy* > proxies;
         GetGameManager()->FindActorsByType(*SimCore::Actors::EntityActorRegistry::MUNITION_TYPE_ACTOR_TYPE, proxies);

         // Declare variable for the loop
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> curProxy = NULL;
         unsigned int munitions = 0;

         // Populate the table with valid MunitionTypeActors
         std::vector<dtCore::ActorProxy*>::iterator iter = proxies.begin();
         for( ; iter != proxies.end(); ++iter )
         {
            curProxy = dynamic_cast<SimCore::Actors::MunitionTypeActorProxy*> (*iter);
            if( curProxy.valid() )
            {
               if( mMunitionTypeTable->AddMunitionType( curProxy ) )
               {
                  ++munitions;
               }
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ConvertMunitionInfoActorsToDetonationActors(const std::string& mapName)
      {
         // Load the map file
         dtCore::Map* map = NULL;
         try
         {
            map = &dtCore::Project::GetInstance().GetMap(mapName);
         }
         catch(const dtUtil::Exception &e)
         {
            std::ostringstream oss;
            oss << "ERROR! Failed to load the munitions type table named: " << mapName <<
               " because: " << e.What() << ". You will not be able to see detonations.";

            LOG_ERROR(oss.str());
            return;
         }

         //lets map a mapping to add the detonation prototypes to the proper MunitionTypes
         typedef std::map<const SimCore::Actors::MunitionEffectsInfoActor*, dtCore::UniqueId> InfoToIdMap;
         InfoToIdMap effectMapping;

         //now iterate through all the munition info's
         dtCore::Map& actorMap = *map;
         std::vector<dtCore::RefPtr<dtCore::ActorProxy> > proxies;
         actorMap.GetAllProxies(proxies);

         dtCore::RefPtr<SimCore::Actors::MunitionEffectsInfoActorProxy> infoProxy = NULL;

         // Populate the table with valid MunitionTypeActors
         std::vector<dtCore::RefPtr<dtCore::ActorProxy> >::iterator iter = proxies.begin();
         for(; iter != proxies.end(); ++iter)
         {
            infoProxy = dynamic_cast<SimCore::Actors::MunitionEffectsInfoActorProxy*>(iter->get());
            if(infoProxy.valid())
            {
               dtCore::RefPtr<SimCore::Actors::DetonationActorProxy> detonationProxy;
               GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, detonationProxy);

               map->AddProxy(*detonationProxy);

               ConvertSingleMunitionInfo(*infoProxy, *detonationProxy);

               effectMapping.insert(std::make_pair(dynamic_cast<SimCore::Actors::MunitionEffectsInfoActor*>(infoProxy->GetDrawable()), detonationProxy->GetId()));
            }
         }

         //Now assign the id's to the munition types
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> curProxy = NULL;

         // Populate the table with valid MunitionTypeActors
         iter = proxies.begin();
         for(; iter != proxies.end(); ++iter)
         {
            curProxy = dynamic_cast<SimCore::Actors::MunitionTypeActorProxy*>(iter->get());
            if(curProxy.valid())
            {
               SimCore::Actors::MunitionTypeActor& munitionActor = *static_cast<SimCore::Actors::MunitionTypeActor*>(curProxy->GetDrawable());

               if(munitionActor.GetEffectsInfoActor() != NULL)
               { 
                  InfoToIdMap::iterator iter = effectMapping.find(munitionActor.GetEffectsInfoActor());
                  if(iter != effectMapping.end())
                  {
                     munitionActor.SetDetonationActor((*iter).second);
                  }
               }
            }
         }

         dtCore::Project::GetInstance().SaveMap(*map);
         proxies.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::ConvertSingleMunitionInfo(SimCore::Actors::MunitionEffectsInfoActorProxy& infoProxy, SimCore::Actors::DetonationActorProxy& detonationProxy)
      {
         SimCore::Actors::MunitionEffectsInfoActor& infoActor = *static_cast<SimCore::Actors::MunitionEffectsInfoActor*>(infoProxy.GetDrawable());
         SimCore::Actors::DetonationActor& detonationActor = *static_cast<SimCore::Actors::DetonationActor*>(detonationProxy.GetDrawable());

         detonationActor.SetName(infoActor.GetName());
         detonationProxy.SetInitialOwnership(dtGame::GameActorProxy::Ownership::PROTOTYPE);
         
         dtCore::ResourceDescriptor desc;
         
         desc = static_cast<dtCore::ResourceActorProperty*>(infoProxy.GetProperty("Ground Impact Effect"))->GetValue();
         detonationActor.SetGroundImpactEffect(desc);

         desc = static_cast<dtCore::ResourceActorProperty*>(infoProxy.GetProperty("Ground Impact Sound"))->GetValue();
         detonationActor.SetGroundImpactSound(desc);

         detonationActor.SetGroundImpactLight(infoActor.GetGroundImpactLight());


         desc = static_cast<dtCore::ResourceActorProperty*>(infoProxy.GetProperty("Entity Impact Effect"))->GetValue();
         detonationActor.SetEntityImpactEffect(desc);

         desc = static_cast<dtCore::ResourceActorProperty*>(infoProxy.GetProperty("Entity Impact Sound"))->GetValue();
         detonationActor.SetEntityImpactSound(desc);

         detonationActor.SetEntityImpactLight(infoActor.GetEntityImpactLight());

         //there were no human impact effects on the MunitionEffectsInfoActor so we skip setting them here


         desc = static_cast<dtCore::ResourceActorProperty*>(infoProxy.GetProperty("Smoke Effect"))->GetValue();
         detonationActor.SetSmokeEffect(desc);

         detonationActor.SetSmokeLifeTime(infoActor.GetSmokeLifeTime());
      }
   }
}
