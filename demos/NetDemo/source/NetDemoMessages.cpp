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



   ////////////////////////////////////////////////////////////////////////////////
   // ENTITY ACTION MESSAGE CODE
   ////////////////////////////////////////////////////////////////////////////////
   const dtUtil::RefString EntityActionMessage::PARAMETER_ACTION("Action");
   const dtUtil::RefString EntityActionMessage::PARAMETER_LOCATION("Location");
   const dtUtil::RefString EntityActionMessage::PARAMETER_OWNER_ID("OwnerID");
   const dtUtil::RefString EntityActionMessage::PARAMETER_POINTS("Points");
   const dtUtil::RefString EntityActionMessage::PARAMETER_RESOURCE_POINTS("ResourcePoints");

   ////////////////////////////////////////////////////////////////////////////////
   EntityActionMessage::EntityActionMessage()
      : BaseClass()
      , mParamAction(new dtGame::EnumMessageParameter(PARAMETER_ACTION, EntityAction::UNKNOWN.GetName()))
      , mParamLocation(new dtGame::Vec3MessageParameter(PARAMETER_LOCATION, osg::Vec3()))
      , mParamOwnerID(new dtGame::StringMessageParameter(PARAMETER_OWNER_ID, ""))
      , mParamPoints(new dtGame::IntMessageParameter(PARAMETER_POINTS, 0))
      , mParamResourcePoints(new dtGame::IntMessageParameter(PARAMETER_RESOURCE_POINTS, 0))
   {
      AddParameter(mParamAction.get());
      AddParameter(mParamLocation.get());
      AddParameter(mParamOwnerID.get());
      AddParameter(mParamPoints.get());
      AddParameter(mParamResourcePoints.get());
   }
   
   ////////////////////////////////////////////////////////////////////////////////
   EntityActionMessage::~EntityActionMessage()
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::SetAction(const EntityAction& action)
   {
      mParamAction->SetValue(action.GetName());
   }

   ////////////////////////////////////////////////////////////////////////////////
   const EntityAction& EntityActionMessage::GetAction() const
   {
      return *EntityAction::GetValueForName(mParamAction->GetValue());
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::SetLocation(const osg::Vec3& location)
   {
      mParamLocation->SetValue(location);
   }

   ////////////////////////////////////////////////////////////////////////////////
   const osg::Vec3& EntityActionMessage::GetLocation() const
   {
      return mParamLocation->GetValue();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::SetOwnerID(const std::string& ownerId)
   {
      mParamOwnerID->SetValue(ownerId);
   }

   ////////////////////////////////////////////////////////////////////////////////
   const std::string& EntityActionMessage::GetOwnerID() const
   {
      return mParamOwnerID->GetValue();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::SetPoints(int points)
   {
      mParamPoints->SetValue(points);
   }

   ////////////////////////////////////////////////////////////////////////////////
   int EntityActionMessage::GetPoints() const
   {
      return mParamPoints->GetValue();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::SetResourcePoints(int points)
   {
      mParamResourcePoints->SetValue(points);
   }

   ////////////////////////////////////////////////////////////////////////////////
   int EntityActionMessage::SetResourcePoints() const
   {
      return mParamResourcePoints->GetValue();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void EntityActionMessage::Set(const EntityActionMessageParams& paramStruct)
   {
      if(paramStruct.mActionType != NULL)
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
