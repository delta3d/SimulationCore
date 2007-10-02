/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
 */
#include <dtGame/gmcomponent.h>

#include <dtUtil/enumeration.h>

#include <SimCore/Export.h>

namespace dtHLAGM
{
   class HLAComponent;
}

namespace SimCore
{
   namespace Components
   {
      class SIMCORE_EXPORT HLAConnectionComponent : public dtGame::GMComponent
      {
         public:

             class SIMCORE_EXPORT ConnectionState : public dtUtil::Enumeration
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
