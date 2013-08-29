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

            FlareActorProxy();

            virtual void BuildPropertyMap();

            virtual void CreateDrawable();

         protected:
            virtual ~FlareActorProxy();

         private:
      };

   }
}


#endif
