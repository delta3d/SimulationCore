/*
 * Copyright, 2007, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtCore/transform.h>
#include <dtGame/gamemanager.h>
#include "NetDemoUtils.h"
#include "NetDemoMessages.h"
#include "NetDemoMessageTypes.h"



namespace NetDemo
{
   ////////////////////////////////////////////////////////////////////////////////
   void MessageUtils::SendActionMessage(dtGame::GameActor& sendingActor,
      const EntityActionMessageParams& params)
   {
      dtGame::GameManager* gm = sendingActor.GetGameActorProxy().GetGameManager();
      if(gm != NULL)
      {
         // Create the message.
         dtCore::RefPtr<dtGame::Message> message;
         gm->GetMessageFactory().CreateMessage(NetDemo::MessageType::ENTITY_ACTION, message);
         
         // Set the message parameters.
         EntityActionMessage* actionMessage = static_cast<EntityActionMessage*>(message.get());
         actionMessage->Set(params);
         actionMessage->SetSendingActorId(sendingActor.GetUniqueId());

         // Send the message to the rest of the system and the network.
         gm->SendMessage(*message);
         gm->SendNetworkMessage(*message);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void MessageUtils::SendScoreMessage(dtGame::GameActor& sendingActor,
      const dtCore::UniqueId& aboutActorId, int points)
   {
      // Set the important parameters for the score message.
      EntityActionMessageParams params;
      params.mAboutActorId = aboutActorId;
      params.mPoints = points;
      params.mActionType = &EntityAction::SCORE;

      // Get the location of the hit actor.
      dtCore::Transform xform;
      sendingActor.GetTransform(xform);
      xform.GetTranslation(params.mLocation);

      // Create, set and send the message with the specified parameters.
      SendActionMessage(sendingActor, params);
   }

}
