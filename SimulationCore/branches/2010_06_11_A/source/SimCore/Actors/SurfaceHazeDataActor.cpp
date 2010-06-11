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
#include <SimCore/Actors/SurfaceHazeDataActor.h>
#include <dtDAL/enginepropertytypes.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      SurfaceHazeDataActor::SurfaceHazeDataActor( SurfaceHazeDataActorProxy &proxy )
         : BaseClass(proxy)
         , mExtinctionCoefficient(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SurfaceHazeDataActor::~SurfaceHazeDataActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void SurfaceHazeDataActor::SetExtinctionCoefficient( float value )
      {
         mExtinctionCoefficient = value;
      }

      //////////////////////////////////////////////////////////////////////////
      float SurfaceHazeDataActor::GetExtinctionCoefficient() const
      {
         return mExtinctionCoefficient;
      }

      

      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString SurfaceHazeDataActorProxy::PROPERTY_EXTINCTION_COEFFICIENT("Extinction Coefficient");
      
      //////////////////////////////////////////////////////////////////////////
      SurfaceHazeDataActorProxy::SurfaceHazeDataActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SurfaceHazeDataActorProxy::~SurfaceHazeDataActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void SurfaceHazeDataActorProxy::CreateActor()
      {
         SetActor( *new SurfaceHazeDataActor(*this) );
      }

      //////////////////////////////////////////////////////////
      void SurfaceHazeDataActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const std::string GROUP("Surface Haze Data");

         SurfaceHazeDataActor* actor = NULL;
         GetActor( actor );

         // FLOAT PROPERTIES
         AddProperty(new dtDAL::FloatActorProperty(
            SurfaceHazeDataActorProxy::PROPERTY_EXTINCTION_COEFFICIENT,
            SurfaceHazeDataActorProxy::PROPERTY_EXTINCTION_COEFFICIENT, 
            dtDAL::FloatActorProperty::SetFuncType(actor, &SurfaceHazeDataActor::SetExtinctionCoefficient),
            dtDAL::FloatActorProperty::GetFuncType(actor, &SurfaceHazeDataActor::GetExtinctionCoefficient),
            "Visibility extinction coefficient for cell[0] at the specified latitude and longitude; this is originally a 2D array property will any number of cells.",
            GROUP));
      }

   }
}
