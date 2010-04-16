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
// DELTA 3D
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtCore/camera.h>
#include <dtCore/transformable.h>
#include <dtCore/scene.h>
#include <dtDAL/project.h>
#include <dtDAL/map.h>
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
#include <SimCore/Actors/WeaponFlashActor.h>
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
      const dtUtil::RefString MunitionsComponent::DEFAULT_NAME("MunitionsComponent");

      //////////////////////////////////////////////////////////////////////////
      MunitionsComponent::MunitionsComponent( const std::string& name )
         : dtGame::GMComponent(name)
         , mMunitionConfigPath("Configs:MunitionsConfig.xml")
         , mIsector(new dtCore::BatchIsector)
         , mLastDetonationTime(0.0f)
         , mEffectsManager(new WeaponEffectsManager)
         , mMaximumActiveMunitions(200)
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
         std::string resourcePath = dtDAL::Project::GetInstance()
            .GetResourcePath(dtDAL::ResourceDescriptor( munitionConfigPath ));
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
            // Check for existing table
            existingTable = GetMunitionDamageTable( (*iter)->GetName() );

            // Replace the existing table with the new one; reduce success if insert fails
            if( existingTable != NULL && ! RemoveMunitionDamageTable( existingTable->GetName() ) )
            {
               std::stringstream ss;
               ss << "Failure: MunitionsComponent.LoadMunitionDamageTables could not remove existing munition table \""
                  << existingTable->GetName() << "\"" << std::endl;
               LOG_WARNING( ss.str() );
            }

            // Simply add the new table; reduce success if insert fails
            if( ! AddMunitionDamageTable( *iter ) )
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

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionsComponent::LoadMunitionTypeTable( const std::string& mapName )
      {
         // Prepare the table for a fresh load of data.
         if( mMunitionTypeTable.valid() )
         {
            mMunitionTypeTable->Clear();
         }
         else
         {
            mMunitionTypeTable = new MunitionTypeTable;
         }

         // Load the map file
         dtDAL::Map *map = NULL;
         try
         {
            map = &dtDAL::Project::GetInstance().GetMap(mapName);
         }
         catch(const dtUtil::Exception &e)
         {
            std::ostringstream oss;
            oss << "ERROR! Failed to load the munitions type table named: " << mapName <<
               " because: " << e.What() << ". You will not be able to see detonations.";

            LOG_ERROR(oss.str());
            return 0;
         }
         dtDAL::Map &actorMap = *map;
         std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > proxies;
         actorMap.GetAllProxies( proxies );

         // Declare variable for the loop
         dtCore::RefPtr<SimCore::Actors::MunitionTypeActorProxy> curProxy = NULL;
         unsigned int munitions = 0;

         // Populate the table with valid MunitionTypeActors
         std::vector<dtCore::RefPtr<dtDAL::ActorProxy> >::iterator iter = proxies.begin();
         for( ; iter != proxies.end(); ++iter )
         {
            curProxy = dynamic_cast<SimCore::Actors::MunitionTypeActorProxy*> (iter->get());
            if( curProxy.valid() )
            {
               if( mMunitionTypeTable->AddMunitionType( curProxy ) )
               {
                  ++munitions;
               }
            }
         }

         proxies.clear();

         return munitions;
      }

      //////////////////////////////////////////////////////////////////////////
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
            const DetonationMessage& detMessage =
               dynamic_cast<const DetonationMessage&> (message);

            const SimCore::Actors::MunitionTypeActor* munitionType
               = GetMunition( detMessage.GetMunitionType(), GetDefaultMunitionName() );

            if( munitionType != NULL )
            {
               // Is this Direct Fire?
               bool explosion = munitionType->GetFamily().IsExplosive();
               if( ! explosion && ! message.GetAboutActorId().ToString().empty() )
               {
                  DamageHelper* helper = GetHelperByEntityId( message.GetAboutActorId() );
                  if( helper != NULL )
                  {
                     helper->ProcessDetonationMessage( detMessage, *munitionType, true );
                  }
               }
               else // this is Indirect Fire
               {
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
               LOG_ERROR(oss.str());
            }
         }

         // HANDLE SHOT FIRED - this message is mostly just for visuals. Ie, it doesn't
         // do any damage. For direct & indirect, the damage occurs on the Detonation message.
         else if( type == SimCore::MessageType::SHOT_FIRED )
         {
            const ShotFiredMessage& shotMessage =
               dynamic_cast<const ShotFiredMessage&> (message);

            const SimCore::Actors::MunitionTypeActor* munitionType
               = GetMunition( shotMessage.GetMunitionType(), GetDefaultMunitionName() );

            if( munitionType != NULL )
            {
               DamageHelper* helper = NULL;
               // Is this Direct Fire?
               if( ! message.GetAboutActorId().ToString().empty() )
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
               if(message.GetSource() != GetGameManager()->GetMachineInfo())
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
               LOG_ERROR(oss.str());
            }
         }
         // Capture the player
         else if(message.GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD)
         {
            dtGame::GameActorProxy* proxy
               = GetGameManager()->FindGameActorById(message.GetAboutActorId());

            if ( proxy == NULL || proxy->IsRemote() ) { return; }

            mPlayer = dynamic_cast<SimCore::Actors::StealthActor*>(proxy->GetActor());

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
            mLastDetonationTime = 0.0f;
         }
         else if( type == dtGame::MessageType::INFO_MAP_LOADED )
         {
            std::vector<dtDAL::ActorProxy*> terrains;
            GetGameManager()->FindActorsByType( *SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, terrains );

            if( ! terrains.empty() )
            {
               mIsector->SetQueryRoot( terrains[0]->GetActor() );
            }

            CleanupCreatedMunitionsQueue();
         }
         else if( type == dtGame::MessageType::INFO_RESTARTED
            || type == dtGame::MessageType::INFO_MAP_UNLOAD_BEGIN )
         {
            mPlayer = NULL;
            ClearCreatedMunitionsQueue();
            ClearRegisteredEntities();
            ClearTables();
            mLastDetonationTime = 0.0f;

            if( type == dtGame::MessageType::INFO_RESTARTED)
            {
               LoadMunitionDamageTables( mMunitionConfigPath );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      DamageHelper* MunitionsComponent::GetHelperByEntityId( const dtCore::UniqueId id )
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
      bool MunitionsComponent::AddMunitionDamageTable( dtCore::RefPtr<MunitionDamageTable>& table )
      {
         if( ! table.valid() ) { return false; }

         return mNameToMunitionDamageTableMap.insert(
               std::make_pair( table->GetName(), table.get() )
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
            LOG_ERROR( ss.str() );
            return;
         }

         // Find the remote actor who sent the fire message so that flash and
         // sound effects can be applied to it.
         dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> proxy
            = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>
            (GetGameManager()->FindActorById( message.GetSendingActorId() ));

         // Prevent further processing if no visible entity was found to have
         // fired the shot.
         if( ! proxy.valid() ) { return; }

         // Access the entity directly instead of though a proxy
         SimCore::Actors::Platform* entity = dynamic_cast<SimCore::Actors::Platform*> (&proxy->GetGameActor());

         if( entity == NULL ) { return; }

         // Access the node collector that will be used for finding DOF attach
         // points on the entity's model geometry.
         dtUtil::NodeCollector* nodeCollector = entity->GetNodeCollector();

         if( nodeCollector == NULL ) { return; }

         // Prepare variable to be used in the weapons cycling loop
         float curDotProd = -1.0f;
         float lastDotProd = -1.0f;
         osg::Vec3 trajectoryNormal( message.GetInitialVelocityVector() );
         trajectoryNormal.normalize();
         osgSim::DOFTransform* curDof = NULL;
         osgSim::DOFTransform* bestDof = NULL;

         // Loop though and find a weapon on the entity's model that closely matches
         // the fired munition's initial fired direction. The weapon that matches
         // the direction the closest will be assigned the flash and sound effects.
         int limit = nodeCollector->GetTransformNodeMap().size();
         for( int weapon = 1; weapon <= limit; ++weapon )
         {
            // DOF's will have an assumed nomenclature in the format of
            // "hotspot_" followed by a 2 digit number, starting at one.
            std::stringstream ss;
            ss << "hotspot_" << (weapon < 10 ? "0" : "") << weapon;
            curDof = nodeCollector->GetDOFTransform(ss.str());
            if( curDof == NULL ) { continue; }

            // If there is more than one weapon, compare current the weapon's
            // direction with the munition's trajectory
            if( limit > 1 )
            {
               osg::Matrix mtx;
               osg::Vec3 hpr;
               hpr = curDof->getCurrentHPR();
               dtUtil::MatrixUtil::HprToMatrix( mtx, hpr );
               osg::Vec3 weaponDirection( mtx.ptr()[8], mtx.ptr()[9], mtx.ptr()[10] );
               weaponDirection.normalize();
               curDotProd = weaponDirection * trajectoryNormal;
            }

            if( bestDof == NULL || curDotProd > lastDotProd )
            {
               bestDof = curDof;
               lastDotProd = curDotProd;
            }
         }

         // Attach a flash effect if a DOF was found.
         if( bestDof != NULL )
         {
            // Get the player's location in the world so that sound effects can
            // be attenuated when created in 3D space.
            dtCore::Transform playerXform;
            GetGameManager()->GetApplication().GetCamera()->GetTransform(playerXform);
            osg::Vec3 playerPos;
            playerXform.GetTranslation(playerPos);
            // Initiate the weapon effect
            mEffectsManager->ApplyWeaponEffect(
               *entity, bestDof, *effects, playerPos );
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
               if( probability == 1.0f || dtUtil::RandFloat( 0.0f, 1.0 ) < probability )
               {
                  // ...generate a tracer effect request...
                  dtCore::RefPtr<MunitionEffectRequest> effectRequest
                     = new MunitionEffectRequest( quantity, 0.05f, *effects );
                  effectRequest->SetVelocity( message.GetInitialVelocityVector() );
                  effectRequest->SetOwner( entity );
                  effectRequest->SetMunitionModelFile( munitionType.GetModel() );

                  if( entity != NULL && bestDof != NULL )
                  {
                     // ...from the fire point on the firing entity.
                     osg::Matrix mtx;
                     entity->GetAbsoluteMatrix( bestDof, mtx );

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
         //Sort of a hack.  Detonations cannot be drawn if they come so quickly.
         double simTime = GetGameManager()->GetSimulationTime();
         if ( simTime > mLastDetonationTime && simTime - mLastDetonationTime <= 0.05f )
         {
            LOG_DEBUG("Skipping detonations that occur too close together.");
            return;
         }
         else
            mLastDetonationTime = simTime;

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
            LOG_ERROR( ss.str() );
            return;
         }

         // Figure out if we hit an entity or if we hit the ground!
         bool hitEntity = ! message.GetAboutActorId().ToString().empty();
         bool entityIsHuman = false;
         if( hitEntity )
         {
            dtDAL::ActorProxy * targetProxy = GetGameManager()->FindActorById(message.GetAboutActorId());
            // Note - the mIsector here is kinda wonky cause it holds the terrain Drawable.
            // Change hitEntity to false if this is found to be the terrain actor.
            if(targetProxy != NULL)
            {
               hitEntity = ! (targetProxy != NULL
                  && ( dynamic_cast<SimCore::Actors::PagedTerrainPhysicsActor*>(targetProxy->GetActor()) != NULL
                  || dynamic_cast<SimCore::Actors::TerrainActor*>(targetProxy->GetActor()) != NULL ) );

               // Check to see if we hit a person. Needs a different effect
               entityIsHuman = (hitEntity &&
                  dynamic_cast<SimCore::Actors::Human*>(targetProxy->GetActor()) != NULL);
            }
         }

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
         int fidID = 0;

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

         // Prepare a detonation actor to be placed into the scene
         RefPtr<SimCore::Actors::DetonationActorProxy> proxy;
         GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, proxy);

         if(!proxy.valid())
         {
            LOG_ERROR("Failed to create the detonation proxy");
            return;
         }

         SimCore::Actors::DetonationActor *da = dynamic_cast<SimCore::Actors::DetonationActor*>(proxy->GetActor());
         if(da == NULL)
         {
            LOG_ERROR("Received a detonation actor proxy that did not contain a detonation actor. Ignoring.");
            return;
         }

         // Set the detonation's position
         pos = message.GetDetonationLocation();
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

         // Delay particles to ensure that sound reaches the player at the time
         // particles start spawning.
         if( mPlayer.valid() )
         {
            dtCore::Transform xform;
            mPlayer->GetTransform(xform);
            osg::Vec3 playerPos;
            xform.GetTranslation(playerPos);
            da->CalculateDelayTime(playerPos);
         }

         // Set properties on the detonation actor
         std::string curValue;

         // Load the particle effect
         if (hitEntity && entityIsHuman) // For humans, we play a different effect
         {
            // do nothing for right now
         }
         else if( hitEntity && effects->HasEntityImpactEffect() )
         {
            curValue = effects->GetEntityImpactEffect();
            if (!curValue.empty())
            {
               da->LoadDetonationFile( curValue );
            }
         }
         else
         {
            curValue = effects->GetGroundImpactEffect();
            if (!curValue.empty())
            {
               da->LoadDetonationFile( curValue );
            }
         }

         // Load the sound
         if (hitEntity && entityIsHuman) // For humans, we play a different effect
         {
            // do nothing for shooting a player. For now we have no hit human thud.
         }
         else if( hitEntity && effects->HasEntityImpactSound() )
         {
            curValue = effects->GetEntityImpactSound();
            if( ! curValue.empty() )
            {
               da->LoadSoundFile( curValue );
               da->SetMaximumSoundDistance( effects->GetEntityImpactSoundMaxDistance() );
            }
         }
         else
         {
            curValue = effects->GetGroundImpactSound();
            if( ! curValue.empty() )
            {
               da->LoadSoundFile( curValue );
               da->SetMaximumSoundDistance( effects->GetGroundImpactSoundMaxDistance() );
            }
         }

         // Load smoke effect
         curValue = effects->GetSmokeEffect();
         if( ! curValue.empty() ) { da->LoadSmokeFile( curValue ); }
         da->SetLingeringSmokeSecs( effects->GetSmokeLifeTime() );

         // Determine if the detonation should have physics applied to its particles.
         bool avoidPhysics = munitionType.GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_ROUND
            || munitionType.GetFamily() == SimCore::Actors::MunitionFamily::FAMILY_UNKNOWN;
         da->SetPhysicsEnabled( ! avoidPhysics );

         // Prepare the reference light effect type
         curValue = effects->GetEntityImpactLight();
         std::string lightString = dtUtil::Trim(curValue); // Stage put a bunch of bad whitenoise in some strings
         if( ! hitEntity || lightString.empty() )
         {
            curValue = effects->GetGroundImpactLight();
         }
         da->SetLightName( curValue );

         // Add the newly created detonation to the scene
         GetGameManager()->AddActor(da->GetGameActorProxy(), false, false);

         AddMunitionToCreatedMunitionsQueue(da->GetUniqueId());
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::OnAddedToGM()
      {
         dtGame::GMComponent::OnAddedToGM();
         mIsector->SetScene( &GetGameManager()->GetScene() );
         mEffectsManager->SetGameManager( GetGameManager() );

         // Load a value for the maximum active munitions setting. Default is usally something like 200.
         dtUtil::ConfigProperties& config = GetGameManager()->GetConfiguration();
         std::string stringValue = config.GetConfigPropertyValue(this->GetName() + ".MaximumActiveMunitions");
         if (!stringValue.empty())
         {
            int newValue = dtUtil::ToType<int>(stringValue);
            if (newValue > 0)
               mMaximumActiveMunitions = newValue;
            else
               LOG_ERROR("Received bag number from configuration file for MaximumActiveMunitions on the Munitions Component.");
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
      void MunitionsComponent::SetDefaultMunitionName( const std::string& munitionName )
      {
         mDefaultMunitionName = munitionName;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& MunitionsComponent::GetDefaultMunitionName() const
      {
         return mDefaultMunitionName;
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
               << defaultMunitionName << "\".\n\tDefault munition"
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

            LOG_ERROR( oss.str() );
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
      void MunitionsComponent::SetMaximumActiveMunitions(int newMax)
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

            dtDAL::ActorProxy *frontProxy = GetGameManager()->FindActorById(frontId);
            if (frontProxy != NULL)
               GetGameManager()->DeleteActor(*frontProxy);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsComponent::AddMunitionToCreatedMunitionsQueue(const dtCore::UniqueId &uniqueId)
      {
         // Add it to the back, and then clean up if we have too many.
         mCreatedMunitionsQueue.push_back(uniqueId);

         int curSize = mCreatedMunitionsQueue.size();
         if (curSize > mMaximumActiveMunitions)
         {
            CleanupCreatedMunitionsQueue();
         }
      }
   }
}
