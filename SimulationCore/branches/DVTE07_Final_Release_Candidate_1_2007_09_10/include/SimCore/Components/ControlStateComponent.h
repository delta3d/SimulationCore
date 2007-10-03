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

#ifndef _CONTROL_STATE_COMPONENT_H_
#define _CONTROL_STATE_COMPONENT_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/ControlStateActor.h>



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
         ControlStateInfo() : mWeaponSelected(0) {}
         unsigned mWeaponSelected;
         dtCore::RefPtr<osg::Node> mWeaponModel;
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
            static const std::string DOF_NAME_WEAPON;
            static const std::string DOF_NAME_WEAPON_HOTSPOT;

            static const unsigned VEHICLE_STATION_TYPE;

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

            bool AttachWeaponOnVehicle( osg::Node& weapon, SimCore::Actors::Platform& vehicle, const std::string& dofName );
            bool DetachWeaponOnVehicle( osg::Node& weapon, SimCore::Actors::Platform& vehicle, const std::string& dofName );
      
            bool DetachRemoteWeaponOnVehicle( SimCore::Actors::Platform& vehicle, const std::string& dofName );

            bool SetRemoteWeaponOnVehicleVisible( SimCore::Actors::Platform& vehicle, const std::string& dofName, bool visible );
      
            /**
             * Access list of file names for weapon models that this component will be
             * responsible for loading and attaching to remote Delta3D DVTE vehicles
             * that need weapons displayed.
             * @return List of file names for weapon models that this component will
             * be responsible for loading..
             */
            std::vector<std::string>& GetWeaponModelFileList() { return mWeaponModelFileList; }
            const std::vector<std::string>& GetWeaponModelFileList() const { return mWeaponModelFileList; }

            /**
             * Get the name of the weapon file assigned to the specified weapon number.
             * @param weaponIndex Index of the weapon used in the application. The index will
             *        map to the index of the file name in the contained file name list.
             * @return file name of the weapon with the specified index; empty string if not found.
             */
            const std::string& GetWeaponModelFileName( unsigned weaponIndex );

            void SetSelectedWeapon( unsigned weaponIndex );
            unsigned GetSelectedWeapon() const;

            osg::Node* GetWeaponModelOnVehicle( const dtCore::UniqueId& vehicleID );

            virtual bool IsGunnerControlState( const SimCore::Actors::ControlStateActor& controlState );

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
      
            SimCore::Actors::ControlStateActor* FindVehicleControlState( const SimCore::Actors::Platform& vehicle );
            
            const std::string CreateStationName( unsigned stationNumber ) const;

            ControlStateInfo* GetRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID );
            bool AddRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID, ControlStateInfo* info );
            bool RemoveRemoteGunnerControlStateInfo( const dtCore::UniqueId& vehicleID );

            ControlStateInfo* GetRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID );
            bool AddRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID, ControlStateInfo* info );
            bool RemoveRemoteVehicleControlStateInfo( const dtCore::UniqueId& vehicleID );

            bool IsVehicleControlState( const SimCore::Actors::ControlStateActor& controlState ) const;

            void UpdateWeaponOnVehicle( ControlStateInfo& controlStateInfo );

         private:
            // Remote Gunner Map
            typedef std::map<dtCore::UniqueId, dtCore::RefPtr<ControlStateInfo> > RemoteControlStateMap;
            RemoteControlStateMap mRemoteGunnerMap;
            RemoteControlStateMap mRemoteVehicleMap;

            // Weapon Model File List
            std::vector<std::string> mWeaponModelFileList;

            ControlStateInfo* GetControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID );
            bool AddControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID, ControlStateInfo* info );
            bool RemoveControlStateInfo( RemoteControlStateMap& infoMap, const dtCore::UniqueId& vehicleID );
      
      };

   }
}

#endif
