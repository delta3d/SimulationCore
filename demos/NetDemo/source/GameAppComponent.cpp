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

#include "GameAppComponent.h"

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PlayerActor.h>
#include <dtCore/camera.h>
#include <dtCore/transform.h>
#include <dtABC/application.h>
#include <dtGame/messagetype.h>
#include <dtActors/playerstartactorproxy.h>
#include <dtActors/engineactorregistry.h>

namespace NetDemo
{
   GameAppComponent::GameAppComponent(const std::string& name)
   : BaseGameAppComponent(name)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::ProcessMessage(const dtGame::Message& msg)
   {
      if (msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
         InitializePlayer();
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void GameAppComponent::InitializePlayer()
   {
      dtCore::RefPtr<dtGame::GameActorProxy> ap;

      // create a player actor, walk run jump and drink :)
      GetGameManager()->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, ap);

      // make the stealh aware
      mStealth = static_cast<SimCore::Actors::PlayerActor*>(ap->GetActor());

      // make the camera a child
      mStealth->AddChild(GetGameManager()->GetApplication().GetCamera());

      // add this actor to the game manager
      GetGameManager()->AddActor(mStealth->GetGameActorProxy(), false, false);

      mStealth->SetName("Player");

      //////////////TEMP HACK
      dtActors::PlayerStartActorProxy* startPosProxy = NULL;
      GetGameManager()->FindActorByType(*dtActors::EngineActorRegistry::PLAYER_START_ACTOR_TYPE, startPosProxy);
      if (startPosProxy != NULL)
      {
         dtCore::Transformable* actor = NULL;
         startPosProxy->GetActor(actor);
         dtCore::Transform xform;
         actor->GetTransform(xform);
         mStealth->SetTransform(xform);
      }
      ////////////////////////
   }
}
