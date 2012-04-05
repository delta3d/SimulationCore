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
#include <dtGame/gmcomponent.h>

#include <StealthViewer/GMApp/Export.h>

namespace StealthGM
{
   // Forward declarations
   class ConfigurationObjectInterface;

   class STEALTH_GAME_EXPORT ViewerConfigComponent : public dtGame::GMComponent
   {
      public:

         static const std::string DEFAULT_NAME;

         /// Constructor
         ViewerConfigComponent(const std::string &name = DEFAULT_NAME);

         /**
          * Override to process messages
          * @param msg The message to process
          */
         virtual void ProcessMessage(const dtGame::Message &msg);

         /**
          * Adds a configurable object to the list
          * @param object The new config object
          */
         void AddConfigObject(ConfigurationObjectInterface &object);

         /**
          * Removes a configurable object from the list
          * @param object The object to remove
          */
         void RemoveConfigObject(ConfigurationObjectInterface &object);

      protected:

         /// Destructor
         virtual ~ViewerConfigComponent();

      private:

         std::vector<dtCore::RefPtr<ConfigurationObjectInterface> > mConfigurationObjects;
   };
}
