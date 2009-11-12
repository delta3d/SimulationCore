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
 * @author David Guthrie
 */
#include <StealthViewer/GMApp/StealthGameEntryPoint.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <StealthViewer/GMApp/StealthMessageProcessor.h>
#include <StealthViewer/GMApp/ViewerConfigComponent.h>

#include <dtCore/camera.h>
#include <dtCore/system.h>
#include <dtCore/particlesystem.h>
#include <dtCore/scene.h>

#include <dtUtil/coordinates.h>

#include <dtAudio/audiomanager.h>

#include <dtDAL/project.h>
#include <dtDAL/actorproperty.h>

#include <dtABC/application.h>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/binarylogstream.h>
#include <dtGame/logtag.h>
#include <dtGame/logkeyframe.h>
#include <dtGame/logstatus.h>
#include <dtGame/loggermessages.h>
#include <dtGame/logcontroller.h>
#include <dtGame/serverloggercomponent.h>
#include <dtGame/gameapplication.h>
#include <dtGame/environmentactor.h>

#include <dtHLAGM/hlacomponent.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/StealthActor.h>

#include <SimCore/Components/ViewerNetworkPublishingComponent.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/ControlStateComponent.h>
#include <SimCore/Tools/GPS.h>
#include <SimCore/Tools/Compass.h>
#include <SimCore/Tools/Binoculars.h>
#include <SimCore/MessageType.h>
#include <SimCore/WeaponTypeEnum.h>

#include <SimCore/HLA/HLAConnectionComponent.h>

#include <osg/ApplicationUsage>
#include <osg/ArgumentParser>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
#endif

using dtCore::RefPtr;
using dtCore::ObserverPtr;

