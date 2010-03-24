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

         UniformAtmosphereActor& actor = static_cast<UniformAtmosphereActor&>(GetGameActor());

         AddProperty(new dtDAL::FloatActorProperty("Visibility Distance", "Visibility Distance", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetVisibilityDistance), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetVisibilityDistance), 
            "Maximum distance one can see into atmospheric haze, measured in kilometers"));

         AddProperty(new dtDAL::FloatActorProperty("Cloud Base Height", "Cloud Base Height", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetCloudBaseHeight), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetCloudBaseHeight), 
            "Lowest elevation of the primary cloud layer in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Cloud Top Height", "Cloud Top Height", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetCloudTopHeight), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetCloudTopHeight), 
            "Highest elevation of the primary cloud layer in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Cloud Thickness", "Cloud Thickness", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetCloudThickness), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetCloudThickness), 
            "Visible distance into the primary cloud layer, measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Fog Cover", "Fog Cover", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetFogCover), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetFogCover), 
            "Percentage of fog density"));

         AddProperty(new dtDAL::EnumActorProperty<CloudType>("Cloud Type", "Cloud Type", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetCloudType), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetCloudType), 
            "The cloud pattern being simulated","Environment"));

         AddProperty(new dtDAL::FloatActorProperty("Fog Thickness", "Fog Thickness", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetFogThickness), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetFogThickness), 
            "Visible distance into the fog layer, measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Precipitation Rate", "Precipitation Rate", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetPrecipitationRate), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetPrecipitationRate), 
            "Rate at which precipitation falls, measured in millimeters per hour"));

         AddProperty(new dtDAL::FloatActorProperty("Wind Speed X", "Wind Speed X", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetWindSpeedX), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetWindSpeedX), 
            "Wind velocity in the world X axis"));

         AddProperty(new dtDAL::FloatActorProperty("Wind Speed Y", "Wind Speed Y", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetWindSpeedY), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetWindSpeedY), 
            "Wind velocity in the world Y axis"));

         AddProperty(new dtDAL::EnumActorProperty<PrecipitationType>("Precipitation Type", "Precipitation Type", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetPrecipitationType), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetPrecipitationType), 
            "The precipitation pattern being simulated","Environment"));
         
         AddProperty(new dtDAL::FloatActorProperty("Extinction Coefficient", "Extinction Coefficient", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetExtinctionCoefficient), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetExtinctionCoefficient), 
            "A rate for measuring the loss of clarity in the air"));
        
         AddProperty(new dtDAL::FloatActorProperty("Cloud Coverage", "Cloud Coverage", 
            dtDAL::MakeFunctor(actor, &UniformAtmosphereActor::SetCloudCoverage), 
            dtDAL::MakeFunctorRet(actor, &UniformAtmosphereActor::GetCloudCoverage), 
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
