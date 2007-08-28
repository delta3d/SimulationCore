/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#include <prefix/SimCorePrefix-src.h>
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
         if( staticTime != NULL )
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
         DayTimeActor& actor = static_cast<DayTimeActor&>(GetGameActor());

         AddProperty(new dtDAL::IntActorProperty("Time Of Day", "Time Of Day", 
            dtDAL::MakeFunctor(actor, &DayTimeActor::SetTime), 
            dtDAL::MakeFunctorRet(actor, &DayTimeActor::GetTime), 
            "The time of day measured in milliseconds"));

         AddProperty(new dtDAL::IntActorProperty("Prime Meridian Hour Offset", 
            "Prime Meridian Hour Offset", 
            dtDAL::MakeFunctor(actor, &DayTimeActor::SetPrimeMeridianHourOffset), 
            dtDAL::MakeFunctorRet(actor, &DayTimeActor::GetPrimeMeridianHourOffset), 
            "The time zone hour offset from the Prime Meridian"));
      }

   }
}
