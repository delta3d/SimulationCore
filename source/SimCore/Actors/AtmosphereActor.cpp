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
#include <dtGame/gameactor.h>
#include <SimCore/Actors/AtmosphereActor.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(CloudType);
      CloudType CloudType::CIRRUS("CIRRUS"); 
      CloudType CloudType::CIRROCUMULUS("CIRROCUMULUS"); 
      CloudType CloudType::CIRROSTRATUS("CIRROSTRATUS"); 
      CloudType CloudType::ALTOCUMULUS("ALTOCUMULUS");
      CloudType CloudType::ALTOSTRATUS("ALTOSTRATUS");
      CloudType CloudType::NIMBOSTRATUS("NIMBOSTRATUS");
      CloudType CloudType::STRATOCUMULUS("STRATOCUMULUS");
      CloudType CloudType::STRATUS("STRATUS");
      CloudType CloudType::CUMULUS("CUMULUS");
      CloudType CloudType::CUMULONIMBUS("CUMULONIMBUS");
      CloudType CloudType::CLEAR("CLEAR");

      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(PrecipitationType);
      PrecipitationType PrecipitationType::UNKNOWN("UNKNOWN");
      PrecipitationType PrecipitationType::NONE("NONE");
      PrecipitationType PrecipitationType::RAIN("RAIN");
      PrecipitationType PrecipitationType::SNOW("SNOW");
      PrecipitationType PrecipitationType::FREEZING_RAIN("FREEZING RAIN");
      PrecipitationType PrecipitationType::SLEET("SLEET");
      PrecipitationType PrecipitationType::HAIL("HAIL");
      PrecipitationType PrecipitationType::GRAUPEL("GRAUPEL");
      PrecipitationType PrecipitationType::OTHER("OTHER");

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      AtmosphereActorProxy::AtmosphereActorProxy()
      {
         SetClassName("SimCore::Actors::AtmosphereActor");
      }

      //////////////////////////////////////////////////////////
      AtmosphereActorProxy::~AtmosphereActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void AtmosphereActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      AtmosphereActor::AtmosphereActor( dtGame::GameActorProxy &proxy )
         : IGActor(proxy)
      {

      }

      //////////////////////////////////////////////////////////
      AtmosphereActor::~AtmosphereActor()
      {

      }

   }

}
