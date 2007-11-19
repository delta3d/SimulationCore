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
