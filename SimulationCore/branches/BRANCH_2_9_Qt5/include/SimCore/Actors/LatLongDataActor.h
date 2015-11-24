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
#ifndef LAT_LONG_DATA_ACTOR_H
#define LAT_LONG_DATA_ACTOR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtGame/gameactor.h>



namespace SimCore
{
   namespace Actors
   {
      class LatLongDataActorProxy;

      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LatLongDataActor : public dtGame::GameActor
      {
         public:
            typedef dtGame::GameActor BaseClass;

            LatLongDataActor(LatLongDataActorProxy &proxy);

            void SetLatitude( double latitude );
            double GetLatitude() const;

            void SetLongitude( double longitude );
            double GetLongitude() const;

            void SetWaveHeightSignificant( float value );
            float GetWaveHeightSignificant() const;

         protected:
            virtual ~LatLongDataActor();

         private:
            double mLatitude;
            double mLongitude;
            float mWaveHeightSignificant;
      };



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LatLongDataActorProxy : public dtGame::GameActorProxy
      {
         public:
            static const dtUtil::RefString PROPERTY_LATITUDE;
            static const dtUtil::RefString PROPERTY_LONGITUDE;

            typedef dtGame::GameActorProxy BaseClass;

            LatLongDataActorProxy();

            virtual void CreateDrawable();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

         protected:
            virtual ~LatLongDataActorProxy();

         private:
      };

   }
}

#endif
