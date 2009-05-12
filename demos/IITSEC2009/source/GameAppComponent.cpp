/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
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

namespace IITSEC2009Game
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
