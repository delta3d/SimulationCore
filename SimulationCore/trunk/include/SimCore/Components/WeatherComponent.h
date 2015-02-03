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
 * @author Eddie Johnson
 */

#ifndef _WEATHER_COMPONENT_H_
#define _WEATHER_COMPONENT_H_

#include <string>
#include <SimCore/Export.h>
#include <osg/Vec3>
#include <dtCore/refptr.h>
#include <dtGame/gmcomponent.h>
#include <dtABC/weather.h>
#include <SimCore/Actors/IGEnvironmentActor.h>
#include <osgParticle/PrecipitationEffect>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/AtmosphereData.h>

namespace osg
{
   class Node;
}

namespace dtActors
{
   class BasicEnvironmentActorProxy;
}

namespace dtCore
{
   class UniqueId;
   class Environment;
}

namespace dtGame
{
   class Message;
}

namespace dtAudio
{
   class Sound;
}

namespace SimCore
{
   namespace Actors
   {
      class UniformAtmosphereActor;
      class DayTimeActorProxy;
   }

   namespace Components
   {
      /**
       * @class WeatherComponent
       * @brief component used to handle all weather changes in the application.
       */
      class SIMCORE_EXPORT WeatherComponent : public dtGame::GMComponent
      {
      public:
         typedef dtGame::GMComponent BaseClass;
         static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;
         static const std::string DEFAULT_NAME;

         static const dtUtil::RefString THUNDER_TIMER_NAME;

         // Constructor
         // @param name The name by which this component is called from the GameManager
         WeatherComponent( dtCore::SystemComponentType& type = *TYPE );

         void Clear();

         // Re-initialize
         void Reset();

         //creates our day time actor
         /*virtual*/ void OnAddedToGM();

         // Set the main environmental actor responsible for all environment rendering
         void SetEphemerisEnvironment( SimCore::Actors::IGEnvironmentActor* env );

         SimCore::Actors::IGEnvironmentActor* GetEphemerisEnvironment();
         const SimCore::Actors::IGEnvironmentActor* GetEphemerisEnvironment() const;

         // Set & Get the average elevation from which visibility fog should be calculated.
         void SetBaseElevation( double elevation );
         double GetBaseElevation() const;

         // Set the limit at which all fog must be solid, to hide ends of terrain.
         // @param maxVisibility Visibility measured in meters
         void SetMaxVisibility( double maxVisibility );
         // @return the max visibility measured in meters
         double GetMaxVisibility() const;

         // Set the limit at which all fog must be solid when looking straight down at the terrain
         // @param maxVisibility Visibility measured in meters
         void SetMaxElevationVisibility( double maxVisibility );
         // @return the max visibility measured in meters
         double GetMaxElevationVisibility() const;

         // Handle messages passed to this component
         virtual void ProcessMessage( const dtGame::Message& msg );

         /**
          * This will effect how far out the precipitation will start
          * This can be used to keep the precipitation out of the vehicle
          */
         void SetPrecipStart(float start);
         float GetPrecipStart() const;

         // Set the near clipping plane distance used in the application.
         // The camera will be updated on the next call to UpdateWeather.
         // @param nearClip The near clipping plane distance
         // NOTE: This component will not assume use of an
         // external constant; thus, this component shall not
         // limit the app on near clipping plane distance and
         // can be dynamic if required by the application.
         void SetNearClipPlane( float nearClip ) { mNearClipPlane = nearClip; }
         float GetNearClipPlane() const { return mNearClipPlane; }

         // Set the far clipping plane distance used in the application.
         // The camera will be updated on the next call to UpdateWeather.
         // @param farClip The far clipping plane distance
         void SetFarClipPlane( float farClip ) { mFarClipPlane = farClip; }
         float GetFarClipPlane() const { return mFarClipPlane; }

         // Set the near and far clipping plane distances used in the application.
         // The camera will be updated on the next call to UpdateWeather.
         // @param nearClip The near clipping plane distance
         // @param farClip The far clipping plane distance
         void SetNearFarClipPlanes( float nearClip, float farClip )
         {
            mNearClipPlane = nearClip;
            mFarClipPlane = farClip;
         }

         // Enable or disable the component's ability to modify the clipping planes.
         void SetAdjustClipPlanes( bool allowAjust ) { mAllowClipAjust = allowAjust; }

         // The following functions are used for sub-classes
         // and unit tests.
         Actors::DayTimeActorProxy* GetDayTimeActor() { return mDayTime.get(); }
         const Actors::DayTimeActorProxy* GetDayTimeActor() const { return mDayTime.get(); }

