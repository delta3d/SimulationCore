/* -*-c++-*-
* Driver Demo
* Copyright (C) 2008, Alion Science and Technology Corporation
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
*
* @author Curtiss Murphy
*/
#ifndef DRIVER_GAME_ENTRY_POINT_H_
#define DRIVER_GAME_ENTRY_POINT_H_

#include <SimCore/HLA/BaseHLAGameEntryPoint.h>
#include <GameAppComponent.h>
#include <DriverExport.h>

namespace dtDAL
{
   class ActorProxy;
}

namespace dtGame
{
   class GameManager;
}

class StealthInputComponent;

namespace DriverDemo
{
   class DRIVER_DEMO_EXPORT DriverGameEntryPoint: public SimCore::HLA::BaseHLAGameEntryPoint
   {
      public:
         typedef SimCore::HLA::BaseHLAGameEntryPoint BaseClass;
         DriverGameEntryPoint();
         virtual ~DriverGameEntryPoint();

         /**
          * Called to initialize the game application.
          * @param app the current application
          */
         virtual void Initialize(dtGame::GameApplication& app, int argc, char **argv);

         /**
          * Override the method to create the game manager.
          */
         //virtual dtCore::ObserverPtr<dtGame::GameManager> CreateGameManager(dtCore::Scene& scene);

         /**
          * Called after all startup related code is run.
          * @param app the current application
          */
         virtual void OnStartup(dtGame::GameApplication& app);

         virtual void InitializeComponents(dtGame::GameManager &gm);

         virtual void HLAConnectionComponentSetup(dtGame::GameManager &gm);

      private:

         static const std::string ApplicationLibraryName;
         dtCore::RefPtr<GameAppComponent> gameAppComponent;

         char **mArgv;
         int mArgc;
   };
}

#endif
