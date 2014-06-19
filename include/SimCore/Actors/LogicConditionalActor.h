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
#include <dtCore/observerptr.h>
#include <dtCore/actorproxy.h>
#include <osg/Group>


namespace dtCore
{
   class GameEvent;
}


namespace SimCore
{
   namespace Actors
   {
      class LogicConditionalActor;

      //////////////////////////////////////////////////////////////////////////
      // Empty drawable
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LogicConditionalDrawable: public dtCore::DeltaDrawable
      {
         public:
            typedef dtCore::DeltaDrawable BaseClass;

            LogicConditionalDrawable(const std::string& name);

            virtual osg::Node* GetOSGNode();
            virtual const osg::Node* GetOSGNode() const;


         protected:
            virtual ~LogicConditionalDrawable();

         private:
            dtCore::RefPtr<osg::Group> mPlaceholderNode; // Required by delta3d, but we don't need it.
      };



      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LogicConditionalActor : public dtCore::BaseActorObject
      {
         public:
            static const dtUtil::RefString CLASS_NAME;
            static const dtUtil::RefString PROPERTY_TRUE_EVENT;
            static const dtUtil::RefString PROPERTY_FALSE_EVENT;
            static const dtUtil::RefString PROPERTY_IS_TRUE;

            typedef dtCore::BaseActorObject BaseClass;

            LogicConditionalActor();

            virtual void CreateDrawable();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

            void SetTrueEvent( dtCore::GameEvent* gameEvent );
            dtCore::GameEvent* GetTrueEvent();

            /// The False event can be checked to set it true.
            void SetFalseEvent( dtCore::GameEvent* gameEvent );
            dtCore::GameEvent* GetFalseEvent();

            /*
            * When created, IsTrue is a starting condition. Then, it is set
            * when the controlling logic actor receives one of the true/false events
            */
            void SetIsTrue(bool value) { mIsTrue = value; }
            bool GetIsTrue() const {return mIsTrue; }

         protected:
            virtual ~LogicConditionalActor();

         private:
            dtCore::ObserverPtr<dtCore::GameEvent> mTrueEvent;
            dtCore::ObserverPtr<dtCore::GameEvent> mFalseEvent;
            bool mIsTrue;
      };

   }
}

#endif