         Actors::UniformAtmosphereActor* GetAtmosphereActor() { return mAtmosphere.get(); }
         const Actors::UniformAtmosphereActor* GetAtmosphereActor() const { return mAtmosphere.get(); }

         void SetUpdatesEnabled(bool enable) { mUpdatesEnabled = enable; }
         bool GetUpdatesEnabled() const      { return mUpdatesEnabled;   }

         // Updates the fog based on the recently set view elevation value.
         // NOTE: This function assumes the GameManager is accessible.
         void UpdateFog();

         void UpdateDayTime();

         // Update the weather info based on the nearest weather objects to the user.
         void UpdateWeather();

         //Toggel Fog
         void ToggleFog();

         /***
          *  The thunder sounds are selected and played randomly,
          *     it is possible to add as many sounds as you would like.
          *
          */
         void AddThunderSound(dtAudio::Sound& sound);
         bool RemoveThunderSound(dtAudio::Sound& sound);

         const std::vector< dtCore::RefPtr<dtAudio::Sound> >& GetThunderSounds() const;
         
         ///This sets the min and max range in seconds for playing random thunder sounds
         void SetRandomThunderRangeInSeconds(float from, float to);
         void GetRandomThunderRangeInSeconds(float& from, float& to) const;
        
         /***
          *  The rain sounds are selected based on the speed of the rain
          *     with the 0 element being a gentle shower and the last element being
          *     the heaviest.  It will divide the precipitation range by the number
          *     of elements so the number of rain sounds can be variable.
          */
         void AddRainSound(dtAudio::Sound& sound);
         bool RemoveRainSound(dtAudio::Sound& sound);

         const std::vector< dtCore::RefPtr<dtAudio::Sound> >& GetRainSounds() const;        

      protected:

         // Destructor
         virtual ~WeatherComponent();

         dtABC::Weather::CloudType ClassifyClouds( const Actors::UniformAtmosphereActor& atmos );
         dtABC::Weather::WindType ClassifyWind( const Actors::UniformAtmosphereActor& atmos );
         dtABC::Weather::VisibilityType ClassifyVisibility( const Actors::UniformAtmosphereActor& atmos );
         dtABC::Weather::TimePeriod ClassifyTimePeriod( unsigned int time_msec );
         dtABC::Weather::Season ClassifySeason( unsigned int time_msec );

         void SetCoordinates();

         //this sets our state set on our root node to have the proper draw order and
         //prevent rendering of the snow within the vehicle
         void SetStateSet(osg::Node* node);

         float				   mPrecipRate;
         osg::ref_ptr<osgParticle::PrecipitationEffect> mPrecipEffect;

         /**
          * Helper method to help reduce some code
          */
         void AssignNewProxy(const dtCore::UniqueId &id);

      private:

         typedef std::vector<dtCore::RefPtr<dtAudio::Sound> > SoundArray;
         bool RemoveSoundFromArray(dtAudio::Sound&, SoundArray&);
         void UpdateRainSoundFX();
         void PlayRandomThunder();
         void ResetThunderTimer();
         void ClearRainSoundFX();
         float MapPrecipRate(float rate);

         bool   mAllowClipAjust;
         float  mPrecipStart;
         float  mNearClipPlane;
         float  mPreviousNearClipPlane;
         float  mFarClipPlane;
         float  mPreviousFarClipPlane;
         float  mThunderRangeMin, mThunderRangeMax;
         double mBaseElevation;
         double mMaxVisibility;
         double mMaxElevationVis;
         bool   mUpdatesEnabled;

         SimCore::Actors::PrecipitationType* mCurrentPrecipType;

         dtCore::RefPtr<Actors::UniformAtmosphereActor> mAtmosphere;
         dtCore::ObserverPtr<Actors::DayTimeActorProxy> mDayTime;

         // The Weather object is being referenced to gain access
         // to clouds. If it were not for this reason, the Environment
         // object would instead be referenced directly.
         dtCore::RefPtr<SimCore::Actors::IGEnvironmentActor> mEnvironmentActor;

         
         dtCore::ObserverPtr<dtAudio::Sound> mCurrentRainSound;
         dtCore::ObserverPtr<dtAudio::Sound> mCurrentThunderSound;
         
         std::vector< dtCore::RefPtr<dtAudio::Sound> > mThunderSounds;
         std::vector< dtCore::RefPtr<dtAudio::Sound> > mRainSounds;

      };
   }
}

#endif
