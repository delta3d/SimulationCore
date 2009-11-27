/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @author Bradley Anderegg
*/

#ifndef NETDEMO_AISTATE_H
#define NETDEMO_AISTATE_H

#include <DemoExport.h>
#include <dtUtil/enumeration.h>
#include <dtAI/npcstate.h>
#include <osg/Vec3>
#include <dtCore/transformable.h>
#include <dtCore/observerptr.h>

namespace NetDemo
{

   class NETDEMO_EXPORT AIStateType : public dtAI::NPCState::Type
   {
      DECLARE_ENUM(AIStateType);

   public:
     AIStateType(const std::string& stateName);

     //movement
     static const AIStateType  AI_STATE_SPAWN;
     static const AIStateType  AI_STATE_DIE;
     static const AIStateType  AI_STATE_IDLE;
     static const AIStateType  AI_STATE_FIND_TARGET;
     static const AIStateType  AI_STATE_GO_TO_WAYPOINT;
     static const AIStateType  AI_STATE_ATTACK;
     static const AIStateType  AI_STATE_FIRE_LASER;
     static const AIStateType  AI_STATE_EVADE;
     static const AIStateType  AI_STATE_FOLLOW;
     static const AIStateType  AI_STATE_FLOCK;
     static const AIStateType  AI_STATE_WANDER;
     static const AIStateType  AI_STATE_DETONATE;

   protected:
     ~AIStateType();

   private:
     AIStateType(); //not implemented by design

   };

   template <class T>
   class AIState: public dtAI::NPCState
   {
   public:
      typedef dtAI::NPCState BaseClass;

      //at the moment the ObjectFactory must use default constructors :(
      AIState(): BaseClass(){}

      //should we have these? I can't decide!
      //operator T&() {return mStateData;}
      //operator const T&() const {return mStateData;}

      T mStateData;
   };


   struct WaypointData
   {
     void operator=(const osg::Vec3& v)
     {
       mCurrentWaypoint = v;
     }

     osg::Vec3 mCurrentWaypoint;
   };

   typedef AIState<WaypointData> GoToWaypointState;


   struct TargetData
   {
      void operator=(dtCore::Transformable& v)
      {
         mTarget = &v;
      }

      osg::Vec3 mLastPos;
      dtCore::ObserverPtr<const dtCore::Transformable> mTarget;
   };

   typedef AIState<TargetData> AttackState;


} //namespace NetDemo


#endif //NETDEMO_AISTATE_H
