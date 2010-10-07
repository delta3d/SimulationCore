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
#include <osgDB/ReadFile>
#include <dtCore/transform.h>
#include <dtDAL/project.h>
#include <dtGame/gamemanager.h>
#include <dtGame/messagefactory.h>
#include <dtUtil/fileutils.h>
#include <SimCore/Actors/WeaponActor.h>
#include "NetDemoUtils.h"
#include "NetDemoMessages.h"
#include "NetDemoMessageTypes.h"
#include "Actors/TowerActor.h"



namespace NetDemo
{
   ////////////////////////////////////////////////////////////////////////////////
   void MessageUtils::SendSimpleMessage(const NetDemo::MessageType& messageType, 
      dtGame::GameManager& gm)
   {
      dtCore::RefPtr<dtGame::Message> message;
      gm.GetMessageFactory().CreateMessage(messageType, message);
      gm.SendMessage(*message);
   }


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

      // Find the Player ID.
      dtGame::GameActor* scoringActor = NULL;
      GetActorFromGM(*sendingActor.GetGameActorProxy().GetGameManager(),
         aboutActorId, scoringActor);
      if(scoringActor != NULL)
      {
         scoringActor = FindOwnerForActor(*scoringActor);
         params.mOwnerId = scoringActor->GetUniqueId();
      }

      // Create, set and send the message with the specified parameters.
      SendActionMessage(sendingActor, params);
   }

   ////////////////////////////////////////////////////////////////////////////////
   osg::Node* LoadNodeFile(const std::string& projectRelativePath)
   {
      dtDAL::ResourceDescriptor descriptor(projectRelativePath);

      std::string resPath;
      try
      {
         resPath = dtDAL::Project::GetInstance().GetResourcePath(descriptor);
      }
      catch (...)
      {
         LOG_ERROR("Could not locate file \"" + projectRelativePath + "\"");
      }
      
      osg::Node* node = NULL;
      if(dtUtil::FileUtils::GetInstance().FileExists(resPath))
      {
         node = osgDB::readNodeFile(resPath);
      }

      return node;
   }

   ////////////////////////////////////////////////////////////////////////////////
   dtGame::GameActor* FindOwnerForActor(dtGame::GameActor& actor)
   {
      dtGame::GameActor* owner = &actor;

      SimCore::Actors::WeaponActor* weapon = dynamic_cast<SimCore::Actors::WeaponActor*>(&actor);
      NetDemo::TowerActor* tower = dynamic_cast<NetDemo::TowerActor*>(&actor);
      
      if(weapon != NULL)
      {
         owner = dynamic_cast<dtGame::GameActor*>(weapon->GetOwner());

         tower = dynamic_cast<NetDemo::TowerActor*>(owner);
      }

      if(tower != NULL)
      {
         // TODO:
         // Get owner from the tower
      }

      return owner;
   }

}
