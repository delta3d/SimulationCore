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
#include <SimCore/Actors/LogicConditionalActor.h>
#include <dtCore/enginepropertytypes.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // EMPTY DRAWABLE CODE
      //////////////////////////////////////////////////////////////////////////
      LogicConditionalDrawable::LogicConditionalDrawable(const std::string& name)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      LogicConditionalDrawable::~LogicConditionalDrawable()
      {
      }

      /////////////////////////////////////////////////////////////////////////////
      osg::Node* LogicConditionalDrawable::GetOSGNode()
      {
         return mPlaceholderNode.get();
      }

      /////////////////////////////////////////////////////////////////////////////
      const osg::Node* LogicConditionalDrawable::GetOSGNode() const
      {
         return mPlaceholderNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString LogicConditionalActor::PROPERTY_TRUE_EVENT("True Event");
      const dtUtil::RefString LogicConditionalActor::PROPERTY_FALSE_EVENT("False Event");
      const dtUtil::RefString LogicConditionalActor::PROPERTY_IS_TRUE("IsTrue");
      const dtUtil::RefString LogicConditionalActor::CLASS_NAME("SimCore::Logic::LogicConditionalActor");
      
      //////////////////////////////////////////////////////////////////////////
      LogicConditionalActor::LogicConditionalActor()
      : mIsTrue(false)
      {
         SetClassName(LogicConditionalActor::CLASS_NAME);
      }

      //////////////////////////////////////////////////////////////////////////
      LogicConditionalActor::~LogicConditionalActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void LogicConditionalActor::CreateDrawable()
      {
         SetDrawable( *new LogicConditionalDrawable("LogicConditional") );
      }

      //////////////////////////////////////////////////////////
      void LogicConditionalActor::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const std::string GROUP("Logic Data"); // only used to group props in STAGE

         LogicConditionalActor* actor = this;

         AddProperty(new dtCore::BooleanActorProperty(PROPERTY_IS_TRUE, PROPERTY_IS_TRUE, 
            dtCore::BooleanActorProperty::SetFuncType(actor, &LogicConditionalActor::SetIsTrue),
            dtCore::BooleanActorProperty::GetFuncType(actor, &LogicConditionalActor::GetIsTrue),
            "The initial or current true state. Generally set by a parent logic controller based on the true and false events.",
            GROUP));

         AddProperty(new dtCore::GameEventActorProperty(*this, PROPERTY_TRUE_EVENT, PROPERTY_TRUE_EVENT, 
            dtCore::GameEventActorProperty::SetFuncType(actor, &LogicConditionalActor::SetTrueEvent),
            dtCore::GameEventActorProperty::GetFuncType(actor, &LogicConditionalActor::GetTrueEvent),
            "The event to look for to set this actor to TRUE. Used by a parent logic controller.",
            GROUP));

         AddProperty(new dtCore::GameEventActorProperty(*this, PROPERTY_FALSE_EVENT, PROPERTY_FALSE_EVENT, 
            dtCore::GameEventActorProperty::SetFuncType(actor, &LogicConditionalActor::SetFalseEvent),
            dtCore::GameEventActorProperty::GetFuncType(actor, &LogicConditionalActor::GetFalseEvent),
            "The event to look for to set this actor to FALSE. Used by a parent logic controller.",
            GROUP));

      }
      /////////////////////////////////////////////////////////////////////////////
      void LogicConditionalActor::SetTrueEvent(dtCore::GameEvent* pEvent)
      {
         mTrueEvent = pEvent;
      }

      /////////////////////////////////////////////////////////////////////////////
      dtCore::GameEvent* LogicConditionalActor::GetTrueEvent()
      {
         return mTrueEvent.get();
      }

      /////////////////////////////////////////////////////////////////////////////
      void LogicConditionalActor::SetFalseEvent(dtCore::GameEvent* pEvent)
      {
         mFalseEvent = pEvent;
      }

      /////////////////////////////////////////////////////////////////////////////
      dtCore::GameEvent* LogicConditionalActor::GetFalseEvent()
      {
         return mFalseEvent.get();
      }

   }
}
