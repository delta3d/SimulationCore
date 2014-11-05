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
 * @author Eddie Johnson, Curtiss Murphy
 */
#include <dtGame/gmcomponent.h>

#include <dtUtil/enumeration.h>

#include <SimCore/HLA/Export.h>
#include <vector>

namespace dtHLAGM
{
   class HLAComponent;
}

namespace dtActors
{
   class CoordinateConfigActor;
}

namespace SimCore
{
   namespace HLA
   {
      // Have to copy this because DIS is an optional dependency.
      class DISConnectionData
      {
      public:
         DISConnectionData();
         DT_DECLARE_ACCESSOR(std::string, IPAddress);
         DT_DECLARE_ACCESSOR(unsigned, Port);
         DT_DECLARE_ACCESSOR(bool, BroadcastPort);

         DT_DECLARE_ACCESSOR(unsigned char, ExerciseId);
         DT_DECLARE_ACCESSOR(unsigned short, SiteId);
         DT_DECLARE_ACCESSOR(unsigned short, ApplicationId);
         DT_DECLARE_ACCESSOR(unsigned, MTU);
         DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, ActorXMLFile);

      };

      ///////////////////////////////////////////////////////////////////////////////
      /// This component manages connecting and disconnecting from the network. It is used by the StealthViewer
      class SIMCORE_HLA_EXPORT HLAConnectionComponent : public dtGame::GMComponent
      {
         public:

            ///////////////////////////////////////////////////////////////////////
            class SIMCORE_HLA_EXPORT ConnectionState : public dtUtil::Enumeration
            {
               DECLARE_ENUM(ConnectionState);

               public:
                  static const ConnectionState STATE_NOT_CONNECTED;
                  static const ConnectionState STATE_CONNECTING;
                  static const ConnectionState STATE_CONNECTED;
                  static const ConnectionState STATE_ERROR;

               protected:
                  ConnectionState(const std::string &name) : dtUtil::Enumeration(name)
                  {
                     AddInstance(this);
                  }
            };

            ///////////////////////////////////////////////////////////////////////
            class SIMCORE_HLA_EXPORT ConnectionType : public dtUtil::Enumeration
            {
               DECLARE_ENUM(ConnectionType);
            public:
               static ConnectionType TYPE_NONE;;
               static ConnectionType TYPE_HLA;
               static ConnectionType TYPE_CLIENTSERVER;
               static ConnectionType TYPE_DIS;
               static ConnectionType TYPE_OTHER;

            protected:
               ConnectionType(const std::string &name) : dtUtil::Enumeration(name)
               {
                  AddInstance(this);
               }
            };


            static const std::string DEFAULT_NAME;
            
            /// Constructor
            HLAConnectionComponent(const std::string &name = DEFAULT_NAME);

            /**
             * Sets the map name to load
             * @param mapName The name of the map
             */
            void AddMap(const std::string &mapName) { mMapNames.push_back(mapName); }

            /**
             * Sets the config file to use
             * @param file The file
             */
            void SetConfigFile(const std::string& file) { mConfigFile = file; }

            /**
             * Sets the fed ex to connect to
             * @param fedex The name of the federation execution
             */
            void SetFedEx(const std::string &fedex) { mFedEx = fedex; }

            /**
             * Sets the fed name
             * @param name The name of the federation
             */
            void SetFedName(const std::string& name) { mFedName = name; }

            /**
             * Sets the fed file
             * @param file The name of the federation file
             */
            void SetFedFile(const std::string& file) { mFedFile = file; }

            /**
             * Sets the rid file to use
             * @param file The name of the rid file
             */
            void SetRidFile(const std::string& file) { mRidFile = file; }

            /**
             * Returns the config file
             * @return mConfigFile
             */
            const std::string& GetConfigFile() const { return mConfigFile; }
            
            /**
             * Returns the fed ex
             * @return mFedEx
             */
            const std::string& GetFedEx() const { return mFedEx; }

