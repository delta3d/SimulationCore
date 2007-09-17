/*
* DVTE Stealth Viewer
* Copyright (C) 2006, Alion Science and Technology. 
*
* @author Eddie Johnson
*/
#include <StealthViewer/Qt/ThemedWeatherSettings.h>
#include <StealthViewer/Qt/ui_ThemedWeatherSettingsUi.h>

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
