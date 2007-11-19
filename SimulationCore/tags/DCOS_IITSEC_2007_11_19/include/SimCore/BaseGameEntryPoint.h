/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
 * @author Eddie Johnson
 */
#ifndef _BASE_GAME_ENTRY_POINT_H_
#define _BASE_GAME_ENTRY_POINT_H_

#include <dtGame/gameentrypoint.h>
#include <SimCore/Export.h>

namespace osg
{
   class ArgumentParser;
}

namespace dtGame
{
   class GameManager;
   class GameApplication;
}

namespace dtHLAGM
{
   class HLAComponent;
}

namespace dtDAL
{
   class ActorProxy;
}

namespace SimCore
{
   class SIMCORE_EXPORT BaseGameEntryPoint : public dtGame::GameEntryPoint
   {
      public:

         static const std::string PROJECT_CONTEXT_DIR;
         static const std::string LIBRARY_NAME;
         
         // The clipping plane used by this app.
         // The application should not have to rely
         // on binocular near clipping plane.
         static const float PLAYER_NEAR_CLIP_PLANE;
         static const float PLAYER_FAR_CLIP_PLANE;

         /// Name of the config property pointing to the directory holding the project context.
         static const std::string CONFIG_PROP_PROJECT_CONTEXT_PATH;
         /// Property enable shader-based human/character skinning
         static const std::string CONFIG_PROP_USE_GPU_CHARACTER_SKINNING;
         static const std::string CONFIG_PROP_DEVELOPERMODE;
         static const std::string CONFIG_PROP_GMSTATS;

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
          * Override the method to create the game manager.
          */
         //virtual dtCore::ObserverPtr<dtGame::GameManager> CreateGameManager(dtCore::Scene& scene);

         /**
          * Called after all startup related code is run.
          * @param gameManager The game manager to init
          */
         virtual void OnStartup(dtGame::GameApplication &app);

         /// called from external to 'end' the parser so anyone can
         /// mess with the parser wherever they need and not tied into
         /// an exact way of calling the parser shutdown.
         virtual void FinalizeParser();

         /// Initialize the components of the class
         virtual void InitializeComponents();

      protected:
         
         /// reads the values of command line parameters and config options set the project context
         void AssignProjectContext(dtGame::GameManager &gm);
         /// if the UI is not enabled, will load the map specified on the command line.
         void PreLoadMap();

         /// called virtual for loading specific maps
         virtual void HLAConnectionComponentSetup(dtGame::GameManager &gm);

         /// creates and configures the HLA Component.
         virtual dtCore::RefPtr<dtHLAGM::HLAComponent> CreateAndSetupHLAComponent(dtGame::GameManager &gm);

         /// Destructor
         virtual ~BaseGameEntryPoint();

         virtual void CreateEnvironment(dtGame::GameManager &gameManager);

         osg::ArgumentParser *parser;

         dtCore::RefPtr<dtDAL::ActorProxy> terrainActor;

         std::string mMapName;
         std::string mFederationExecutionName;
         std::string mFederateName;
         std::string mFedFileResource;
         std::string mFedFileName;
         std::string mFedMappingFileName;
         std::string mProjectPath;
         float mAspectRatio;
         float mLingeringShotEffectSecs;
         //int mStatisticsInterval;
         osg::Vec3 mStartPos;
         bool mIsUIRunning;
         bool mMissingRequiredCommandLineOption;
   };
}
#endif
