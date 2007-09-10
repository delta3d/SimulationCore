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
#ifndef STEALTH_GAME_ENTRY_POINT_H_
#define STEALTH_GAME_ENTRY_POINT_H_

#include <dtUtil/exception.h>
#include <dtCore/refptr.h>
#include <SimCore/BaseGameEntryPoint.h>
#include <StealthViewer/GMApp/Export.h>
#include <StealthViewer/GMApp/StealthHUD.h>

namespace dtDAL
{
   class ActorProxy;
}

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
}

namespace StealthGM
{
   class StealthInputComponent;

   class STEALTH_GAME_EXPORT StealthGameEntryPoint: public SimCore::BaseGameEntryPoint
   {
      public:
         StealthGameEntryPoint();
         virtual ~StealthGameEntryPoint();

         /**
          * Called to initialize the game application.
          * @param app the current application
          */
         virtual void Initialize(dtGame::GameApplication& app, int argc, char **argv);

         /**
          * Override the method to create the game manager.
          */
         virtual dtCore::ObserverPtr<dtGame::GameManager> CreateGameManager(dtCore::Scene& scene);

         /**
          * Called after all startup related code is run.
          * @param app the current application
          */
         virtual void OnStartup();

         virtual void OnShutdown();

      protected:
         /**
          * Create and add enabled tools to the input component
          * and the HUD toolbar
          */
         virtual void InitializeTools();

         virtual void HLAConnectionComponentSetup();

      private:
         //dtCore::RefPtr<dtGame::DeadReckoningComponent> mDrComp;
         dtCore::ObserverPtr<SimCore::Actors::StealthActor> mStealth;
         //std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > mActors;

         dtCore::RefPtr<StealthHUD> mHudGUI;

         dtCore::RefPtr<dtGame::LogController> mLogController;
         dtCore::RefPtr<dtGame::ServerLoggerComponent> mServerLogger;
         bool mEnableLogging;
         bool mEnablePlayback;
         bool mHasBinoculars;
         bool mHasCompass;
         bool mHasLRF;
         bool mHasGPS;
         bool mHasNightVis;
         bool mHasMap;
   };

}
#endif
