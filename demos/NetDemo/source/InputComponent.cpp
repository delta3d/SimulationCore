 
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

* David Guthrie

*/

#include <InputComponent.h>

#include <dtGame/messagetype.h>

#include <dtABC/application.h>

#include <osgGA/GUIEventAdapter>



#include <dtActors/engineactorregistry.h>

#include <dtActors/playerstartactorproxy.h>

#include <dtActors/gamemeshactor.h>



#include <dtPhysics/physicsmaterialactor.h>

#include <dtPhysics/physicscomponent.h>

#include <dtPhysics/bodywrapper.h>



#include <SimCore/BaseGameEntryPoint.h>



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



         dtCore::Transformable* xformable;

         gap->GetActor(xformable);

         mMotionModel->SetTarget(xformable);

      }

      else if (message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)

      {

         //SetupMaterialsAndTerrain();

      }

   }



   //////////////////////////////////////////////////////////////

/*   void InputComponent::SetupMaterialsAndTerrain()

   {

      dtPhysics::MaterialActorProxy* terrainMaterial = NULL;

      static const dtUtil::RefString TERRAIN_MATERIAL_NAME("Terrain Material");

      GetGameManager()->FindActorByName(TERRAIN_MATERIAL_NAME, terrainMaterial);



      dtGame::GameActorProxy* terrain = NULL;

      static const dtUtil::RefString TERRAIN_NAME("Terrain");

      GetGameManager()->FindActorByName(TERRAIN_NAME, terrain);

      if (terrain == NULL)

      {

         LOG_ERROR("No Terrain was found, physics can't be loaded.");

         return;

      }



      dtPhysics::PhysicsComponent* physicsComponent = NULL;

      GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComponent);

      if (physicsComponent == NULL)

      {

         LOG_ERROR("No Physics Component was found.");

         return;

      }

   }

*/

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



   //////////////////////////////////////////////////////////////

   bool InputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)

   {

      bool keyUsed = true;

      switch(key)

      {

         case '\n':

         case '\r':

         case 'p':

         {

            FireSomething();

            break;

         }

         case 'r':

         {

            DoRayCast();

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

         default:

            keyUsed = false;

      }



      if(!keyUsed)

         return BaseClass::HandleKeyPressed(keyboard, key);

      else 

         return keyUsed;

   }



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



}

