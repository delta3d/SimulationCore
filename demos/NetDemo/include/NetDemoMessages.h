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
   class NETDEMO_EXPORT EntityActionMessage : public dtGame::Message
   {
      public:
         typedef dtGame::Message BaseClass;

         static const dtUtil::RefString PARAMETER_ACTION;
         static const dtUtil::RefString PARAMETER_LOCATION;
         static const dtUtil::RefString PARAMETER_OWNER_ID;
         static const dtUtil::RefString PARAMETER_POINTS;
         static const dtUtil::RefString PARAMETER_RESOURCE_POINTS;

         EntityActionMessage();

         void SetAction(const EntityAction& action);
         const EntityAction& GetAction() const;

         void SetLocation(const osg::Vec3& location);
         const osg::Vec3& GetLocation() const;

         void SetOwnerID(const std::string& ownerId);
         const std::string& GetOwnerID() const;

         void SetPoints(int points);
         int GetPoints() const;

         void SetResourcePoints(int points);
         int SetResourcePoints() const;

      protected:
         virtual ~EntityActionMessage();

      private:
         dtCore::RefPtr<dtGame::EnumMessageParameter> mParamAction;
         dtCore::RefPtr<dtGame::Vec3MessageParameter> mParamLocation;
         dtCore::RefPtr<dtGame::StringMessageParameter> mParamOwnerID;
         dtCore::RefPtr<dtGame::IntMessageParameter> mParamPoints;
         dtCore::RefPtr<dtGame::IntMessageParameter> mParamResourcePoints;
   };
}

#endif
