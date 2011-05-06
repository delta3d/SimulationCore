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

#ifndef _CONTROL_STATE_COMPONENT_H_
#define _CONTROL_STATE_COMPONENT_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/ControlStateActor.h>

#include <dtDAL/resourcedescriptor.h>


////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class DeltaDrawable;
   class UniqueId;
}

namespace dtGame
{
   class ActorUpdateMessage;
   class Message;
}

namespace osg
{
   class Node;
}

namespace SimCore
{
   namespace Components
   {
      ////////////////////////////////////////////////////////////////////////////////
      // CONTROL STATE INFO
      ////////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ControlStateInfo : public dtCore::Base
      {
         public:
            ControlStateInfo() 
               : mGunnerModelAttached(false),
               mWeaponSelected(0)
            {
            }

            bool mGunnerModelAttached;
            unsigned mWeaponSelected;
            dtCore::RefPtr<osg::Node> mWeaponModel;
            dtCore::RefPtr<osg::Node> mGunnerModel;
            dtCore::RefPtr<SimCore::Actors::ControlStateActor> mControlState;

         protected:
            virtual ~ControlStateInfo() {}
      };



      ////////////////////////////////////////////////////////////////////////////////
      // CONTROL STATE COMPONENT
      ////////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ControlStateComponent : public dtGame::GMComponent
      {
         public:
            static const std::string DEFAULT_NAME;
            static const std::string STATION_NAME_PREFIX;
            static const std::string CONTROL_NAME_WEAPON;
            static const std::string CONTROL_NAME_TURRET_HEADING;
            static const std::string DOF_NAME_TURRET;
            static const std::string DOF_NAME_WEAPON;
            static const std::string DOF_NAME_WEAPON_HOTSPOT;

            static const int VEHICLE_STATION_TYPE;

            ControlStateComponent( const std::string& name = DEFAULT_NAME );

            virtual void ProcessMessage( const dtGame::Message& message );
      
            SimCore::Actors::Platform* GetVehicleByControlState( const SimCore::Actors::ControlStateActor& controlState );
      
            SimCore::Actors::ControlStateActor* GetControlState( const dtCore::UniqueId& actorID );
            const SimCore::Actors::ControlStateActor* GetControlState( const dtCore::UniqueId& actorID ) const;

            // OVERRIDE!!!
            virtual SimCore::Actors::ControlStateActor* GetVehicleControlState() { return NULL; }
      
            bool IsStationAvailableOnVehicle( unsigned station, const SimCore::Actors::Platform& vehicle );

            // OVERRIDE!!!
            virtual bool HasVehicleControl( const dtCore::UniqueId& vehicleID ) const { return false; }

            bool AttachModelOnVehicle( osg::Node& weapon, SimCore::Actors::Platform& vehicle, const std::string& dofName );
            bool DetachModelOnVehicle( osg::Node& weapon, SimCore::Actors::Platform& vehicle, const std::string& dofName );
      
            bool DetachRemoteWeaponOnVehicle( SimCore::Actors::Platform& vehicle, const std::string& dofName );

            bool SetRemoteWeaponOnVehicleVisible( SimCore::Actors::Platform& vehicle, const std::string& dofName, bool visible );
      
            /**
             * Access list of file names for weapon models that this component will be
             * responsible for loading and attaching to remote Delta3D DVTE vehicles
             * that need weapons displayed.
             * @return List of file names for weapon models that this component will
             * be responsible for loading..
             */
            const std::vector<dtDAL::ResourceDescriptor>& GetWeaponModelResourceList() const;
            void SetWeaponModelResourceList(const std::vector<dtDAL::ResourceDescriptor>& newList);

            /**
             * Get the name of the weapon reasource assigned to the specified weapon number.
             * @param weaponIndex Index of the weapon used in the application. The index will
             *        map to the index of the file name in the contained file name list.
             * @return file name of the weapon with the specified index; empty string if not found.
             */
            const dtDAL::ResourceDescriptor& GetWeaponModelResource( unsigned weaponIndex );

            void SetSelectedWeapon( unsigned weaponIndex );
            unsigned GetSelectedWeapon() const;

            osg::Node* GetWeaponModelOnVehicle( const dtCore::UniqueId& vehicleID );

            // Looks up the weapon id selected on the vehicle control state.
            unsigned GetWeaponOnVehicleControlState( const dtCore::UniqueId& vehicleID );

            virtual bool IsGunnerControlState( const SimCore::Actors::ControlStateActor& controlState );

            SimCore::Actors::ControlStateActor* FindVehicleControlState( const SimCore::Actors::Platform& vehicle );

            SimCore::Actors::ControlStateActor* FindVehicleControlState( const dtCore::UniqueId& vehicle );

         protected:
            virtual ~ControlStateComponent();
      
            bool IsStationAvailableOnVehicle( unsigned station,
               const SimCore::Actors::ControlStateActor& vehicleControl );
      
            // OVERRIDE!!!
            virtual void HandleEntityDeleted( const dtCore::UniqueId& entityID );
      
            // OVERRIDE!!!
            virtual void HandleControlStateUpdate( SimCore::Actors::ControlStateActor* controlState );

            // OVERRIDE!!!
            virtual void HandleControlStateDelete( const SimCore::Actors::ControlStateActor* controlState );
      
            void HandleControlStateWeaponSwap( SimCore::Actors::ControlStateActor& controlState );
      
            
            const std::string CreateStationName( unsigned stationNumber ) const;

            ControlStateInfo* GetRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID );
            bool AddRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID, ControlStateInfo& info );
            bool RemoveRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID );

            ControlStateInfo* GetRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID );
            bool AddRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID, ControlStateInfo& info );
            bool RemoveRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID );

            bool IsVehicleControlState( const SimCore::Actors::ControlStateActor& controlState ) const;

            void UpdateWeaponOnVehicle( ControlStateInfo& controlStateInfo );

         private:
            // Remote Gunner Map
            typedef std::map<dtCore::UniqueId, dtCore::RefPtr<ControlStateInfo> > RemoteControlStateMap;
            RemoteControlStateMap mRemoteGunnerMap;
            RemoteControlStateMap mRemoteVehicleMap;

            // Weapon Model File List
            std::vector<dtDAL::ResourceDescriptor> mWeaponModelFileList;

            ControlStateInfo* GetControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID );
            bool AddControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID, ControlStateInfo& info );
            bool RemoveControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID );
      
      };

   }
}

#endif