            /**
             * Returns the fed name
             * @return mFedName
             */
            const std::string& GetFedName() const { return mFedName; }

            /**
             * Returns the fed file
             * @return mFedFile
             */
            const std::string& GetFedFile() const { return mFedFile; }

            /**
             * Returns the rid file
             * @return mRidFile
             */
            const std::string& GetRidFile() const { return mRidFile; }

            DT_DECLARE_ACCESSOR(std::string, RTIStandard);

            /** 
             * Returns the currect connection state
             * @return mState
             * @see class ConnectionState
             */
            const ConnectionState& GetConnectionState() const { return *mState; }


            /// Server IP Address is only used with type ClientServer
            void SetServerIPAddress(const std::string &newValue) { mServerIPAddress = newValue; }
            /// Server IP Address is only used with type ClientServer
            const std::string& GetServerIPAddress() const { return mServerIPAddress; }

            /// Server Port is only used with type ClientServer
            void SetServerPort(const std::string &newValue) { mServerPort = newValue; }
            /// Server Port is only used with type ClientServer
            const std::string& GetServerPort() const { return mServerPort; }

            /// Connection Type indicates what type of network we are connecting with.
            void SetConnectionType(ConnectionType& newValue) { mConnectionType = &newValue; }
            /// Connection Type indicates what type of network we are connecting with.
            const ConnectionType& GetConnectionType() const { return *mConnectionType; }

            /// Game Name (ex NetDemo) is only used with type ClientServer
            void SetServerGameName(const std::string &newValue) { mServerGameName = newValue; }
            /// Game Name (ex NetDemo) is only used with type ClientServer
            const std::string& GetServerGameName() const { return mServerGameName; }

            /// Game Version (ex 1) is only used with type ClientServer
            void SetServerGameVersion(int newValue) { mServerGameVersion = newValue; }
            /// Game Version (ex 1) is only used with type ClientServer
            int GetServerGameVersion() const { return mServerGameVersion; }

            // Gets and sets the whole struct by copy.
            DT_DECLARE_ACCESSOR(DISConnectionData, DISConnectionData);

            /**
             * Tells this component to start the initial connection to a network. First 
             * thing it does it reload the maps. The map_loaded message then triggers the connection
             */
            void StartNetworkConnection();
            
            /** 
             * Do the actual connection to the network. Called externally, or on the map loaded message.
             * It's called reconnect, because the AAR playback/record process messes with our connection a lot.
             */
            void DoReconnectToNetwork();

            /**
             * Disconnects from the federation
             */
            void Disconnect(bool closeMap = true);

            /** 
             * Returns true if any of our network connections are valid.
             */
            bool IsConnected(); 

            /**
             * Processes messages to we can sync up events on ticks
             * @param msg The message to process
             */
            virtual void ProcessMessage(const dtGame::Message &msg);

         protected:
            /// Connect to the HLA network. Called internally - after map is loaded based on mConnectionType
            void DoConnectToHLA(dtActors::CoordinateConfigActor* ccActor);
            
            /// Connect to the client server network. Called internally - after map is loaded based on mConnectionType
            void DoConnectToClientServer(dtActors::CoordinateConfigActor* ccActor);

            void AddComponentsForConnectionType();

            /**
             * Returns a reference to the HLAGMComponent we use
             * @return The component
             */
            dtHLAGM::HLAComponent* GetHLAComponent();

            /// Destructor
            virtual ~HLAConnectionComponent();


         private:

            std::vector<std::string> mMapNames;
            std::string mConfigFile;
            std::string mFedEx;
            std::string mFedName;
            std::string mFedFile;
            std::string mRidFile;
            const ConnectionType *mConnectionType; 
            std::string mServerIPAddress;
            std::string mServerPort;
            std::string mServerGameName;
            int mServerGameVersion;

            bool mPausedDuringConnectionFrame;

            const ConnectionState *mState;
      };
   }
}
