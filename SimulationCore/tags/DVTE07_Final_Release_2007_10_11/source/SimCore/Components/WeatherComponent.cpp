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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>

#include <dtABC/application.h>
#include <dtABC/weather.h>

#include <dtActors/basicenvironmentactorproxy.h>
#include <dtActors/engineactorregistry.h>
#include <dtActors/coordinateconfigactor.h>

#include <dtCore/camera.h>
#include <dtCore/environment.h>
#include <dtCore/isector.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>

#include <dtUtil/mathdefines.h>

#include <SimCore/Actors/DayTimeActor.h>
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
         mCurElevation(0.0),
         mMaxVisibility(40000.0),
         mMaxElevationVis(15000.0),
         mUpdatesEnabled(true),
         mLastCloudType(dtABC::Weather::CLOUD_FEW),
         mLastWindType(dtABC::Weather::WIND_NONE),
         mLastVisType(dtABC::Weather::VIS_FAR),
         mLastTimePeriod(dtABC::Weather::TIME_DAY),
         mLastSeason(dtABC::Weather::SEASON_SUMMER)
      {
         m_fPrecipRate = 0;
         m_spPrecipEffect = NULL;
      }

      //////////////////////////////////////////////////////////
      WeatherComponent::~WeatherComponent()
      {
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::Clear()
      {
         mAtmosphere = NULL;
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
         GetGameManager()->CreateActor(*Actors::EntityActorRegistry::DAYTIME_ACTOR_TYPE, mDayTime);
         SimCore::Actors::DayTimeActor* dayTimeActor = dynamic_cast<SimCore::Actors::DayTimeActor*>(mDayTime->GetActor());
         if( dayTimeActor != NULL )
         {
            time_t timeStamp = time(NULL);
            tm* currTime = localtime( &timeStamp );
            if( currTime != NULL )
            {
               dayTimeActor->SetYear( currTime->tm_year );
               dayTimeActor->SetMonth( currTime->tm_mon );
               dayTimeActor->SetDay( currTime->tm_mday );
               dayTimeActor->SetHour( currTime->tm_hour );
               dayTimeActor->SetMinute( currTime->tm_min );
               dayTimeActor->SetSecond( currTime->tm_sec );
            }
            GetGameManager()->AddActor(*mDayTime,false,false);
         }
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::SetEphemerisEnvironment(  SimCore::Actors::IGEnvironmentActor* env  )
      {
         mEphemerisEnvironmentActor = env;
      }

      //////////////////////////////////////////////////////////
      SimCore::Actors::IGEnvironmentActor* WeatherComponent::GetEphemerisEnvironment()
      {
         return mEphemerisEnvironmentActor.get();
      }

      //////////////////////////////////////////////////////////
      const SimCore::Actors::IGEnvironmentActor* WeatherComponent::GetEphemerisEnvironment() const
      {
         return mEphemerisEnvironmentActor.get();
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
         if(m_spPrecipEffect.valid())
         {
            SetStateSet(m_spPrecipEffect.get());
         }
      }

      void WeatherComponent::SetStateSet(osg::Node* node)
      {
         osg::StateSet* ss = m_spPrecipEffect->getOrCreateStateSet();                        
         osg::Depth* depth = new osg::Depth(osg::Depth::LEQUAL, mPrecipStart, 1.0); 
         ss->setAttributeAndModes(depth,osg::StateAttribute::ON ); 
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

         //if(!mUpdatesEnabled)
         //   return; 

         // Calculate the elevation and far clip plane based on elevation 
         float elevation = dtUtil::Abs(mCurElevation - mBaseElevation);
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
            camera->SetPerspective(camera->GetHorizontalFov(), camera->GetVerticalFov(), newNearClip, newFarClip);
         }

         // Set the visibility on core env or ephemeris only if updates are
         // not enabled. This implies that no updates are being received from the weather server.
         // If a weather server is connected, set the visibility based on the view distance
         // reported by the weather server.
         if(!mUpdatesEnabled)
         {
            float vis = newFarClip * 0.6f; // Make fog distance a tad less than the far clip. Helps account for the boxy effect of culling & clipping. 
            if(mEphemerisEnvironmentActor.valid())
            {
               if(!dtUtil::Equivalent(mEphemerisEnvironmentActor->GetVisibility(), vis))
               {
                  mEphemerisEnvironmentActor->SetVisibility(vis);
               }
            }
         }
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::SetViewElevation( double elevation )
      {
         mCurElevation = elevation;
         UpdateFog();
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
         if(type == dtGame::MessageType::TICK_LOCAL || 
            type == dtGame::MessageType::TICK_REMOTE)
         {
            if(mEphemerisEnvironmentActor.valid())
            {
               dtCore::Transform positionTransform;
               GetGameManager()->GetApplication().GetCamera()->GetTransform(positionTransform);
               mEphemerisEnvironmentActor->SetSkyDomesCenter(positionTransform.GetTranslation());
            }

            return;
         }

         // Check for a NEW weather object
         if(type == dtGame::MessageType::INFO_ACTOR_UPDATED ||
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
            else if(mEphemerisEnvironmentActor.valid() && msg.GetAboutActorId() == mEphemerisEnvironmentActor->GetUniqueId())
            {
               mEphemerisEnvironmentActor = NULL;
            }
         } 

         else if( type == dtGame::MessageType::INFO_MAP_LOADED)
         {
         }

         // Check for the environment object to see if it was loaded in the map file
         else if (type == dtGame::MessageType::INFO_ENVIRONMENT_CHANGED)
         {
            dtDAL::ActorProxy *proxy = GetGameManager()->FindActorById(msg.GetAboutActorId());
            SimCore::Actors::IGEnvironmentActorProxy* igproxy = 
               dynamic_cast<SimCore::Actors::IGEnvironmentActorProxy*>(proxy);
            if(igproxy == NULL)
            {
               LOG_WARNING("An actor was found, but is not an IGEnvironmentActorProxy");
               LOG_DEBUG("The type of the actor is: " + proxy->GetActorType().GetName());
            }
            else
            {
               SetEphemerisEnvironment(static_cast<SimCore::Actors::IGEnvironmentActor*>
                  (igproxy->GetActor()));
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
         if(!mUpdatesEnabled || !mEphemerisEnvironmentActor.valid())
            return;

         // Update the time of day
         if(mDayTime.valid())
         {
            std::vector<dtDAL::ActorProxy*> actors;
            const dtDAL::ActorType* type = GetGameManager()->FindActorType("dtutil", "Coordinate Config");
            GetGameManager()->FindActorsByType(*type, actors);
            if(!actors.empty())
            {
               // Get the offset of time zones
               dtActors::CoordinateConfigActorProxy *coordConfigActorProxy = dynamic_cast<dtActors::CoordinateConfigActorProxy*>(actors[0]);
               if(coordConfigActorProxy != NULL)
               {
                  dtActors::CoordinateConfigActor *coordConfigActor
                     = dynamic_cast<dtActors::CoordinateConfigActor*>
                     (coordConfigActorProxy->GetActor());

                  // Compensate for the time zone
                  osg::Vec3d geoOffset;
                  if(coordConfigActor != NULL)
                  {
                     dtUtil::Coordinates coords = const_cast<dtUtil::Coordinates&> (coordConfigActor->GetCoordinates());
                     coordConfigActor->SetIncomingCoordinateType(
                        const_cast<dtUtil::IncomingCoordinateType&>
                        (dtUtil::IncomingCoordinateType::GEODETIC));
                     geoOffset = coords.ConvertToRemoteTranslation(geoOffset);

                     // Adjust the time. Convert the degrees to hours
                     //                  geoOffset[1] = geoOffset[1]/360.0f*24.0f+0.5; // 0.5 to round up
                     mEphemerisEnvironmentActor->SetLatitudeAndLongitude(geoOffset[0],
                                                                           geoOffset[1]);
                  }
               }
            }
         }
         else
         {
            LOG_WARNING("WeatherComponent has no time of day data");
         }

         UpdateDateAndTime();
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::UpdateDateAndTime()
      {
         if(!mDayTime.valid())
            return;

         Actors::DayTimeActor* timeActor = 
            static_cast<Actors::DayTimeActor*>(mDayTime->GetActor());

         // Change the environment time
         mEphemerisEnvironmentActor->SetTimeAndDate(
            timeActor->GetYear()+1900,
            timeActor->GetMonth()+1,
            timeActor->GetDay(),
            timeActor->GetHour(),
            timeActor->GetMinute(),
            timeActor->GetSecond());
      }

      //////////////////////////////////////////////////////////
      void WeatherComponent::UpdateWeather()
      {
         if(!mUpdatesEnabled)
            return;

         if(!mEphemerisEnvironmentActor.valid()) 
         { 
            return; 
         }

         // Update the weather conditions
         if(mAtmosphere.valid())
         {
            Actors::UniformAtmosphereActor* atmosActor = 
               static_cast<Actors::UniformAtmosphereActor*>(mAtmosphere->GetActor());

            // Change Clouds
            // TODO

            // Change Visibility and Fog
            // --- Atmosphere holds visibility in Km, but the environment object expects meters.
            mEphemerisEnvironmentActor->SetVisibility(atmosActor->GetVisibilityDistance()*1000.0f);
            UpdateFog();

            // Change the Cloud Coverage
            mEphemerisEnvironmentActor->ChangeClouds((atmosActor->GetCloudCoverage() / 10), 
                                                      atmosActor->GetWindSpeedX(), 
                                                      atmosActor->GetWindSpeedY());
            // Change Wind
            // --- The following forces an actor update message for which
            //     components and entity may be listening.
            osg::Vec2 wind(atmosActor->GetWind());
            mEphemerisEnvironmentActor->SetWind(osg::Vec3( wind[0], wind[1], 0.0 ));

            // Change Precipitation
            // --- This change must be applied before wind changes occur,
            //     just in case precipitation uses particle effects or
            //     some other precipitation effect needs to respond to
            //     wind forces.
            // TODO: Precipitation effect shutdown and setup
            if (//m_iPrecipType != atmosActor->GetPrecipitationType() ||
               m_fPrecipRate != atmosActor->GetPrecipitationRate())
            {
               m_fPrecipRate = atmosActor->GetPrecipitationRate();	// mm/hr

               // Map rate to a rough 0-1 density based on meteorologist classifications.
               // A weather server may send specific values in {0,2,5,12,25,75} mm/hr.
               if (m_fPrecipRate < 0.1)
                  m_fPrecipRate = 0.0;
               else if (m_fPrecipRate < 2.5)	// "light rain" is < 2.5 mm/hr
                  m_fPrecipRate = 0.2;
               else if (m_fPrecipRate < 7.6)
                  m_fPrecipRate = 0.4;
               else if (m_fPrecipRate < 16)	// "heavy rain" is > 7.6 mm/hr
                  m_fPrecipRate = 0.6;
               else if (m_fPrecipRate < 50)	// "downpour"   is > 16 mm/hr
                  m_fPrecipRate = 0.8;
               else
                  m_fPrecipRate = 1.0;

               bool toContinue = true;
               if(m_fPrecipRate < 0.01)
               {
                  if(m_spPrecipEffect.valid())
                  {
                     GetGameManager()->GetEnvironmentActor()->GetActor()->GetOSGNode()->asGroup()->removeChild(m_spPrecipEffect.get());
                     m_spPrecipEffect = NULL;
                  }
                  toContinue = false;
               }

               if(toContinue)
               {
                  // Create or change the effect
                  if(!m_spPrecipEffect.valid())
                  {
                     m_spPrecipEffect = new osgParticle::PrecipitationEffect;
                     m_spPrecipEffect->setNodeMask(0x00000010);
                     
                     SetStateSet(m_spPrecipEffect.get());

                     GetGameManager()->GetEnvironmentActor()->GetActor()->GetOSGNode()->asGroup()->addChild(m_spPrecipEffect.get());
                  }

                  if(atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::FREEZING_RAIN
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::RAIN)
                  {
                     m_spPrecipEffect->rain(m_fPrecipRate);
                  }
                  else if(atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::SNOW
                     || atmosActor->GetPrecipitationType() == SimCore::Actors::PrecipitationType::HAIL)
                  {
                     m_spPrecipEffect->snow(m_fPrecipRate);
                  }

                  m_spPrecipEffect->setWind(osg::Vec3( wind[0], wind[1], 0.0f ));
               }
            }
            // Notify the app that the environment actor has changed.
            mEphemerisEnvironmentActor->GetGameActorProxy().NotifyActorUpdate();
         }
         else
         {
            LOG_WARNING("WeatherComponent has no Atmospheric data.");
         }
      }

      //////////////////////////////////////////////////////////
      dtABC::Weather::CloudType WeatherComponent::ClassifyClouds( const Actors::UniformAtmosphereActor& atmos )
      {
         SimCore::Actors::CloudType& type  = atmos.GetCloudType();

         if( type == SimCore::Actors::CloudType::ALTOCUMULUS
            || type == SimCore::Actors::CloudType::CIRROCUMULUS )
         {
            return dtABC::Weather::CLOUD_BROKEN;
         }
         if( type == SimCore::Actors::CloudType::ALTOSTRATUS
            || type == SimCore::Actors::CloudType::STRATUS
            || type == SimCore::Actors::CloudType::NIMBOSTRATUS)
         {
            return dtABC::Weather::CLOUD_OVERCAST;
         }
         if( type == SimCore::Actors::CloudType::CUMULUS
            || type == SimCore::Actors::CloudType::CUMULONIMBUS )
         {
            return dtABC::Weather::CLOUD_SCATTERED;
         }
         if( type == SimCore::Actors::CloudType::CLEAR )
         {
            return dtABC::Weather::CLOUD_CLEAR;
         }

         // SimCore::Actors::CloudType::STRATOCUMULUS
         // SimCore::Actors::CloudType::CIRRUS
         // SimCore::Actors::CloudType::CIRROSTRATUS
         return dtABC::Weather::CLOUD_FEW;
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
      dtABC::Weather::VisibilityType WeatherComponent::ClassifyVisibility( const Actors::UniformAtmosphereActor& atmos )
      {
         // The following was documented in dtABC::Weather:
         //
         // VIS_UNLIMITED = 0, ///<no restrictions
         // VIS_FAR = 1,       ///<50km
         // VIS_MODERATE = 2,  ///<25km
         // VIS_LIMITED = 3,   ///<8km
         // VIS_CLOSE = 4      ///<1.5km

         float distance = atmos.GetVisibilityDistance();

         if( distance < 1.5f )
            return dtABC::Weather::VIS_CLOSE;

         if( distance < 8.0f )
            return dtABC::Weather::VIS_LIMITED;

         if( distance < 25.0f )
            return dtABC::Weather::VIS_MODERATE;

         if( distance < 50.0f )
            return dtABC::Weather::VIS_FAR;

         return dtABC::Weather::VIS_UNLIMITED;
      }

      void WeatherComponent::AssignNewProxy(const dtCore::UniqueId &id)
      {
         dtDAL::ActorProxy *actor = GetGameManager()->FindActorById(id);
         const dtDAL::ActorType &type = actor->GetActorType();

         if(type == *SimCore::Actors::EntityActorRegistry::UNIFORM_ATMOSPHERE_ACTOR_TYPE)
         {
            Actors::UniformAtmosphereActorProxy* proxy = 
               static_cast<Actors::UniformAtmosphereActorProxy*>(actor);

            if(mAtmosphere.valid() && mAtmosphere->GetId() != proxy->GetId())
            {
               LOG_WARNING("WARNING! WeatherComponent.mAtmosphere points to another AtmosphereActor\
                           The old AtmosphereActor may not have been removed.");
               mAtmosphere = NULL;
            }

            if(!mAtmosphere.valid()) 
            { 
               mAtmosphere = proxy; 
            }

            UpdateWeather();
         }
         else if(type == *SimCore::Actors::EntityActorRegistry::DAYTIME_ACTOR_TYPE)
         {
            Actors::DayTimeActorProxy* proxy = static_cast<Actors::DayTimeActorProxy*>(actor);
            if(mDayTime.valid() && mDayTime->GetId() != proxy->GetId())
            {
               LOG_WARNING("WARNING! WeatherComponent.mDayTime points to another DayTimeActor\
                           The old DayTimeActor may not have been removed.");
               mDayTime = NULL;
            }

            if(!mDayTime.valid()) 
            { 
               mDayTime = proxy; 
            }

            UpdateDayTime();
         }
         else if(type == *dtActors::EngineActorRegistry::ENVIRONMENT_ACTOR_TYPE)
         {
            SimCore::Actors::IGEnvironmentActorProxy* igproxy = 
               dynamic_cast<SimCore::Actors::IGEnvironmentActorProxy*>(actor);
            if(igproxy != NULL)
            {
               SetEphemerisEnvironment(static_cast<SimCore::Actors::IGEnvironmentActor*> 
                  (igproxy->GetActor()));
            }
         }
      }
   }
}
