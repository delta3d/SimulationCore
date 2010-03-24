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

namespace SimCore
{
   namespace Actors
   {
      class UniformAtmosphereActorProxy;
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
            // The default component name, used when looking it up on the GM.
            static const std::string DEFAULT_NAME;

            // Constructor
            // @param name The name by which this component is called from the GameManager
            WeatherComponent( const std::string& name = DEFAULT_NAME );

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

            Actors::UniformAtmosphereActorProxy* GetAtmosphereActor() { return mAtmosphere.get(); }
            const Actors::UniformAtmosphereActorProxy* GetAtmosphereActor() const { return mAtmosphere.get(); }

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


            bool   mAllowClipAjust;
            float  mPrecipStart;
            float  mNearClipPlane;
            float  mPreviousNearClipPlane;
            float  mFarClipPlane;
            float  mPreviousFarClipPlane;
            double mBaseElevation;
            double mMaxVisibility;
            double mMaxElevationVis;
            bool   mUpdatesEnabled;

            dtABC::Weather::CloudType mLastCloudType;
            dtABC::Weather::WindType mLastWindType;
            dtABC::Weather::VisibilityType mLastVisType;
            dtABC::Weather::TimePeriod mLastTimePeriod;
            dtABC::Weather::Season mLastSeason;

            dtCore::RefPtr<Actors::UniformAtmosphereActorProxy> mAtmosphere;
            dtCore::ObserverPtr<Actors::DayTimeActorProxy> mDayTime;

            // The Weather object is being referenced to gain access
            // to clouds. If it were not for this reason, the Environment
            // object would instead be referenced directly.
            dtCore::RefPtr<SimCore::Actors::IGEnvironmentActor> mEnvironmentActor;
      };
   }
}

#endif
