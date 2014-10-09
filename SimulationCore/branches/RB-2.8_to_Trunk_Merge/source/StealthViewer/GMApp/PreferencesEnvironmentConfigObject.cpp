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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/PreferencesEnvironmentConfigObject.h>

#include <SimCore/Components/WeatherComponent.h>

#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/UniformAtmosphereActor.h>

#include <dtUtil/mathdefines.h>
#include <dtCore/system.h>
#include <dtGame/gamemanager.h>
#include <dtUtil/datetime.h>

#include <osg/io_utils>
#include <sstream>

namespace StealthGM
{
   PreferencesEnvironmentConfigObject::PreferencesEnvironmentConfigObject() :
      mUseNetworkSettings(true),
      mUseThemedSettings(false),
      mUseCustomSettings(false),
      mNetworkHour(0),
      mNetworkMinute(0),
      mNetworkSeconds(0),
      mCustomHour(0),
      mCustomMinute(0),
      mCustomSeconds(0),
      mVisibilityDistance(0.0f),
      mPrecipitation(&SimCore::Actors::PrecipitationType::NONE)
   {

   }

   PreferencesEnvironmentConfigObject::~PreferencesEnvironmentConfigObject()
   {

   }

   void PreferencesEnvironmentConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      // Special case
      // Updated flag is triggered when the user modifies a value in the UI
      // However, if receiving network weather, we need to make sure the values
      // are set here from JSAF so they can be displayed in the UI.
      dtGame::GMComponent* component = gameManager.GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME);
      if(component == NULL)
         return;

      SimCore::Components::WeatherComponent& weatherComp = static_cast<SimCore::Components::WeatherComponent&>(*component);
      SimCore::Actors::UniformAtmosphereActor* atmosphere = weatherComp.GetAtmosphereActor();
      SimCore::Actors::DayTimeActorProxy* dayTimeProxy = weatherComp.GetDayTimeActor();
      SimCore::Actors::IGEnvironmentActor* igEnv = weatherComp.GetEphemerisEnvironment();

      // Special case
      // If we are in network mode, our values need to be set from the
      // the weather component. This code does not need to be applied
      // Since the weather component does this already
      if (GetUseNetworkSettings())
      {
         // Network time
         if (dayTimeProxy != NULL)
         {
            if (igEnv != NULL)
            {
               dtUtil::DateTime dt = igEnv->GetDateTime();

               mNetworkHour    = dt.GetHour();
               mNetworkMinute  = dt.GetMinute();
               mNetworkSeconds = int(dt.GetSecond());
            }
         }

         // Visibility and weather (precipitation)
         if (atmosphere != NULL)
         {
            mVisibilityDistance = atmosphere->GetVisibilityDistance();

            mPrecipitation = &atmosphere->GetPrecipitationType();
         }
      }

      // Nothing changed?
      if(!IsUpdated())
         return;


      // Turn updates in the component off or on
      weatherComp.SetUpdatesEnabled(GetUseNetworkSettings());

      if(igEnv != NULL && GetUseCustomSettings())
      {
         dtUtil::DateTime dt = igEnv->GetDateTime();
         dt.SetHour(mCustomHour);
         dt.SetMinute(mCustomMinute);
         dt.SetSecond(mCustomSeconds);

         dtCore::System::GetInstance().SetSimulationClockTime(dtCore::Timer_t(dt.GetTime()) * 1000000LL);
         //igEnv->SetTimeFromSystem();
      }

      SetIsUpdated(false);
   }

   void PreferencesEnvironmentConfigObject::SetUseNetworkSettings()
   {
      mUseNetworkSettings = true;
      mUseThemedSettings  = false;
      mUseCustomSettings  = false;
      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetUseThemedSettings()
   {
      mUseNetworkSettings = false;
      mUseThemedSettings  = true;
      mUseCustomSettings  = false;
      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetUseCustomSettings()
   {
      mUseNetworkSettings = false;
      mUseThemedSettings  = false;
      mUseCustomSettings  = true;
      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetNetworkHour(int hour)
   {
      if(hour < 0 || hour > 23)
         dtUtil::Clamp(hour, 0, 23);

      mNetworkHour = hour;

      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetNetworkMinute(int min)
   {
      if(min < 0 || min > 59)
         dtUtil::Clamp(min, 0, 59);

      mNetworkMinute = min;

      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetNetworkSecond(int sec)
   {
      if(sec < 0 || sec > 59)
         dtUtil::Clamp(sec, 0, 59);

      mNetworkSeconds = sec;

      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetCustomHour(int hour)
   {
      if(hour < 0 || hour > 23)
         dtUtil::Clamp(hour, 0, 23);

      mCustomHour = hour;

      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetCustomMinute(int min)
   {
      if(min < 0 || min > 59)
         dtUtil::Clamp(min, 0, 59);

      mCustomMinute = min;

      SetIsUpdated(true);
   }

   void PreferencesEnvironmentConfigObject::SetCustomSecond(int sec)
   {
      if(sec < 0 || sec > 59)
         dtUtil::Clamp(sec, 0, 59);

      mCustomSeconds = sec;

      SetIsUpdated(true);
   }

   std::string PreferencesEnvironmentConfigObject::GetTimeOfDayAsString() const
   {
      std::ostringstream oss;

      if(GetUseNetworkSettings())
      {
         if(GetNetworkHour() < 10)
            oss << "0";

         oss << GetNetworkHour() << ":";

         if(GetNetworkMinute() < 10)
            oss << "0";

         oss << GetNetworkMinute() << ":";

         if(GetNetworkSecond() < 10)
            oss << "0";

         oss << GetNetworkSecond();
      }
      else if(GetUseCustomSettings())
      {
         if(GetCustomHour() < 10)
            oss << "0";

         oss << GetCustomHour() << ":";

         if(GetCustomMinute() < 10)
            oss << "0";

         oss << GetCustomMinute() << ":";

         if(GetCustomSecond() < 10)
            oss << "0";

         oss << GetCustomSecond();
      }
      else if(GetUseThemedSettings())
      {

      }

      return oss.str();
   }
}
