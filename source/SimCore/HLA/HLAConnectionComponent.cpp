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
#include <prefix/SimCorePrefix.h>
#include <SimCore/HLA/HLAConnectionComponent.h>
#include <SimCore/IGExceptionEnum.h>
#include <SimCore/Utilities.h>
#include <SimCore/Components/TimedDeleterComponent.h>

#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/ddmcalculatorgeographic.h>

#ifdef DIS_CONNECTIONS_AVAILABLE
   #include <dtDIS/mastercomponent.h>
   #include <dtDIS/sharedstate.h>
#endif

#include <dtActors/engineactorregistry.h>
#include <dtActors/coordinateconfigactor.h>

#include <dtGame/exceptionenum.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gmsettings.h>
#include <dtABC/application.h>

#include <dtDAL/project.h>

#include <dtUtil/stringutils.h>

#include <dtNetGM/servernetworkcomponent.h>
#include <dtNetGM/clientnetworkcomponent.h>

#include <sstream>

namespace SimCore
{
   namespace HLA
   {
      IMPLEMENT_ENUM(HLAConnectionComponent::ConnectionState);
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED("NOT_CONNECTED");
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_CONNECTING("CONNECTING");
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_CONNECTED("CONNECTED");
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_ERROR("ERROR");

      IMPLEMENT_ENUM(HLAConnectionComponent::ConnectionType);
      HLAConnectionComponent::ConnectionType HLAConnectionComponent::ConnectionType::TYPE_NONE("NONE");
      HLAConnectionComponent::ConnectionType HLAConnectionComponent::ConnectionType::TYPE_HLA("TYPE_HLA");
      HLAConnectionComponent::ConnectionType HLAConnectionComponent::ConnectionType::TYPE_CLIENTSERVER("TYPE_CLIENTSERVER");
      HLAConnectionComponent::ConnectionType HLAConnectionComponent::ConnectionType::TYPE_DIS("TYPE_DIS");
      HLAConnectionComponent::ConnectionType HLAConnectionComponent::ConnectionType::TYPE_OTHER("TYPE_OTHER");

      const std::string HLAConnectionComponent::DEFAULT_NAME("HLAConnectionComponent");

      ///////////////////////////////////////////////////////////////////////
      HLAConnectionComponent::HLAConnectionComponent(const std::string &name)
         : dtGame::GMComponent(name)
         , mRidFile("RTI.rid") // default to an RTI.rid file so that there is something to find.
         , mConnectionType(&ConnectionType::TYPE_NONE)
         , mServerGameVersion(1)
         , mState(&ConnectionState::STATE_NOT_CONNECTED)
      {

      }

      ///////////////////////////////////////////////////////////////////////
      HLAConnectionComponent::~HLAConnectionComponent()
      {

      }

      ///////////////////////////////////////////////////////////////////////
      dtHLAGM::HLAComponent* HLAConnectionComponent::GetHLAComponent()
      {
         dtHLAGM::HLAComponent* component;
         GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME, component);

