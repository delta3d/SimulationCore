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

#include <AIState.h>

namespace NetDemo
{

   //////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(AIStateType);

   const AIStateType AIStateType::AI_STATE_SPAWN("AI_STATE_SPAWN");
   const AIStateType AIStateType::AI_STATE_DIE("AI_STATE_DIE");
   const AIStateType AIStateType::AI_STATE_IDLE("AI_STATE_IDLE");
   const AIStateType AIStateType::AI_STATE_FIND_TARGET("AI_STATE_FIND_TARGET");
   const AIStateType AIStateType::AI_STATE_GO_TO_WAYPOINT("AI_STATE_GO_TO_WAYPOINT");
   const AIStateType AIStateType::AI_STATE_ATTACK("AI_STATE_ATTACK");
   const AIStateType AIStateType::AI_STATE_FIRE_LASER("AI_STATE_FIRE_LASER");
   const AIStateType AIStateType::AI_STATE_EVADE("AI_STATE_EVADE");
   const AIStateType AIStateType::AI_STATE_FOLLOW("AI_STATE_FOLLOW");
   const AIStateType AIStateType::AI_STATE_FLOCK("AI_STATE_FLOCK");
   const AIStateType AIStateType::AI_STATE_WANDER("AI_STATE_WANDER");
   const AIStateType AIStateType::AI_STATE_DETONATE("AI_STATE_DETONATE");


   AIStateType::AIStateType(const std::string& pName): dtUtil::Enumeration(pName)
   {
   }

   AIStateType::~AIStateType()
   {
   }

} //namespace NetDemo
