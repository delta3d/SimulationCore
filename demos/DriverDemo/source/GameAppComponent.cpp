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
//#include <SimCore/Actors/Platform.h>
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
//#include <SimCore/Actors/NxAgeiaFourWheelVehicleActor.h>
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

///////////////////////////////////
// for terrain loading
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/CommandLineObject.h>
///////////////////////////////////

#ifdef AGEIA_PHYSICS
   #include <NxAgeiaWorldComponent.h>
#endif

namespace DriverDemo
{
   //////////////////////////////////////////////////////////////////////////
   const std::string GameAppComponent::DEFAULT_NAME                 = "GameAppComponent";
   const std::string GameAppComponent::APPLICATION_NAME             = "Driver Demo";
   const std::string GameAppComponent::CMD_LINE_STARTING_POSITION   = "StartingPosition";
   const std::string GameAppComponent::CMD_LINE_VEHICLE_CALLSIGN    = "VehicleCallSign";
   const std::string GameAppComponent::CMD_LINE_WEAPON              = "Weapon";
   const std::string GameAppComponent::CMD_LINE_START_HEADING       = "StartHeading";
   //const std::string GameAppComponent::CMD_LINE_START_LAT           = "StartLat";
   //const std::string GameAppComponent::CMD_LINE_START_LON           = "StartLon";
   //const std::string GameAppComponent::CMD_LINE_START_MGRS          = "StartMGRS";

   //////////////////////////////////////////////////////////////////////////
   GameAppComponent::GameAppComponent(const std::string &name) 
      : SimCore::Components::BaseGameAppComponent(name)
     , mLatitudeStart(0)
      , mLongitudeStart(0)
      , mStartingPosition(100.0f, 100.0f, 20.0f)
      , mWaitForVehicle(false) 
      , mStartingCoordSet(false)
   {
      
   }

   //////////////////////////////////////////////////////////////////////////
   GameAppComponent::~GameAppComponent(void)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::ProcessMessage(const dtGame::Message &msg)
   {
      SimCore::CommandLineObject* commandLineObject = GetCommandLineObject();

      if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         UpdatePlayerStartingPosition();
      }

