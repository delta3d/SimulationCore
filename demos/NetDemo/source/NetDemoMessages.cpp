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
#include "NetDemoMessages.h"



namespace NetDemo
{
   ////////////////////////////////////////////////////////////////////////////////
   // ENTITY ACTION CODE
   ////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(EntityAction);
   EntityAction EntityAction::UNKNOWN("UNKNOWN");
   EntityAction EntityAction::SCORE("SCORE");
   
   ////////////////////////////////////////////////////////////////////////////////
   EntityAction::EntityAction(const std::string& name)
      : BaseClass(name)
   {
      AddInstance(this);
   }


   DT_IMPLEMENT_MESSAGE_BEGIN(EntityActionMessage)
      DT_ADD_PARAMETER(std::string, ActionEnum)
      DT_ADD_PARAMETER(osg::Vec3, Location)
      DT_ADD_PARAMETER(std::string, OwnerID)
      DT_ADD_PARAMETER(int, Points)
      DT_ADD_PARAMETER(int, ResourcePoints)
   DT_IMPLEMENT_MESSAGE_END()

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::SetAction(const EntityAction& action)
   {
      SetActionEnum(action.GetName());
   }

   ////////////////////////////////////////////////////////////////////////////////
   const EntityAction& EntityActionMessage::GetAction() const
   {
      return *EntityAction::GetValueForName(GetActionEnum());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::Set(const EntityActionMessageParams& paramStruct)
   {
      if(paramStruct.mActionType != nullptr)
      {
         SetAction(*paramStruct.mActionType);
      }

      SetPoints(paramStruct.mPoints);
      SetResourcePoints(paramStruct.mResourcePoints);
      SetLocation(paramStruct.mLocation);
      SetAboutActorId(paramStruct.mAboutActorId);
      SetSendingActorId(paramStruct.mSendingActorId);
      SetOwnerID(paramStruct.mOwnerId.ToString());
   }

}
