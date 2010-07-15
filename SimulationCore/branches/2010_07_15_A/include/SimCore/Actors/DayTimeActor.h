/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
 * @author Chris Rodgers
 */
#ifndef _DAYTIME_ACTOR_H_
#define _DAYTIME_ACTOR_H_

#include <SimCore/Actors/IGActor.h>
#include <dtGame/gameactor.h>
#include <ctime>

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT DayTimeActor : public IGActor
      {
      public:

         /// Constructor
         DayTimeActor(dtGame::GameActorProxy &proxy);

         // The following accessor and mutator are used
         // to satisfy the SIGNED IntActorProperty in
         // the BuildActorProperty function call.
         void SetTime( int timeValue );
         int GetTime() const { return (signed int) mTime; }

         int GetSecond() const { return mExpandedTime.tm_sec; }
         int GetMinute() const { return mExpandedTime.tm_min; }
         int GetHour() const { return (24+mExpandedTime.tm_hour)%24; }
         int GetDay() const { return mExpandedTime.tm_mday; }
         int GetMonth() const { return mExpandedTime.tm_mon; }
         int GetYear() const { return mExpandedTime.tm_year; }

         void SetSecond(int sec){ mExpandedTime.tm_sec = sec; }
         void SetMinute(int min){ mExpandedTime.tm_min = min; }
         void SetHour(int hour){mExpandedTime.tm_hour = hour; }
         void SetDay(int day){ mExpandedTime.tm_mday = day; }
         void SetMonth(int month){ mExpandedTime.tm_mon = month; }
         void SetYear(int year){ mExpandedTime.tm_year = year; }

         void SetPrimeMeridianHourOffset( int hourOffset );
         int GetPrimeMeridianHourOffset() const { return mPrimeMeridianHourOffset; }

      protected:

         /// Destructor
         virtual ~DayTimeActor();

      private:

         // The time of day captured as milliseconds
         unsigned int mTime;
         struct tm mExpandedTime;
         int mPrimeMeridianHourOffset;

      };

      class SIMCORE_EXPORT DayTimeActorProxy : public dtGame::GameActorProxy
      {
      public:

         /// Constructor
         DayTimeActorProxy();

         /// Creates the actor
         void CreateActor() { SetActor(*new DayTimeActor(*this)); }

         /// Adds the properties associated with this actor
         void BuildPropertyMap();

      protected:

         /// Destructor
         virtual ~DayTimeActorProxy();

      private:

      };
   }
}

#endif
