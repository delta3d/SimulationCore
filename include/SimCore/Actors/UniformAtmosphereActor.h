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
#ifndef _UNIFORM_ATMOSPHERE_ACTOR_H_
#define _UNIFORM_ATMOSPHERE_ACTOR_H_

#include <SimCore/Export.h>
#include <SimCore/Actors/AtmosphereActor.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT UniformAtmosphereActor : public AtmosphereActor
      {
      public:

         /// Constructor
         UniformAtmosphereActor(dtGame::GameActorProxy &proxy);

         void SetVisibilityDistance( float distance ) { mVisibility = distance; }
         float GetVisibilityDistance() const { return mVisibility; }

         void SetCloudType( CloudType& type ) { mCloudType = &type; }
         CloudType& GetCloudType() const { return *mCloudType; }

         void SetCloudBaseHeight( float height ) { mCloudBaseHeight = height; }
         float GetCloudBaseHeight() const { return mCloudBaseHeight; }

         void SetCloudTopHeight( float height ) { mCloudTopHeight = height; }
         float GetCloudTopHeight() const { return mCloudTopHeight; }

         void SetCloudThickness( float thickness ) { mCloudThickness = thickness; }
         float GetCloudThickness() const { return mCloudThickness; }

         void SetFogCover( float cover ) { mFogCover = cover; }
         float GetFogCover() const { return mFogCover; }

         void SetFogThickness( float thickness ) { mFogThickness = thickness; }
         float GetFogThickness() const { return mFogThickness; }

         void SetPrecipitationType( PrecipitationType& type ) { mPrecipType = &type; }
         PrecipitationType& GetPrecipitationType() const { return *mPrecipType; }

         void SetPrecipitationRate( float rate ) { mPrecipRate = rate; }
         float GetPrecipitationRate() const { return mPrecipRate; }

         void SetExtinctionCoefficient( float extinction ) { mExtinctionCoefficient = extinction; }
         float GetExtinctionCoefficient() const { return mExtinctionCoefficient; }

         void SetWindSpeedX( float speedX ) { mWindSpeed[0] = speedX; }
         float GetWindSpeedX() const { return mWindSpeed.x(); }

         void SetWindSpeedY( float speedY ) { mWindSpeed[1] = speedY; }
         float GetWindSpeedY() const { return mWindSpeed.y(); }

         void SetWind( const osg::Vec2& speed ) { mWindSpeed = speed; }
         const osg::Vec2& GetWind() const { return mWindSpeed; }

      protected:

         /// Destructor
         virtual ~UniformAtmosphereActor();

      private:

         // Note: All numeric values are assumed to be metric

         float mVisibility;      // km
         float mCloudBaseHeight; // m
         float mCloudTopHeight;  // m
         float mCloudThickness;  // m
         float mFogCover;        // %
         float mFogThickness;    // m
         float mPrecipRate;      // mm/h
         float mExtinctionCoefficient; // 1/KM

         CloudType* mCloudType;
         PrecipitationType* mPrecipType;

         osg::Vec2 mWindSpeed;   // m/s

      };

      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT UniformAtmosphereActorProxy : public AtmosphereActorProxy
      {
      public:

         /// Constructor
         UniformAtmosphereActorProxy();

         /// Adds the properties associated with this actor
         virtual void BuildPropertyMap();

         /// Creates the actor
         void CreateActor() { SetActor(*new UniformAtmosphereActor(*this)); }

      protected:

         /// Destructor
         virtual ~UniformAtmosphereActorProxy();

      private:

      };
   }
}

#endif
