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
#ifndef LOGIC_CONDITIONAL_ACTOR_H
#define LOGIC_CONDITIONAL_ACTOR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtCore/deltadrawable.h>
#include <dtUtil/refcountedbase.h>
#include <dtDAL/actorproxy.h>
#include <osg/Group>


namespace dtDAL
{
   class GameEvent;
}


namespace SimCore
{
   namespace Actors
   {
      class LogicConditionalActorProxy;

      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE - This actor is a bit odd.  By itself, it doesn't do anything
      // Rather, it is used by a logic actor (such as LogicOnEvent) to perform some 
      // sort of logic when events occur. It has an event that maps to true & false
      // as well as a starting/current IsTrue value.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LogicConditionalActor: public dtCore::DeltaDrawable
      {
         public:
            typedef dtCore::DeltaDrawable BaseClass;

            LogicConditionalActor(const std::string& name);

            virtual osg::Node* GetOSGNode();
            virtual const osg::Node* GetOSGNode() const;

            void SetTrueEvent( dtDAL::GameEvent* gameEvent );
            dtDAL::GameEvent* GetTrueEvent();

            /// The False event can be checked to set it true.
            void SetFalseEvent( dtDAL::GameEvent* gameEvent );
            dtDAL::GameEvent* GetFalseEvent();

            /* 
            * When created, IsTrue is a starting condition. Then, it is set 
            * when the controlling logic actor receives one of the true/false events 
            */
            void SetIsTrue(bool value) { mIsTrue = value; }
            bool GetIsTrue() const {return mIsTrue; }

         protected:
            virtual ~LogicConditionalActor();

         private:
            osg::ref_ptr<osg::Group> mPlaceholderNode; // Required by Delta3D, but we don't need it.
            std::weak_ptr<dtDAL::GameEvent> mTrueEvent;
            std::weak_ptr<dtDAL::GameEvent> mFalseEvent;
            bool mIsTrue;
      };



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LogicConditionalActorProxy : public dtDAL::ActorProxy
      {
         public:
            static const dtUtil::RefString CLASS_NAME;
            static const dtUtil::RefString PROPERTY_TRUE_EVENT;
            static const dtUtil::RefString PROPERTY_FALSE_EVENT;
            static const dtUtil::RefString PROPERTY_IS_TRUE;

            typedef dtDAL::ActorProxy BaseClass;

            LogicConditionalActorProxy();

            virtual void CreateActor();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

            /// Returns a useful reference to our actor. If no actor is created yet, this will likely crash.
            LogicConditionalActor &GetActorAsConditional()
            {
               return *(static_cast<LogicConditionalActor*>(GetDrawable()));
            }


         protected:
            virtual ~LogicConditionalActorProxy();

         private:
      };

   }
}

#endif
