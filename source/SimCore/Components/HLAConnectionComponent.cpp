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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Components/HLAConnectionComponent.h>
#include <SimCore/IGExceptionEnum.h>

#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/ddmcalculatorgeographic.h>

#include <dtActors/engineactorregistry.h>
#include <dtActors/coordinateconfigactor.h>

namespace SimCore
{
   namespace Components
   {
      IMPLEMENT_ENUM(HLAConnectionComponent::ConnectionState);
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED("NOT_CONNECTED");
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_CONNECTING("CONNECTING");
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_CONNECTED("CONNECTED");
      const HLAConnectionComponent::ConnectionState HLAConnectionComponent::ConnectionState::STATE_ERROR("ERROR");

      const std::string HLAConnectionComponent::DEFAULT_NAME = "HLAConnectionComponent";

      HLAConnectionComponent::HLAConnectionComponent(const std::string &name) : 
         dtGame::GMComponent(name), 
         mRidFile("RTI.rid"), // default to an RTI.rid file so that there is something to find.
         mState(&ConnectionState::STATE_NOT_CONNECTED)
      {

      }

      HLAConnectionComponent::~HLAConnectionComponent()
      {

      }

      dtHLAGM::HLAComponent& HLAConnectionComponent::GetHLAComponent()
      {
         dtGame::GMComponent *component = 
            GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME);

         if(component == NULL)
         {
            throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, 
               "Failed to find the HLAComponent on the GameManager. Aborting application.", 
               __FILE__, __LINE__);
         }

         return static_cast<dtHLAGM::HLAComponent&>(*component);
      }
      
      void HLAConnectionComponent::ProcessMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            dtHLAGM::HLAComponent& hlaComp = GetHLAComponent();
            hlaComp.ClearConfiguration();
            
            dtHLAGM::HLAComponentConfig componentConfig;
            try
            {
               componentConfig.LoadConfiguration(hlaComp, mConfigFile);
               hlaComp.JoinFederationExecution(mFedEx, mFedFile, mFedName, mRidFile);
            }
            catch(const dtUtil::Exception &e)
            {
               mState = &HLAConnectionComponent::ConnectionState::STATE_ERROR;
               throw e;
            }

            mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTED;

            std::vector<dtDAL::ActorProxy*> proxies;
            GetGameManager()->FindActorsByType(*dtActors::EngineActorRegistry::COORDINATE_CONFIG_ACTOR_TYPE, proxies);
            if(proxies.empty())
            {
               LOG_ERROR("Failed to find a coordinate config actor in the map. Using default values.");
               return;
            }

            dtActors::CoordinateConfigActor* ccActor; 
            proxies[0]->GetActor(ccActor);

            hlaComp.GetCoordinateConverter() = ccActor->GetCoordinateConverter();
            std::vector<dtHLAGM::DDMRegionCalculator*> calcs;
            hlaComp.GetDDMSubscriptionCalculators().GetCalculators(calcs);
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

      void HLAConnectionComponent::Connect()
      {
         if(mMapNames.empty())
         {
            throw dtUtil::Exception(IGExceptionEnum::INVALID_CONNECTION_DATA, 
               "You have tried to connect when no maps have been specified. \
                Please specify the name of the map to load for this connection", __FILE__, __LINE__);
         }

         // HACK: Let the HLAComponent know about the prototype maps it
         // should add along with the terrain map. This should be a UI feature.
         //
         // If the component had disconnected, the map names previously pushed
         // onto the HLA component's map list will have been cleared and lost.
         // This ensures that all commonly used proto maps are loaded along with the terrain
         // and ensures that the environment actor can be instantiated from a prototype.
         mMapNames.push_back("DVTEPrototypes");
         mMapNames.push_back("DVTEMaterials");
         mMapNames.push_back("DVTEActors");

         // Temporary fix added by Eddie. This is not particularly hackish, and 
         // maintains support for both the Stealth Viewer and the other apps
         // that requires multiple map support
         GetGameManager()->ChangeMapSet(mMapNames, false, true);
         mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTING;
      }

      void HLAConnectionComponent::Disconnect()
      {
         GetGameManager()->CloseCurrentMap();
         mMapNames.clear();
         dtHLAGM::HLAComponent& hlaComp = GetHLAComponent();
         try
         {
            hlaComp.LeaveFederationExecution();
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }

         mState = &ConnectionState::STATE_NOT_CONNECTED;
      }
   }  
}
