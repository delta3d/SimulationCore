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
#ifndef _ATMOSPHERE_ACTOR_H_
#define _ATMOSPHERE_ACTOR_H_

#include <SimCore/Actors/IGActor.h>
#include <dtUtil/enumeration.h>

namespace SimCore
{
   namespace Actors
   {
      /**
      * Type of Cloud Cover - identifies the type of cloud cover being simulated.
      */
      class SIMCORE_EXPORT CloudType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(CloudType);
      public:
         static CloudType CIRRUS; 
         static CloudType CIRROCUMULUS; 
         static CloudType CIRROSTRATUS; 
         static CloudType ALTOCUMULUS;
         static CloudType ALTOSTRATUS;
         static CloudType NIMBOSTRATUS;
         static CloudType STRATOCUMULUS;
         static CloudType STRATUS;
         static CloudType CUMULUS;
         static CloudType CUMULONIMBUS;
         static CloudType CLEAR;

      private:
         CloudType(const std::string &name) : dtUtil::Enumeration(name)
         {
            AddInstance(this);
         }
      };

      /**
      * Type of Precipitation - identifies the precipitation type being simulated.
      */

      class SIMCORE_EXPORT PrecipitationType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(PrecipitationType);
      public:
         static PrecipitationType UNKNOWN;
         static PrecipitationType NONE;
         static PrecipitationType RAIN;
         static PrecipitationType SNOW;
         static PrecipitationType FREEZING_RAIN;
         static PrecipitationType SLEET;
         static PrecipitationType HAIL;
         static PrecipitationType GRAUPEL;
         static PrecipitationType OTHER;

      private:
         PrecipitationType(const std::string &name) : dtUtil::Enumeration(name)
         {
            AddInstance(this);
         }
      };


      class SIMCORE_EXPORT AtmosphereActor : public IGActor
      {
      public:

         /// Constructor
         AtmosphereActor(dtGame::GameActorProxy &proxy);

      protected:

         /// Destructor
         virtual ~AtmosphereActor();

      private:

      };

      class SIMCORE_EXPORT AtmosphereActorProxy : public dtGame::GameActorProxy
      {
      public:

         /// Constructor
         AtmosphereActorProxy();

         /// Creates the actor
         void CreateActor() { SetActor(*new AtmosphereActor(*this)); }

         /// Adds the properties associated with this actor
         virtual void BuildPropertyMap();

      protected:

         /// Destructor
         virtual ~AtmosphereActorProxy();

      private:

      };
   }
}

#endif
