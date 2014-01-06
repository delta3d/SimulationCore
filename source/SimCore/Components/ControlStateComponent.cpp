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
#include <osg/Node>
#include <osgSim/DOFTransform>
#include <dtCore/uniqueid.h>
#include <dtUtil/nodecollector.h>
#include <dtDAL/actorproxy.h>
#include <dtDAL/project.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <SimCore/MessageType.h>
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
      const std::string ControlStateComponent::DOF_NAME_TURRET("dof_turret_01");
      const std::string ControlStateComponent::DOF_NAME_WEAPON("dof_gun_01");
      const std::string ControlStateComponent::DOF_NAME_WEAPON_HOTSPOT("hotspot_01");

      const int ControlStateComponent::VEHICLE_STATION_TYPE = -1;

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateComponent::ControlStateComponent( const std::string& name )
      : dtGame::GMComponent(name)
      , mDisableRemoteWeaponModelSwap(false)
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
            // Listen for remote deletes
            if (message.GetSource() != GetGameManager()->GetMachineInfo())
            {
               SimCore::Actors::ControlStateActor* controlState = GetControlState(message.GetAboutActorId());
               if( controlState != nullptr )
               {
                  HandleControlStateDelete( controlState );
               }
            }
            // Is the vehicle being deleted?
            else
            {
               // Notify local control states of an entity being deleted.
               // Remove all local controls states pointing to the entity.
               HandleEntityDeleted( message.GetAboutActorId() );
            }
         }

      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::Platform* ControlStateComponent::GetVehicleByControlState(
         const SimCore::Actors::ControlStateActor& controlState )
      {
         if( GetGameManager() != nullptr )
         {
            // Get the associated vehicle.
            dtDAL::ActorProxy* proxy = GetGameManager()->FindActorById( controlState.GetEntityID() );
            if( proxy != nullptr )
            {
               return dynamic_cast<SimCore::Actors::Platform*>(proxy->GetDrawable());
            }
         }
         return nullptr;
      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ControlStateActor* ControlStateComponent::GetControlState(
         const dtCore::UniqueId& actorID )
      {
         dtGame::GameManager* gm = GetGameManager();

         if( gm != nullptr )
         {
            dtDAL::ActorProxy* proxy = gm->FindActorById( actorID );

            if( proxy != nullptr
               && proxy->GetActorType() == *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE )
            {
               return static_cast<SimCore::Actors::ControlStateActor*>(proxy->GetDrawable());
            }
         }

         return nullptr;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::ControlStateActor* ControlStateComponent::GetControlState(
         const dtCore::UniqueId& actorID ) const
      {
         const dtGame::GameManager* gm = GetGameManager();

         if( gm != nullptr )
         {
            const dtDAL::ActorProxy* proxy = gm->FindActorById( actorID );

            if( proxy != nullptr
               && proxy->GetActorType() == *SimCore::Actors::EntityActorRegistry::CONTROL_STATE_ACTOR_TYPE )
            {
               return static_cast<const SimCore::Actors::ControlStateActor*>(proxy->GetActor());
            }
         }

         return nullptr;
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleEntityDeleted( const dtCore::UniqueId& entityID )
      {
         RemoveRemoteGunnerControlStateInfo( entityID );
         RemoveRemoteVehicleControlStateInfo( entityID );
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleControlStateUpdate( SimCore::Actors::ControlStateActor* controlState )
      {
         if( controlState != nullptr )
         {
            HandleControlStateWeaponSwap( *controlState );
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleControlStateDelete( const SimCore::Actors::ControlStateActor* controlState )
      {
         if( controlState == nullptr )
         {
            return;
         }

         // Update the gunner control state info
         ControlStateInfo* controlInfo = GetRemoteGunnerControlStateInfo( controlState->GetEntityID() );
         if( controlInfo != nullptr && controlInfo->mControlState.valid() )
         {
            if( IsGunnerControlState( *controlState ) && controlInfo->mGunnerModel.valid() )
            {
               // DEBUG: std::cout << "Attempt removing gunner model" << std::endl;
               if( controlInfo != nullptr && controlInfo->mGunnerModel.valid() )
               {
                  SimCore::Actors::Platform* vehicle
                     = dynamic_cast<SimCore::Actors::Platform*>
                     (controlInfo->mControlState->GetEntity());

                  if( vehicle != nullptr )
                  {
                     // DEBUG: std::cout << "\tRemoving gunner model" << std::endl;
                     controlInfo->mGunnerModelAttached = false;
                     DetachModelOnVehicle( *(controlInfo->mGunnerModel), *vehicle, DOF_NAME_TURRET );
                  }
               }
            }

            if( controlInfo->mControlState.get() == controlState )
            {
               controlInfo->mControlState = nullptr;
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::HandleControlStateWeaponSwap( SimCore::Actors::ControlStateActor& controlState )
      {
         // Determine if the control state is a remote gunner.
         if( controlState.IsRemote() )
         {
            const dtCore::UniqueId& vehicleID = controlState.GetEntityID();
            bool isVehicleControlState = IsVehicleControlState( controlState );

            // A vehicle or gunner control state will have a weapon ID (index).
            SimCore::Actors::DiscreteControl* control = controlState.GetDiscreteControl( CONTROL_NAME_WEAPON );

            // Determine if the weapon was swapped.
            if( control != nullptr )
            {
               //DEBUG: std::cout << "\nGunnerControlState:\n\tweapon(" << control->GetCurrentState() << ")" << std::endl;

               // This control state should have its previous weapon values tracked.
               ControlStateInfo* info = isVehicleControlState
                  ? GetRemoteVehicleControlStateInfo( vehicleID )
                  : GetRemoteGunnerControlStateInfo( vehicleID );

               // Add the control state info if one does not exist for the current vehicle.
               if( info == nullptr )
               {
                  // Use a local struct to pass values to be inserted into this
                  // component's control state info map for gunner control states.
                  if( isVehicleControlState )
                  {
                     AddRemoteVehicleControlStateInfo( vehicleID, *(new ControlStateInfo) );

                     // Retrieve the control state info so that the rest of the function
                     // operate with it.
                     info = GetRemoteVehicleControlStateInfo( vehicleID );
                  }
                  else
                  {
                     AddRemoteGunnerControlStateInfo( vehicleID, *(new ControlStateInfo) );

                     // Retrieve the control state info so that the rest of the function
                     // operate with it.
                     info = GetRemoteGunnerControlStateInfo( vehicleID );
                  }
               }

               if( info != nullptr )
               {
                  // Change the weapon on the associated vehicle if the weapon
                  // selection has changed.
                  if( info->mWeaponSelected != unsigned(control->GetCurrentState())
                     || ! info->mControlState.valid() )
                  {
                     // This could be the gunner or vehicle control state.
                     info->mControlState = &controlState;

                     // Update the old value with the current value
                     info->mWeaponSelected = unsigned(control->GetCurrentState());

                     // Update references to vehicle-only control states
                     if( ! isVehicleControlState )
                     {
                        ControlStateInfo* vehicleControlInfo = GetRemoteVehicleControlStateInfo( vehicleID );

                        dtDAL::ActorProxy* proxy = GetGameManager()->FindActorById(vehicleID);
                        SimCore::Actors::Platform* vehicle = proxy != nullptr
                           ? dynamic_cast<SimCore::Actors::Platform*>(proxy->GetDrawable()) : nullptr;

                        if( vehicle != nullptr && vehicleControlInfo != nullptr && vehicleControlInfo->mWeaponModel.valid() )
                        {
                           DetachModelOnVehicle( *vehicleControlInfo->mWeaponModel, *vehicle, DOF_NAME_WEAPON );
                        }
                     }

                     // Swap the weapon on the vehicle
                     UpdateWeaponOnVehicle( *info );
                  }

                  if( IsGunnerControlState( controlState ) )
                  {
                     if( ! info->mGunnerModel.valid() )
                     {
                        // DEBUG: std::cout << "Create gunner model" << std::endl;
                        static const std::string gunnerModelFile("StaticMeshes/marine_ringmount/marine_ringmount.ive");
                        osg::ref_ptr<osg::Node> cachedModel;
                        SimCore::Actors::IGActor::LoadFileStatic( gunnerModelFile, cachedModel, info->mGunnerModel );
                     }
                     if( info->mGunnerModel.valid() && ! info->mGunnerModelAttached )
                     {
                        dtDAL::ActorProxy* proxy = GetGameManager()->FindActorById(vehicleID);
                        SimCore::Actors::Platform* vehicle = proxy != nullptr
                           ? dynamic_cast<SimCore::Actors::Platform*>(proxy->GetDrawable()) : nullptr;

                        if( vehicle != nullptr )
                        {
                           info->mGunnerModelAttached = true;
                           AttachModelOnVehicle( *(info->mGunnerModel), *vehicle, DOF_NAME_TURRET );
                        }
                     }
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

         if( vehicleControl == nullptr )
         {
            return false;
         }

         return IsStationAvailableOnVehicle( station, *vehicleControl );
      }

      ////////////////////////////////////////////////////////////////////////////////
      unsigned ControlStateComponent::GetWeaponOnVehicleControlState( const dtCore::UniqueId& vehicleID )
      {
         const SimCore::Actors::ControlStateActor* vehicleControl = FindVehicleControlState( vehicleID );

         unsigned result = 0;

         if( vehicleControl != nullptr )
         {
            const SimCore::Actors::DiscreteControl* weaponControl = vehicleControl->GetDiscreteControl(CONTROL_NAME_WEAPON);
            if( weaponControl != nullptr )
            {
               result = unsigned(weaponControl->GetCurrentState());
            }
         }

         return result;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::IsStationAvailableOnVehicle(
         unsigned station, const SimCore::Actors::ControlStateActor& vehicleControl )
      {
         const SimCore::Actors::DiscreteControl* stationControl
            = vehicleControl.GetDiscreteControl( CreateStationName( station ) );

         return stationControl != nullptr && stationControl->GetCurrentState() == 0;
      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ControlStateActor* ControlStateComponent::FindVehicleControlState( const SimCore::Actors::Platform& vehicle )
      {
         const dtCore::UniqueId& vehicleID = vehicle.GetUniqueId();
         return FindVehicleControlState(vehicleID);
      }

      ////////////////////////////////////////////////////////////////////////////////
      SimCore::Actors::ControlStateActor* ControlStateComponent::FindVehicleControlState( const dtCore::UniqueId& vehicleID )
      {
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
         SimCore::Actors::ControlStateActor* curControl = nullptr;
         const unsigned limit = controlStates.size();
         for( unsigned i = 0; i < limit; ++i )
         {
            if( controlStates[i] != nullptr )
            {
               curControl = static_cast<SimCore::Actors::ControlStateActor*>(controlStates[i]->GetDrawable());

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
         return nullptr;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::AttachModelOnVehicle(
         osg::Node& model, SimCore::Actors::Platform& vehicle, const std::string& dofName )
      {
         dtUtil::NodeCollector* nodeCollector = vehicle.GetNodeCollector();
         if( nodeCollector != nullptr )
         {
            osgSim::DOFTransform* dof = nodeCollector->GetDOFTransform( dofName );
            if( dof != nullptr )
            {
               // If this is the weapon model being attached, add the
               // weapon's hotspot to the vehicle's node collector so
               // that the hot spot can be found in the munitions component
               // for rendering flash effects on the remote vehicle.
               if( dofName == DOF_NAME_WEAPON )
               {
                  //this fixes a bug where the hmmwv may have a weapon loaded from the model
                  //but the hotspot does not get removed because the control state component
                  //thinks there is no weapon because IT didn't add one
                  if(nodeCollector->GetDOFTransform(DOF_NAME_WEAPON_HOTSPOT) != nullptr)
                  {
                     nodeCollector->RemoveDOFTransform(DOF_NAME_WEAPON_HOTSPOT);
                  }

                  dof->removeChildren(0, dof->getNumChildren());
                  dof->addChild( &model );

                  // Get access to the hot spot on the weapon model
                  std::shared_ptr<dtUtil::NodeCollector> weaponNodeCollector
                     = new dtUtil::NodeCollector(&model, dtUtil::NodeCollector::DOFTransformFlag);
                  osgSim::DOFTransform* hotspotDof = weaponNodeCollector->GetDOFTransform(DOF_NAME_WEAPON_HOTSPOT);

                  if( hotspotDof != nullptr )
                  {
                     // DEBUG: std::cout << "Hotspot found on weapon model" << std::endl;
                     vehicle.GetNodeCollector()->AddDOFTransform(DOF_NAME_WEAPON_HOTSPOT, *hotspotDof );
                  }
               }
               else
               {
                  dof->addChild( &model );
               }
               return true;
            }
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::DetachModelOnVehicle(
         osg::Node& model, SimCore::Actors::Platform& vehicle, const std::string& dofName )
      {
         // DEBUG: std::cout << "DetachModelOnVehicle: " << dofName << std::endl;
         dtUtil::NodeCollector* nodeCollector = vehicle.GetNodeCollector();
         if( nodeCollector != nullptr )
         {
            osgSim::DOFTransform* dof = nodeCollector->GetDOFTransform( dofName );
            if( dof != nullptr )
            {
               // DEBUG: std::cout << "\tRemove Child" << std::endl;
               dof->removeChild( &model );

               if( dofName == DOF_NAME_WEAPON )
               {
                  vehicle.GetNodeCollector()->RemoveDOFTransform(DOF_NAME_WEAPON_HOTSPOT);
               }
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

         if( weapon != nullptr )
         {
            return DetachModelOnVehicle( *weapon, vehicle, dofName );
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::SetRemoteWeaponOnVehicleVisible(
         SimCore::Actors::Platform& vehicle, const std::string& dofName, bool visible )
      {
         osg::Node* weapon = GetWeaponModelOnVehicle( vehicle.GetUniqueId() );
         if( weapon != nullptr )
         {
            weapon->setNodeMask( visible ? 0xFFFFFFFF : 0x0 );
            return true;
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const std::vector<dtDAL::ResourceDescriptor>& ControlStateComponent::GetWeaponModelResourceList() const
      {
         return mWeaponModelFileList;
      }

      ////////////////////////////////////////////////////////////////////////////////
      void ControlStateComponent::SetWeaponModelResourceList(const std::vector<dtDAL::ResourceDescriptor>& newList)
      {
         mWeaponModelFileList = newList;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const dtDAL::ResourceDescriptor& ControlStateComponent::GetWeaponModelResource( unsigned weaponIndex )
      {
         if( ! mWeaponModelFileList.empty() && mWeaponModelFileList.size() > weaponIndex )
         {
            // DEBUG:
            // std::cout << "WeaponModelFileList:\n\tsize(" << mWeaponModelFileList.size() << ")"
            //   << "\n\tchangeToWeapon(" << weaponIndex << ")" << std::endl;
            return mWeaponModelFileList[weaponIndex];
         }
         return dtDAL::ResourceDescriptor::NULL_RESOURCE;
      }

      ////////////////////////////////////////////////////////////////////////////////
      osg::Node* ControlStateComponent::GetWeaponModelOnVehicle( const dtCore::UniqueId& vehicleID )
      {
         ControlStateInfo* info = GetRemoteGunnerControlStateInfo( vehicleID );
         if( info == nullptr )
         {
            info = GetRemoteVehicleControlStateInfo( vehicleID );
         }

         if( info != nullptr )
         {
            return info->mWeaponModel.get();
         }

         return nullptr;
      }

      ////////////////////////////////////////////////////////////////////////////////
      const std::string ControlStateComponent::CreateStationName( unsigned stationNumber ) const
      {
         std::stringstream ss;
         ss << STATION_NAME_PREFIX << stationNumber;
         return ss.str();
      }

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateInfo* ControlStateComponent::GetRemoteGunnerControlStateInfo(
         const dtCore::UniqueId& vehicleID )
      {
         return GetControlStateInfo( mRemoteGunnerMap, vehicleID );
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::AddRemoteGunnerControlStateInfo(
         const dtCore::UniqueId& vehicleID, ControlStateInfo& info )
      {
         return AddControlStateInfo( mRemoteGunnerMap, vehicleID, info );
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::RemoveRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID )
      {
         return RemoveControlStateInfo( mRemoteGunnerMap, vehicleID );
      }

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateInfo* ControlStateComponent::GetRemoteVehicleControlStateInfo(
         const dtCore::UniqueId& vehicleID )
      {
         return GetControlStateInfo( mRemoteVehicleMap, vehicleID );
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::AddRemoteVehicleControlStateInfo(
         const dtCore::UniqueId& vehicleID, ControlStateInfo& info )
      {
         return AddControlStateInfo( mRemoteVehicleMap, vehicleID, info );
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::RemoveRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID )
      {
         return RemoveControlStateInfo( mRemoteVehicleMap, vehicleID );
      }

      ////////////////////////////////////////////////////////////////////////////////
      ControlStateInfo* ControlStateComponent::GetControlStateInfo(
         RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID )
      {
         RemoteControlStateMap::iterator foundIter = infoMap.find( vehicleID );
         return foundIter != infoMap.end() ? foundIter->second.get() : nullptr;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::AddControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID, ControlStateInfo& info )
      {
         return infoMap.insert( std::make_pair(vehicleID, &info) ).second;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::RemoveControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID )
      {
         RemoteControlStateMap::iterator foundIter = infoMap.find( vehicleID );
         if( foundIter != infoMap.end() )
         {
            // Since the control state info is being removed, it must mean that control
            // of the weapon is not being used. Remove the loaded weapon model from the
            // remote entity that is associated with the control state.
            ControlStateInfo* controlInfo = GetRemoteGunnerControlStateInfo( vehicleID );
            if( controlInfo != nullptr )
            {
               SimCore::Actors::Platform* vehicle = nullptr;
               if( controlInfo->mControlState.valid() )
               {
                  vehicle = GetVehicleByControlState( *(controlInfo->mControlState) );
               }

               if( vehicle != nullptr && controlInfo->mWeaponModel.valid() )
               {
                  DetachModelOnVehicle( *controlInfo->mWeaponModel, *vehicle, DOF_NAME_WEAPON );
               }
            }

            // Finally remove the control state info
            infoMap.erase( foundIter );
            return true;
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::IsVehicleControlState( const SimCore::Actors::ControlStateActor& controlState ) const
      {
         const static std::string firstStation(CreateStationName(0));
         return controlState.GetContinuousControl( firstStation ) != nullptr;
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

         // DEBUG: std::cout << "UpdateWeaponOnVehicle:\n\tvehicle(" << (vehicle!=nullptr?"VALID":"nullptr") << ")" << std::endl;

         if( vehicle != nullptr )
         {
            // Attempt removal of the old weapon model.
            if( controlStateInfo.mWeaponModel.valid() )
            {
               DetachModelOnVehicle( *(controlStateInfo.mWeaponModel.get()), *vehicle, DOF_NAME_WEAPON );
               controlStateInfo.mWeaponModel = nullptr;
            }

            // Attempt attachment of a new weapon model.
            dtDAL::ResourceDescriptor weaponResource = GetWeaponModelResource( controlStateInfo.mWeaponSelected );
            bool isModelLoaded = false;
            if (weaponResource != dtDAL::ResourceDescriptor::NULL_RESOURCE)
            {
               const std::string& weaponFileName = dtDAL::Project::GetInstance().GetResourcePath(weaponResource);
               osg::ref_ptr<osg::Node> cachedModel;

               isModelLoaded
                  = SimCore::Actors::IGActor::LoadFileStatic( weaponFileName, cachedModel, controlStateInfo.mWeaponModel );
            }

            if( isModelLoaded )
            {
               if( controlStateInfo.mWeaponModel.valid() )
               {
                  if (!mDisableRemoteWeaponModelSwap)
                  {
                     // This call breakes the HMMWV
                     AttachModelOnVehicle( *(controlStateInfo.mWeaponModel.get()), *vehicle, DOF_NAME_WEAPON );
                  }
               }
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool ControlStateComponent::IsGunnerControlState( const SimCore::Actors::ControlStateActor& controlState )
      {
         return nullptr != controlState.GetContinuousControl( CONTROL_NAME_TURRET_HEADING );
      }

   }
}
