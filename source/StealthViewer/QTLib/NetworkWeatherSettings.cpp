/*
* DVTE Stealth Viewer
* Copyright (C) 2006, Alion Science and Technology. 
*
* @author Eddie Johnson
*/
#include <StealthQt/NetworkWeatherSettings.h>
#include <StealthQt/ui_NetworkWeatherSettingsUi.h>

namespace StealthQt
{
   NetworkWeatherSettings::NetworkWeatherSettings(QWidget *parent) :
      QDialog(parent), 
      mUi(new Ui::NetworkWeatherSettings)
   {
      mUi->setupUi(this);
   }

   NetworkWeatherSettings::~NetworkWeatherSettings()
   {
      delete mUi;
      mUi = NULL;
   }
}
