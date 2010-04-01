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
 * @author David Guthrie
 */
#include <SimCore/HLA/BaseHLAGameEntryPoint.h>

#include <dtUtil/log.h>
#include <dtCore/refptr.h>
#include <dtDAL/project.h>

#include <dtGame/gameapplication.h>

#include <dtHLAGM/hlacomponent.h>
#include <dtHLAGM/hlacomponentconfig.h>
#include <dtHLAGM/ddmcameracalculatorgeographic.h>
#include <dtHLAGM/ddmmultienumeratedcalculator.h>

#include <SimCore/HLA/HLACustomParameterTranslator.h>
#include <SimCore/HLA/HLAConnectionComponent.h>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>

#include <iostream>

using dtCore::RefPtr;

namespace SimCore
{

   namespace HLA
   {
      /////////////////////////////////////////////////////////////
      BaseHLAGameEntryPoint::BaseHLAGameEntryPoint()
      {
      }

      /////////////////////////////////////////////////////////////
      BaseHLAGameEntryPoint::~BaseHLAGameEntryPoint()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHLAGameEntryPoint::HLAConnectionComponentSetup(dtGame::GameManager &gm)
      {
         if(!IsUIRunning())
         {
            //dtGame::GameManager &gameManager = *GetGameManager();
            SimCore::HLA::HLAConnectionComponent *hlaCC;
            gm.GetComponentByName(SimCore::HLA::HLAConnectionComponent::DEFAULT_NAME, hlaCC);
            if(hlaCC != NULL)
            {
               const std::string fedMappingFile = dtDAL::Project::GetInstance().
                  GetResourcePath(dtDAL::ResourceDescriptor(mFedMappingResource, mFedMappingResource));
               const std::string fedFile = dtDAL::Project::GetInstance().
                  GetResourcePath(dtDAL::ResourceDescriptor(mFedFileResource, mFedFileResource));

               hlaCC->AddMap(mMapName);
               hlaCC->SetConfigFile(fedMappingFile);
               hlaCC->SetFedEx(mFederationExecutionName);
               hlaCC->SetFedName(mFederateName);
               hlaCC->SetFedFile(fedFile);

               // loads all maps
               hlaCC->StartNetworkConnection();
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtHLAGM::HLAComponent> BaseHLAGameEntryPoint::CreateAndSetupHLAComponent(dtGame::GameManager &gm)
      {
         dtCore::RefPtr<dtHLAGM::HLAComponent> hft = new dtHLAGM::HLAComponent;

         RefPtr<dtHLAGM::DDMCameraCalculatorGeographic> camCalc = new dtHLAGM::DDMCameraCalculatorGeographic;
         camCalc->SetCamera(gm.GetApplication().GetCamera());
         camCalc->SetName("Ground");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*camCalc);

         camCalc = new dtHLAGM::DDMCameraCalculatorGeographic;
         camCalc->SetCamera(gm.GetApplication().GetCamera());
         camCalc->SetName("Air");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*camCalc);

         camCalc = new dtHLAGM::DDMCameraCalculatorGeographic;
         camCalc->SetCamera(gm.GetApplication().GetCamera());
         camCalc->SetName("Sea");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*camCalc);

         camCalc = new dtHLAGM::DDMCameraCalculatorGeographic;
         camCalc->SetCamera(gm.GetApplication().GetCamera());
         camCalc->SetName("Lifeform");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*camCalc);

         camCalc = new dtHLAGM::DDMCameraCalculatorGeographic;
         camCalc->SetCamera(gm.GetApplication().GetCamera());
         camCalc->SetName("Stealth");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*camCalc);

         RefPtr<dtHLAGM::DDMMultiEnumeratedCalculator> multiCalc;

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("Blip");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("Fire");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("Detonation");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("AmbientEnvironment");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("TimeQuery");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("TimeValue");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         multiCalc = new dtHLAGM::DDMMultiEnumeratedCalculator;
         multiCalc->SetName("LoggerPlayback");
         hft->GetDDMSubscriptionCalculators().AddCalculator(*multiCalc);

         return hft;
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHLAGameEntryPoint::Initialize(dtGame::GameApplication& app, int argc, char **argv)
      {
         BaseClass::Initialize(app, argc, argv);

         if(parser == NULL)
            parser = new osg::ArgumentParser(&argc, argv);

         if(!IsUIRunning())
         {
            parser->getApplicationUsage()->addCommandLineOption("--federationExecutionName",
                     "Name of the federation execution to use");
            parser->getApplicationUsage()->addCommandLineOption("--fedFileName",
                     "Name of the federation file to use");
            parser->getApplicationUsage()->addCommandLineOption("--fedMappingFileResource",
                     "Name of the federation mapping resource file to load.");
            parser->getApplicationUsage()->addCommandLineOption("--federateName",
                     "A string used to identify this application in a list of federates. It does not have to be unique.");

            if (!parser->read("--federationExecutionName", mFederationExecutionName))
            {
               std::cerr << "Please specify the name of the federation execution to use with the --federationExecutionName option." << std::endl;
               mMissingRequiredCommandLineOption = true;
            }

            if (!parser->read("--fedFileName", mFedFileResource))
            {
               std::cerr << "Please specify the name of the federation file name to use with the --fedFileName option."  << std::endl;
               mMissingRequiredCommandLineOption = true;
            }

            if (!parser->read("--fedMappingFileResource", mFedMappingResource))
            {
               std::cerr << "Please specify the name of the mapping file to use with federation file " + mFedMappingResource  << std::endl;
               mMissingRequiredCommandLineOption = true;
            }

            if(!parser->read("--federateName", mFederateName))
            {
               mFederateName = "Stealth Viewer";
            }
         }
      }

      void BaseHLAGameEntryPoint::InitializeComponents(dtGame::GameManager &gm)
      {
         RefPtr<dtHLAGM::HLAComponent>  hft   = CreateAndSetupHLAComponent(gm);
         RefPtr<HLAConnectionComponent> hlacc = new HLAConnectionComponent;

         gm.AddComponent(*hft, dtGame::GameManager::ComponentPriority::NORMAL);
         gm.AddComponent(*hlacc, dtGame::GameManager::ComponentPriority::NORMAL);

         SimCore::Components::MunitionsComponent* munitionsComp;
         gm.GetComponentByName(SimCore::Components::MunitionsComponent::DEFAULT_NAME, munitionsComp);

         if (munitionsComp == NULL)
         {
            LOG_WARNING("No munitions component was added with the default name. "
                     "The custom parameter translater will not be added to the HLA Component.");
         }
         else
         {
            // Create a munition specific parameter translator
            dtCore::RefPtr<HLACustomParameterTranslator> munitionParamTranslator
               = new HLACustomParameterTranslator;
            // Allow the translator access to the table that maps
            // munition DIS identifiers to the munition names.
            munitionParamTranslator->SetMunitionTypeTable( munitionsComp->GetMunitionTypeTable() );
            // Enable the HLA component to translate MUNITION_TYPE parameters from network.
            hft->AddParameterTranslator( *munitionParamTranslator );
         }

         // called virtual, will get ur overridden version first.
         HLAConnectionComponentSetup(gm);
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseHLAGameEntryPoint::OnShutdown(dtGame::GameApplication &app)
      {
         dtGame::GameManager &gameManager = *app.GetGameManager();//*GetGameManager();
         dtHLAGM::HLAComponent* hft =
            static_cast<dtHLAGM::HLAComponent*>(gameManager.GetComponentByName(dtHLAGM::HLAComponent::DEFAULT_NAME));

         gameManager.RemoveComponent(*hft);
         hft = NULL;
      }
   }

}
