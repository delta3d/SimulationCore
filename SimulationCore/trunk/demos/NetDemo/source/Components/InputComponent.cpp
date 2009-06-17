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
 David Guthrie
*/
#include <osgGA/GUIEventAdapter>

#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <dtABC/application.h>
#include <dtActors/engineactorregistry.h>
#include <dtActors/playerstartactorproxy.h>
#include <dtActors/gamemeshactor.h>
#include <dtPhysics/physicsmaterialactor.h>
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/bodywrapper.h>
#include <dtCore/deltawin.h>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderprogram.h>

#include <SimCore/BaseGameEntryPoint.h>
#include <SimCore/Utilities.h>

#include <States.h>
#include <Components/InputComponent.h>
// TEMP STUFF FOR VEHICLE
#include <Actors/HoverVehicleActor.h>
#include <Actors/HoverVehiclePhysicsHelper.h>
#include <Actors/EnemyMine.h>

#include <osg/io_utils>
#include <iostream>

namespace NetDemo
{
   //////////////////////////////////////////////////////////////
   InputComponent::InputComponent(const std::string& name)
      : SimCore::Components::BaseInputComponent(name)
   {

   }

   //////////////////////////////////////////////////////////////
   InputComponent::~InputComponent()
   {

   }

   //////////////////////////////////////////////////////////////
   void  InputComponent::ProcessMessage(const dtGame::Message& message)
   {
      if (message.GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD
               && message.GetSource() == GetGameManager()->GetMachineInfo())
      {
         dtGame::GameActorProxy* gap = GetGameManager()->FindGameActorById(message.GetAboutActorId());
         if (gap == NULL)
         {
            LOG_ERROR("Got a player entered world message, but no player was found.")
               return;
         }
         else if (gap->IsRemote()) // Somebody else's player.
         {
            return;
         }

         dtCore::Transformable* xformable;
         gap->GetActor(xformable);
         mMotionModel->SetTarget(xformable);
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == message.GetMessageType())
      {
         HandleActorUpdateMessage(message);
      }
      else if (message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
      }
   }

