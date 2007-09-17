/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <osg/Node>
#include <osgSim/DOFTransform>
#include <dtCore/uniqueid.h>
#include <dtCore/nodecollector.h>
#include <dtDAL/actorproxy.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/IGActor.h>
#include <SimCore/Components/ControlStateComponent.h>



namespace SimCore
{
   namespace Components
   {
      ////////////////////////////////////////////////////////////////////////////////
      // CONTROL STATE COMPONENT
      ////////////////////////////////////////////////////////////////////////////////
      const std::string ControlStateComponent::DEFAULT_NAME("ControlStateComponent");
      const std::string ControlStateComponent::STATION_NAME_PREFIX("Station:");

      // Control can be used by gunner and vehicle control states.
      const std::string ControlStateComponent::CONTROL_NAME_WEAPON("Weapon");

      // Control name in a control state that is used to identify a control state
      // as a gunner type controls state.
      const std::string ControlStateComponent::CONTROL_NAME_TURRET_HEADING("TurretHeading");

      // Weapon DOF name for remote vehicles
      const std::string ControlStateComponent::DOF_NAME_WEAPON("dof_gun_01");
      const std::string ControlStateComponent::DOF_NAME_WEAPON_HOTSPOT("hotspot_01");

      const unsigned ControlStateComponent::VEHICLE_STATION_TYPE = -1;

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateComponent::ControlStateComponent( const std::string& name )
         : dtGame::GMComponent(name)
      {
      }

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateComponent::~ControlStateComponent()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::ProcessMessage( const dtGame::Message& message )
      {
         const dtGame::MessageType& type = message.GetMessageType();

         // Avoid any additional comparisons on message type since
         // tick messages will be the most frequently received.
         if( type == dtGame::MessageType::TICK_LOCAL 
            || type == dtGame::MessageType::TICK_REMOTE )
         {
            return;
         }
         
         if( type == SimCore::MessageType::INFO_ACTOR_CREATED
            || type == SimCore::MessageType::INFO_ACTOR_UPDATED )
         {
            const dtGame::ActorUpdateMessage& updateMsg 
               = static_cast<const dtGame::ActorUpdateMessage&>(message);

            // Listen for remote updates
            if( updateMsg.GetSource() != GetGameManager()->GetMachineInfo() )
            {
               if( updateMsg.GetActorType() == SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE )
               {
                  SimCore::Actors::ControlStateActor* controlState = GetControlState( updateMsg.GetAboutActorId() );
                  HandleControlStateUpdate( controlState );
               }
            }
         }
         else if( type == SimCore::MessageType::INFO_ACTOR_DELETED )
         {
            const dtGame::ActorDeletedMessage& deleteMsg 
               = static_cast<const dtGame::ActorDeletedMessage&>(message);

            // Listen for remote deletes
            if( deleteMsg.GetSource() != GetGameManager()->GetMachineInfo() )
            {
               SimCore::Actors::ControlStateActor* controlState = GetControlState( deleteMsg.GetAboutActorId() );
               if( controlState != NULL )
               {
                  HandleControlStateDelete( controlState );
               }
            }
            // Is the vehicle being deleted?
            else 
            {
               // Notify local control states of an entity being deleted.
               // Remove all local controls states pointing to the entity.
               HandleEntityDeleted( deleteMsg.GetAboutActorId() );
            }
         }

      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::Platform* ControlStateComponent::GetVehicleByControlState(
         const SimCore::Actors::ControlStateActor& controlState )
      {
         if( GetGameManager() != NULL )
         {
            // Get the associated vehicle.
            dtDAL::ActorProxy* proxy = GetGameManager()->FindActorById( controlState.GetEntityID() );
            if( proxy != NULL )
            {
               return dynamic_cast<SimCore::Actors::Platform*>(proxy->GetActor());
            }
         }
         return NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ControlStateActor* ControlStateComponent::GetControlState( 
         const dtCore::UniqueId& actorID )
      {
         dtGame::GameManager* gm = GetGameManager();

         if( gm != NULL )
         {
            dtDAL::ActorProxy* proxy = gm->FindActorById( actorID );
            
            if( proxy != NULL
               && proxy->GetActorType() == *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE )
            {
               return static_cast<SimCore::Actors::ControlStateActor*>(proxy->GetActor());
            }
         }

         return NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::ControlStateActor* ControlStateComponent::GetControlState( 
         const dtCore::UniqueId& actorID ) const
      {
         const dtGame::GameManager* gm = GetGameManager();

         if( gm != NULL )
         {
            const dtDAL::ActorProxy* proxy = gm->FindActorById( actorID );

            if( proxy != NULL
               && proxy->GetActorType() == *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE )
            {
               return static_cast<const SimCore::Actors::ControlStateActor*>(proxy->GetActor());
            }
         }

         return NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleEntityDeleted( const dtCore::UniqueId& entityID )
      {
         RemoveRemoteGunnerControlStateInfo( entityID );
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleControlStateUpdate( SimCore::Actors::ControlStateActor* controlState )
      {
         if( controlState != NULL )
         {
            HandleControlStateWeaponSwap( *controlState );
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleControlStateDelete( const SimCore::Actors::ControlStateActor* controlState )
      {
         if( controlState == NULL )
         {
            return;
         }

         // Update the gunner control state info
         ControlStateInfo* controlInfo = GetRemoteGunnerControlStateInfo( controlState->GetEntityID() );
         if( controlInfo != NULL )
         {
            if( controlInfo->mControlState.valid() && controlInfo->mControlState.get() == controlState )
            {
               controlInfo->mControlState = NULL;
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleControlStateWeaponSwap( SimCore::Actors::ControlStateActor& controlState )
      {
         // Determine if the control state is a remote gunner.
         if( controlState.IsRemote() )
         {
            const std::string& vehicleID = controlState.GetEntityID();

            // A vehicle or gunner control state will have a weapon ID (index).
            SimCore::Actors::DiscreteControl* control = controlState.GetDiscreteControl( CONTROL_NAME_WEAPON );

            // Determine if the weapon was swapped.
            if( control != NULL )
            {
               //DEBUG: std::cout << "\nGunnerControlState:\n\tweapon(" << control->GetCurrentState() << ")" << std::endl;

               // This control state should have its previous weapon values tracked.
               ControlStateInfo* info = GetRemoteGunnerControlStateInfo( control->GetUniqueId() );

               // Add the control state info if one does not exist for the current vehicle.
               if( info == NULL )
               {
                  // Use a local struct to pass values to be inserted into this
                  // component's control state info map for gunner control states.
                  ControlStateInfo newInfo;
                  AddRemoteGunnerControlStateInfo( vehicleID, newInfo );

                  // Retrieve the control state info so that the rest of the function
                  // operate with it.
                  info = GetRemoteGunnerControlStateInfo( vehicleID );
               }

               if( info != NULL )
               {
                  // Change the weapon on the associated vehicle if the weapon
                  // selection has changed.
                  if( info->mWeaponSelected != unsigned(control->GetCurrentState())
                     || ! info->mControlState.valid() )
                  {
                     // Determine if the current control state is the vehicle control
                     // state, and if so, replace it with the gunner control state if
                     // the incoming control state represents the gunner.
                     if( info->mControlState.valid() )
                     {
                        // Determine if the incoming object is a gunner control state by
                        // looking for a control that only a gunner control state will contain.
                        if( IsGunnerControlState( *info->mControlState ) )
                        {
                           info->mControlState = &controlState; // this is a gunner control state.
                        }
                     }
                     else
                     {
                        // This could be the gunner or vehicle control state, but
                        // will most likely be the vehicle control state.
                        info->mControlState = &controlState;
                     }
                     // Update the old value with the current value
                     info->mWeaponSelected = unsigned(control->GetCurrentState());

                     // Swap the weapon on the vehicle
                     UpdateWeaponOnVehicle( *info );
                  }
               }
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::IsStationAvailableOnVehicle( 
         unsigned station, const SimCore::Actors::Platform& vehicle )
      {
         const SimCore::Actors::ControlStateActor* vehicleControl = FindVehicleControlState( vehicle );

         if( vehicleControl == NULL )
         {
            return false;
         }

         return IsStationAvailableOnVehicle( station, *vehicleControl );
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::IsStationAvailableOnVehicle(
         unsigned station, const SimCore::Actors::ControlStateActor& vehicleControl )
      {
         const SimCore::Actors::DiscreteControl* stationControl 
            = vehicleControl.GetDiscreteControl( CreateStationName( station ) );

         return stationControl != NULL && stationControl->GetCurrentState() == 0;
      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ControlStateActor* ControlStateComponent::FindVehicleControlState( const SimCore::Actors::Platform& vehicle )
      {
         const std::string& vehicleID = vehicle.GetUniqueId().ToString();

         // Determine if the current vehicle control state is the one for the specified vehicle.
         if( HasVehicleControl( vehicleID ) )
         {
            // DEBUG: std::cout << "\n\tFound (local) vehicle control state for vehicle: " << vehicleID << "\n" << std::endl;
            return GetVehicleControlState();
         }

         // Capture all control states so that they can be searched to find the ones
         // referencing the specified vehicle.
         std::vector<dtDAL::ActorProxy*> controlStates;
         GetGameManager()->FindActorsByType( *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE, controlStates );

         // Iterate through all the control states to find the ones referencing the vehicle.
         SimCore::Actors::ControlStateActor* curControl = NULL;
         const unsigned limit = controlStates.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            if( controlStates[i] != NULL )
            {
               curControl = static_cast<SimCore::Actors::ControlStateActor*>(controlStates[i]->GetActor());

               // Determine if the current control state is referencing the specified vehicle.
               if( curControl->GetEntityID() == vehicleID )
               {
                  // Simulation mode NONE is used by the vehicle control state.
                  if( curControl->GetStationType() == VEHICLE_STATION_TYPE )
                  {
                     // DEBUG: std::cout << "\n\tFound vehicle control state for vehicle: " << vehicleID << "\n" << std::endl;
                     return curControl;
                  }
               }
            }
         }

         // DEBUG: std::cout << "\n\tNO vehicle control state found for: " << vehicleID << "\n" << std::endl;
         return NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::AttachWeaponOnVehicle(
         osg::Node& weapon, SimCore::Actors::Platform& vehicle, const std::string& dofName )
      {
         dtCore::NodeCollector* nodeCollector = vehicle.GetNodeCollector();
         if( nodeCollector != NULL )
         {
            osgSim::DOFTransform* dof = nodeCollector->GetDOFTransform( dofName );
            if( dof != NULL )
            {
               dof->addChild( &weapon );

               // Get access to the hot spot on the weapon model
               dtCore::RefPtr<dtCore::NodeCollector> weaponNodeCollector
                  = new dtCore::NodeCollector(&weapon,dtCore::NodeCollector::DOFTransformFlag);
               osgSim::DOFTransform* hotspotDof = weaponNodeCollector->GetDOFTransform(DOF_NAME_WEAPON_HOTSPOT);

               if( hotspotDof != NULL )
               {
                  // DEBUG: std::cout << "Hotspot found on weapon model" << std::endl;
                  vehicle.GetNodeCollector()->AddDOFTransform(DOF_NAME_WEAPON_HOTSPOT, *hotspotDof );
               }
               return true;
            }
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::DetachWeaponOnVehicle(
         osg::Node& weapon, SimCore::Actors::Platform& vehicle, const std::string& dofName )
      {
         dtCore::NodeCollector* nodeCollector = vehicle.GetNodeCollector();
         if( nodeCollector != NULL )
         {
            osgSim::DOFTransform* dof = nodeCollector->GetDOFTransform( dofName );
            if( dof != NULL )
            {
               dof->removeChild( &weapon );

               vehicle.GetNodeCollector()->RemoveDOFTransform(DOF_NAME_WEAPON_HOTSPOT);
               return true;
            }
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::DetachRemoteWeaponOnVehicle( 
         SimCore::Actors::Platform& vehicle, const std::string& dofName )
      {
         // Get the weapon model associated with the vehicle.
         osg::Node* weapon = GetWeaponModelOnVehicle( vehicle.GetUniqueId() );

         if( weapon != NULL )
         {
            return DetachWeaponOnVehicle( *weapon, vehicle, dofName );
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const std::string& ControlStateComponent::GetWeaponModelFileName( unsigned weaponIndex )
      {
         static const std::string EMPTY("");

         if( ! mWeaponModelFileList.empty() && mWeaponModelFileList.size() > weaponIndex )
         {
            // DEBUG:
            // std::cout << "WeaponModelFileList:\n\tsize(" << mWeaponModelFileList.size() << ")"
            //   << "\n\tchangeToWeapon(" << weaponIndex << ")" << std::endl;
            return mWeaponModelFileList[weaponIndex];
         }
         return EMPTY;
      }

      ////////////////////////////////////////////////////////////////////////////////
      osg::Node* ControlStateComponent::GetWeaponModelOnVehicle( const dtCore::UniqueId& vehicleID )
      {
         ControlStateInfo* info = GetRemoteGunnerControlStateInfo( vehicleID );

         if( info != NULL )
         {
            return info->mWeaponModel.get();
         }

         return NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const std::string ControlStateComponent::CreateStationName( unsigned stationNumber )
      {
         std::stringstream ss;
         ss << STATION_NAME_PREFIX << stationNumber;
         return ss.str();
      }

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateComponent::ControlStateInfo* ControlStateComponent::GetRemoteGunnerControlStateInfo( 
         const dtCore::UniqueId& vehicleID )
      {
         RemoteGunnerMap::iterator foundIter = mRemoteGunnerMap.find( vehicleID );
         return foundIter != mRemoteGunnerMap.end() ? &foundIter->second : NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::AddRemoteGunnerControlStateInfo( 
         const dtCore::UniqueId& vehicleID, const ControlStateComponent::ControlStateInfo& info )
      {
         return mRemoteGunnerMap.insert( std::make_pair(vehicleID, info) ).second;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::RemoveRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID )
      {
         RemoteGunnerMap::iterator foundIter = mRemoteGunnerMap.find( vehicleID );
         if( foundIter != mRemoteGunnerMap.end() )
         {
            // Since the control state info is being removed, it must mean that control
            // of the weapon is not being used. Remove the loaded weapon model from the
            // remote entity that is associated with the control state.
            ControlStateInfo* controlInfo = GetRemoteGunnerControlStateInfo( vehicleID );
            if( controlInfo != NULL )
            {
               SimCore::Actors::Platform* vehicle = NULL;
               if( controlInfo->mControlState.valid() )
               {
                  vehicle = GetVehicleByControlState( *(controlInfo->mControlState) );
               }

               if( vehicle != NULL && controlInfo->mWeaponModel.valid() )
               {
                  DetachWeaponOnVehicle( *controlInfo->mWeaponModel, *vehicle, DOF_NAME_WEAPON );
               }
            }

            // Finally remove the control state info
            mRemoteGunnerMap.erase( foundIter );
            return true;
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::UpdateWeaponOnVehicle( ControlStateInfo& controlStateInfo )
      {
         if( ! controlStateInfo.mControlState.valid() )
         {
            // TODO: LOG WARNING?
            return;
         }

         // Get the associated vehicle.
         SimCore::Actors::Platform* vehicle = GetVehicleByControlState( *controlStateInfo.mControlState );

         // DEBUG: std::cout << "UpdateWeaponOnVehicle:\n\tvehicle(" << (vehicle!=NULL?"VALID":"NULL") << ")" << std::endl;

         if( vehicle != NULL )
         {
            // Attempt removal of the old weapon model.
            if( controlStateInfo.mWeaponModel.valid() )
            {
               DetachWeaponOnVehicle( *(controlStateInfo.mWeaponModel.get()), *vehicle, DOF_NAME_WEAPON );
               controlStateInfo.mWeaponModel = NULL;
            }

            // Attempt attachment of a new weapon model.
            const std::string& weaponFileName = GetWeaponModelFileName( controlStateInfo.mWeaponSelected );
            dtCore::RefPtr<osg::Node> cachedModel;

            bool isModelLoaded 
               = SimCore::Actors::IGActor::LoadFileStatic( weaponFileName, cachedModel, controlStateInfo.mWeaponModel, true );

            // DEBUG: std::cout << "\tweapon(" << (isModelLoaded?weaponFileName:"NULL") << ")" << std::endl;
            
            if( isModelLoaded )
            {
               if( controlStateInfo.mWeaponModel.valid() )
               {
                  AttachWeaponOnVehicle( *(controlStateInfo.mWeaponModel.get()), *vehicle, DOF_NAME_WEAPON );
               }
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::IsGunnerControlState( const SimCore::Actors::ControlStateActor& controlState )
      {
         return NULL != controlState.GetContinuousControl( CONTROL_NAME_TURRET_HEADING );
      }

   }
}
