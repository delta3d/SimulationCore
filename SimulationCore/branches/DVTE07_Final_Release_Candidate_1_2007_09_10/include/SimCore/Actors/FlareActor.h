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

#ifndef _FLARE_ACTOR_H_
#define _FLARE_ACTOR_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Actors/BaseEntity.h>



namespace SimCore
{
   namespace Actors
   {
      class FlareActorProxy;

      //////////////////////////////////////////////////////////////////////////
      // FLARE ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT FlareActor : public BaseEntity
      {
         public:
            static const std::string& PROPERTY_CELL;
            static const std::string& PROPERTY_GUISE;
            static const std::string& PROPERTY_NUMBER_OF_SOURCES;
            static const std::string& PROPERTY_TIME_SINCE_DETONATION;
            static const std::string& PROPERTY_HEIGHT;
            static const std::string& PROPERTY_HEIGHT_DELTA;
            static const std::string& PROPERTY_PEAK_ANGLE;
            static const std::string& PROPERTY_PEAK_ANGLE_DELTA;
            static const std::string& PROPERTY_SOURCE_INTENSITY;
            static const std::string& PROPERTY_MODEL_TYPE;
            static const std::string& PROPERTY_LIGHT_NAME;
            static const std::string& PROPERTY_PARTICLE_FILE;

            FlareActor( FlareActorProxy& proxy );

            void SetCell( int cell );
            int GetCell() const;

            void SetGuise( int guise );
            int GetGuise() const;

            void SetNumberOfSources( int numberSources );
            int GetNumberOfSources() const;

            void SetTimeSinceDetonation( int timeSinceDetonation );
            int GetTimeSinceDetonation() const; // FOM declares this as unsigned

            void SetHeight( float height );
            float GetHeight() const;

            void SetHeightDelta( float heightDelta );
            float GetHeightDelta() const; // ? is this the correct name; never changed from federate

            void SetPeakAngle( float angle );
            float GetPeakAngle()const;

            void SetPeakAngleDelta( float angleDelta );
            float GetPeakAngleDelta()const; // ? is this the correct name; never changed from federate

            void SetSourceIntensity( float intensity );
            float GetSourceIntensity() const;

            void SetModelType( const std::string& modelType );
            const std::string& GetModelType() const; // is one byte incoming, use enum
            std::string GetModelTypeString() const; // is one byte incoming, use enum

            void SetParticleFileName( const std::string& fileName );
            std::string GetParticleFileName() const;

            void SetLightName( const std::string& fileName );
            std::string GetLightName() const;

            void LoadParticlesFile( const std::string& fileName );

            virtual void HandleModelDrawToggle(bool draw);

            virtual void OnEnteredWorld();

         protected:
            virtual ~FlareActor();

         private:
            int mCell;
            int mGuise;
            int mNumberSources;
            int mTimeSinceDetonation; // ms // FOM declares this as unsigned
            float mHeight;
            float mHeightDelta;
            float mPeakAngle;
            float mPeakAngleDelta;
            float mSourceIntensity;
            std::string mModelType;
            std::string mParticleFileName;
            std::string mLightName;
            dtCore::RefPtr<dtCore::ParticleSystem> mParticles;
      };



      //////////////////////////////////////////////////////////////////////////
      // FLARE ACTOR PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT FlareActorProxy : public BaseEntityActorProxy
      {
         public:
            FlareActorProxy();

            virtual void BuildPropertyMap();

            virtual void CreateActor();

         protected:
            virtual ~FlareActorProxy();

         private:
      };

   }
}


#endif
