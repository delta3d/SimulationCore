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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/OceanDataActor.h>
#include <dtDAL/enginepropertytypes.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      OceanDataActor::OceanDataActor( OceanDataActorProxy &proxy )
         : BaseClass(proxy)
         , mWaveHeightSignificant(0.0f)
         , mWaveDirectionPrimary(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      OceanDataActor::~OceanDataActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanDataActor::SetWaveHeightSignificant( float value )
      {
         mWaveHeightSignificant = value;
      }

      //////////////////////////////////////////////////////////////////////////
      float OceanDataActor::GetWaveHeightSignificant() const
      {
         return mWaveHeightSignificant;
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanDataActor::SetWaveDirectionPrimary( float angleDegrees )
      {
         mWaveDirectionPrimary = angleDegrees;
      }

      //////////////////////////////////////////////////////////////////////////
      float OceanDataActor::GetWaveDirectionPrimary() const
      {
         return mWaveDirectionPrimary;
      }

      

      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString OceanDataActorProxy::PROPERTY_WAVE_DIRECTION_PRIMARY("Wave Direction Primary");
      const dtUtil::RefString OceanDataActorProxy::PROPERTY_WAVE_HEIGHT_SIGNIFICANT("Wave Height Significant");
      
      //////////////////////////////////////////////////////////////////////////
      OceanDataActorProxy::OceanDataActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      OceanDataActorProxy::~OceanDataActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanDataActorProxy::CreateActor()
      {
         SetActor( *new OceanDataActor(*this) );
      }

      //////////////////////////////////////////////////////////
      void OceanDataActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const std::string GROUP("Ocean Data");

         OceanDataActor* actor = NULL;
         GetActor( actor );

         // FLOAT PROPERTIES
         AddProperty(new dtDAL::FloatActorProperty(
            OceanDataActorProxy::PROPERTY_WAVE_HEIGHT_SIGNIFICANT,
            OceanDataActorProxy::PROPERTY_WAVE_HEIGHT_SIGNIFICANT, 
            dtDAL::MakeFunctor( *actor, &OceanDataActor::SetWaveHeightSignificant ), 
            dtDAL::MakeFunctorRet( *actor, &OceanDataActor::GetWaveHeightSignificant ), 
            "Wave height significant for cell[0] at the specified latitude and longitude; this is originally a 2D array property will any number of cells.",
            GROUP));

         AddProperty(new dtDAL::FloatActorProperty(
            OceanDataActorProxy::PROPERTY_WAVE_DIRECTION_PRIMARY,
            OceanDataActorProxy::PROPERTY_WAVE_DIRECTION_PRIMARY, 
            dtDAL::MakeFunctor( *actor, &OceanDataActor::SetWaveDirectionPrimary ), 
            dtDAL::MakeFunctorRet( *actor, &OceanDataActor::GetWaveDirectionPrimary ), 
            "Wave direction (in degrees) for cell[0] at the specified latitude and longitude; this is originally a 2D array property will any number of cells.",
            GROUP));
      }

   }
}
