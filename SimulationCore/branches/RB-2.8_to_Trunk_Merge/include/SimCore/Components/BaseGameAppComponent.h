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
 * @author Allen Danklefsen, Curtiss Murphy
 */

#ifndef _BASE_GAMEAPP_COMPONENT_
#define _BASE_GAMEAPP_COMPONENT_

// project includes needed
#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>
#include <dtUtil/deprecationmgr.h>

namespace dtGame
{
   class Message;
   class TickMessage;
}

namespace osg
{
   class ArgumentParser;
}

namespace SimCore
{
   class CommandLineObject;

   namespace Components
   {
      ///////////////////////////////////////////////////////
      //    THIS IS DEPRECATED. PLEASE USE GameStateComponent INSTEAD
      ///////////////////////////////////////////////////////
      class SIMCORE_EXPORT BaseGameAppComponent : public dtGame::GMComponent
      {
      public:
         static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;

         static const std::string DEFAULT_NAME;
         static const std::string CMD_LINE_PROJECTPATH;
         static const std::string CMD_LINE_STATISTICS_INTERVAL;
         static const std::string CMD_LINE_MAP_NAME;

         /// Constructor
         DEPRECATE_FUNC BaseGameAppComponent(dtCore::SystemComponentType& type = *TYPE);

         /**
          * Processes messages sent from the Game Manager
          * @param The message to process
          * @see dtGame::GameManager
          */
         virtual void ProcessMessage(const dtGame::Message &msg);

         SimCore::CommandLineObject* GetCommandLineObject() {return mCommandLineObject.get();}

         /// initializes command line to parser and sets the options in the command
         /// line object.
         virtual void InitializeCommandLineOptionsAndRead(osg::ArgumentParser* parser);

         // A default load map behavior. Similar to OnConnect() in the HLAConnectionComponent
         // Only needed if your app does not have HLA. Call from your GameEntryPoint
         void LoadMaps(const std::string& inMapName);


      protected:
         /// Destructor
         virtual ~BaseGameAppComponent(void);

         virtual void ProcessTick(const dtGame::TickMessage &msg);

      private:
         dtCore::RefPtr<SimCore::CommandLineObject> mCommandLineObject;
      };
   } // namespace
} // namespace
#endif
