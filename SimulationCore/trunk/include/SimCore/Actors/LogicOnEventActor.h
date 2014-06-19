/* -*-c++-*-
* Simulation Core
* Copyright 2010, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
*/
#ifndef LOGIC_ON_EVENT_ACTOR_H
#define LOGIC_ON_EVENT_ACTOR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtGame/gameactor.h>
#include <dtCore/gameevent.h>

namespace dtGame
{
   class GameEventMessage;
}

namespace SimCore
{
   namespace Actors
   {
      class LogicOnEventDrawable;
      class LogicConditionalActor;

      //////////////////////////////////////////////////////////////////////////
      // Actor CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LogicOnEventActor : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;
         static const dtUtil::RefString CLASS_NAME;
         static const dtUtil::RefString PROPERTY_LOGIC_TYPE;
         static const dtUtil::RefString PROPERTY_EVENT_TO_FIRE;
         static const dtUtil::RefString PROPERTY_CONDITIONALS;

         ////////////////////////////////////////////////
         class SIMCORE_EXPORT LogicTypeEnum : public dtUtil::Enumeration
         {
            DECLARE_ENUM(LogicTypeEnum);
         public:
            static LogicTypeEnum BOOLEAN_OR;
            static LogicTypeEnum BOOLEAN_AND;
            const std::string& GetDisplayName();
         private:
            LogicTypeEnum(const std::string& name, const std::string &displayName);
            const std::string mDisplayName;
         };

         // Standard Actor Proxy Behaviors
         LogicOnEventActor();
         virtual void CreateDrawable();
         virtual void BuildPropertyMap();
         virtual void OnEnteredWorld();
         /// The event to fire when our condition is satisfied.
         void SetEventToFire( dtCore::GameEvent* gameEvent );
         dtCore::GameEvent* GetEventToFire();

         /// Decides whether we consider ALL (AND) or ANY (OR) when looking at the conditional events
         void SetLogicType(LogicOnEventActor::LogicTypeEnum& logicType);
         LogicOnEventActor::LogicTypeEnum& GetLogicType() const;

         /// This method holds onto to the status from the last time we got a relevant game event
         bool GetCurrentStatus() const {return mCurrentStatus; }

         virtual void ProcessGameEventMessage(const dtGame::GameEventMessage& gem);
         virtual void ProcessGameEvent(const dtCore::GameEvent& gameEvent);

         void SendGameEventMessage(dtCore::GameEvent& gameEvent);

         /**
         * Conditional Array actor property functors.
         */
         void AddConditional(dtCore::UniqueId id);
         void SetChildConditional(dtCore::UniqueId value);
         dtCore::UniqueId GetChildConditional();

         void ConditionalArraySetIndex(int index);
         dtCore::UniqueId ConditionalArrayGetDefault();
         std::vector<dtCore::UniqueId> ConditionalArrayGetValue();
         void ConditionalArraySetValue(const std::vector<dtCore::UniqueId>& value);

      protected:
         virtual ~LogicOnEventActor();
         void ResolveDirtyList();
      private:

         LogicOnEventActor::LogicTypeEnum* mLogicType;
         dtCore::ObserverPtr<dtCore::GameEvent> mEventToFire;
         std::vector< dtCore::ObserverPtr< LogicConditionalActor > > mConditionsListAsActors;
         bool mCurrentStatus; // false
         bool mConditionListIsDirty;

         // List of child conditions
         std::vector<dtCore::UniqueId> mConditions;
         int mConditionsIndex;
      };


      //////////////////////////////////////////////////////////////////////////
      // Drawable CODE - This actor holds onto an array of LogicConditionalActors
      // Then, when it gets an event, it walks through the array and checks to 
      // see if it has any matches. If so, it updates the child, and then updates
      // itself. Depending on the settings, it fires an event.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LogicOnEventDrawable : public dtGame::GameActor
      {
         public:
            typedef dtGame::GameActor BaseClass;

            LogicOnEventDrawable(LogicOnEventActor& actor);


         protected:
            virtual ~LogicOnEventDrawable();


         private:
      };



   }
}

#endif
