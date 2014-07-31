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
#include <dtCore/enginepropertytypes.h>

namespace SimCore
{
   namespace Actors
   {
      UniformAtmosphereActor::UniformAtmosphereActor()
      :         mVisibility(100.0f), // km
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
         SetClassName("SimCore::Actors::UniformAtmosphereActor");
      }

      //////////////////////////////////////////////////////////
      UniformAtmosphereActor::~UniformAtmosphereActor()
      {

      }

      //////////////////////////////////////////////////////////
      void UniformAtmosphereActor::BuildPropertyMap()
      {
         GameActorProxy::BuildPropertyMap();

         UniformAtmosphereActor* actor = this;

         AddProperty(new dtCore::FloatActorProperty("Visibility Distance", "Visibility Distance", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetVisibilityDistance),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetVisibilityDistance),
            "Maximum distance one can see into atmospheric haze, measured in kilometers"));

         AddProperty(new dtCore::FloatActorProperty("Cloud Base Height", "Cloud Base Height", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudBaseHeight),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudBaseHeight),
            "Lowest elevation of the primary cloud layer in meters"));

         AddProperty(new dtCore::FloatActorProperty("Cloud Top Height", "Cloud Top Height", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudTopHeight),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudTopHeight),
            "Highest elevation of the primary cloud layer in meters"));

         AddProperty(new dtCore::FloatActorProperty("Cloud Thickness", "Cloud Thickness", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudThickness),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudThickness),
            "Visible distance into the primary cloud layer, measured in meters"));

         AddProperty(new dtCore::FloatActorProperty("Fog Cover", "Fog Cover", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetFogCover),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetFogCover),
            "Percentage of fog density"));

         AddProperty(new dtCore::EnumActorProperty<CloudType>("Cloud Type", "Cloud Type", 
            dtCore::EnumActorProperty<CloudType>::SetFuncType(actor, &UniformAtmosphereActor::SetCloudType),
            dtCore::EnumActorProperty<CloudType>::GetFuncType(actor, &UniformAtmosphereActor::GetCloudType),
            "The cloud pattern being simulated","Environment"));

         AddProperty(new dtCore::FloatActorProperty("Fog Thickness", "Fog Thickness", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetFogThickness),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetFogThickness),
            "Visible distance into the fog layer, measured in meters"));

         AddProperty(new dtCore::FloatActorProperty("Precipitation Rate", "Precipitation Rate", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetPrecipitationRate),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetPrecipitationRate),
            "Rate at which precipitation falls, measured in millimeters per hour"));

         AddProperty(new dtCore::FloatActorProperty("Wind Speed X", "Wind Speed X", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetWindSpeedX),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetWindSpeedX),
            "Wind velocity in the world X axis"));

         AddProperty(new dtCore::FloatActorProperty("Wind Speed Y", "Wind Speed Y", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetWindSpeedY),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetWindSpeedY),
            "Wind velocity in the world Y axis"));

         AddProperty(new dtCore::EnumActorProperty<PrecipitationType>("Precipitation Type", "Precipitation Type", 
            dtCore::EnumActorProperty<PrecipitationType>::SetFuncType(actor, &UniformAtmosphereActor::SetPrecipitationType),
            dtCore::EnumActorProperty<PrecipitationType>::GetFuncType(actor, &UniformAtmosphereActor::GetPrecipitationType),
            "The precipitation pattern being simulated","Environment"));
         
         AddProperty(new dtCore::FloatActorProperty("Extinction Coefficient", "Extinction Coefficient", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetExtinctionCoefficient),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetExtinctionCoefficient),
            "A rate for measuring the loss of clarity in the air"));
        
         AddProperty(new dtCore::FloatActorProperty("Cloud Coverage", "Cloud Coverage", 
            dtCore::FloatActorProperty::SetFuncType(actor, &UniformAtmosphereActor::SetCloudCoverage),
            dtCore::FloatActorProperty::GetFuncType(actor, &UniformAtmosphereActor::GetCloudCoverage),
            "A relative amount of cloud coverage in the sky"));
      }

      void UniformAtmosphereActor::CreateDrawable()
      {
         SetDrawable(*new dtCore::Transformable);
      }

   }

}
