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
         SetHideDTCorePhysicsProps(true);
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
      AtmosphereActor::AtmosphereActor( dtGame::GameActorProxy& owner )
         : IGActor(owner)
      {

      }

      //////////////////////////////////////////////////////////
      AtmosphereActor::~AtmosphereActor()
      {

      }

   }

}
