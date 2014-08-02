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
#include <prefix/SimCorePrefix.h>

#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <dtABC/application.h>
#include <dtABC/weather.h>

#include <dtActors/engineactorregistry.h>
#include <dtActors/coordinateconfigactor.h>

#include <dtCore/camera.h>
#include <dtCore/environment.h>
#include <dtCore/isector.h>
#include <dtCore/system.h>
#include <dtCore/transform.h>

#include <dtCore/actorproperty.h>
#include <dtCore/enginepropertytypes.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/nodemask.h>

#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Tools/Binoculars.h>

#include <osg/Depth>
#include <osg/StateSet>

#include <ctime>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////
      // Weather Component Code
      //////////////////////////////////////////////////////////
      const std::string WeatherComponent::DEFAULT_NAME = "WeatherComponent";

      //////////////////////////////////////////////////////////
      WeatherComponent::WeatherComponent( const std::string& name )
         : dtGame::GMComponent(name),
         mAllowClipAjust(true),
         mPrecipStart(0.0f),
         mNearClipPlane(SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE),
         mPreviousNearClipPlane(0.0),
         mFarClipPlane(SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE),
         mPreviousFarClipPlane(0.0),
         mBaseElevation(0.0), // 600 was the old default height for some terrains
         mMaxVisibility(40000.0),
         mMaxElevationVis(15000.0),
         mUpdatesEnabled(true),
         mLastCloudType(dtABC::Weather::CLOUD_FEW),
         mLastWindType(dtABC::Weather::WIND_NONE),
         mLastVisType(dtABC::Weather::VIS_FAR),
         mLastTimePeriod(dtABC::Weather::TIME_DAY),
         mLastSeason(dtABC::Weather::SEASON_SUMMER)
      {
         mPrecipRate = 0;
         mPrecipEffect = NULL;
      }

      //////////////////////////////////////////////////////////
      WeatherComponent::~WeatherComponent()
      {
      }

      //////////////////////////////////////////////////////////
      /*void WeatherComponent::BuildPropertyMap()
      {
         dtGame::GMComponent::BuildPropertyMap();

         const std::string GROUPNAME = "Weather Component Values";

         AddProperty(new dtCore::DoubleActorProperty("Base Elevation","Base Elevation",
            dtCore::MakeFunctor(*this,&WeatherComponent::SetBaseElevation),
            dtCore::MakeFunctorRet(*this,&WeatherComponent::GetBaseElevation),
            "", GROUPNAME));

         AddProperty(new dtCore::DoubleActorProperty("Max Visibility","Max Visibility",
            dtCore::MakeFunctor(*this,&WeatherComponent::SetMaxVisibility),
            dtCore::MakeFunctorRet(*this,&WeatherComponent::GetMaxVisibility),
            "", GROUPNAME));
      }*/

      //////////////////////////////////////////////////////////
      void WeatherComponent::Clear()
      {
         mAtmosphere = NULL;
         mEnvironmentActor = NULL;
         // Do not set the day time actor to NULL because one
         // is needed if the simulation is not connected to a
         // "Time Master". If a "Time Master" is connected, the
         // day time actor will be replaced automatically.
         //mDayTime = NULL;
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::Reset()
      {
         Clear();
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::OnAddedToGM()
      {

      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::SetEphemerisEnvironment(  SimCore::Actors::IGEnvironmentActor* env  )
      {
         mEnvironmentActor = env;
      }

      //////////////////////////////////////////////////////////
      SimCore::Actors::IGEnvironmentActor* WeatherComponent::GetEphemerisEnvironment()
      {
         return mEnvironmentActor.get();
      }

      //////////////////////////////////////////////////////////
      const SimCore::Actors::IGEnvironmentActor* WeatherComponent::GetEphemerisEnvironment() const
      {
         return mEnvironmentActor.get();
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::SetBaseElevation( double elevation )
      {
         mBaseElevation = elevation;
      }

      //////////////////////////////////////////////////////////
      double WeatherComponent::GetBaseElevation() const
      {
         return mBaseElevation;
      }


      //////////////////////////////////////////////////////////
      void WeatherComponent::SetPrecipStart(float start)
      {
         mPrecipStart = start;

         //if we already have precipitation, then go ahead and set its start distance now
         if(mPrecipEffect.valid())
         {
            SetStateSet(mPrecipEffect.get());
         }
      }

      void WeatherComponent::SetStateSet(osg::Node* node)
      {
         osg::StateSet* ss = mPrecipEffect->getOrCreateStateSet();
         osg::Depth* depth = new osg::Depth(osg::Depth::LEQUAL, mPrecipStart, 1.0);                       
         depth->setWriteMask(false);

         ss->setAttributeAndModes(depth,osg::StateAttribute::ON || osg::StateAttribute::OVERRIDE);
         ss->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_PRECIPITATION, "DepthSortedBin");
      }

      //////////////////////////////////////////////////////////
      float WeatherComponent::GetPrecipStart() const
      {
         return mPrecipStart;
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::UpdateFog()
      {
         dtCore::Camera* camera = GetGameManager()->GetApplication().GetCamera();

         dtCore::Transform xform;
         camera->GetTransform(xform);
         //if(!mUpdatesEnabled)
         //   return;

         // Calculate the elevation and far clip plane based on elevation
         float elevation = dtUtil::Abs(xform.GetTranslation().z() - mBaseElevation);
         float newFarClip = mFarClipPlane + elevation * 2.0f;
         // The FAR can't be shorter than current far, or larger than maxVis
         dtUtil::Clamp(newFarClip, mFarClipPlane, (float) mMaxVisibility);
         float newNearClip = mNearClipPlane * (newFarClip / mFarClipPlane); // push the near out to scale with the far

         // Update the near/far - if we are allowed and if the values have changed
         if( mAllowClipAjust && (!dtUtil::Equivalent(newNearClip, mPreviousNearClipPlane) ||
               !dtUtil::Equivalent(newFarClip, mPreviousFarClipPlane)))
         {
            mPreviousNearClipPlane = newNearClip;
            mPreviousFarClipPlane = newFarClip;

            camera->SetPerspectiveParams(camera->GetVerticalFov(),
                                         camera->GetAspectRatio(),
                                         newNearClip, newFarClip);
         }

         // Set the visibility on core env or ephemeris
         // If a weather server is connected, set the visibility based on the view distance
         // reported by the weather server.
         float vis = mPreviousFarClipPlane; 

         if(mAtmosphere.valid())
         {
            Actors::UniformAtmosphereActor* atmosActor =
               static_cast<Actors::UniformAtmosphereActor*>(mAtmosphere->GetDrawable());

            if(atmosActor != NULL)
            {
               vis = atmosActor->GetVisibilityDistance()*1000.0f;
            }
         }

         if(mEnvironmentActor.valid())
         {
            if(!dtUtil::Equivalent(mEnvironmentActor->GetVisibility(), vis))
            {
               dtUtil::Clamp<float>(vis, 0.0f, mPreviousFarClipPlane - 100.0f);

               mEnvironmentActor->SetVisibility(vis);
            }
         }

      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::SetMaxVisibility( double maxVisibility )
      {
         mMaxVisibility = maxVisibility;
      }

      //////////////////////////////////////////////////////////
      double WeatherComponent::GetMaxVisibility() const
      {
         return mMaxVisibility;
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::SetMaxElevationVisibility( double maxVisibility )
      {
         mMaxElevationVis = maxVisibility;
      }

      //////////////////////////////////////////////////////////
      double WeatherComponent::GetMaxElevationVisibility() const
      {
         return mMaxElevationVis;
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::ProcessMessage( const dtGame::Message& msg )
      {
         // NOTE: The order of the message types checked in this function
         // are in the assumed order of the MOST frequent to the LEAST frequent
         // for optimization purposes

         // Use a local reference to avoid multiple calls to Message.GetMessageType()
         const dtGame::MessageType& type = msg.GetMessageType();

         // Avoid extra comparisons as this is a mere tick message
         if(type == dtGame::MessageType::TICK_LOCAL)
         {
            if(mEnvironmentActor.valid())
            {
               dtCore::Transform positionTransform;
               GetGameManager()->GetApplication().GetCamera()->GetTransform(positionTransform);
               osg::Vec3 pos;
               positionTransform.GetTranslation(pos);
               mEnvironmentActor->SetSkyDomesCenter(pos);

               mEnvironmentActor->SetTimeFromSystem();
            }

            UpdateFog();
         }
         // Check for a NEW weather object
         else if(type == dtGame::MessageType::INFO_ACTOR_UPDATED ||
            type == dtGame::MessageType::INFO_ACTOR_CREATED)
         {
            // Convert the message to its true form
            const dtGame::ActorUpdateMessage& actorUpdateMsg =
               static_cast<const dtGame::ActorUpdateMessage&> (msg);

            AssignNewProxy(actorUpdateMsg.GetAboutActorId());
         }
         // Check for a DELETED weather object
         else if( type == dtGame::MessageType::INFO_ACTOR_DELETED )
         {
            if( mAtmosphere.valid() && msg.GetAboutActorId() == mAtmosphere->GetId() )
            {
               mAtmosphere = NULL;
            }
            else if( mDayTime.valid() && msg.GetAboutActorId() == mDayTime->GetId() )
            {
               mDayTime = NULL;
            }
            else if(mEnvironmentActor.valid() && msg.GetAboutActorId() == mEnvironmentActor->GetUniqueId())
            {
               mEnvironmentActor = NULL;
            }
         }

         else if( type == dtGame::MessageType::INFO_MAP_LOADED)
         {
            SetCoordinates();

            if (mEnvironmentActor == NULL)
            {
               LOG_ERROR("No environment actor exists after MAP_LOADED. It is possible you did not include any AdditionalMaps in your config.xml.");
               return;
            }

            //NOTE: we added the ability to override the system clock using the environment actor
            //to avoid users having an unexpected start time because they didnt set the date time
            //on the environment actor we add a property called InitializeSystemClock on the IGEnvironmentActor

            // This sets the time the very first time a map changes to NOON. Without this,
            // the game ends up in some weird time zone offset from the current time clock. It's confusing.
            
            dtUtil::DateTime dt(mEnvironmentActor->GetDateTime());

            if(!mEnvironmentActor->GetInitializeSystemClock())
            {
               dt.SetToLocalTime(); // try to sync up the month/day/year first.
               dt.SetHour(12);
               dt.SetMinute(0);
               dt.SetSecond(0.0f);
            }

            LOGN_DEBUG("WeatherComponent.cpp", "Setting Sim Clock Time to Noon");
            dtCore::System::GetInstance().SetSimulationClockTime(dtCore::Timer_t(dt.GetTime()) * 1000000LL);
            LOGN_DEBUG("WeatherComponent.cpp", "Updating the mEnvironment Actor to the system");
            mEnvironmentActor->SetTimeFromSystem();

            //enable lens flare
            //std::cout << std::endl << "Enabling lens flare" << std::endl << std::endl;

            //mEnvironmentActor->SetEnableLensFlare(true);

         }

         // Check for the environment object to see if it was loaded in the map file
         else if (type == dtGame::MessageType::INFO_ENVIRONMENT_CHANGED)
         {
            dtCore::ActorProxy *proxy = GetGameManager()->FindActorById(msg.GetAboutActorId());
            SimCore::Actors::IGEnvironmentActorProxy* igproxy =
               dynamic_cast<SimCore::Actors::IGEnvironmentActorProxy*>(proxy);
            if(igproxy == NULL)
            {
               LOGN_WARNING("WeatherComponent.cpp", "An actor was found, but is not an IGEnvironmentActorProxy");
               LOGN_DEBUG("WeatherComponent.cpp","The type of the actor is: " + proxy->GetActorType().GetFullName());
               SetEphemerisEnvironment(NULL);
            }
            else
            {
               LOGN_DEBUG("WeatherComponent.cpp","Setting Environment Actor to: Id\"" + proxy->GetId().ToString()
                          + "\" Actor Type: \"" + proxy->GetActorType().GetFullName() + "\"");
               SetEphemerisEnvironment(static_cast<SimCore::Actors::IGEnvironmentActor*>
                  (igproxy->GetDrawable()));
            }
         }
         // Check for application state changes
         else if( type == dtGame::MessageType::COMMAND_RESTART
            || type == dtGame::MessageType::INFO_MAP_CHANGE_BEGIN )
         {
            // These states will cause most or all entities to be cleared
            // from the current running simulation. Reset this component
            // as it should be expected.
            Reset();
         }
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::UpdateDayTime()
      {
         if(!mEnvironmentActor.valid())
            return;

         // Update the time of day
         if(mDayTime.valid())
         {
            SetCoordinates();

            if(mUpdatesEnabled)
            {
               Actors::DayTimeActor* timeActor =
                  static_cast<Actors::DayTimeActor*>(mDayTime->GetDrawable());

               float offset = mEnvironmentActor->GetDateTime().GetGMTOffset();
               dtUtil::DateTime dt(dtUtil::DateTime::TimeOrigin::GMT_TIME);
               dt.SetTime(timeActor->GetTime());
               dt.AdjustTimeZone(offset);

               dtCore::System::GetInstance().SetSimulationClockTime(dtCore::Timer_t(dt.GetTimeInSeconds() * 1e6));
            }
         }
         else
         {
            LOG_WARNING("WeatherComponent has no time of day data");
         }
      }


      void WeatherComponent::SetCoordinates()
      {
         if(!mEnvironmentActor.valid())
            return;

         dtActors::CoordinateConfigActorProxy* coordConfigActorProxy = NULL;
         GetGameManager()->FindActorByType(*dtActors::EngineActorRegistry::COORDINATE_CONFIG_ACTOR_TYPE,
                  coordConfigActorProxy);
         // Get the offset of time zones
         if(coordConfigActorProxy != NULL)
         {
            dtActors::CoordinateConfigActor* coordConfigActor = NULL;
            coordConfigActorProxy->GetDrawable(coordConfigActor);

            // Compensate for the time zone
            osg::Vec3d geoOffset;
            if(coordConfigActor != NULL)
            {
               dtUtil::Coordinates coords = coordConfigActor->GetCoordinates();
               coords.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
               geoOffset = coords.ConvertToRemoteTranslation(geoOffset);

               mEnvironmentActor->SetLatitudeAndLongitude(geoOffset[0],
                  geoOffset[1]);

               dtUtil::DateTime dt(mEnvironmentActor->GetDateTime());
               dt.SetGMTOffset(geoOffset[0], geoOffset[1], false);
               mEnvironmentActor->SetDateTime(dt);
            }
         }
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::UpdateWeather()
      {
         if (!mUpdatesEnabled)
            return;

         if (!mEnvironmentActor.valid())
         {
            return;
         }

         // Update the weather conditions
         if (mAtmosphere.valid())
         {
            Actors::UniformAtmosphereActor* atmosActor =
               static_cast<Actors::UniformAtmosphereActor*>(mAtmosphere->GetDrawable());

            // Change Clouds
            // TODO

            // Change Visibility and Fog
            // --- Atmosphere holds visibility in Km, but the environment object expects meters.

            UpdateFog();

            // Change the Cloud Coverage
            mEnvironmentActor->ChangeClouds(int(atmosActor->GetCloudCoverage() / 10),
                                                      atmosActor->GetWindSpeedX(),
                                                      atmosActor->GetWindSpeedY());
            // Change Wind
            // --- The following forces an actor update message for which
            //     components and entity may be listening.
            osg::Vec3 wind(atmosActor->GetWind(), 0.0f);
            mEnvironmentActor->SetWind(wind);//osg::Vec3( wind[0], wind[1], 0.0 ));

            // Change Precipitation
            // --- This change must be applied before wind changes occur,
            //     just in case precipitation uses particle effects or
            //     some other precipitation effect needs to respond to
            //     wind forces.
            // TODO: Precipitation effect shutdown and setup
            if (//m_iPrecipType != atmosActor->GetPrecipitationType() ||
               mPrecipRate != atmosActor->GetPrecipitationRate())
            {
               mPrecipRate = atmosActor->GetPrecipitationRate();	// mm/hr

               // Map rate to a rough 0-1 density based on meteorologist classifications.
               // A weather server may send specific values in {0,2,5,12,25,75} mm/hr.
               if (mPrecipRate < 0.1)
                  mPrecipRate = 0.0;
               else if (mPrecipRate < 2.5)	// "light rain" is < 2.5 mm/hr
                  mPrecipRate = 0.2;
               else if (mPrecipRate < 7.6)
                  mPrecipRate = 0.4;
               else if (mPrecipRate < 16)	// "heavy rain" is > 7.6 mm/hr
                  mPrecipRate = 0.6;
               else if (mPrecipRate < 50)	// "downpour"   is > 16 mm/hr
                  mPrecipRate = 0.8;
               else
                  mPrecipRate = 1.0;

               bool toContinue = true;
               if(mPrecipRate < 0.01 || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::NONE)
               {
                  if(mPrecipEffect.valid())
                  {
                     mEnvironmentActor->GetOSGNode()->asGroup()->removeChild(mPrecipEffect.get());
                     mPrecipEffect = NULL;
                  }
                  toContinue = false;
               }

               if(toContinue)
               {
                  // Create or change the effect
                  if(!mPrecipEffect.valid())
                  {
                     mPrecipEffect = new osgParticle::PrecipitationEffect;
                     mPrecipEffect->setNodeMask(dtUtil::NodeMask::TRANSPARENT_EFFECTS);//prior value- (0x00000010);

                     SetStateSet(mPrecipEffect.get());

                     mEnvironmentActor->GetOSGNode()->asGroup()->addChild(mPrecipEffect.get());
                  }

                  if(atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::FREEZING_RAIN
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::RAIN
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::OTHER
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::UNKNOWN)
                  {
                     mPrecipEffect->rain(mPrecipRate);
                  }
                  else if(atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::SNOW
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::HAIL
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::SLEET
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::GRAUPEL)
                  {
                     mPrecipEffect->snow(mPrecipRate);
                  }
                  else if (atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::NONE)
                  {
                     // It really shouldn't even get here because it's shut off above.
                     mPrecipEffect->rain(0.0f);
                  }

                  mPrecipEffect->setWind(osg::Vec3( wind[0], wind[1], 0.0f ));
               }
            }
            // Notify the app that the environment actor has changed.
            mEnvironmentActor->GetGameActorProxy().NotifyFullActorUpdate();
         }
         else
         {
            LOG_WARNING("WeatherComponent has no Atmospheric data.");
         }
      }

      //////////////////////////////////////////////////////////
      dtABC::Weather::CloudType WeatherComponent::ClassifyClouds( const Actors::UniformAtmosphereActor& atmos )
      {
         dtABC::Weather::CloudType result = dtABC::Weather::CLOUD_FEW;

         SimCore::Actors::CloudType& type  = atmos.GetCloudType();

         if (type == SimCore::Actors::CloudType::ALTOCUMULUS
            || type == SimCore::Actors::CloudType::CIRROCUMULUS)
         {
            result =  dtABC::Weather::CLOUD_BROKEN;
         }
         else if ( type == SimCore::Actors::CloudType::ALTOSTRATUS
            || type == SimCore::Actors::CloudType::STRATUS
            || type == SimCore::Actors::CloudType::NIMBOSTRATUS)
         {
            result =  dtABC::Weather::CLOUD_OVERCAST;
         }
         else if (type == SimCore::Actors::CloudType::CUMULUS
            || type == SimCore::Actors::CloudType::CUMULONIMBUS)
         {
            result =  dtABC::Weather::CLOUD_SCATTERED;
         }
         else if (type == SimCore::Actors::CloudType::CLEAR)
         {
            result =  dtABC::Weather::CLOUD_CLEAR;
         }

         return result;
      }

      //////////////////////////////////////////////////////////
      dtABC::Weather::WindType WeatherComponent::ClassifyWind( const Actors::UniformAtmosphereActor& atmos )
      {
         double speed = atmos.GetWind().length();

         if( speed > 50.0 ) // above HEAVY
            return dtABC::Weather::WIND_SEVERE;

         if( speed > 30.0 ) // above MODERATE
            return dtABC::Weather::WIND_HEAVY;

         if( speed > 10.0 ) // above LIGHT
            return dtABC::Weather::WIND_MODERATE;

         if( speed > 5.0 ) // above BREEZE
            return dtABC::Weather::WIND_LIGHT;

         if( speed > 0.05 ) // above negligible breeze
            return dtABC::Weather::WIND_BREEZE;

         return dtABC::Weather::WIND_NONE;
      }
      //////////////////////////////////////////////////////////
      dtABC::Weather::VisibilityType WeatherComponent::ClassifyVisibility(const Actors::UniformAtmosphereActor& atmos)
      {
         // The following was documented in dtABC::Weather:
         //
         // VIS_UNLIMITED = 0, ///<no restrictions
         // VIS_FAR = 1,       ///<50km
         // VIS_MODERATE = 2,  ///<25km
         // VIS_LIMITED = 3,   ///<8km
         // VIS_CLOSE = 4      ///<1.5km

         float distance = atmos.GetVisibilityDistance();

         if (distance < 1.5f)
            return dtABC::Weather::VIS_CLOSE;

         if (distance < 8.0f)
            return dtABC::Weather::VIS_LIMITED;

         if (distance < 25.0f)
            return dtABC::Weather::VIS_MODERATE;

         if (distance < 50.0f)
            return dtABC::Weather::VIS_FAR;

         return dtABC::Weather::VIS_UNLIMITED;
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::AssignNewProxy(const dtCore::UniqueId& id)
      {
         dtCore::ActorProxy* actor = GetGameManager()->FindActorById(id);
         if (actor == NULL)
         {
            return;
         }

         const dtCore::ActorType& type = actor->GetActorType();

         if (type == *SimCore::Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE)
         {
            Actors::UniformAtmosphereActorProxy* proxy =
               static_cast<Actors::UniformAtmosphereActorProxy*>(actor);

            if (mAtmosphere.valid() && mAtmosphere->GetId() != proxy->GetId())
            {
               LOG_WARNING("WARNING! WeatherComponent.mAtmosphere points to another AtmosphereActor"
                           "The old AtmosphereActor may not have been removed.");
               mAtmosphere = NULL;
            }

            if (!mAtmosphere.valid())
            {
               mAtmosphere = proxy;
            }

            UpdateWeather();
         }
         else if (type == *SimCore::Actors::EntityActorRegistry::DAYTIME_ACTOR_TYPE)
         {
            Actors::DayTimeActorProxy* proxy = static_cast<Actors::DayTimeActorProxy*>(actor);
            if (mDayTime.valid() && mDayTime->GetId() != proxy->GetId())
            {
               LOG_WARNING("WARNING! WeatherComponent.mDayTime points to another DayTimeActor\
                           The old DayTimeActor may not have been removed.");
               mDayTime = NULL;
            }

            if (!mDayTime.valid())
            {
               mDayTime = proxy;
            }

            UpdateDayTime();
         }
      }
   }
}

