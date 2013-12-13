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
#include <GameAppComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/Human.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/actorupdatemessage.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/matrixutil.h>

///////////////////////////////////
// for command line parsing
#include <osg/ApplicationUsage>
///////////////////////////////////

///////////////////////////////////
// sets default camera perspective
#include <dtCore/camera.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtGame/gameactor.h>

#include <dtABC/application.h>
#include <SimCore/BaseGameEntryPoint.h>
///////////////////////////////////

///////////////////////////////////
// For tools
//#include <SimCore/Tools/Binoculars.h>
//#include <SimCore/Tools/Compass.h>
//#include <SimCore/Tools/GPS.h>
//#include <SimCore/Actors/ControlStateActor.h>
///////////////////////////////////

///////////////////////////////////
// for player initialization
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <SimCore/Components/RenderingSupportComponent.h> //for light/shadow
#include <DriverArticulationHelper.h>
#include <dtActors/coordinateconfigactor.h>
#include <dtActors/playerstartactorproxy.h>
#include <dtUtil/coordinates.h>
#include <dtCore/batchisector.h>
///////////////////////////////////

#include <DriverInputComponent.h>
#include <DriverHUD.h>
#include <HoverTargetActor.h>
#include <HoverVehicleActor.h>

///////////////////////////////////
// for terrain loading
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/CommandLineObject.h>
///////////////////////////////////

namespace DriverDemo
{
   //////////////////////////////////////////////////////////////////////////
   const std::string GameAppComponent::DEFAULT_NAME                 = "GameAppComponent";
   const std::string GameAppComponent::APPLICATION_NAME             = "Driver Demo";
   const std::string GameAppComponent::CMD_LINE_STARTING_POSITION   = "StartingPosition";
   const std::string GameAppComponent::CMD_LINE_VEHICLE_PROTOTYPE_NAME = "VehicleName";
   const std::string GameAppComponent::CMD_LINE_WEAPON              = "Weapon";
   const std::string GameAppComponent::CMD_LINE_START_HEADING       = "StartHeading";

   //////////////////////////////////////////////////////////////////////////
   GameAppComponent::GameAppComponent(const std::string& name)
      : SimCore::Components::BaseGameAppComponent(name)
      , mWaitForVehicle(false)
   {

   }

