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
#ifndef _BASE_GAME_ENTRY_POINT_H_
#define _BASE_GAME_ENTRY_POINT_H_

#include <dtGame/gameentrypoint.h>
#include <SimCore/Export.h>
#include <dtCore/refptr.h>
#include <dtDAL/actorproxy.h>
#include <osg/Vec3>
#include <string>

namespace osg
{
   class ArgumentParser;
}

namespace dtGame
{
   class GameManager;
   class GameApplication;
}

namespace dtDAL
{
   class BaseActorObject;
}

namespace SimCore
{
   class SIMCORE_EXPORT BaseGameEntryPoint : public dtGame::GameEntryPoint
   {
      public:

         static const std::string PROJECT_CONTEXT_DIR;
         static const std::string PROJECT_CONFIG_FILE;
         static const std::string LIBRARY_NAME;

         /**
          * The clipping plane used by this app.
          * The application should not have to rely
          * on binocular near clipping plane.
          */
         static const float PLAYER_NEAR_CLIP_PLANE;

         /**
          * The clipping plane used by this app.
          * The application should not have to rely
          * on binocular near clipping plane.
          */
         static const float PLAYER_FAR_CLIP_PLANE;

         /// Name of the config property pointing to the directory holding the project context.
         static const std::string CONFIG_PROP_PROJECT_CONTEXT_PATH;
         /// Property enable shader-based human/character skinning
         static const std::string CONFIG_PROP_USE_GPU_CHARACTER_SKINNING;
         static const std::string CONFIG_PROP_DEVELOPERMODE;
         static const std::string CONFIG_PROP_ASPECT_RATIO;
         static const std::string CONFIG_PROP_MUNITION_MAP;
         static const std::string CONFIG_PROP_MUNITION_CONFIG_FILE;
         static const std::string CONFIG_PROP_MUNITION_DEFAULT;

         /// Constructor
         BaseGameEntryPoint();

         /**
          * Called to initialize the game application.
          * @param app the current application
          * @param argc number of startup arguments.
          * @param argv array of string pointers to the arguments.
          * @throwns dtUtil::Exception if initialization fails.
          */
         virtual void Initialize(dtGame::GameApplication& app, int argc, char **argv);

         /**
          * Called after all startup related code is run.
          * @param gameManager The game manager to init
          */
         virtual void OnStartup(dtGame::GameApplication &app);

         /// May be overridden to allow subclassed to add components
         virtual void InitializeComponents(dtGame::GameManager &gm) {};

         /**
          * called from external to 'end' the parser so anyone can
          * mess with the parser wherever they need and not tied into
          * an exact way of calling the parser shutdown.
          */
         virtual void FinalizeParser();

         /// @return true if this GM app is running inside of a GUI, like the Stealth Viewer Qt UI.
         bool IsUIRunning() { return mIsUIRunning; }

      protected:

         /// reads the values of command line parameters and config options set the project context
         void AssignProjectContext(dtGame::GameManager &gm);
         /// if the UI is not enabled, will load the map specified on the command line.
         void PreLoadMap();

         /**
          * Reads the aspect ratio first from the command line setting, then from the config. Finally
          * it will read the current window setting and asign the value to the closest of 1.33 or 1.6.
          * warning. setting to the aspect ratio to anything other than 1.33 or 1.6 will result
          * in incorrect results for the binoculars and laser range finder.
          */
         void AssignAspectRatio(dtGame::GameApplication &app);


         /// Destructor
         virtual ~BaseGameEntryPoint();

         osg::ArgumentParser* parser;

         dtCore::RefPtr<dtDAL::BaseActorObject> terrainActor;

         std::string mMapName;
         std::string mProjectPath;
         float mAspectRatio;
         float mLingeringShotEffectSecs;
         //int mStatisticsInterval;
         osg::Vec3 mStartPos;
         bool mMissingRequiredCommandLineOption;

      private:
         bool mIsUIRunning;
   };
}
#endif
