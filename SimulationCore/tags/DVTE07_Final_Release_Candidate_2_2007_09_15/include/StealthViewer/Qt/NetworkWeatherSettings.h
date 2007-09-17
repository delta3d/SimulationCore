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
   class NetworkWeatherSettings;
}

namespace StealthQt
{
   class NetworkWeatherSettings : public QDialog
   {
      public:

         /// Constructor
         NetworkWeatherSettings(QWidget *parent = NULL);

         /// Destructor
         virtual ~NetworkWeatherSettings();

      protected:

         Ui::NetworkWeatherSettings *mUi;
   };
}
