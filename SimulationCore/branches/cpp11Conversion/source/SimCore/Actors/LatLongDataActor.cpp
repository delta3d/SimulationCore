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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/LatLongDataActor.h>
#include <dtDAL/enginepropertytypes.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      LatLongDataActor::LatLongDataActor( LatLongDataActorProxy &proxy )
         : BaseClass(proxy)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      LatLongDataActor::~LatLongDataActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void LatLongDataActor::SetLatitude( double latitude )
      {
         mLatitude = latitude;
      }

      //////////////////////////////////////////////////////////////////////////
      double LatLongDataActor::GetLatitude() const
      {
         return mLatitude;
      }

      //////////////////////////////////////////////////////////////////////////
      void LatLongDataActor::SetLongitude( double longitude )
      {
         mLongitude = longitude;
      }

      //////////////////////////////////////////////////////////////////////////
      double LatLongDataActor::GetLongitude() const
      {
         return mLongitude;
      }

      

      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString LatLongDataActorProxy::PROPERTY_LATITUDE("Latitude");
      const dtUtil::RefString LatLongDataActorProxy::PROPERTY_LONGITUDE("Longitude");
      
      //////////////////////////////////////////////////////////////////////////
      LatLongDataActorProxy::LatLongDataActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      LatLongDataActorProxy::~LatLongDataActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void LatLongDataActorProxy::CreateDrawable()
      {
         SetDrawable( *new LatLongDataActor(*this) );
      }

      //////////////////////////////////////////////////////////
      void LatLongDataActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const std::string GROUP("Lat Long Data");

         LatLongDataActor* actor = NULL;
         GetActor( actor );

         // DOUBLE PROPERTIES
         AddProperty(new dtDAL::DoubleActorProperty(
            LatLongDataActorProxy::PROPERTY_LATITUDE,
            LatLongDataActorProxy::PROPERTY_LATITUDE, 
            dtDAL::DoubleActorProperty::SetFuncType(actor, &LatLongDataActor::SetLatitude),
            dtDAL::DoubleActorProperty::GetFuncType(actor, &LatLongDataActor::GetLatitude),
            "Latitude location relevant to the data.",
            GROUP));

         AddProperty(new dtDAL::DoubleActorProperty(
            LatLongDataActorProxy::PROPERTY_LONGITUDE,
            LatLongDataActorProxy::PROPERTY_LONGITUDE, 
            dtDAL::DoubleActorProperty::SetFuncType(actor, &LatLongDataActor::SetLongitude),
            dtDAL::DoubleActorProperty::GetFuncType(actor, &LatLongDataActor::GetLongitude),
            "Longitude location relevant to the data.",
            GROUP));
      }

   }
}
