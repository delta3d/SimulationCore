/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
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
