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
   class CustomWeatherSettings;
}

namespace StealthQt
{
   class CustomWeatherSettings : public QDialog
   {
      public:

         /// Constructor
         CustomWeatherSettings(QWidget *parent = NULL);

         /// Destructor
         virtual ~CustomWeatherSettings();

      protected:

         Ui::CustomWeatherSettings *mUi;
   };
}
