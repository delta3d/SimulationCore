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
#ifndef _PREFERENCES_ENVIRONMENT_CONFIG_OBJECT_H_
#define _PREFERENCES_ENVIRONMENT_CONFIG_OBJECT_H_

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

#include <dtActors/basicenvironmentactorproxy.h>

#include <dtUtil/enumeration.h>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT PreferencesEnvironmentConfigObject : public ConfigurationObjectInterface
   {
      public:

         /// Constructor
         PreferencesEnvironmentConfigObject();

         /**
          * Base class override
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

         /**
          * Sets using network settings
          */
         void SetUseNetworkSettings();

         /**
          * Returns true if using network settings
          * @return mUseNetworkSettings
          */
         bool GetUseNetworkSettings() const { return mUseNetworkSettings; }

         /**
          * Sets using themed settings
          */
         void SetUseThemedSettings();

         /**
          * Returns true if using themed settings
          * @return mUseNetworkSettings
          */
         bool GetUseThemedSettings() const { return mUseThemedSettings; }

         /**
          * Sets using custom settings
          */
         void SetUseCustomSettings();

         /**
          * Returns true if using custom settings
          * @return mUseNetworkSettings
          */
         bool GetUseCustomSettings() const { return mUseCustomSettings; }

         /**
          * Sets the network hour
          * @param hour The hour in military time
          */
         void SetNetworkHour(int hour);

         /**
          * Gets the network hour
          * @return mNetworkHour
          */
         int GetNetworkHour() const { return mNetworkHour; }

         /**
          * Sets the network min
          * @param min The min in military time
          */
         void SetNetworkMinute(int min);

         /**
          * Gets the network min
          * @return mNetworkMinute
          */
         int GetNetworkMinute() const { return mNetworkMinute; }

         /**
          * Sets the network second
          * @param sec The second in military time
          */
         void SetNetworkSecond(int sec);

         /**
          * Gets the network second
          * @return mNetworkSecond
          */
         int GetNetworkSecond() const { return mNetworkSeconds; }

         /**
          * Sets the cloud cover
          * @param cover The new cloud cover
          */
         void SetCloudCover(dtActors::BasicEnvironmentActor::CloudCoverEnum &cover)
         { mCloudCover = &cover; SetIsUpdated(true); }

         /**
          * Set overload
          */
         void SetCloudCover(const std::string &cover);

         /**
          * Gets the cloud cover
          * @return mCloudCover
          */
         const dtActors::BasicEnvironmentActor::CloudCoverEnum& GetCloudCover() const { return *mCloudCover; }

         /**
          * Sets the visibility
          * @param vis The new visibility
          */
         void SetVisibility(dtActors::BasicEnvironmentActor::VisibilityTypeEnum &vis)
         { mVisibility = &vis; SetIsUpdated(true); }

         /**
          * Set overload
          */
         void SetVisibility(const std::string &vis);

         /**
          * Returns the visibility
          * @return mVisibility
          */
         const dtActors::BasicEnvironmentActor::VisibilityTypeEnum& GetVisibility() const { return *mVisibility; }

         /**
          * Sets the weather theme
          * @param theme The new weather theme
          */
         void SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum &theme) 
         { mWeatherTheme = &theme; SetIsUpdated(true); }

         /**
          * Set overload 
          */
         void SetWeatherTheme(const std::string &theme);

         /**
          * Returns the weather theme
          * @return mWeatherTheme
          */
         const dtActors::BasicEnvironmentActor::WeatherThemeEnum& GetWeatherTheme() const { return *mWeatherTheme; }

         /**
          * Sets the time theme
          * @param theme The new time theme
          */
         void SetTimeTheme(dtActors::BasicEnvironmentActor::TimePeriodEnum &theme) 
         { mTimeTheme = &theme; SetIsUpdated(true); }

         /**
          * Set overload
          */
         void SetTimeTheme(const std::string &theme);

         /**
          * Returns the visibility
          * @return mVisibility
          */
         const dtActors::BasicEnvironmentActor::TimePeriodEnum& GetTimeTheme() const { return *mTimeTheme; }

         /**
          * Sets the Custom hour
          * @param hour The hour in military time
          */
         void SetCustomHour(int hour);

         /**
          * Gets the Custom hour
          * @return mCustomHour
          */
         int GetCustomHour() const { return mCustomHour; }

         /**
          * Sets the Custom min
          * @param min The min in military time
          */
         void SetCustomMinute(int min);

         /**
          * Gets the Custom min
          * @return mCustomMinute
          */
         int GetCustomMinute() const { return mCustomMinute; }

         /**
          * Sets the Custom second
          * @param sec The second in military time
          */
         void SetCustomSecond(int sec);

         /**
          * Gets the Custom second
          * @return mCustomSecond
          */
         int GetCustomSecond() const { return mCustomSeconds; }

         /**
          * Returns the time of day used currently as a string
          * @param a string version of the time of day in HH:MM:SS format
          */
         std::string GetTimeOfDayAsString() const;

         /**
          * Returns the visibility from JSAF
          * @return a string for visibility
          */
         float GetVisibilityDistance() const { return mVisibilityDistance; }

         const std::string& GetPrecipitationAsString() const { return mPrecipitation->GetName(); }

      protected:

         /// Destructor
         virtual ~PreferencesEnvironmentConfigObject();

      private:

         bool mUseNetworkSettings;
         bool mUseThemedSettings;
         bool mUseCustomSettings;
         int mNetworkHour;
         int mNetworkMinute;
         int mNetworkSeconds;
         dtActors::BasicEnvironmentActor::CloudCoverEnum     *mCloudCover;
         dtActors::BasicEnvironmentActor::VisibilityTypeEnum *mVisibility;
         dtActors::BasicEnvironmentActor::WeatherThemeEnum   *mWeatherTheme;
         dtActors::BasicEnvironmentActor::TimePeriodEnum     *mTimeTheme;
         int mCustomHour;
         int mCustomMinute;
         int mCustomSeconds;
         float mVisibilityDistance;
         dtUtil::Enumeration *mPrecipitation;
   };
}
#endif