   //////////////////////////////////////////////////////////////
   void InputComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();
      mMotionModel = new dtCore::FlyMotionModel(GetGameManager()->GetApplication().GetKeyboard(),
               GetGameManager()->GetApplication().GetMouse(), dtCore::FlyMotionModel::OPTION_DEFAULT);
   }

   //////////////////////////////////////////////////////////////
   void InputComponent::OnRemovedFromGM()
   {
      BaseClass::OnRemovedFromGM();
      mMotionModel = NULL;
   }

   ////////////////////////////////////////////////////////////////////
   void InputComponent::HandleActorUpdateMessage(const dtGame::Message& msg)
   {
/*
      const dtGame::ActorUpdateMessage &updateMessage =
         static_cast<const dtGame::ActorUpdateMessage&> (msg);

      // PLAYER STATUS - if it's ours, then check to see if our vehicle changed
      if (updateMessage.GetActorType() == NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE && 
         updateMessage.GetSource() == GetGameManager()->GetMachineInfo())
      {
         // Find the actor in the GM
         dtGame::GameActorProxy *gap = GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId());
         if(gap == NULL) 
            return;
         PlayerStatusActor *statusActor = dynamic_cast<PlayerStatusActor*>(gap->GetActor());
         if(statusActor == NULL)  
            return;

         HandlePlayerStatusUpdated(statusActor);

      }

      // SERVER STATUS
      else if (!mIsServer && updateMessage.GetActorType() == NetDemoActorRegistry::SERVER_GAME_STATUS_ACTOR_TYPE)
      {
         // Find the actor in the GM
         dtGame::GameActorProxy *gap = GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId());
         if(gap == NULL)  return;
         ServerGameStatusActor *serverStatus = static_cast<ServerGameStatusActor*>(gap->GetActor());

         // If not the server, do a print out...
         std::ostringstream oss;
         oss << "Server Status Updated: [" << serverStatus->GetGameStatus().GetName() << "], Wave[" << 
            serverStatus->GetWaveNumber() << "] Players[" << serverStatus->GetNumPlayers() << "] TimeLeft[" << 
            serverStatus->GetTimeLeftInCurState() << "], EnemiesKilt[" << serverStatus->GetNumEnemiesKilled() << "].";
         LOG_ALWAYS(oss.str());
      }


*/



   }

   //////////////////////////////////////////////////////////////
   bool InputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
   {
      bool keyUsed = true;
      switch(key)
      {
         case '\n':
         case '\r':
         case 'u':
         {
            FireSomething();
            break;
         }
         case 'r':
         {
            DoRayCast();
            break;
         }

         case 'p':
         {
            if (SimCore::Utils::IsDevModeOn(*GetGameManager())) 
            {
               dtCore::ShaderManager::GetInstance().ReloadAndReassignShaderDefinitions("Shaders/ShaderDefs.xml");
               //ToggleEntityShaders();
               LOG_ALWAYS("Reloading All Shaders...");
            }
            break;
         }

         case '5':
            {
               /////////////////////////////////////////////////////////
               LOG_ALWAYS("TEST - HACK - Attempting to create vehicle!!! ");
               // Hack stuff - add a vehicle here. For testing purposes.  
               dtCore::RefPtr<HoverVehicleActorProxy> testVehicleProxy = NULL;
               SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(), 
                  "Hover Vehicle", testVehicleProxy, "Check your additional maps in config.xml (compare to config_example.xml).");
               HoverVehicleActor *vehicleActor = dynamic_cast<HoverVehicleActor*>(&testVehicleProxy->GetGameActor());
               vehicleActor->SetHasDriver(true);
               GetGameManager()->AddActor(*testVehicleProxy, false, true);

               break;
            }

         case 't':
            {
               /////////////////////////////////////////////////////////
               LOG_ALWAYS("TEST - HACK - CREATING TARGET!!! ");
               // Hack stuff - add a vehicle here. For testing purposes.  
               dtCore::RefPtr<EnemyMineActorProxy> testEnemyMine = NULL;
               SimCore::Utils::CreateActorFromPrototypeWithException(*GetGameManager(), 
                  "Enemy Mine Prototype", testEnemyMine, "Check your additional maps in config.xml (compare to config_example.xml).");
               GetGameManager()->AddActor(*testEnemyMine, false, true);

               break;
            }

         case osgGA::GUIEventAdapter::KEY_Insert:
            {
               std::string developerMode;
               developerMode = GetGameManager()->GetConfiguration().GetConfigPropertyValue
                  (SimCore::BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE, "false");
               if (developerMode == "true" || developerMode == "1")
               {
                  GetGameManager()->GetApplication().SetNextStatisticsType();
               }
            }
            break;

         case 'o':
            {
               // Go forward 5 mins in time
               IncrementTime(+5);
            }
            break;

         case 'i':
            {
               // go back 5 mins in time
               IncrementTime(-5);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_Escape:
            {
               // Escapce key should act as one would expect, to escape from the
               // program in some manner, even if it means going through the menu system.
               GetAppComponent()->DoStateTransition(&Transition::TRANSITION_BACK);
            }
            break;

         case osgGA::GUIEventAdapter::KEY_Tab:
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
               app.GetWindow()->SetFullScreenMode(!app.GetWindow()->GetFullScreenMode());
            }
            break;

         default:
            keyUsed = false;
      }

      if(!keyUsed)
         return BaseClass::HandleKeyPressed(keyboard, key);
      else 
         return keyUsed;
   }

   //////////////////////////////////////////////////////////////
   void InputComponent::DoRayCast()
   {
      dtCore::Transform xform;
      GetGameManager()->GetApplication().GetCamera()->GetTransform(xform);
      osg::Matrix m;
      xform.Get(m);
      dtPhysics::VectorType rayDir(m(1, 0), m(1, 1), m(1, 2));

      dtPhysics::RayCast ray;
      dtPhysics::RayCast::Report report;

      ray.SetOrigin(xform.GetTranslation());
      ray.SetDirection(rayDir * 500);

      dtPhysics::PhysicsWorld::GetInstance().TraceRay(ray, report);

      if (report.mHasHitObject)
      {
         std::cout << "Ray Hit at position: " << report.mHitPos << "  Distance: " << report.mDistance << std::endl;
      }
      else
      {
         std::cout << "No Ray Hit." << std::endl;
      }
   }

   //////////////////////////////////////////////////////////////
   void InputComponent::FireSomething()
   {
      dtPhysics::MaterialActorProxy* projectileMaterial = NULL;
      static const dtUtil::RefString PROJECTILE_MATERIAL_NAME("Projectile Material");
      GetGameManager()->FindActorByName(PROJECTILE_MATERIAL_NAME, projectileMaterial);

      if (projectileMaterial == NULL)
      {
         LOG_ERROR("Can't create a projectile, the material is NULL");
         return;
      }

      dtActors::GameMeshActorProxy* projectilePrototype = NULL;
      static const dtUtil::RefString PROJECTILE_CRATE_NAME("Crate");
      GetGameManager()->FindPrototypeByName(PROJECTILE_CRATE_NAME, projectilePrototype);

      if (projectilePrototype == NULL)
      {
         LOG_ERROR("Can't create a projectile, the prototype NULL");
         return;
      }

      dtCore::RefPtr<dtActors::GameMeshActorProxy> projectile = NULL;
      GetGameManager()->CreateActorFromPrototype(projectilePrototype->GetId(), projectile);

      projectile->SetName("Silly Crate");

      dtPhysics::PhysicsComponent* physicsComponent = NULL;
      GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComponent);
      if (physicsComponent == NULL)
      {
         LOG_ERROR("No Physics Component was found.");
         return;
      }

      dtCore::RefPtr<dtPhysics::PhysicsHelper> helper = new dtPhysics::PhysicsHelper(*projectile);
      mHelpers.push_back(helper);

      helper->SetMaterialActor(projectileMaterial);
      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject();
      helper->AddPhysicsObject(*physicsObject);
      physicsObject->SetName(projectile->GetName());
      //physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::BOX);
      physicsObject->SetMass(3.0f);
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
      physicsObject->SetExtents(osg::Vec3(1.0f, 1.0f, 1.0f));
      //physicsObject->CreateFromProperties(projectile->GetActor()->GetOSGNode());
      //physicsObject->SetActive(true);

      dtCore::Transformable* actor;
      projectile->GetActor(actor);
      dtCore::Transform xform;
      xform.MakeIdentity();
      actor->SetTransform(xform);
      physicsObject->CreateFromProperties(actor->GetOSGNode());
      physicsObject->SetActive(true);


      //dtCore::Transform xform;
      GetGameManager()->GetApplication().GetCamera()->GetTransform(xform);
      osg::Matrix m;
      xform.Get(m);
      dtPhysics::VectorType force(100 * m(1, 0), 100 * m(1, 1), 100 * m(1, 2));
      physicsObject->GetBodyWrapper()->ApplyImpulse(force);

      physicsObject->SetTransform(xform);
      //dtCore::Transformable* actor;
      //projectile->GetActor(actor);
      actor->SetTransform(xform);

      GetGameManager()->AddActor(*projectile, false, true);

      physicsComponent->RegisterHelper(*helper);
   }

   /////////////////////////////////////////////////////////////////////////////
   GameLogicComponent* InputComponent::GetAppComponent()
   {
      if( ! mAppComp.valid() )
      {
         GameLogicComponent* comp = NULL;
         GetGameManager()->GetComponentByName( GameLogicComponent::DEFAULT_NAME, comp );
         mAppComp = comp;
      }

      if( ! mAppComp.valid() )
      {
         LOG_ERROR( "Input Component cannot access the Game App Component." );
      }

      return mAppComp.get();
   }

}
