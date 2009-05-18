/*
* Copyright, 2009, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author David Guthrie
*/

#include "GameEntryPoint.h"
#include "InputComponent.h"
#include "GameAppComponent.h"
#include "ConfigParameters.h"

#include <SimCore/CollisionGroupEnum.h>

#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>

#include <dtPhysics/physicscomponent.h>

#include <dtNetGM/servernetworkcomponent.h>
#include <dtNetGM/clientnetworkcomponent.h>
#include <dtNetGM/machineinfomessage.h>

#include <osg/ApplicationUsage>
#include <osg/ArgumentParser>

namespace NetDemo
{
   ///////////////////////////////////////////////////////////////////////////
   extern "C" NETDEMO_EXPORT dtGame::GameEntryPoint* CreateGameEntryPoint()
   {
      return new GameEntryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" NETDEMO_EXPORT void DestroyGameEntryPoint(dtGame::GameEntryPoint* entryPoint)
   {
      delete entryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   GameEntryPoint::GameEntryPoint()
   {

   }

   ///////////////////////////////////////////////////////////////////////////
   GameEntryPoint::~GameEntryPoint()
   {

   }

   ///////////////////////////////////////////////////////////////////////////
   void GameEntryPoint::Initialize(dtGame::GameApplication& app, int argc, char** argv)
   {
      mArgv = argv;
      mArgc = argc;

      // this is set cause argc and argv passed into the function
      // goes out of scope, cause they are sent by pointer.
      if (parser == NULL)
         parser = new osg::ArgumentParser(&mArgc, mArgv);

      parser->getApplicationUsage()->setCommandLineUsage("Res Game Application [options] value ...");

      BaseClass::Initialize(app, argc, argv);
   }

   ///////////////////////////////////////////////////////////////////////////
   void GameEntryPoint::OnStartup(dtGame::GameApplication& app)
   {
      // call base class we're done initializing
      BaseClass::OnStartup(app);

      FinalizeParser();

      ///////////////////// TEMP HACK - This UI should control this ////////////////////
      SimCore::Components::BaseGameAppComponent* gameAppComp = NULL;
      app.GetGameManager()->GetComponentByName(GameAppComponent::DEFAULT_NAME, gameAppComp);
      gameAppComp->LoadMaps(mMapName);
      //////////////////////////////////////////////////////////////////////////////////
   }

   ///////////////////////////////////////////////////////////////////////////
   void GameEntryPoint::InitializeComponents(dtGame::GameManager& gm)
   {
      // This call does a fair bit of work. For one thing, it causes the HLA component
      // to connect to the federation.
      BaseClass::InitializeComponents(gm);

      dtCore::RefPtr<GameAppComponent> gameAppComp = new GameAppComponent();
      gm.AddComponent(*gameAppComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameAppComp->InitializeCommandLineOptionsAndRead(parser);

      dtCore::RefPtr<dtPhysics::PhysicsWorld> world = new dtPhysics::PhysicsWorld(gm.GetConfiguration());
      world->Init();
      dtCore::RefPtr<dtPhysics::PhysicsComponent> physicsComponent = new dtPhysics::PhysicsComponent(*world, false);
      gm.AddComponent(*physicsComponent,
               dtGame::GameManager::ComponentPriority::NORMAL);
      SimCore::CollisionGroup::SetupDefaultGroupCollisions(*physicsComponent);

      gm.AddComponent(*new InputComponent(), dtGame::GameManager::ComponentPriority::NORMAL);

      SetupClientServerNetworking(gm);
   }

   ///////////////////////////////////////////////////////////
   void GameEntryPoint::SetupClientServerNetworking(dtGame::GameManager& gm)
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
               new dtNetGM::ClientNetworkComponent("ResGame", 1);
            gm.AddComponent(*clientComp, dtGame::GameManager::ComponentPriority::NORMAL);
            int serverPort = dtUtil::ToType<int>(configParams.GetConfigPropertyValue("dtNetGM.ServerPort", "7329"));
            const std::string host = configParams.GetConfigPropertyValue("dtNetGM.ServerHost", "127.0.0.1");
            if (clientComp->SetupClient(host, serverPort))
            {
               dtCore::RefPtr<dtNetGM::MachineInfoMessage> message;
               gm.GetMessageFactory().CreateMessage(dtGame::MessageType::NETCLIENT_REQUEST_CONNECTION, message);
               message->SetDestination(clientComp->GetServer());
               message->SetMachineInfo(gm.GetMachineInfo());
               gm.SendNetworkMessage(*message);
            }
         }
      }
   }

}
