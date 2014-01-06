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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/BaseGameAppComponent.h>

#include <dtGame/gamemanager.h>
#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/message.h>
#include <dtGame/actorupdatemessage.h>

#include <SimCore/CommandLineObject.h>
#include <SimCore/IGExceptionEnum.h>
#include <SimCore/Utilities.h>

///////////////////////////////////
// for command line parsing
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osgUtil/CullVisitor>
#include <osgUtil/SceneView>
#include <osgUtil/StateGraph>

///////////////////////////////////

namespace SimCore
{
   namespace Components
   {
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string BaseGameAppComponent::DEFAULT_NAME = "BaseGameAppComponent";
      const std::string BaseGameAppComponent::CMD_LINE_PROJECTPATH            = "ProjectPath";
      const std::string BaseGameAppComponent::CMD_LINE_STATISTICS_INTERVAL    = "StatisticsInterval";
      const std::string BaseGameAppComponent::CMD_LINE_MAP_NAME               = "MapName";

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      BaseGameAppComponent::BaseGameAppComponent(const std::string& name)
         : dtGame::GMComponent(name)
      {
         mCommandLineObject = new SimCore::CommandLineObject();
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      BaseGameAppComponent::~BaseGameAppComponent(void)
      {
         mCommandLineObject = NULL;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void BaseGameAppComponent::ProcessMessage(const dtGame::Message& msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
            ProcessTick(static_cast<const dtGame::TickMessage&>(msg));
         }

         /*else if(msg.GetMessageType() == dtGame::MessageType::TICK_REMOTE){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_PUBLISHED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED){}
         else if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED){}*/
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void BaseGameAppComponent::ProcessTick(const dtGame::TickMessage &msg)
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void BaseGameAppComponent::InitializeCommandLineOptionsAndRead(osg::ArgumentParser* parser)
      {
         if(parser == NULL)
         {
            LOG_ERROR("Parser is NULL in InitializeCommandLineOptionsAndRead\
                      , no initing will occur");
            return;
         }

         parser->getApplicationUsage()->addCommandLineOption("--projectPath", "The path (either relative or absolute) to the project context you wish to use. This defaults to the current working directory.");
         parser->getApplicationUsage()->addCommandLineOption("--mapName", "The name of the map to load in. This must be a map that is located within the project path specified");
         parser->getApplicationUsage()->addCommandLineOption("--statisticsInterval", "The interval the game manager will use to print statistics, in seconds");

         int tempInt = 0;
         std::string tempString;

         if (parser->read("--projectPath", tempString))
         {
            dtCore::RefPtr<dtDAL::NamedStringParameter> parameter
               = new dtDAL::NamedStringParameter(CMD_LINE_PROJECTPATH, tempString);
            mCommandLineObject->AddParameter(parameter.get());
         }

         if(parser->read("--statisticsInterval", tempInt))
         {
            dtCore::RefPtr<dtDAL::NamedIntParameter> parameter
               = new dtDAL::NamedIntParameter(CMD_LINE_STATISTICS_INTERVAL, tempInt);
            mCommandLineObject->AddParameter(parameter.get());
         }

         if(parser->read("--mapName", tempString))
         {
            dtCore::RefPtr<dtDAL::NamedStringParameter> parameter
               = new dtDAL::NamedStringParameter(CMD_LINE_MAP_NAME, tempString);
            mCommandLineObject->AddParameter(parameter.get());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BaseGameAppComponent::LoadMaps(const std::string& inMapName)
      {
         // This behavior is similar to what happens in the HLAConnectionComponent.
         // Apps that use the HLAConnectionComponent should NOT need to call this.

         // look up the map name from the command line args. If none supplied, we error out.
         std::string mapName = inMapName;
         if (inMapName.empty())
         {
            // The BaseGameEntryPoint seems to pull mapname off of the command line options
            // so it usually isn't here by this time, but we will check as a last resort.
            SimCore::CommandLineObject* commandLineObject = GetCommandLineObject();
            const dtDAL::NamedStringParameter* mapNameParam = dynamic_cast<const dtDAL::NamedStringParameter*>
               (commandLineObject->GetParameter(CMD_LINE_MAP_NAME));
            if( mapNameParam != NULL )
            {
               mapName = mapNameParam->GetValue();
            }
         }

         if (mapName.empty())
         {
            throw SimCore::IGException("No base Map Name found. Please specify a map in the command line options with --mapName MyMap.", __FILE__, __LINE__);
         }

         SimCore::Utils::LoadMaps(*GetGameManager(), mapName);
      }



   }
}
