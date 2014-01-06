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
#ifndef STEALTH_GAME_ENTRY_POINT_H_
#define STEALTH_GAME_ENTRY_POINT_H_

#include <dtUtil/exception.h>
#include <dtUtil/refcountedbase.h>
#include <SimCore/HLA/BaseHLAGameEntryPoint.h>
#include <StealthViewer/GMApp/Export.h>
#include <StealthViewer/GMApp/StealthHUD.h>

namespace dtGame
{
   class GameApplication;
   class GameManager;
   class DeadReckoningComponent;
   class MachineInfo;
   class EnvironmentActor;
   class ServerLoggerComponent;
   class LogController;
   //class ClientGameManager;
}


namespace dtHLAGM 
{
   class HLAComponent;
}

namespace SimCore 
{
   namespace Actors
   {
      class StealthActor;
   }

   namespace Tools
   {
      class Compass360;
   }
}

namespace StealthGM
{
   class StealthInputComponent;

   class STEALTH_GAME_EXPORT StealthGameEntryPoint: public SimCore::HLA::BaseHLAGameEntryPoint
   {
      public:
         static const std::string CONFIG_HAS_BINOCS;
         static const std::string CONFIG_HAS_COMPASS;
         static const std::string CONFIG_HAS_COMPASS_360;
         static const std::string CONFIG_HAS_GPS;
         static const std::string CONFIG_HAS_NIGHT_VISION;
         static const std::string CONFIG_HAS_MAP_TOOL;

         StealthGameEntryPoint();
         virtual ~StealthGameEntryPoint();

         /**
          * Called to initialize the game application.
          * @param app the current application
          */
         virtual void Initialize(dtABC::BaseABC& app, int argc, char **argv);

         /**
          * Override the method to create the game manager.
          */
         //virtual std::weak_ptr<dtGame::GameManager> CreateGameManager(dtCore::Scene& scene);

         /**
          * Called after all startup related code is run.
          * @param app the current application
          */
         virtual void OnStartup(dtABC::BaseABC& app, dtGame::GameManager& gameManager);

         //virtual void OnShutdown(dtABC::BaseABC& app, dtGame::GameManager& gamemanager);

      protected:
         /**
          * Create and add enabled tools to the input component
          * and the HUD toolbar
          */
         virtual void InitializeTools(dtGame::GameManager& gm);

         virtual void HLAConnectionComponentSetup(dtGame::GameManager& gm);

      private:
         //std::shared_ptr<dtGame::DeadReckoningComponent> mDrComp;
         std::weak_ptr<SimCore::Actors::StealthActor> mStealth;

         std::shared_ptr<StealthHUD> mHudGUI;

         std::shared_ptr<dtGame::LogController> mLogController;
         std::shared_ptr<dtGame::ServerLoggerComponent> mServerLogger;
         bool mEnableLogging;
         bool mEnablePlayback;
         bool mHasBinoculars;
         bool mHasCompass;
         bool mHasCompass360;
         bool mHasLRF;
         bool mHasGPS;
         bool mHasNightVis;
         bool mHasMap;

   };

}
#endif
