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
#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <dtDAL/enginepropertytypes.h>

namespace SimCore
{
   namespace Actors
   {

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      UniformAtmosphereActorProxy::UniformAtmosphereActorProxy()
      {
         SetClassName("SimCore::Actors::UniformAtmosphereActor");
      }

      //////////////////////////////////////////////////////////
      UniformAtmosphereActorProxy::~UniformAtmosphereActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void UniformAtmosphereActorProxy::BuildPropertyMap()
      {
         AtmosphereActorProxy::BuildPropertyMap();

         UniformAtmosphereActor* actor = NULL;
         GetActor(actor);

         AddProperty(new dtDAL::FloatActorProperty("Visibility Distance", "Visibility Distance", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetVisibilityDistance),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetVisibilityDistance),
            "Maximum distance one can see into atmospheric haze, measured in kilometers"));

         AddProperty(new dtDAL::FloatActorProperty("Cloud Base Height", "Cloud Base Height", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudBaseHeight),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudBaseHeight),
            "Lowest elevation of the primary cloud layer in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Cloud Top Height", "Cloud Top Height", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudTopHeight),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudTopHeight),
            "Highest elevation of the primary cloud layer in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Cloud Thickness", "Cloud Thickness", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudThickness),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudThickness),
            "Visible distance into the primary cloud layer, measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Fog Cover", "Fog Cover", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetFogCover),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetFogCover),
            "Percentage of fog density"));

         AddProperty(new dtDAL::EnumActorProperty<CloudType>("Cloud Type", "Cloud Type", 
            dtDAL::EnumActorProperty<CloudType>::SetFuncType(actor, &UniformAtmosphereActor::SetCloudType),
            dtDAL::EnumActorProperty<CloudType>::GetFuncType(actor, &UniformAtmosphereActor::GetCloudType),
            "The cloud pattern being simulated","Environment"));

         AddProperty(new dtDAL::FloatActorProperty("Fog Thickness", "Fog Thickness", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetFogThickness),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetFogThickness),
            "Visible distance into the fog layer, measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Precipitation Rate", "Precipitation Rate", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetPrecipitationRate),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetPrecipitationRate),
            "Rate at which precipitation falls, measured in millimeters per hour"));

         AddProperty(new dtDAL::FloatActorProperty("Wind Speed X", "Wind Speed X", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetWindSpeedX),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetWindSpeedX),
            "Wind velocity in the world X axis"));

         AddProperty(new dtDAL::FloatActorProperty("Wind Speed Y", "Wind Speed Y", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetWindSpeedY),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetWindSpeedY),
            "Wind velocity in the world Y axis"));

         AddProperty(new dtDAL::EnumActorProperty<PrecipitationType>("Precipitation Type", "Precipitation Type", 
            dtDAL::EnumActorProperty<PrecipitationType>::SetFuncType(actor, &UniformAtmosphereActor::SetPrecipitationType),
            dtDAL::EnumActorProperty<PrecipitationType>::GetFuncType(actor, &UniformAtmosphereActor::GetPrecipitationType),
            "The precipitation pattern being simulated","Environment"));
         
         AddProperty(new dtDAL::FloatActorProperty("Extinction Coefficient", "Extinction Coefficient", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetExtinctionCoefficient),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetExtinctionCoefficient),
            "A rate for measuring the loss of clarity in the air"));
        
         AddProperty(new dtDAL::FloatActorProperty("Cloud Coverage", "Cloud Coverage", 
            dtDAL::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudCoverage),
            dtDAL::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudCoverage),
            "A relative amount of cloud coverage in the sky"));
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      UniformAtmosphereActor::UniformAtmosphereActor( dtGame::GameActorProxy &proxy )
         : AtmosphereActor(proxy),
         mVisibility(100.0f), // km
         mCloudBaseHeight(0.0f), // m
         mCloudTopHeight(0.0f), // m
         mCloudThickness(0.0f), // m
         mFogCover(0.0f), // %
         mFogThickness(0.0f), // m
         mPrecipRate(0.0f), // mm/h
         mExtinctionCoefficient(0.0f),
         mCloudCoverage(0.0f),
         mCloudType(&CloudType::CLEAR),
         mPrecipType(&PrecipitationType::NONE)

      {

      }

      //////////////////////////////////////////////////////////
      UniformAtmosphereActor::~UniformAtmosphereActor()
      {

      }

   }

}
