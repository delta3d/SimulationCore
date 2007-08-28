/*
* DVTE Stealth Viewer
* Copyright (C) 2006, Alion Science and Technology. 
*
* @author Eddie Johnson
*/
#include <QtGui/QDialog>

// Forward declarations
namespace Ui
{
   class ThemedWeatherSettings;
}

namespace StealthQt
{
   class ThemedWeatherSettings : public QDialog
   {
      public:

         /// Constructor
         ThemedWeatherSettings(QWidget *parent = NULL);

         /// Destructor
         virtual ~ThemedWeatherSettings();

      protected:

         Ui::ThemedWeatherSettings *mUi;
   };
}
