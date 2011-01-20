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

#include <AIEvent.h>


namespace NetDemo
{
   //////////////////////////////////////////////////////////////////////////

   IMPLEMENT_ENUM(AIEvent);

   const AIEvent AIEvent::AI_EVENT_ARRIVED("AI_EVENT_ARRIVED");
   const AIEvent AIEvent::AI_EVENT_GO_TO_POINT("AI_EVENT_GO_TO_POINT");
   const AIEvent AIEvent::AI_EVENT_FIRE_LASER("AI_EVENT_FIRE_LASER");
   const AIEvent AIEvent::AI_EVENT_ENEMY_TARGETED("AI_EVENT_ENEMY_TARGETED");
   const AIEvent AIEvent::AI_EVENT_TOOK_DAMAGE("AI_EVENT_TOOK_DAMAGE");
   const AIEvent AIEvent::AI_EVENT_DAMAGE_CRITICAL("AI_EVENT_DAMAGE_CRITICAL");
   const AIEvent AIEvent::AI_EVENT_TARGET_KILLED("AI_EVENT_TARGET_KILLED");
   const AIEvent AIEvent::AI_EVENT_NO_TARGET_FOUND("AI_EVENT_NO_TARGET_FOUND");

   AIEvent::AIEvent(const std::string& pName): dtAI::NPCEvent(pName)
   {
   }

   AIEvent::~AIEvent()
   {
   }

}//namespace NetDemo
