/*
* DVTE Stealth Viewer
* Copyright (C) 2006, Alion Science and Technology. 
*
* @author Eddie Johnson
*/
#include <StealthQt/ThemedWeatherSettings.h>
#include <StealthQt/ui_ThemedWeatherSettingsUi.h>

namespace StealthQt
{
   ThemedWeatherSettings::ThemedWeatherSettings(QWidget *parent) :
      QDialog(parent), 
      mUi(new Ui::ThemedWeatherSettings)
   {
      mUi->setupUi(this);
   }

ThemedWeatherSettings::~ThemedWeatherSettings()
   {
      delete mUi;
      mUi = NULL;
   }
}
