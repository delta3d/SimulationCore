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
