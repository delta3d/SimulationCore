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

#include <GameEntryPoint.h>
#include <NetDemoMessageTypes.h>
#include <Components/InputComponent.h>
#include <Components/GameLogicComponent.h>
#include <ConfigParameters.h>
#include <Components/GUIComponent.h>
#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/Components/VolumeRenderingComponent.h>

#include <dtDAL/project.h>

#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>

#include <dtPhysics/physicscomponent.h>

#include <dtUtil/fileutils.h>

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

   const std::string GameEntryPoint::APP_NAME("NetDemo");

   ///////////////////////////////////////////////////////////////////////////
   GameEntryPoint::GameEntryPoint()
      : mIsServer(true)
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

      SetMapName("NetDemo");

      parser->getApplicationUsage()->setCommandLineUsage("Res Game Application [options] value ...");

      BaseClass::Initialize(app, argc, argv);
   }

   ///////////////////////////////////////////////////////////////////////////
   void GameEntryPoint::OnStartup(dtGame::GameApplication& app)
   {
      BaseClass::OnStartup(app);

      FinalizeParser();

      GameLogicComponent* gameAppComp = NULL;
      app.GetGameManager()->GetComponentByName(GameLogicComponent::DEFAULT_NAME, gameAppComp);
      gameAppComp->SetMapName( GetMapName() );
   }

   ///////////////////////////////////////////////////////////////////////////
   void GameEntryPoint::InitializeComponents(dtGame::GameManager& gm)
   {
      // This processes remote actors and such on the network
      dtCore::RefPtr<SimCore::Components::ViewerMessageProcessor> defaultProcessor =
         new SimCore::Components::ViewerMessageProcessor();
      gm.AddComponent(*defaultProcessor, dtGame::GameManager::ComponentPriority::HIGHEST);

      // This call does a fair bit of work. For one thing, it causes the HLA component
      // to connect to the federation.
      BaseClass::InitializeComponents(gm);

      // Our GameAppComponent does a lot of the game based logic such as state management,
      // creating the vehicle, and changing terrains when the server tells us to.
      dtCore::RefPtr<GameLogicComponent> gameAppComp = new GameLogicComponent();
      gm.AddComponent(*gameAppComp, dtGame::GameManager::ComponentPriority::NORMAL);
      //gameAppComp->InitializeCommandLineOptionsAndRead(parser);
      // Load state transitions.
      const char pathSep = dtUtil::FileUtils::PATH_SEPARATOR;
      gameAppComp->LoadTransitions(dtDAL::Project::GetInstance().GetContext()
         + pathSep + "Transitions" + pathSep + "NetDemoTransitions.xml");
      gameAppComp->MakeCurrent( gameAppComp->GetCurrentState() );



      // Physics - we definitely need some of this!
      dtCore::RefPtr<dtPhysics::PhysicsWorld> world = new dtPhysics::PhysicsWorld(gm.GetConfiguration());
      world->Init();
      dtCore::RefPtr<dtPhysics::PhysicsComponent> physicsComponent = new dtPhysics::PhysicsComponent(*world, false);
      gm.AddComponent(*physicsComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      SimCore::CollisionGroup::SetupDefaultGroupCollisions(*physicsComponent);

      // Rendering Support - Gives us lighting, sets up our viewmatrix, and other stuff.
      // We disable the physics cull visitor (which is for large tiled terrains like Terrex)
      // and also disable the static terrain physics (because we solve this manually everytime
      // the server app changes terrains - see the GameAppComponent)
      dtCore::RefPtr<SimCore::Components::RenderingSupportComponent> renderingSupportComponent
         = new SimCore::Components::RenderingSupportComponent();
      renderingSupportComponent->SetEnableCullVisitor(false);
      renderingSupportComponent->SetEnableStaticTerrainPhysics(false);
      gm.AddComponent(*renderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);

      // Keyboard, mouse input, etc...
      InputComponent* inputComp = new InputComponent();
      gm.AddComponent(*inputComp, dtGame::GameManager::ComponentPriority::NORMAL);

      // GUI Component
      GUIComponent* guiComp = new GUIComponent;
      gm.AddComponent(*guiComp, dtGame::GameManager::ComponentPriority::NORMAL);
      guiComp->Initialize();

      SimCore::Components::VolumeRenderingComponent* vrc = new SimCore::Components::VolumeRenderingComponent();
      gm.AddComponent(*vrc, dtGame::GameManager::ComponentPriority::NORMAL);
      vrc->Init();


   }


}
