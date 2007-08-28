/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology. 
 *
 * @author Eddie Johnson
 */
#include <StealthQt/CustomWeatherSettings.h>
#include <StealthQt/ui_CustomWeatherSettingsUi.h>

namespace StealthQt
{
   CustomWeatherSettings::CustomWeatherSettings(QWidget *parent) :
      QDialog(parent), 
      mUi(new Ui::CustomWeatherSettings)
   {
      mUi->setupUi(this);
   }

   CustomWeatherSettings::~CustomWeatherSettings()
   {
      delete mUi;
      mUi = NULL;
   }
}