namespace StealthGM
{
   ///////////////////////////////////////////////////////////////////////////
   extern "C" STEALTH_GAME_EXPORT dtGame::GameEntryPoint* CreateGameEntryPoint()
   {
      return new StealthGameEntryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" STEALTH_GAME_EXPORT void DestroyGameEntryPoint(dtGame::GameEntryPoint* entryPoint)
   {
      delete entryPoint;
   }

   const std::string StealthGameEntryPoint::CONFIG_HAS_BINOCS("HasBinocs");
   const std::string StealthGameEntryPoint::CONFIG_HAS_COMPASS("HasCompass");
   const std::string StealthGameEntryPoint::CONFIG_HAS_GPS("HasGPS");
   const std::string StealthGameEntryPoint::CONFIG_HAS_NIGHT_VISION("HasNightVision");
   const std::string StealthGameEntryPoint::CONFIG_HAS_MAP_TOOL("HasMapTool");

   ///////////////////////////////////////////////////////////////////////////
   StealthGameEntryPoint::StealthGameEntryPoint() :
      mEnableLogging(false),
      mEnablePlayback(false),
      mHasBinoculars(false),
      mHasCompass(false),
      mHasLRF(false),
      mHasGPS(false),
      mHasNightVis(false),
      mHasMap(false)
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   StealthGameEntryPoint::~StealthGameEntryPoint()
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   void StealthGameEntryPoint::Initialize(dtGame::GameApplication& app, int argc, char **argv)
   {
      if(parser == NULL)
         parser = new osg::ArgumentParser(&argc, argv);

      parser->getApplicationUsage()->setCommandLineUsage("Stealth Viewer Application [options] value ...");
      parser->getApplicationUsage()->addCommandLineOption("--enableLogging","Specify 1 or 0 to enable and disable logging of the scene.");
      parser->getApplicationUsage()->addCommandLineOption("--enablePlayback","Specify 1 if this is playback for after action review (AAR)");
      
      int tempValue = 0;
      if(!parser->read("--enableLogging", tempValue))
   	{
   		mEnableLogging = false;
   	}
      else
      {
         mEnableLogging = (tempValue == 1) ? true : false;
         tempValue = 0;
      }
      
   	if(!parser->read("--enablePlayback", tempValue))
   	{
   		mEnablePlayback = false;
   	}
      else
      {
         mEnablePlayback = (tempValue == 1) ? true : false;
      }   
   
      if( parser->read("--hasBinos") )
      {
         mHasBinoculars = true;
      }
      if( parser->read("--hasCompass") )
      {
         mHasCompass = true;
      }
      if( parser->read("--hasGPS") )
      {
         mHasGPS = true;
      }
      if( parser->read("--hasNightVis") )
      {
         mHasNightVis = true;
      }
      if( parser->read("--hasMap") )
      {
         mHasMap = true;
      }
   
      SimCore::HLA::BaseHLAGameEntryPoint::Initialize(app, argc, argv);
   }
   
   ///////////////////////////////////////////////////////////////////////////
   static void ReadBoolConfigProperty(const std::string& name, bool& value, dtUtil::ConfigProperties& config)
   {
      std::string stringValue;
      stringValue = config.GetConfigPropertyValue(name);
      if (!stringValue.empty())
      {
         value = dtUtil::ToType<bool>(stringValue);
      }
   }
   
   ///////////////////////////////////////////////////////////////////////////
   void StealthGameEntryPoint::InitializeTools(dtGame::GameManager &gm)
   {

      dtABC::Application& app = gm.GetApplication();
      ReadBoolConfigProperty(StealthGameEntryPoint::CONFIG_HAS_BINOCS, mHasBinoculars, app);
      ReadBoolConfigProperty(StealthGameEntryPoint::CONFIG_HAS_COMPASS, mHasCompass, app);
      ReadBoolConfigProperty(StealthGameEntryPoint::CONFIG_HAS_GPS, mHasGPS, app);
      ReadBoolConfigProperty(StealthGameEntryPoint::CONFIG_HAS_MAP_TOOL, mHasMap, app);
      ReadBoolConfigProperty(StealthGameEntryPoint::CONFIG_HAS_NIGHT_VISION, mHasNightVis, app);

      // Check for enabled tools
      StealthInputComponent* inputComp = 
         dynamic_cast<StealthInputComponent*>(gm.GetComponentByName(StealthInputComponent::DEFAULT_NAME));
   
      if( inputComp == NULL ) { return; }
   
      if( mHasNightVis )
      {
         // TODO: Add new Night Vision tool
         mHudGUI->AddToolButton("NightVision","F7",false);
      }

      if( mHasNightVis )
      {
         // TODO: this probably needs to be refactored, for the moment I just needed a tool to add which allows us to support NVGS
         SimCore::Tools::Tool* nvgs = new SimCore::Tools::Tool(mHudGUI->GetToolsWindow());
         inputComp->AddTool(*nvgs, SimCore::MessageType::NIGHT_VISION);
         mHudGUI->AddToolButton("NightVision","F7");
      }
   
      if( mHasCompass )
      {
         dtCore::Camera *camera = inputComp->GetGameManager()->GetApplication().GetCamera(); 
         float aspectRatio = camera->GetAspectRatio();
         SimCore::Tools::Compass* compass = new SimCore::Tools::Compass(mHudGUI->GetToolsWindow(), *inputComp->GetGameManager()->GetApplication().GetCamera(), false, aspectRatio);         compass->SetPlayerActor(mStealth.get());
         compass->InitLens( *mHudGUI->GetGUIDrawable() );
         inputComp->AddTool(
            *compass,
            SimCore::MessageType::COMPASS
            );
         mHudGUI->AddToolButton("Compass","F8");
      }

      if( mHasBinoculars )
      {
         SimCore::Tools::Binoculars *binos = 
            new SimCore::Tools::Binoculars(
            *inputComp->GetGameManager()->GetApplication().GetCamera(), mHudGUI->GetToolsWindow());

         inputComp->AddTool(
            *binos,
            SimCore::MessageType::BINOCULARS
            );

         binos->SetPlayerActor(mStealth.get());
         mHudGUI->AddToolButton("Binoculars","F9");
      }
//      if( mHasMap )
//      {
//         SimCore::Tools::Map* mapTool = new SimCore::Tools::Map(mHudGUI->GetToolsWindow());
//
//         inputComp->AddTool( *mapTool, SimCore::MessageType::MAP );
//         mHudGUI->AddToolButton("Map","F11");
//      }
   
   }
   
   ///////////////////////////////////////////////////////////////////////////
   /*ObserverPtr<dtGame::GameManager> StealthGameEntryPoint::CreateGameManager(dtCore::Scene& scene)
   {
      return BaseGameEntryPoint::CreateGameManager(scene);
   }*/

   ///////////////////////////////////////////////////////////////////////////
   void StealthGameEntryPoint::HLAConnectionComponentSetup(dtGame::GameManager &gm)
   {
      SimCore::HLA::BaseHLAGameEntryPoint::HLAConnectionComponentSetup(gm);
   }
   
   ///////////////////////////////////////////////////////////////////////////
   void StealthGameEntryPoint::OnStartup(dtGame::GameApplication& app)
   {
      dtGame::GameManager &gameManager = *app.GetGameManager();//*GetGameManager();
      
      dtCore::Transform stealthStart;
      RefPtr<dtGame::GameActorProxy> ap;

      SimCore::HLA::BaseHLAGameEntryPoint::OnStartup(app);

      // Add Input Component
      dtCore::RefPtr<StealthInputComponent> mInputComponent 
         = new StealthInputComponent(mEnableLogging, 
                                             mEnablePlayback, 
                                             StealthInputComponent::DEFAULT_NAME, 
                                             IsUIRunning());

      gameManager.AddComponent(*mInputComponent, dtGame::GameManager::ComponentPriority::NORMAL);

      // Capture HLA connection parameters for the input component to use later in record/playback 
      // state swapping. Transitions to IDLE should join the federation; PLAYBACK should leave
      // the connection to the federation.
      StealthInputComponent* inputComp = mInputComponent.get();
   
      // AAR logging
      if (mEnableLogging)
      {
         // AAR LOGGING COMPONENTS
         //To enable logging, we must have three parts.  A server logger component, a log controller, 
         // and a log stream to save messages.  
         dtGame::BinaryLogStream *logStream = new dtGame::BinaryLogStream(gameManager.GetMessageFactory());
         mServerLogger = new dtGame::ServerLoggerComponent(*logStream);
         mLogController = new dtGame::LogController;
         gameManager.AddComponent(*mServerLogger, dtGame::GameManager::ComponentPriority::NORMAL);
         gameManager.AddComponent(*mLogController, dtGame::GameManager::ComponentPriority::NORMAL);
   
         // Set auto keyframe interval
         mLogController->RequestSetAutoKeyframeInterval(20.0f);
   
         // Ignore constant actors from being recorded. This protects them from
         // deletion over ServerLoggerComponent state changes; such as transitions
         // from PLAYBACK to IDLE states.
         //--- Terrain & Sky
         // NOTE: Code has been moved to StealthInputComponent.ProcessMessage(INFO_MAP_LOADED).
         // The terrain will not exist at this point, but will exist later when a message is received.
   
         //--- Camera
         //mLogController->RequestAddIgnoredActor(mStealth->GetUniqueId());
      }
   
      // HUD GUI COMPONENT
      mHudGUI = new StealthHUD(gameManager.GetApplication().GetWindow(), 
                               mLogController.get(), 
                               StealthHUD::DEFAULT_NAME, 
                               IsUIRunning());

      gameManager.AddComponent(*mHudGUI, dtGame::GameManager::ComponentPriority::NORMAL);

      mHudGUI->Initialize();

      // Control State Component (for swapping weapons on remote HMMWV vehicles)
      dtCore::RefPtr<SimCore::Components::ControlStateComponent> controlsStateComp
         = new SimCore::Components::ControlStateComponent;
      gameManager.AddComponent(*controlsStateComp, dtGame::GameManager::ComponentPriority::NORMAL);

      std::vector<std::string>& weaponModelFileList = controlsStateComp->GetWeaponModelFileList();
      SimCore::WeaponTypeEnum::GetModelFileUrlList( weaponModelFileList );

      // This function will initialize both the HUD and Input Component
      // with tools that have been enabled in the command line arguments.
      InitializeTools(*app.GetGameManager());

      // Setup Stealth Actor (ie player and camera)
      //if (mStealth == NULL)
      //{
      //   LOG_ERROR("Failed to find the stealth actor");
      //   gameManager.GetApplication().Quit();
      //}
#ifdef AGEIA_PHYSICS
      dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaWorldComponent> ageiaComponent = new dtAgeiaPhysX::NxAgeiaWorldComponent();
      gameManager.AddComponent(*ageiaComponent, dtGame::GameManager::ComponentPriority::NORMAL);
#endif
      //mStealth->SetName("Stealth");
      //mStealth->SetTransform(stealthStart);
      //gameManager.AddActor(mStealth->GetGameActorProxy(), false, false);
      //gameManager.PublishActor(mStealth->GetGameActorProxy());
      //mStealth->AddChild(gameManager.GetApplication().GetCamera());
   
      // The stealth processor component is now obsolete since the updates to the AAR ignore actor behavior.
      RefPtr<StealthMessageProcessor> stealthProcessor = new StealthMessageProcessor;
      gameManager.AddComponent(*stealthProcessor, 
                               dtGame::GameManager::ComponentPriority::HIGHEST);
      gameManager.AddComponent(*new StealthGM::ViewerConfigComponent, 
                               dtGame::GameManager::ComponentPriority::HIGHER);

      //this enables night vision
      dtCore::RefPtr<SimCore::Components::RenderingSupportComponent> renderingSupportComponent 
         = new SimCore::Components::RenderingSupportComponent();
      renderingSupportComponent->SetEnableCullVisitor(false);

      gameManager.AddComponent(*renderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);

      if(!IsUIRunning())
      {
         const std::string fedFile = dtDAL::Project::GetInstance().
            GetResourcePath(dtDAL::ResourceDescriptor(mFedFileResource, mFedFileResource));
         // Capture HLA connection parameters for the input component to use later in record/playback 
         // state swapping. Transitions to IDLE should join the federation; PLAYBACK should leave
         // the connection to the federation.
         inputComp->SetConnectionParameters(mFederationExecutionName, fedFile, mFederateName);
      }
   }

}