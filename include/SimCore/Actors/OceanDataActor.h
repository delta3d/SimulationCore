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
#ifndef OCEAN_DATA_ACTOR_H
#define OCEAN_DATA_ACTOR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <SimCore/Actors/LatLongDataActor.h>



namespace SimCore
{
   namespace Actors
   {
      class OceanDataActorProxy;

      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT OceanDataActor : public LatLongDataActor
      {
         public:
            typedef LatLongDataActor BaseClass;

            OceanDataActor(OceanDataActorProxy &proxy);

            void SetWaveHeightSignificant( float value );
            float GetWaveHeightSignificant() const;

         protected:
            virtual ~OceanDataActor();

         private:
            float mWaveHeightSignificant;
      };



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT OceanDataActorProxy : public LatLongDataActorProxy
      {
         public:
            static const dtUtil::RefString PROPERTY_WAVE_HEIGHT_SIGNIFICANT;

            typedef LatLongDataActorProxy BaseClass;

            OceanDataActorProxy();

            virtual void CreateActor();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

         protected:
            virtual ~OceanDataActorProxy();

         private:
      };

   }
}

#endif