         return component;
      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::ProcessMessage(const dtGame::Message &msg)
      {
         // The connect process starts by loading a map. Once that is done, THEN we
         // can connect to the network.
         if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            DoReconnectToNetwork();
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::DoReconnectToNetwork()
      {
         // Look for a coordinate config actor.
         dtActors::CoordinateConfigActor* ccActor = NULL;
         std::vector<dtDAL::ActorProxy*> proxies;
         GetGameManager()->FindActorsByType(*dtActors::EngineActorRegistry::COORDINATE_CONFIG_ACTOR_TYPE, proxies);
         if(proxies.empty())
         {
            LOG_ERROR("!!!! ERROR !!!! -- Failed to find a coordinate config actor in the map. This will likely cause major runtime problems or even a crash!!!");
         }
         else
         {
            proxies[0]->GetActor(ccActor);
         }

         // HLA
         if (*mConnectionType == ConnectionType::TYPE_HLA)
         {
            DoConnectToHLA(ccActor);
            mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTED;
         }
         // CLIENT SERVER
         else if (*mConnectionType == ConnectionType::TYPE_CLIENTSERVER)
         {
            DoConnectToClientServer(ccActor);
            mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTED;
         }
         // DIS
         else if (*mConnectionType == ConnectionType::TYPE_DIS)
         {
            mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTED;
         }

         // Unpause the GM after we connect. Else, we might get stuck after a replay.
         GetGameManager()->SetPaused(false);
      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::DoConnectToHLA(dtActors::CoordinateConfigActor* ccActor)
      {
         dtHLAGM::HLAComponent* hlaComp = GetHLAComponent();
         if(hlaComp == NULL)
         {
            throw dtGame::InvalidParameterException(
               "Failed to find the HLAComponent on the GameManager. Aborting...", __FILE__, __LINE__);
         }
         hlaComp->ClearConfiguration();

         dtHLAGM::HLAComponentConfig componentConfig;
         try
         {
            componentConfig.LoadConfiguration(*hlaComp, mConfigFile);
            hlaComp->JoinFederationExecution(mFedEx, mFedFile, mFedName, mRidFile);
         }
         catch(const dtUtil::Exception &e)
         {
            mState = &HLAConnectionComponent::ConnectionState::STATE_ERROR;
            throw e;
         }

         // Use our coordinate converter if we have one
         if (ccActor != NULL)
         {
            hlaComp->GetCoordinateConverter() = ccActor->GetCoordinateConverter();
            std::vector<dtHLAGM::DDMRegionCalculator*> calcs;
            hlaComp->GetDDMSubscriptionCalculators().GetCalculators(calcs);
            for(size_t i = 0; i < calcs.size(); ++i)
            {
               dtHLAGM::DDMCalculatorGeographic* geoCalc = dynamic_cast<dtHLAGM::DDMCalculatorGeographic*>(calcs[i]);
               if(geoCalc != NULL)
               {
                  geoCalc->SetCoordinateConverter(ccActor->GetCoordinateConverter());
               }
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::DoConnectToClientServer(dtActors::CoordinateConfigActor* ccActor)
      {
         // Curt - Do Something here!!!
         //LOG_ALWAYS("Time to do some client server connection!!!");

         // Find the existing client component. If none exists, then create it.
         dtNetGM::ClientNetworkComponent* clientNetworkComponent;
         GetGameManager()->GetComponentByName(dtNetGM::ClientNetworkComponent::DEFAULT_NAME, clientNetworkComponent);
         if(clientNetworkComponent == NULL)
         {
            mState = &HLAConnectionComponent::ConnectionState::STATE_ERROR;
            throw dtGame::InvalidParameterException(
               "Error - could not find Client Networking Component in the Game Manager.", __FILE__, __LINE__);
         }

         // Setup our client and then send a client connection request.
         if (clientNetworkComponent != NULL)
         {
            std::string noticeMessage = "Setting up client networking component for host[" + mServerIPAddress + 
               "] using port[" + mServerPort + "].";
            LOG_WARNING(noticeMessage);

            int serverPort = dtUtil::ToType<int>(mServerPort);
            //clientNetworkComponent->SetNetworkOptions("NetDemo", 1);
            if (clientNetworkComponent->SetupClient(mServerIPAddress, serverPort))
            {
               clientNetworkComponent->SendRequestConnectionMessage();
            }
            else 
            {
               mState = &HLAConnectionComponent::ConnectionState::STATE_ERROR;
               std::string errorMsg = "Failed to establish a client connection to server[" + mServerIPAddress +
                  "] using port[" + mServerPort + "].";
               throw dtGame::InvalidParameterException(errorMsg, __FILE__, __LINE__);
            }
         }

      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::StartNetworkConnection()
      {
         if(mMapNames.empty())
         {
            throw SimCore::IGException("You have tried to connect when no maps have been specified. \
                Please specify the name of the map to load for this connection", __FILE__, __LINE__);
         }
         else if (GetGameManager() == NULL)
         {
            throw dtUtil::Exception(
               "You have tried to connect without adding this component to the Game Manager.", __FILE__, __LINE__);
         }

         // Start in NOT connected state - set to connected when we succeed.
         mState = &HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED;

         // Determine if the specified map is valid.
         std::string& mapName = mMapNames[0];

         try
         {
            // Set the client & server roles, which are different based on network type.
            // For HLA, use CLIENT AND SERVER
            if (*mConnectionType == ConnectionType::TYPE_HLA)
            {
               GetGameManager()->GetGMSettings().SetServerRole(true);
               GetGameManager()->GetGMSettings().SetClientRole(true);
            }
            else if (*mConnectionType == ConnectionType::TYPE_CLIENTSERVER)
            {
               // For ClientServer, as the Stealth Viewer, we are only a CLIENT
               GetGameManager()->GetGMSettings().SetServerRole(false);
               GetGameManager()->GetGMSettings().SetClientRole(true);
            }
            else if (*mConnectionType == ConnectionType::TYPE_DIS)
            {
               // For DIS, as the Stealth Viewer, we are CLIENT AND SERVER
               GetGameManager()->GetGMSettings().SetServerRole(true);
               GetGameManager()->GetGMSettings().SetClientRole(true);
            }

            SimCore::Utils::LoadMaps(*GetGameManager(), mapName);
            mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTING;
         }
         catch(const dtUtil::Exception& e)
         {
            mMapNames.clear();
            mState = &HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED;
            throw e;
         }

         // We recreate the Client Network Component with each connection because there
         // are config options that can only be set in the constructor
         // Note - this only works because we are not in the middle of a message from the GM.
         if (*mConnectionType == ConnectionType::TYPE_CLIENTSERVER)
         {
            //LOG_WARNING("Creating new client networking component during connection.");
            dtNetGM::ClientNetworkComponent* clientNetworkComponent;
            GetGameManager()->GetComponentByName(dtNetGM::ClientNetworkComponent::DEFAULT_NAME, clientNetworkComponent);
            if(clientNetworkComponent == NULL) // if not already created, create one. Remove this eventually, see two lines down.
            {
               dtCore::RefPtr<dtNetGM::ClientNetworkComponent> clientNetworkComponent = 
                  new dtNetGM::ClientNetworkComponent(mServerGameName, mServerGameVersion);
               // NOTE - The GM needs to be modified to support adding a component during a message - 12/21/09 CMM
               GetGameManager()->AddComponent(*clientNetworkComponent, dtGame::GameManager::ComponentPriority::NORMAL);
            }
         }
#ifdef DIS_CONNECTIONS_AVAILABLE
         else if (*mConnectionType == ConnectionType::TYPE_DIS)
         {
            dtDIS::MasterComponent* disComponent;
            GetGameManager()->GetComponentByName(dtDIS::MasterComponent::DEFAULT_NAME, disComponent);
            if(disComponent != NULL) // if it was already created, remove the old one and replace it with a new one
            {
               GetGameManager()->RemoveComponent(*disComponent);
            }

            dtDIS::ConnectionData disConnectionData;
            disConnectionData.ip = mDISIPAddress;
            disConnectionData.port = mDISPort;
            disConnectionData.exercise_id = mDISExerciseID;
            disConnectionData.site_id = mDISSiteID;
            disConnectionData.application_id = mDISApplicationID;
            disConnectionData.MTU = mDISMTU;

            dtDIS::SharedState* disConfig = new dtDIS::SharedState("", mDISActorXMLFile);
            disConfig->SetConnectionData(disConnectionData);

            disComponent = new dtDIS::MasterComponent(disConfig);
            GetGameManager()->AddComponent(*disComponent);
         }
#endif
      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::Disconnect(bool closeMap)
      {
         if( *mState != ConnectionState::STATE_NOT_CONNECTED )
         {
            if (closeMap)
            {
               GetGameManager()->CloseCurrentMap();
               mMapNames.clear();
            }

            // Next 2 lines taken from StealthInputComponent.cpp
            // Clear any munitions or particle effects that may have been around BEFORE we joined.
            // This is necessary to stop problems with lingering effects during and after playback
            SimCore::Components::TimedDeleterComponent *deleterComp =
               dynamic_cast<SimCore::Components::TimedDeleterComponent*> (GetGameManager()->
               GetComponentByName(SimCore::Components::TimedDeleterComponent::DEFAULT_NAME));
            if (deleterComp != NULL)
               deleterComp->Clear();


            // DISCONNECT HLA
            dtHLAGM::HLAComponent* hlaComp = GetHLAComponent();
            if (hlaComp != NULL && hlaComp->IsConnectedToFederation())
            {
               try
               {
                  hlaComp->LeaveFederationExecution();
               }
               catch (const dtUtil::Exception& ex)
               {
                  ex.LogException(dtUtil::Log::LOG_ERROR);
               }
            }

            // DISCONNECT CLIENT SERVER
            dtNetGM::ClientNetworkComponent* clientNetworkComponent;
            GetGameManager()->GetComponentByName(dtNetGM::ClientNetworkComponent::DEFAULT_NAME, clientNetworkComponent);
            if(clientNetworkComponent != NULL)
            {
               if (clientNetworkComponent->IsConnectedClient())
               {
                  //clientNetworkComponent->ShutdownNetwork();
                  clientNetworkComponent->Disconnect();
               }
               // NOTE - Removing components is not valid on the GM. Once the GM is modified to handle this, 
               // This should be put back. CMM 12/21/09
               // We always delete the network component, because we have to re-construct one each time (see above)
               //GetGameManager()->RemoveComponent(*clientNetworkComponent);
            }

            mState = &ConnectionState::STATE_NOT_CONNECTED;
         }
      }

      ///////////////////////////////////////////////////////////////////////
      bool HLAConnectionComponent::IsConnected()
      {
         // This method is used as part of the aar start and stop. That shuts down our network and we have
         // to reconnect

         // check HLA.
         dtHLAGM::HLAComponent* hlaComp = GetHLAComponent();
         if (hlaComp != NULL && hlaComp->IsConnectedToFederation())
         {
            return true;
         }

         // check client server
         dtNetGM::ClientNetworkComponent* clientNetworkComponent;
         GetGameManager()->GetComponentByName(dtNetGM::ClientNetworkComponent::DEFAULT_NAME, clientNetworkComponent);
         if(clientNetworkComponent != NULL && clientNetworkComponent->IsConnectedClient())
         {
            return true;
         }

         return false;
      }
   }
}
