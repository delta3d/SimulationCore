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
* @author Curtiss Murphy
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/LogicOnEventActor.h>
#include <SimCore/Actors/LogicConditionalActor.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/arrayactorproperty.h>
#include <dtCore/namedparameter.h>
#include <dtGame/message.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/invokable.h>
#include <dtGame/exceptionenum.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(LogicOnEventActor::LogicTypeEnum);
      LogicOnEventActor::LogicTypeEnum::LogicTypeEnum(const std::string& name, const std::string& displayName)
         : dtUtil::Enumeration(name)
         , mDisplayName(displayName)
      {
         AddInstance(this);
      }
      const std::string& LogicOnEventActor::LogicTypeEnum::GetDisplayName()
      { 
         return mDisplayName; 
      }
      LogicOnEventActor::LogicTypeEnum LogicOnEventActor::LogicTypeEnum::BOOLEAN_OR("OR", "Listens for events on the children and triggers if ANY of them are true.");
      LogicOnEventActor::LogicTypeEnum LogicOnEventActor::LogicTypeEnum::BOOLEAN_AND("AND", "Listens for events on the children and triggers only when ALL of them are true.");


      //////////////////////////////////////////////////////////////////////////
      // Drawable CODE
      //////////////////////////////////////////////////////////////////////////
      LogicOnEventDrawable ::LogicOnEventDrawable ( LogicOnEventActor& proxy )
         : BaseClass(proxy)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      LogicOnEventDrawable::~LogicOnEventDrawable()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      // Actor CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString LogicOnEventActor::CLASS_NAME("SimCore::Logic::LogicOnEventDrawable");
      const dtUtil::RefString LogicOnEventActor::PROPERTY_LOGIC_TYPE("Logic Type");
      const dtUtil::RefString LogicOnEventActor::PROPERTY_EVENT_TO_FIRE("Event To Fire");
      const dtUtil::RefString LogicOnEventActor::PROPERTY_CONDITIONALS("Conditional Array");
      
      //////////////////////////////////////////////////////////////////////////
      LogicOnEventActor::LogicOnEventActor()
      : mLogicType(&LogicOnEventActor::LogicTypeEnum::BOOLEAN_AND)
      , mCurrentStatus(false)
      , mConditionListIsDirty(true)
      , mConditionsIndex(0)
      {
         SetClassName(LogicOnEventActor::CLASS_NAME);
      }

      //////////////////////////////////////////////////////////////////////////
      LogicOnEventActor::~LogicOnEventActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::OnEnteredWorld()
      {
         RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT, dtUtil::MakeFunctor(&LogicOnEventActor::ProcessGameEventMessage, this));
      }

      //////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::CreateDrawable()
      {
         SetDrawable( *new LogicOnEventDrawable(*this) );
      }

      //////////////////////////////////////////////////////////
      void LogicOnEventActor::BuildPropertyMap()
      {
         static const dtUtil::RefString PROPERTY_CONDITION("Condition");
         static const dtUtil::RefString PROP_LOGIC_TYPE_DESC("Indicates whether we check AND or OR on our child conditionals before firing our event.");
         static const dtUtil::RefString PROP_EVENT_TO_FIRE_DESC("The event to fire when this set of conditionals is met.");
         static const dtUtil::RefString PROP_CONDITION_DESC("One of the conditionals used for the OnEvent logic.");
         static const dtUtil::RefString PROP_CONDITIONAL_LIST_DESC("A list of conditionals that this logic actor controls.");

         BaseClass::BuildPropertyMap();

         const std::string GROUP("Logic Data");

         LogicOnEventActor* actor = this;

         AddProperty(new dtCore::EnumActorProperty<LogicOnEventActor::LogicTypeEnum>(
            PROPERTY_LOGIC_TYPE, PROPERTY_LOGIC_TYPE,
            dtCore::EnumActorProperty<LogicOnEventActor::LogicTypeEnum>::SetFuncType
               (actor, &LogicOnEventActor::SetLogicType),
            dtCore::EnumActorProperty<LogicOnEventActor::LogicTypeEnum>::GetFuncType
               (actor, &LogicOnEventActor::GetLogicType),
            PROP_LOGIC_TYPE_DESC, GROUP));

         AddProperty(new dtCore::GameEventActorProperty(*this, PROPERTY_EVENT_TO_FIRE, PROPERTY_EVENT_TO_FIRE, 
            dtCore::GameEventActorProperty::SetFuncType(actor, &LogicOnEventActor::SetEventToFire),
            dtCore::GameEventActorProperty::GetFuncType(actor, &LogicOnEventActor::GetEventToFire),
            PROP_EVENT_TO_FIRE_DESC, GROUP));


         // The following 2 props go together. Part of the array thing.
         // A Conditional in the Conditional List
         dtCore::ActorIDActorProperty* actorProp = new dtCore::ActorIDActorProperty(
            *this, PROPERTY_CONDITION, PROPERTY_CONDITION,
            dtCore::ActorIDActorProperty::SetFuncType(actor, &LogicOnEventActor::SetChildConditional),
            dtCore::ActorIDActorProperty::GetFuncType(actor, &LogicOnEventActor::GetChildConditional),
            "SimCore::Logic::LogicConditionalActor", PROP_CONDITION_DESC, GROUP);
         // The Task List.
         AddProperty(new dtCore::ArrayActorProperty<dtCore::UniqueId>(
            PROPERTY_CONDITIONALS, PROPERTY_CONDITIONALS, PROP_CONDITIONAL_LIST_DESC,
            dtCore::ArrayActorProperty<dtCore::UniqueId>::SetIndexFuncType(actor, &LogicOnEventActor::ConditionalArraySetIndex),
            dtCore::ArrayActorProperty<dtCore::UniqueId>::GetDefaultFuncType(actor, &LogicOnEventActor::ConditionalArrayGetDefault),
            dtCore::ArrayActorProperty<dtCore::UniqueId>::GetArrayFuncType(actor, &LogicOnEventActor::ConditionalArrayGetValue),
            dtCore::ArrayActorProperty<dtCore::UniqueId>::SetArrayFuncType(actor, &LogicOnEventActor::ConditionalArraySetValue),
            actorProp, GROUP));

      }
      ////////////////////////////////////////////////////////////////////////////////////
      LogicOnEventActor::LogicTypeEnum& LogicOnEventActor::GetLogicType() const
      {
         return *mLogicType;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::SetLogicType(LogicOnEventActor::LogicTypeEnum& logicType)
      {
         mLogicType = &logicType;
      }

      /////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::SetEventToFire(dtCore::GameEvent* pEvent)
      {
         mEventToFire = pEvent;
      }

      /////////////////////////////////////////////////////////////////////////////
      dtCore::GameEvent* LogicOnEventActor::GetEventToFire()
      {
         return mEventToFire.get();
      }


      /////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::ProcessGameEventMessage(const dtGame::GameEventMessage& gem)
      {
         const dtCore::GameEvent& gameEvent = *gem.GetGameEvent();

         ProcessGameEvent( gameEvent );
      }

      /////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::ResolveDirtyList()
      {
         dtGame::GameManager* gm = GetGameManager();

         // Resolve dirty list.
         if (mConditionListIsDirty && gm != NULL)
         {
            //std::vector< dtCore::ObserverPtr< LogicConditionalActor > > mConditionsListAsActors;
            mConditionsListAsActors.clear();

            // Look each one up and add it to our normal list.
            unsigned int numConditions = mConditions.size();
            for (unsigned int i = 0; i < numConditions; ++i)
            {
               dtCore::UniqueId& newId = mConditions[i];
               SimCore::Actors::LogicConditionalActor* newActor =
                  dynamic_cast<SimCore::Actors::LogicConditionalActor*>(gm->FindActorById(newId));
               if (newActor != NULL)
               {
                  mConditionsListAsActors.push_back(newActor);
               }
            }

            mConditionListIsDirty = false;
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::ProcessGameEvent(const dtCore::GameEvent& gameEvent)
      {
         bool bMatchingEventFound = false;
         // Initial cumulative value of all children is true when using AND but False for OR
         bool bCumulativeStatusCheck = (mLogicType == &LogicOnEventActor::LogicTypeEnum::BOOLEAN_AND);

         //if(mEventEnableFocus.valid() && gameEvent == *mEventEnableFocus)
         ResolveDirtyList();

         // go through our children. check the true/false conditions.
         unsigned int numConditions = mConditionsListAsActors.size();
         for (unsigned int i = 0; i < numConditions; ++i)
         {
            SimCore::Actors::LogicConditionalActor* condition = mConditionsListAsActors[i].get();
            if (condition == NULL)
            {
               mConditionListIsDirty = true;
               continue;
            }

            if (condition->GetFalseEvent() == &gameEvent)
            {
               bMatchingEventFound = true;
               condition->SetIsTrue(false);
            }
            else if (condition->GetTrueEvent() == &gameEvent)
            {
               bMatchingEventFound = true;
               condition->SetIsTrue(true);
            }

            // Regardless, do a cumulative logic result.
            if (mLogicType == &LogicOnEventActor::LogicTypeEnum::BOOLEAN_AND)
            {
               bCumulativeStatusCheck = bCumulativeStatusCheck && condition->GetIsTrue();
            }
            else  // OR
            {
               bCumulativeStatusCheck = bCumulativeStatusCheck || condition->GetIsTrue();
            }
         }

         // If bMatchingEventFound, use our cumulative status and publish event (or NOT)
         if (bMatchingEventFound)
         {
            mCurrentStatus = bCumulativeStatusCheck;
            if (mCurrentStatus && mEventToFire.valid())
            {
               SendGameEventMessage(*mEventToFire);
            }
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::SendGameEventMessage(dtCore::GameEvent& gameEvent)
      {
         dtGame::GameManager* gm = GetGameManager();
         if( gm != NULL )
         {
            dtCore::RefPtr<dtGame::GameEventMessage> eventMessage;
            gm->GetMessageFactory().CreateMessage( dtGame::MessageType::INFO_GAME_EVENT, eventMessage );

            eventMessage->SetAboutActorId( GetId() );
            eventMessage->SetGameEvent( gameEvent );
            gm->SendMessage( *eventMessage );
         }
      }


      ////////////////////////////////////////////////////////////////////////////////
      // The following methods are all to support the array property
      ////////////////////////////////////////////////////////////////////////////////

      void LogicOnEventActor::AddConditional(dtCore::UniqueId id)
      {
         mConditions.push_back(id);
         mConditionListIsDirty = true;
      }


      ////////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::SetChildConditional(dtCore::UniqueId value)
      {
         if (mConditionsIndex < (int)mConditions.size())
         {
            mConditions[mConditionsIndex] = value;
         }
         mConditionListIsDirty = true;
      }

      ////////////////////////////////////////////////////////////////////////////////
      dtCore::UniqueId LogicOnEventActor::GetChildConditional()
      {
         if (mConditionsIndex < (int)mConditions.size())
         {
            return mConditions[mConditionsIndex];
         }

         return dtCore::UniqueId("");
      }


      ////////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::ConditionalArraySetIndex(int index)
      {
         mConditionsIndex = index;
      }

      ////////////////////////////////////////////////////////////////////////////////
      dtCore::UniqueId LogicOnEventActor::ConditionalArrayGetDefault()
      {
         return dtCore::UniqueId("");
      }

      ////////////////////////////////////////////////////////////////////////////////
      std::vector<dtCore::UniqueId> LogicOnEventActor::ConditionalArrayGetValue()
      {
         return mConditions;
      }

      ////////////////////////////////////////////////////////////////////////////////
      void LogicOnEventActor::ConditionalArraySetValue(const std::vector<dtCore::UniqueId>& value)
      {
         mConditions.clear();

         // Now add all the conditions.
         for (int index = 0; index < (int)value.size(); index++)
         {
            dtCore::UniqueId id = value[index];
            // Check for dups?  Not sure that it really matters
            mConditions.push_back(id);
         }

         mConditionListIsDirty = true;
      }

   }
}
