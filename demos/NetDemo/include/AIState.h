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

namespace NetDemo
{

   class NETDEMO_EXPORT AIState : public dtUtil::Enumeration
   {
      DECLARE_ENUM(AIState);

   public:
     AIState(const std::string& stateName);

     //movement
     static const AIState  AI_STATE_SPAWN;
     static const AIState  AI_STATE_GO_TO_WAYPOINT;
     static const AIState  AI_STATE_FOLLOW_PATH;
     static const AIState  AI_STATE_ATTACK;
     static const AIState  AI_STATE_EVADE;
     static const AIState  AI_STATE_FOLLOW;
     static const AIState  AI_STATE_FLOCK;

   protected:
     ~AIState();

   private:
     AIState(); //not implemented by design

   };

   template <class T>
   struct StateData: public dtAI::NPCState
   {
   public:
      typedef dtAI::NPCState BaseClass;

      //at the moment the ObjectFactory must use default constructors :(
      StateData(): BaseClass(){}
     
      //void Init(const std::string& message);
         
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

   typedef StateData<WaypointData> WaypointStateType;

} //namespace NetDemo


#endif //NETDEMO_AISTATE_H
