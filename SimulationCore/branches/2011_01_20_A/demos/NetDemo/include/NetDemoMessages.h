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

#ifndef NETDEMO_MESSAGES_H
#define NETDEMO_MESSAGES_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtGame/message.h>
#include <dtGame/messagemacros.h>
#include <dtUtil/enumeration.h>
#include "DemoExport.h"



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // ENTITY ACTION CODE
   /////////////////////////////////////////////////////////////////////////////
   class NETDEMO_EXPORT EntityAction : public dtUtil::Enumeration
   {
      DECLARE_ENUM(EntityAction);
      public:
         typedef dtUtil::Enumeration BaseClass;

         static EntityAction UNKNOWN;
         static EntityAction SCORE;

      private:
         EntityAction(const std::string& name);
   };



   /////////////////////////////////////////////////////////////////////////////
   // ENTITY ACTION MESSAGE CODE
   /////////////////////////////////////////////////////////////////////////////
   struct EntityActionMessageParams;

   DT_DECLARE_MESSAGE_BEGIN(EntityActionMessage, dtGame::Message, NETDEMO_EXPORT)

      void SetAction(const EntityAction& action);
      const EntityAction& GetAction() const;

      DECLARE_PARAMETER_INLINE(std::string, ActionEnum)
      DECLARE_PARAMETER_INLINE(osg::Vec3, Location)
      DECLARE_PARAMETER_INLINE(std::string, OwnerID)
      DECLARE_PARAMETER_INLINE(int, Points)
      DECLARE_PARAMETER_INLINE(int, ResourcePoints)
      void Set(const EntityActionMessageParams& paramStruct);

   DT_DECLARE_MESSAGE_END()

   /////////////////////////////////////////////////////////////////////////////
   // Simple parameter struct for simplifying message sending, when used
   // with the message utility methods. This struct mirrors the same parameters
   // as found in the associated message.
   struct EntityActionMessageParams
   {
      EntityActionMessageParams()
         : mActionType(&EntityAction::UNKNOWN)
         , mPoints(0)
         , mResourcePoints(0)
      {
      }

      const EntityAction* mActionType;
      int mPoints;
      int mResourcePoints;
      osg::Vec3 mLocation;
      dtCore::UniqueId mAboutActorId;
      dtCore::UniqueId mSendingActorId;
      dtCore::UniqueId mOwnerId;
   };

}

#endif
