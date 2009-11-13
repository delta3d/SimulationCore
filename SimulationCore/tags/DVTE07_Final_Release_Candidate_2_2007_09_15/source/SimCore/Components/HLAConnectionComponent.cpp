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

#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/ddmcalculatorgeographic.h>

#include <dtActors/engineactorregistry.h>
#include <dtActors/coordinateconfigactor.h>

namespace SimCore
{
   namespace Components
   {
      const std::string &HLAConnectionComponent::DEFAULT_NAME = "HLAConnectionComponent";

      HLAConnectionComponent::HLAConnectionComponent(const std::string &name) : 
         dtGame::GMComponent(name), 
         mRidFile("RTI.rid"), // default to an RTI.rid file so that there is something to find.
         mIsConnected(false)
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
            mIsConnected = true;

            dtHLAGM::HLAComponent& hlaComp = GetHLAComponent();
            hlaComp.ClearConfiguration();
            
            dtHLAGM::HLAComponentConfig componentConfig;
            componentConfig.LoadConfiguration(hlaComp, mConfigFile);
            hlaComp.JoinFederationExecution(mFedEx, mFedFile, mFedName, mRidFile);

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
            for (unsigned i = 0; i < calcs.size(); ++i)
            {
               dtHLAGM::DDMCalculatorGeographic* geoCalc = dynamic_cast<dtHLAGM::DDMCalculatorGeographic*>(calcs[i]);
               
               if (geoCalc != NULL)
               {
                  geoCalc->SetCoordinateConverter(ccActor->GetCoordinateConverter());
               }
            }
         }
      }

      void HLAConnectionComponent::Connect()
      {
         // Temporary fix added by Eddie. This is not particularly hackish, and 
         // maintains support for both the Stealth Viewer and the other apps
         // that requires multiple map support
         if(mMapNames.size() == 1)
         {
            GetGameManager()->ChangeMap(mMapNames[0], false, true);
         }
         else
         {
            GetGameManager()->ChangeMapSet(mMapNames, false, true);
         }
      }

      void HLAConnectionComponent::Disconnect()
      {
         GetGameManager()->CloseCurrentMap();
         dtHLAGM::HLAComponent& hlaComp = GetHLAComponent();
         try
         {
            hlaComp.LeaveFederationExecution();
         }
         catch (const dtUtil::Exception& ex)
         {
            ex.LogException(dtUtil::Log::LOG_ERROR);
         }

         mIsConnected = false;
      }
   }  
}