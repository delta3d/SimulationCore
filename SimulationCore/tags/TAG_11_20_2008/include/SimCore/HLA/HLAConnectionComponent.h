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
 * @author Eddie Johnson
 */
#include <dtGame/gmcomponent.h>

#include <dtUtil/enumeration.h>

#include <SimCore/HLA/Export.h>
#include <vector>

namespace dtHLAGM
{
   class HLAComponent;
}

namespace SimCore
{
   namespace HLA
   {
      class SIMCORE_HLA_EXPORT HLAConnectionComponent : public dtGame::GMComponent
      {
         public:

             class SIMCORE_HLA_EXPORT ConnectionState : public dtUtil::Enumeration
             {
                DECLARE_ENUM(ConnectionState);

                public:

                   static const ConnectionState STATE_NOT_CONNECTED;
                   static const ConnectionState STATE_CONNECTING;
                   static const ConnectionState STATE_CONNECTED;
                   static const ConnectionState STATE_ERROR;

                protected:

                   ConnectionState(const std::string &name) : 
                      dtUtil::Enumeration(name)
                   {
                      AddInstance(this);
                   }
             };

            static const std::string DEFAULT_NAME;
            static const std::string CONFIG_PROP_ADDITIONAL_MAP;
            
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
            void SetConfigFile(const std::string &file) { mConfigFile = file; }

            /**
             * Sets the fed ex to connect to
             * @param fedex The name of the federation execution
             */
            void SetFedEx(const std::string &fedex) { mFedEx = fedex; }

            /**
             * Sets the fed name
             * @param name The name of the federation
             */
            void SetFedName(const std::string &name) { mFedName = name; }

            /**
             * Sets the fed file
             * @param file The name of the federation file
             */
            void SetFedFile(const std::string &file) { mFedFile = file; }

            /**
             * Sets the rid file to use
             * @param file The name of the rid file
             */
            void SetRidFile(const std::string &file) { mRidFile = file; }

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

            /** 
             * Returns the currect connection state
             * @return mState
             * @see class ConnectionState
             */
            const ConnectionState& GetConnectionState() const { return *mState; }

            /**
             * Tells this component to connect to the federation
             */
            void Connect();
            
            /**
             * Disconnects from the federation
             */
            void Disconnect();

            /**
             * Processes messages to we can sync up events on ticks
             * @param msg The message to process
             */
            virtual void ProcessMessage(const dtGame::Message &msg);

            /// Fills a vector with an additional maps  list loaded from the application config.
            void GetAdditionalMaps(std::vector<std::string>& toFill) const;

         protected:

            /**
             * Returns a reference to the HLAGMComponent we use
             * @return The component
             */
            dtHLAGM::HLAComponent& GetHLAComponent();

            /// Destructor
            virtual ~HLAConnectionComponent();


         private:

            std::vector<std::string> mMapNames;
            std::string mConfigFile;
            std::string mFedEx;
            std::string mFedName;
            std::string mFedFile;
            std::string mRidFile;

            const ConnectionState *mState;
      };
   }
}
