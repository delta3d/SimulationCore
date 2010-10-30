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

#ifndef NETDEMO_AIEVENT_H
#define NETDEMO_AIEVENT_H

#include <DemoExport.h>
#include <dtAI/npcevent.h>

namespace NetDemo
{
   class NETDEMO_EXPORT AIEvent: public dtAI::NPCEvent
   {
     DECLARE_ENUM(AIEvent);

   public:
     AIEvent(const std::string& stateName);

     //movement
     static const AIEvent  AI_EVENT_ARRIVED;
     static const AIEvent  AI_EVENT_GO_TO_POINT;
     static const AIEvent  AI_EVENT_FIRE_LASER;
     static const AIEvent  AI_EVENT_ENEMY_TARGETED;
     static const AIEvent  AI_EVENT_TOOK_DAMAGE;
     static const AIEvent  AI_EVENT_DAMAGE_CRITICAL;
     static const AIEvent  AI_EVENT_TARGET_KILLED;
     static const AIEvent  AI_EVENT_NO_TARGET_FOUND;

   protected:
     ~AIEvent();

   private:
     AIEvent(); //not implemented by design

   };
} //namespace NetDemo

#endif //NETDEMO_AIEVENT_H