   //////////////////////////////////////////////////////////////////////////
   GameAppComponent::~GameAppComponent(void)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::ProcessMessage(const dtGame::Message& msg)
   {      
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::InitializeCommandLineOptionsAndRead(osg::ArgumentParser* parser)
   {
      if(parser == NULL)
      {
         LOG_ERROR("Parser is NULL in InitializeCommandLineOptionsAndRead\
                   , no initing will occur");
         return;
      }

      BaseGameAppComponent::InitializeCommandLineOptionsAndRead(parser);

      parser->getApplicationUsage()->addCommandLineOption("--" + CMD_LINE_VEHICLE_PROTOTYPE_NAME,
         "Vehicle we are attaching to - use the prototype actor name in DriverPrototypes.xml", "Hover_Vehicle");

      SimCore::CommandLineObject* commandLineObject = GetCommandLineObject();

      // Get the start heading for the player
      float heading = 0.0f;
      parser->read( "--startHeading", heading );
      dtCore::RefPtr<dtDAL::NamedFloatParameter> paramHeading
         = new dtDAL::NamedFloatParameter(CMD_LINE_START_HEADING, heading);
      commandLineObject->AddParameter(paramHeading.get());

      std::string prototypeName;
      if(parser->read("--" + CMD_LINE_VEHICLE_PROTOTYPE_NAME, prototypeName))
      {
         dtCore::RefPtr<dtDAL::NamedStringParameter> parameter
            = new dtDAL::NamedStringParameter(CMD_LINE_VEHICLE_PROTOTYPE_NAME, prototypeName);
         commandLineObject->AddParameter(parameter.get());
      }
      else
      {
         LOG_ALWAYS("To choose your vehicle, pass '--" + CMD_LINE_VEHICLE_PROTOTYPE_NAME +
            "' on the command line \n     followed by either, 'Hover_Vehicle' or 'Wheeled_Vehicle'.  Defaults to Hover_Vehicle");
      }

   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Actors::BasePhysicsVehicleActor* GameAppComponent::CreateNewVehicle()
   {
      SimCore::Actors::BasePhysicsVehicleActor* vehicle = NULL;
      std::string vehicleName = "Hover_Vehicle"; // The default. Change with command line args.

      SimCore::CommandLineObject* commandLineObject = GetCommandLineObject();
      if(commandLineObject == NULL)
      {
         LOG_ERROR("commandLineObject is null, InitializeVehicle will not occur");
         return NULL;
      }

      // look up the prototype name from the command line args. If the user supplied one,
      // then we use that instead of the default.  This would be like "Hover_Vehicle" or "Wheeled_Vehicle"
      const dtDAL::NamedStringParameter* vehiclePrototypeName
         = dynamic_cast<const dtDAL::NamedStringParameter*>
         (commandLineObject->GetParameter(GameAppComponent::CMD_LINE_VEHICLE_PROTOTYPE_NAME));
      if( vehiclePrototypeName != NULL )
      {
         vehicleName = vehiclePrototypeName->GetValue();
      }

      // Find the vehicle template based on the name. The default is 'Driver_Vehicle'.
      std::vector<dtDAL::ActorProxy*> toFill;
      GetGameManager()->FindPrototypesByName(vehicleName, toFill);


      // CREATE OUR NEW VEHICLE
      if(!toFill.empty())
      {
         dtCore::RefPtr<dtDAL::ActorProxy> ourActualActorProxy =
            GetGameManager()->CreateActorFromPrototype(toFill.front()->GetId());
         if(ourActualActorProxy != NULL)
         {
            vehicle = dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(ourActualActorProxy->GetDrawable());
            if (vehicle != NULL)
            {
               vehicle->SetArticulationHelper( new DriverArticulationHelper );

               // Ensure the vehicle will publish its call-sign via its name property
               vehicle->SetName("VehicleName123");

               GetGameManager()->AddActor(vehicle->GetGameActorProxy(), false, true);

               // Set the vehicle heading.
               const dtDAL::NamedFloatParameter* paramHeading
                  = dynamic_cast<const dtDAL::NamedFloatParameter*>
                  (commandLineObject->GetParameter(GameAppComponent::CMD_LINE_START_HEADING));
               if( paramHeading != NULL )
               {
                  dtCore::Transform theTransform;
                  vehicle->GetTransform(theTransform);
                  theTransform.SetRotation(paramHeading->GetValue(), 0.0f, 0.0f);
                  vehicle->SetTransform(theTransform);
               }


               //NOTE: this will add a cheap shadow effect to the vehicle in the form of a black light
               //dtGame::GMComponent* comp = GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME);
               //if(comp)
               //{
               //   SimCore::Components::RenderingSupportComponent* rsComp = dynamic_cast<SimCore::Components::RenderingSupportComponent*>(comp);
               //   if(rsComp)
               //   {
               //      SimCore::Components::RenderingSupportComponent::DynamicLight* dl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
               //      dl->mIntensity = -2.0f;//a negative intensity will add a negative color, the higher
               //                             //it is the more light it will take to get rid of it
               //      dl->mColor.set(osg::Vec3(1.0f, 1.0f, 1.0f));
               //      dl->mAttenuation.set(2.0, 0.5, 0.2);
               //      dl->mTarget = &(platform->GetGameActorProxy().GetGameActor());
               //      dl->mAutoDeleteLightOnTargetNull = true;

               //      rsComp->AddDynamicLight(dl);
               //   }
               //}
            }
         }
      }

      return vehicle;
   }


   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::InitializePlayer()
   {
      DriverInputComponent* mInputComponent = dynamic_cast<DriverInputComponent*>
         (GetGameManager()->GetComponentByName(DriverInputComponent::DEFAULT_NAME));
      if(mInputComponent == NULL)
      {
         LOG_ERROR("Input component is null, initialize player will not occur");
         return;
      }

      SimCore::CommandLineObject* commandLineObject = NULL;
      commandLineObject = GetCommandLineObject();

      if(commandLineObject == NULL)
      {
         LOG_ERROR("commandLineObject is null, initialize player will not occur");
         return;
      }

      dtCore::RefPtr<dtGame::GameActorProxy> ap;

      // create a player actor, walk run jump and drink :)
      GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, ap);

      // make the stealh aware
      mStealth = static_cast<SimCore::Actors::PlayerActor*>(ap->GetDrawable());

      // make the camera a child
      mStealth->AddChild(GetGameManager()->GetApplication().GetCamera());

      // add this actor to the game manager
      GetGameManager()->AddActor(mStealth->GetGameActorProxy(), false, false);

      // so the input component knows whats going on
      mInputComponent->SetPlayer(mStealth.get());

      mStealth->SetName( "Player" );
   }
} // end namespace.
