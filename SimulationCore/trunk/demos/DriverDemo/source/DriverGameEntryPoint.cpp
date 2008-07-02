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

#include <dtDAL/project.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/CommandLineObject.h>
#include <SimCore/NxCollisionGroupEnum.h>

#include <osg/ApplicationUsage>

// turns mouse off
#include <dtCore/deltawin.h>
#include <dtCore/camera.h>

///////////////////////////////////
// extra components to init
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/PortalComponent.h>
#include <SimCore/Components/TextureProjectorComponent.h>
#include <SimCore/Components/TextureProjectorComponent.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/HLA/HLAConnectionComponent.h>

#include <NxAgeiaWorldComponent.h>

#include <DriverInputComponent.h>
///////////////////////////////////

using dtCore::RefPtr;
using namespace SimCore::NxCollisionGroup;

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
   DriverGameEntryPoint::DriverGameEntryPoint() : SimCore::HLA::BaseHLAGameEntryPoint()
   {
      mArgv = NULL;
      mArgc = 0;
   }
   
   ///////////////////////////////////////////////////////////////////////////
   DriverGameEntryPoint::~DriverGameEntryPoint()
   {
   }
   
   ///////////////////////////////////////////////////////////////////////////
   /*dtCore::ObserverPtr<dtGame::GameManager> DriverGameEntryPoint::CreateGameManager(dtCore::Scene& scene)
   {
      return BaseClass::CreateGameManager(scene);
   }*/
   
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
    
      gameAppComponent = new DriverDemo::GameAppComponent();
      gameManager.AddComponent(*gameAppComponent, dtGame::GameManager::ComponentPriority::HIGHER);
   
      gameAppComponent->InitializeCommandLineOptionsAndRead(parser);
      
      // call base class we're done initializing
      FinalizeParser();
   
      // turn off showing the cursor
      gameManager.GetApplication().GetWindow()->ShowCursor(false);
   
      // call base class
      BaseClass::OnStartup(app);

      // initialize our guy / stealth actor
      gameAppComponent->InitializePlayer();
      
      // Initialize tools and add them to the input component and the HUD toolbar
      gameAppComponent->InitializeTools();
   }
   
   ///////////////////////////////////////////////////////////////////////////
   void DriverGameEntryPoint::HLAConnectionComponentSetup(dtGame::GameManager &gm)
   {
      BaseClass::HLAConnectionComponentSetup(gm);
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
   
      ////////////////////////////////////////////////////////////////////////
      // Initialize the ageia component
      dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaWorldComponent> ageiaComponent 
         = new dtAgeiaPhysX::NxAgeiaWorldComponent();
      gm.AddComponent(*ageiaComponent, dtGame::GameManager::ComponentPriority::NORMAL);
   
      // Configure the Ageia Component
      NxScene& nxScene = ageiaComponent->GetPhysicsScene(std::string("Default"));
   
      // Which groups collide with each other. Typically, on the world and particles collide.
      // Group 0 is the normal default group. Terrain, HMMWV, buildings
      // Group 15 is bullets
      // Group 20 is particles
      // Group 30 is Local Characters
      // Group 31 is Remote Characters
      // DO NOT FORGET - If you want bullets to collide, make sure to update this line: 
      // ourActor->getScene().raycastAllShapes(ourRay, myReport, NX_ALL_SHAPES, (1 << 0) | (1 << 30) | (1 << 31))
      // In NxAgeiaMunitionsPSysActor. You have to add whichever group such as 0, 30, 31, to the '|' list.
      nxScene.setGroupCollisionFlag(GROUP_TERRAIN, GROUP_TERRAIN, true);   // the world interacts with itself
      nxScene.setGroupCollisionFlag(GROUP_TERRAIN, GROUP_PARTICLE, true);  // particles interact with the world
      nxScene.setGroupCollisionFlag(GROUP_BULLET, GROUP_PARTICLE, false);// bullets and particles do not interact
      nxScene.setGroupCollisionFlag(GROUP_BULLET, GROUP_TERRAIN, false); // bullets and the world do NOT interact (this seems odd, but we handle with raycast)
      nxScene.setGroupCollisionFlag(GROUP_PARTICLE, GROUP_PARTICLE, false);// particles do not interact with itself
      nxScene.setGroupCollisionFlag(GROUP_BULLET, GROUP_BULLET, false);// bullets do not interact with itself
      
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_LOCAL, GROUP_HUMAN_LOCAL, true); // characters interact with theirselves
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_LOCAL, GROUP_BULLET, true); // characters interact with bullets
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_LOCAL, GROUP_PARTICLE, true); // characters interact with physics particles
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_LOCAL, GROUP_TERRAIN, true);  // characters interact with world

      // For remote characters, we want to collide with some things, but not the vehicle
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_REMOTE, GROUP_HUMAN_LOCAL, true); // local characters interact with remote characters
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_REMOTE, GROUP_BULLET, true); // remote characters interact with bullets
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_REMOTE, GROUP_PARTICLE, true); // remote characters interact with physics particles
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_REMOTE, GROUP_TERRAIN, false);  // remote characters DO NOT interact with world - don't push the HMMWV

      // water interactions
      nxScene.setGroupCollisionFlag(GROUP_WATER, GROUP_BULLET, false);  //  bullets can hit the water, (turn off so raycast handles it)
      nxScene.setGroupCollisionFlag(GROUP_WATER, GROUP_PARTICLE, true);  // particles interact with the water
      nxScene.setGroupCollisionFlag(GROUP_WATER, GROUP_TERRAIN, false);   // everything in group 0 can not hit the water
      nxScene.setGroupCollisionFlag(GROUP_WATER, GROUP_HUMAN_LOCAL, false); // characters and water do not collide

      // 26 is our boat actor.
      nxScene.setGroupCollisionFlag(GROUP_VEHICLE_WATER, GROUP_WATER, true);  // boats can drive on the water
      nxScene.setGroupCollisionFlag(GROUP_VEHICLE_WATER, GROUP_VEHICLE_WATER, true);  // boats can drive on the water
      nxScene.setGroupCollisionFlag(GROUP_VEHICLE_WATER, GROUP_PARTICLE, true);  // boats and particles
      nxScene.setGroupCollisionFlag(GROUP_VEHICLE_WATER, GROUP_HUMAN_REMOTE, true);  // bullets and boats
      nxScene.setGroupCollisionFlag(GROUP_VEHICLE_WATER, GROUP_TERRAIN, true);  // land & vehicles and boats
      nxScene.setGroupCollisionFlag(GROUP_HUMAN_REMOTE, GROUP_VEHICLE_WATER, false);  // remote characters DO NOT interact with world - don't push the boat

      nxScene.setActorGroupPairFlags(GROUP_HUMAN_LOCAL, GROUP_TERRAIN,
         NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
   
      // contact reports
      // We really only care about the world and the world
      nxScene.setActorGroupPairFlags(GROUP_TERRAIN, GROUP_TERRAIN,
         NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
      ////////////////////////////////////////////////////////////////////////
   

      ////////////////////////////////////////////////////////////////////////
      // rendering support component - terrain tiling atm, dynamic lights, etc...
      dtCore::RefPtr<SimCore::Components::RenderingSupportComponent> renderingSupportComponent 
         = new SimCore::Components::RenderingSupportComponent();
      //the cull visitor must be enabled before we add the rendering support component
      //doing this will swap out the original cullvisitor with our own that adds Ageia physics support
      renderingSupportComponent->SetEnableCullVisitor(true);
      gm.AddComponent(*renderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      

      ////////////////////////////////////////////////////////////////////////
      // The Viewer Message Processor
      RefPtr<SimCore::Components::ViewerMessageProcessor> messageProcessor = 
         new SimCore::Components::ViewerMessageProcessor;
      gm.AddComponent(*messageProcessor, dtGame::GameManager::ComponentPriority::HIGHEST);
   
      ////////////////////////////////////////////////////////////////////////
      // init the input component with what it needs
      dtCore::RefPtr<DriverInputComponent> mInputComponent = new DriverInputComponent("Input Component");

      // Set the starting Position. // NOTE - I'm pretty sure this code is now obsolete - CMM
      const dtDAL::NamedVec3fParameter* param = 
         dynamic_cast<const dtDAL::NamedVec3fParameter*>(commandLineObject->GetParameter(GameAppComponent::CMD_LINE_STARTING_POSITION));
      if(param != NULL)
      {
         // Set the starting position only if the position is not the origin,
         // which is currently at empty space. osg::Vec3 startingPosition;
         osg::Vec3 startingPosition = param->GetValue(); 
         if( startingPosition.length2() > 0 )
         {
            mInputComponent->SetStartPosition( startingPosition );
         }
      }

      gm.AddComponent(*mInputComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      mInputComponent->SetListeners();
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
      //if(callsignName == NULL)
         mHudGUI->SetCallSign( "NoCallSignSet" );
      //else
      //{
      //   mHudGUI->SetCallSign( callsignName->GetValue() );
      //}

      ////////////////////////////////////////////////////////////////////////
      // Disable the weather component's ability to change the clipping planes.
      SimCore::Components::WeatherComponent* weatherComp 
         = dynamic_cast<SimCore::Components::WeatherComponent*>
         (gm.GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME));
      if( weatherComp != NULL )
      {
         weatherComp->SetAdjustClipPlanes(false);
      }
   }
}
