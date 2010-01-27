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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/LogicConditionalActor.h>
#include <dtDAL/enginepropertytypes.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      LogicConditionalActor::LogicConditionalActor(const std::string& name)
         : BaseClass(name)
         , mPlaceholderNode(new osg::Group)
         , mIsTrue(false)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      LogicConditionalActor::~LogicConditionalActor()
      {
      }

      /////////////////////////////////////////////////////////////////////////////
      osg::Node* LogicConditionalActor::GetOSGNode()
      {
         return mPlaceholderNode.get();
      }

      /////////////////////////////////////////////////////////////////////////////
      const osg::Node* LogicConditionalActor::GetOSGNode() const
      {
         return mPlaceholderNode.get();
      }


      /////////////////////////////////////////////////////////////////////////////
      void LogicConditionalActor::SetTrueEvent(dtDAL::GameEvent* pEvent)
      {
         mTrueEvent = pEvent;
      }

      /////////////////////////////////////////////////////////////////////////////
      dtDAL::GameEvent* LogicConditionalActor::GetTrueEvent() 
      {
         return mTrueEvent.get();
      }

      /////////////////////////////////////////////////////////////////////////////
      void LogicConditionalActor::SetFalseEvent(dtDAL::GameEvent* pEvent)
      {
         mFalseEvent = pEvent;
      }

      /////////////////////////////////////////////////////////////////////////////
      dtDAL::GameEvent* LogicConditionalActor::GetFalseEvent() 
      {
         return mFalseEvent.get();
      }


      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString LogicConditionalActorProxy::PROPERTY_TRUE_EVENT("True Event");
      const dtUtil::RefString LogicConditionalActorProxy::PROPERTY_FALSE_EVENT("False Event");
      const dtUtil::RefString LogicConditionalActorProxy::PROPERTY_IS_TRUE("IsTrue");
      const dtUtil::RefString LogicConditionalActorProxy::CLASS_NAME("SimCore::Logic::LogicConditionalActor");
      
      //////////////////////////////////////////////////////////////////////////
      LogicConditionalActorProxy::LogicConditionalActorProxy()
      {
         SetClassName(LogicConditionalActorProxy::CLASS_NAME);
      }

      //////////////////////////////////////////////////////////////////////////
      LogicConditionalActorProxy::~LogicConditionalActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void LogicConditionalActorProxy::CreateActor()
      {
         SetActor( *new LogicConditionalActor("LogicConditional") );
      }

      //////////////////////////////////////////////////////////
      void LogicConditionalActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const std::string GROUP("Logic Data"); // only used to group props in STAGE

         LogicConditionalActor* actor = NULL;
         GetActor( actor );

         AddProperty(new dtDAL::BooleanActorProperty(PROPERTY_IS_TRUE, PROPERTY_IS_TRUE, 
            dtDAL::BooleanActorProperty::SetFuncType(actor, &LogicConditionalActor::SetIsTrue),
            dtDAL::BooleanActorProperty::GetFuncType(actor, &LogicConditionalActor::GetIsTrue),
            "The initial or current true state. Generally set by a parent logic controller based on the true and false events.",
            GROUP));

         AddProperty(new dtDAL::GameEventActorProperty(*this, PROPERTY_TRUE_EVENT, PROPERTY_TRUE_EVENT, 
            dtDAL::GameEventActorProperty::SetFuncType(actor, &LogicConditionalActor::SetTrueEvent),
            dtDAL::GameEventActorProperty::GetFuncType(actor, &LogicConditionalActor::GetTrueEvent),
            "The event to look for to set this actor to TRUE. Used by a parent logic controller.",
            GROUP));

         AddProperty(new dtDAL::GameEventActorProperty(*this, PROPERTY_FALSE_EVENT, PROPERTY_FALSE_EVENT, 
            dtDAL::GameEventActorProperty::SetFuncType(actor, &LogicConditionalActor::SetFalseEvent),
            dtDAL::GameEventActorProperty::GetFuncType(actor, &LogicConditionalActor::GetFalseEvent),
            "The event to look for to set this actor to FALSE. Used by a parent logic controller.",
            GROUP));

      }

   }
}
