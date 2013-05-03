/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Curtiss Murphy
*/
#include <DriverGameEntryPoint.h>

#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>
#include <dtGame/messagefactory.h>

#include <dtDAL/project.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/CommandLineObject.h>
#include <SimCore/CollisionGroupEnum.h>

#include <osg/ApplicationUsage>

// turns mouse off
#include <dtCore/deltawin.h>
#include <dtCore/camera.h>

#include <dtUtil/configproperties.h>

///////////////////////////////////
// extra components to init
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/PortalComponent.h>
#include <SimCore/Components/TextureProjectorComponent.h>
#include <SimCore/Components/TextureProjectorComponent.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/PhysicsTypes.h>

#include <dtPhysics/physicscomponent.h>

#include <DriverInputComponent.h>
///////////////////////////////////

#include <dtNetGM/servernetworkcomponent.h>
#include <dtNetGM/clientnetworkcomponent.h>
#include <dtGame/basemessages.h>

using dtCore::RefPtr;
using namespace SimCore::CollisionGroup;

namespace DriverDemo
{
   ///////////////////////////////////////////////////////////////////////////
   extern "C" DRIVER_DEMO_EXPORT dtGame::GameEntryPoint* CreateGameEntryPoint()
   {
      return new DriverGameEntryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" DRIVER_DEMO_EXPORT void DestroyGameEntryPoint(dtGame::GameEntryPoint* entryPoint)
   {
      delete entryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   DriverGameEntryPoint::DriverGameEntryPoint()
   {
      mArgv = NULL;
      mArgc = 0;
   }

   ///////////////////////////////////////////////////////////////////////////
   DriverGameEntryPoint::~DriverGameEntryPoint()
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   void DriverGameEntryPoint::Initialize(dtGame::GameApplication& app, int argc, char **argv)
   {
      mArgv = argv;
      mArgc = argc;

      // this is set cause argc and argv passed into the function
      // goes out of scope, cause they are sent by pointer.
      if(parser == NULL)
         parser = new osg::ArgumentParser(&mArgc, mArgv);

      parser->getApplicationUsage()->setCommandLineUsage("Driver Demo Application [options] value ...");

      BaseClass::Initialize(app, argc, argv);
   }

   ///////////////////////////////////////////////////////////////////////////
   void DriverGameEntryPoint::OnStartup(dtGame::GameApplication &app)
   {
      dtGame::GameManager &gameManager = *app.GetGameManager();//*GetGameManager();

      // Change log file name
      std::string logFileName = gameManager.GetConfiguration().GetConfigPropertyValue("Delta3DLogFileName");
      if (!logFileName.empty())
         dtUtil::LogFile::SetFileName(logFileName);

      gameAppComponent = new DriverDemo::GameAppComponent();
      gameManager.AddComponent(*gameAppComponent, dtGame::GameManager::ComponentPriority::HIGHER);

      gameAppComponent->InitializeCommandLineOptionsAndRead(parser);

      // call base class we're done initializing
      FinalizeParser();

      // turn off showing the cursor
      gameManager.GetApplication().GetWindow()->SetShowCursor(false);

      // call base class
      BaseClass::OnStartup(app);

#ifndef BUILD_HLA
      // Load all of our maps (base --mapName plus AdditionalMaps
      gameAppComponent->LoadMaps(mMapName);
#endif
      // initialize our guy / stealth actor
      gameAppComponent->InitializePlayer();

   }

   ///////////////////////////////////////////////////////////////////////////
   void DriverGameEntryPoint::InitializeComponents(dtGame::GameManager &gm)
   {
      // This call does a fair bit of work. For one thing, it causes the HLA component
      // to connect to the federation.
      BaseClass::InitializeComponents(gm);

      // Get the command line object. Used below.
      SimCore::CommandLineObject* commandLineObject = NULL;
      GameAppComponent* gameAppComponent = dynamic_cast<GameAppComponent*>
         (gm.GetComponentByName(GameAppComponent::DEFAULT_NAME));
      if(gameAppComponent != NULL)
         commandLineObject = gameAppComponent->GetCommandLineObject();
      if(commandLineObject == NULL)
      {
         LOG_ERROR("Command Line Component not initialized, other components will not\
                   be made, exit now while you still can!");
         return;
      }


      ////////////////////////////////////////////////////////////////
      // PHYSICS
      dtCore::RefPtr<dtPhysics::PhysicsWorld> world = new dtPhysics::PhysicsWorld(gm.GetConfiguration());
      world->Init();
      dtCore::RefPtr<dtPhysics::PhysicsComponent> physicsComponent = new dtPhysics::PhysicsComponent(*world, false);
      gm.AddComponent(*physicsComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      SimCore::CollisionGroup::SetupDefaultGroupCollisions(*physicsComponent);
      // OLD - Physics collision group pairings. 
      //nxScene.setActorGroupPairFlags(GROUP_HUMAN_LOCAL, GROUP_TERRAIN,
      //   NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
      // contact reports
      // We really only care about the world and the world
      //nxScene.setActorGroupPairFlags(GROUP_TERRAIN, GROUP_TERRAIN,
      //   NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);


      // Rendering Support - Gives us lighting, sets up our viewmatrix, and other stuff.
      // We disable the physics cull visitor (which is for large tiled terrains like Terrex)
      // and also enable the static terrain physics (because it's loaded once). 
      dtCore::RefPtr<SimCore::Components::RenderingSupportComponent> renderingSupportComponent
         = new SimCore::Components::RenderingSupportComponent();
      renderingSupportComponent->SetEnableCullVisitor(false);
      renderingSupportComponent->SetMaxSpotLights(1);
      renderingSupportComponent->SetEnableStaticTerrainPhysics(true);
      gm.AddComponent(*renderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);


      ////////////////////////////////////////////////////////////////////////
      // The Viewer Message Processor
      RefPtr<SimCore::Components::ViewerMessageProcessor> messageProcessor =
         new SimCore::Components::ViewerMessageProcessor;
      gm.AddComponent(*messageProcessor, dtGame::GameManager::ComponentPriority::HIGHEST);

      ////////////////////////////////////////////////////////////////////////
      // init the input component with what it needs
      dtCore::RefPtr<DriverInputComponent> mInputComponent = new DriverInputComponent("Input Component");
      gm.AddComponent(*mInputComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      //mInputComponent->SetListeners();
      dtCore::Camera* camera = gm.GetApplication().GetCamera();
      mInputComponent->SetDefaultCameraPerspective(
         camera->GetHorizontalFov(), camera->GetVerticalFov(),
         SimCore::BaseGameEntryPoint::PLAYER_NEAR_CLIP_PLANE,
         SimCore::BaseGameEntryPoint::PLAYER_FAR_CLIP_PLANE);
      ////////////////////////////////////////////////////////////////////////


      ////////////////////////////////////////////////////////////////////////
      // HUD GUI COMPONENT
      //const dtDAL::NamedStringParameter* callsignName =
      //   dynamic_cast<const dtDAL::NamedStringParameter*>(commandLineObject->GetParameter(GameAppComponent::CMD_LINE_VEHICLE_CALLSIGN));

      dtCore::RefPtr<DriverHUD> mHudGUI = new DriverHUD(gm.GetApplication().GetWindow());
      gm.AddComponent(*mHudGUI, dtGame::GameManager::ComponentPriority::NORMAL);
      mHudGUI->Initialize();
      mHudGUI->SetCallSign( "NoCallSignSet" );

      ////////////////////////////////////////////////////////////////////////
      // Disable the weather component's ability to change the clipping planes.
      SimCore::Components::WeatherComponent* weatherComp = NULL;
      gm.GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME, weatherComp);
      if (weatherComp != NULL)
      {
         weatherComp->SetAdjustClipPlanes(false);
      }

      SetupClientServerNetworking(gm);
   }

   ///////////////////////////////////////////////////////////
   void DriverGameEntryPoint::SetupClientServerNetworking(dtGame::GameManager& gm)
   {
      dtUtil::ConfigProperties& configParams = gm.GetConfiguration();
      if (dtUtil::ToType<bool>(configParams.GetConfigPropertyValue("dtNetGM.On", "false")))
      {
         const std::string role = configParams.GetConfigPropertyValue("dtNetGM.Role", "client");
         int serverPort = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.ServerPort", "7329"));
         if (role == "Server" || role == "server" || role == "SERVER")
         {
            dtCore::RefPtr<dtNetGM::ServerNetworkComponent> serverComp =
               new dtNetGM::ServerNetworkComponent("DriverDemo", 1);
            gm.AddComponent(*serverComp, dtGame::GameManager::ComponentPriority::NORMAL);
            serverComp->SetupServer(serverPort);
         }
         else if (role == "Client" || role == "client" || role == "CLIENT")
         {
            dtCore::RefPtr<dtNetGM::ClientNetworkComponent> clientComp =
               new dtNetGM::ClientNetworkComponent("DriverDemo", 1);
            gm.AddComponent(*clientComp, dtGame::GameManager::ComponentPriority::NORMAL);
            int serverPort = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.ServerPort", "7329"));
            const std::string host = configParams.GetConfigPropertyValue("dtNetGM.ServerHost", "127.0.0.1");
            if (clientComp->SetupClient(host, serverPort))
            {
               dtCore::RefPtr<dtGame::MachineInfoMessage> message;
               gm.GetMessageFactory().CreateMessage(dtGame::MessageType::NETCLIENT_REQUEST_CONNECTION, message);
               message->SetDestination(clientComp->GetServer());
               message->SetMachineInfo(gm.GetMachineInfo());
               gm.SendNetworkMessage(*message);
            }
         }
      }
   }
}