      // If we are waiting to attach to our vehicle, then we don't know when it will come into existance. 
      // So, anytime ANY actor gets created, we are going to try to attach to it. The problem is that we are 
      // also waiting for the control state stuff too. So, we need both the Actor AND the Control state.
      else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED
           && mWaitForVehicle == true)
      {
         // Do we need to set our current vehicle???
         // mInputComponent->SetCurrentVehicle(*entityToAttachTo);

      }

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

      parser->getApplicationUsage()->addCommandLineOption("--vehicle_callsign",        
         "Vehicle callsign we are attaching to");

      SimCore::CommandLineObject* commandLineObject = GetCommandLineObject();
      int tempValue = 0;

      double coord = 0.0;
      double coord2 = 0.0;
      mStartingPosition[0] = 0.0f;
      mStartingPosition[1] = 0.0f;
      mStartingPosition[2] = 0.0f;
      if(parser->read("--startX", coord))
      {
         mStartingPosition[0] = coord;   
         mStartingCoordSet = true;
      }
      if(parser->read("--startY", coord))
      {
         mStartingPosition[1] = coord;
         mStartingCoordSet = true;
      }
      if(parser->read("--startZ", coord))
      {
         mStartingPosition[2] = coord;
         mStartingCoordSet = true;
      }

      dtCore::RefPtr<dtDAL::NamedVec3fParameter> parameter 
         = new dtDAL::NamedVec3fParameter(CMD_LINE_STARTING_POSITION, mStartingPosition);
      commandLineObject->AddParameter(parameter.get());

      // Get the start heading for the player
      float heading = 0.0f;
      parser->read( "--startHeading", heading );
      dtCore::RefPtr<dtDAL::NamedFloatParameter> paramHeading 
         = new dtDAL::NamedFloatParameter(CMD_LINE_START_HEADING, heading);
      commandLineObject->AddParameter(paramHeading.get());

      std::string callSign;
      if(parser->read("--vehicle_callsign", callSign))
      {
         dtCore::RefPtr<dtDAL::NamedStringParameter> parameter 
            = new dtDAL::NamedStringParameter(CMD_LINE_VEHICLE_CALLSIGN, callSign);
         commandLineObject->AddParameter(parameter.get());
      }
      
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::UpdatePlayerStartingPosition()
   {
      std::vector<dtDAL::ActorProxy*> toFill;
      GetGameManager()->FindActorsByName("Coordinate Config", toFill);
      
      if(toFill.empty())
         return;
      
      dtUtil::Coordinates coordConvertor = 
         (dynamic_cast<dtActors::CoordinateConfigActor*>(toFill[0]->GetActor()))->GetCoordinateConverter();

      // safety setting for Z. Take the z that was probably set from command params
      float initialZ = mStartingPosition[2];

      bool needToZClamp = false;
            /*
         case PLAYER_START_LAT_LON:
         {
            double x=0, y=0, z =0;
            coordConvertor.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
            mStartingPosition = coordConvertor.ConvertToLocalTranslation(osg::Vec3(mLatitudeStart, mLongitudeStart, 0));
            needToZClamp = true;
         }
         break;
         
         case PLAYER_START_MGRS:
         {
            mStartingPosition = coordConvertor.ConvertMGRSToXYZ(mMGRSStart);
            needToZClamp = true;
         }
         break;
         */

      // now put the z back - the lat/lon conversions don't do this.
      mStartingPosition[2] = initialZ; // in case we don't intersect the terrain

      if(needToZClamp)
      {
         toFill.clear();
         GetGameManager()->FindActorsByName("Terrain", toFill);
         dtDAL::ActorProxy* terrainNode = NULL;
         if(!toFill.empty())
         {
            terrainNode = (dynamic_cast<dtDAL::ActorProxy*>(&*toFill[0]));
            
            dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
            iSector->SetScene( &GetGameManager()->GetScene() );
            iSector->SetQueryRoot(terrainNode->GetActor());
            dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
            osg::Vec3 pos( mStartingPosition[0], mStartingPosition[1], mStartingPosition[2] );
            osg::Vec3 endPos = pos;
            pos[2] += 30000; 
            endPos[2] -= 30000;
            SingleISector.SetSectorAsLineSegment(pos, endPos);
            if( iSector->Update(osg::Vec3(0,0,0), true) )
            {
               if( SingleISector.GetNumberOfHits() > 0 ) 
               {
                  osg::Vec3 hp;
                  SingleISector.GetHitPoint(hp);
                  mStartingPosition[2] = hp[2] + 5;
               }
               else
                  LOG_WARNING("HP == 0, Unable to find Z value for terrain in UpdatePlayerStartingPosition");

            }
            else
            {
               LOG_WARNING("No hit, Unable to find Z value for terrain in UpdatePlayerStartingPosition");
            }
         }
         else
            LOG_WARNING("No terrain actor found, unable to set z to a value if you are using mgrs or lat/lon");
      }

      if(mStartingCoordSet)
      {
         toFill.clear();
         GetGameManager()->FindActorsByName("PlayerStart", toFill);
         if(toFill.empty())
            return;
         else
         {
            LOG_ALWAYS("Changing your starting location to the command line parameters that were sent in");
            dtCore::Transform ourTransform;
            (dynamic_cast<dtActors::PlayerStartActor*>(toFill[0]->GetActor()))->GetTransform(ourTransform);
            ourTransform.SetTranslation(mStartingPosition);
            (dynamic_cast<dtActors::PlayerStartActor*>(toFill[0]->GetActor()))->SetTransform(ourTransform);
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   SimCore::Actors::BasePhysicsVehicleActor *GameAppComponent::CreateNewVehicle(const std::string &vehicleName)
   {
      SimCore::Actors::BasePhysicsVehicleActor* vehicle = NULL;

      SimCore::CommandLineObject* commandLineObject = GetCommandLineObject();
      if(commandLineObject == NULL)
      {
         LOG_ERROR("commandLineObject is null, InitializeVehicle will not occur");
         return NULL;
      }

      // Find the vehicle template based on the name. The default is 'Driver_Vehicle'.
      std::vector<dtDAL::ActorProxy*> toFill;
      GetGameManager()->FindPrototypesByName(vehicleName, toFill);


      // CREATE OUR NEW VEHICLE 
      if(!toFill.empty())
      {
#ifdef AGEIA_PHYSICS
         GMComponent* tempComponent = GetGameManager()->GetComponentByName(dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_NAME);
         dtAgeiaPhysX::NxAgeiaWorldComponent* ageiaComponent = static_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(tempComponent);

         // Configure the Ageia Component
         NxScene& nxScene = ageiaComponent->GetPhysicsScene(std::string("Default"));
         nxScene.setGroupCollisionFlag(30, 0, false);  // characters interact with world
#endif
         dtCore::RefPtr<dtDAL::ActorProxy> ourActualActorProxy = 
            GetGameManager()->CreateActorFromPrototype(toFill.front()->GetId());
         if(ourActualActorProxy != NULL)
         {
            vehicle = dynamic_cast<SimCore::Actors::BasePhysicsVehicleActor*>(ourActualActorProxy->GetActor());
            if (vehicle != NULL)
            {
               vehicle->SetArticulationHelper( new DriverArticulationHelper );

               // Ensure the vehicle will publish its call-sign via its name property
               const dtDAL::NamedStringParameter* callsignName
                  = dynamic_cast<const dtDAL::NamedStringParameter*>
                  (commandLineObject->GetParameter(GameAppComponent::CMD_LINE_VEHICLE_CALLSIGN));
               if( callsignName != NULL )
               {
                  vehicle->SetName( callsignName->GetValue() );
               }

               vehicle->GetPhysicsHelper()->SetVehicleStartingPosition( mStartingPosition );

               GetGameManager()->AddActor(vehicle->GetGameActorProxy(), false, true);

               // Set the vehicle heading.
               const dtDAL::NamedFloatParameter* paramHeading
                  = dynamic_cast<const dtDAL::NamedFloatParameter*>
                  (commandLineObject->GetParameter(GameAppComponent::CMD_LINE_START_HEADING));
               if( paramHeading != NULL )
               {
                  osg::Vec3 hpr( paramHeading->GetValue(), 0.0f, 0.0f );
                  osg::Matrix orient;
                  dtUtil::MatrixUtil::HprToMatrix( orient, hpr );

                  vehicle->GetPhysicsHelper()->SetOrientation( orient );
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
   void GameAppComponent::InitializeTools()
   {
      // nothing to do at this time.
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
      mStealth = static_cast<SimCore::Actors::PlayerActor*>(ap->GetActor());

      // make the camera a child
      mStealth->AddChild(GetGameManager()->GetApplication().GetCamera());

      // add this actor to the game manager
      GetGameManager()->AddActor(mStealth->GetGameActorProxy(), false, false);

      // so the input component knows whats going on
      mInputComponent->SetPlayer(mStealth.get());

      const dtDAL::NamedStringParameter* callsignName
         = dynamic_cast<const dtDAL::NamedStringParameter*>
         (commandLineObject->GetParameter(DriverDemo::GameAppComponent::CMD_LINE_VEHICLE_CALLSIGN));
      if( callsignName != NULL )
      {
         mStealth->SetName( callsignName->GetValue() );
      }
      else
      {
         mStealth->SetName( "Player" );
      }

      // starts it out with the correct offset, if needed
      //mStealth->SetAttachOffset( osg::Vec3(-0.4f,0.3f,0.35f));
   }
} // end dvte namespace.
