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
#ifndef SURFACE_HAZE_DATA_ACTOR_H
#define SURFACE_HAZE_DATA_ACTOR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <SimCore/Actors/LatLongDataActor.h>



namespace SimCore
{
   namespace Actors
   {
      class SurfaceHazeDataActorProxy;

      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT SurfaceHazeDataActor : public LatLongDataActor
      {
         public:
            typedef LatLongDataActor BaseClass;

            SurfaceHazeDataActor(SurfaceHazeDataActorProxy &proxy);

            void SetExtinctionCoefficient( float value );
            float GetExtinctionCoefficient() const;

         protected:
            virtual ~SurfaceHazeDataActor();

         private:
            float mExtinctionCoefficient;
      };



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT SurfaceHazeDataActorProxy : public LatLongDataActorProxy
      {
         public:
            static const dtUtil::RefString PROPERTY_EXTINCTION_COEFFICIENT;

            typedef LatLongDataActorProxy BaseClass;

            SurfaceHazeDataActorProxy();

            virtual void CreateDrawable();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

         protected:
            virtual ~SurfaceHazeDataActorProxy();

         private:
      };

   }
}

#endif
