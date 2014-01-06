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

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/mathdefines.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      DayTimeActor::DayTimeActor(dtGame::GameActorProxy &proxy)
         : IGActor(proxy),
         mTime(0),
         mPrimeMeridianHourOffset(0)
      {
         mExpandedTime.tm_sec = 0;
         mExpandedTime.tm_min = 0;
         mExpandedTime.tm_hour = 0;
         mExpandedTime.tm_mday = 0;
         mExpandedTime.tm_wday = 0;
         mExpandedTime.tm_yday = 0;
         mExpandedTime.tm_mon = 0;
         mExpandedTime.tm_year = 0;
         mExpandedTime.tm_isdst = 0;
      }

      //////////////////////////////////////////////////////////
      DayTimeActor::~DayTimeActor()
      {

      }

      //////////////////////////////////////////////////////////
      void DayTimeActor::SetTime( int timeValue )
      {
         mTime = (unsigned int)(timeValue);
         time_t gmtTime = (time_t)mTime;

         // Obtain the individual time components by
         // writing them to the time struct mExpandedTime.
         // WARNING: gmtime returns a staticly allocated struct.
         tm* staticTime = gmtime( &gmtTime ); 
         if( staticTime != nullptr )
         {
            // Copy the data from the static struct to this
            // object's local time struct.
            mExpandedTime = *staticTime;
         }
      }

      //////////////////////////////////////////////////////////
      void DayTimeActor::SetPrimeMeridianHourOffset( int hourOffset )
      {
         dtUtil::Clamp( hourOffset, -23, 23 );
         mPrimeMeridianHourOffset = hourOffset;
      }

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      DayTimeActorProxy::DayTimeActorProxy()
      {
         SetClassName("SimCore::Actors::DayTimeActor");
      }

      //////////////////////////////////////////////////////////
      DayTimeActorProxy::~DayTimeActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void DayTimeActorProxy::BuildPropertyMap()
      {
         DayTimeActor* actor = nullptr;
         GetActor(actor);

         AddProperty(new dtDAL::IntActorProperty("Time Of Day", "Time Of Day", 
            dtDAL::IntActorProperty::SetFuncType(actor, &DayTimeActor::SetTime),
            dtDAL::IntActorProperty::GetFuncType(actor, &DayTimeActor::GetTime),
            "The time of day measured in milliseconds"));

         AddProperty(new dtDAL::IntActorProperty("Prime Meridian Hour Offset", 
            "Prime Meridian Hour Offset", 
            dtDAL::IntActorProperty::SetFuncType(actor, &DayTimeActor::SetPrimeMeridianHourOffset),
            dtDAL::IntActorProperty::GetFuncType(actor, &DayTimeActor::GetPrimeMeridianHourOffset),
            "The time zone hour offset from the Prime Meridian"));
      }

   }
}
