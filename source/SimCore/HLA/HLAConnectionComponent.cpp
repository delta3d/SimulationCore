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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/HLA/HLAConnectionComponent.h>
#include <SimCore/IGExceptionEnum.h>
#include <SimCore/Utilities.h>

#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/ddmcalculatorgeographic.h>

#include <dtActors/engineactorregistry.h>
#include <dtActors/coordinateconfigactor.h>

#include <dtGame/exceptionenum.h>
#include <dtABC/application.h>

#include <dtDAL/project.h>

#include <dtUtil/stringutils.h>

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

      const std::string HLAConnectionComponent::DEFAULT_NAME("HLAConnectionComponent");

      ///////////////////////////////////////////////////////////////////////
      HLAConnectionComponent::HLAConnectionComponent(const std::string &name) :
         dtGame::GMComponent(name),
         mRidFile("RTI.rid"), // default to an RTI.rid file so that there is something to find.
         mState(&ConnectionState::STATE_NOT_CONNECTED)
      {

      }

      ///////////////////////////////////////////////////////////////////////
      HLAConnectionComponent::~HLAConnectionComponent()
      {

      }

      ///////////////////////////////////////////////////////////////////////
      dtHLAGM::HLAComponent& HLAConnectionComponent::GetHLAComponent()
      {
         dtHLAGM::HLAComponent* component;
         GetGameManager()->GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME, component);

         if(component == NULL)
         {
            throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
               "Failed to find the HLAComponent on the GameManager. Aborting application.",
               __FILE__, __LINE__);
         }

         return *component;
      }

      ///////////////////////////////////////////////////////////////////////
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

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::Connect()
      {
         if(mMapNames.empty())
         {
            throw dtUtil::Exception(IGExceptionEnum::INVALID_CONNECTION_DATA,
               "You have tried to connect when no maps have been specified. \
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
            SimCore::Utils::LoadMaps(*GetGameManager(), mapName);
            mState = &HLAConnectionComponent::ConnectionState::STATE_CONNECTING;
         }
         catch(const dtUtil::Exception& e)
         {
            mMapNames.clear();
            mState = &HLAConnectionComponent::ConnectionState::STATE_NOT_CONNECTED;
            throw e;
         }
      }

      ///////////////////////////////////////////////////////////////////////
      void HLAConnectionComponent::Disconnect()
      {
         if( *mState != ConnectionState::STATE_NOT_CONNECTED )
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
}
