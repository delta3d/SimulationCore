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

#ifndef NETDEMO_UTILS_H
#define NETDEMO_UTILS_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include "DemoExport.h"



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class UniqueId;
}

namespace dtGame
{
   class GameActor;
   class GameManager;
}

namespace NetDemo
{
   struct EntityActionMessageParams;
   class MessageType;

   /////////////////////////////////////////////////////////////////////////////
   // MESSAGE UTILS CODE
   /////////////////////////////////////////////////////////////////////////////
   class NETDEMO_EXPORT MessageUtils
   {
      public:
         /**
          * Convenience method for sending out a simple NetDemo message with no data params
          * @param messageType The NetDemo message type.
          * @param gm The game manager.
          */
         static void SendSimpleMessage(const NetDemo::MessageType& messageType, 
            dtGame::GameManager& gm);

         /**
          * Convenience method for handling the creation of a message and
          * setting all of its parameters.
          * @param sendingActor The actor initiating the message and who
          *        has access to the Game Manager for creating and sending
          *        the new message.
          * @param params Struct that contains the same collection of properties
          *        as that of the new message. The struct simplifies this method's
          *        parameter list and allows changes to the message's interface
          *         without breaking this method's signature.
          */
         static void SendActionMessage(dtGame::GameActor& sendingActor,
            const EntityActionMessageParams& params);

         /**
          * Convenience method that subsequently calls SendActionMessage
          * with a few of the parameters already set.
          * @param sendingActor The actor being scored on/hit.
          * @param aboutActorId The ID of the actor that will receive the score.
          * @param points Points awarded for the score.
          */
         static void SendScoreMessage(dtGame::GameActor& sendingActor,
            const dtCore::UniqueId& aboutActorId, int points);

      private:
         MessageUtils() {}
         virtual ~MessageUtils() {}
   };



   /////////////////////////////////////////////////////////////////////////////
   // GENERAL USE FUNCTIONS
   /////////////////////////////////////////////////////////////////////////////
   osg::Node* LoadNodeFile(const std::string& projectRelativePath);

   /////////////////////////////////////////////////////////////////////////////
   dtGame::GameActor* FindOwnerForActor(dtGame::GameActor& actor);

   /////////////////////////////////////////////////////////////////////////////
   template<typename T_Actor >
   void GetActorFromGM(dtGame::GameManager& gm, const dtCore::UniqueId& actorId, T_Actor*& outActor)
   {
      dtGame::GameActorProxy* proxy = nullptr;
      gm.FindActorById(actorId, proxy);

      if(proxy != nullptr)
      {
         proxy->GetActor(outActor);
      }
      else
      {
         outActor = nullptr;
      }
   }

} // END - NetDemo namespace

#endif
