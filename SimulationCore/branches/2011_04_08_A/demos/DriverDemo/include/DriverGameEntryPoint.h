/* -*-c++-*-
* Driver Demo - DriverGameEntryPoint (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @author Curtiss Murphy
*/
#ifndef DRIVER_GAME_ENTRY_POINT_H_
#define DRIVER_GAME_ENTRY_POINT_H_

#ifdef BUILD_HLA
#define DRIVER_DEMO_BASE_GAME_ENTRY_CLASS SimCore::HLA::BaseHLAGameEntryPoint
#include <SimCore/HLA/BaseHLAGameEntryPoint.h>
#else
#define DRIVER_DEMO_BASE_GAME_ENTRY_CLASS SimCore::BaseGameEntryPoint
#include <SimCore/BaseGameEntryPoint.h>
#endif

#include <GameAppComponent.h>
#include <DriverExport.h>

namespace dtGame
{
   class GameManager;
}

class StealthInputComponent;

namespace DriverDemo
{
   class DRIVER_DEMO_EXPORT DriverGameEntryPoint: public DRIVER_DEMO_BASE_GAME_ENTRY_CLASS
   {
      public:
         typedef DRIVER_DEMO_BASE_GAME_ENTRY_CLASS BaseClass;
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

         virtual void InitializeComponents(dtGame::GameManager& gm);

      private:

         virtual void SetupClientServerNetworking(dtGame::GameManager& gm);

         static const std::string ApplicationLibraryName;
         dtCore::RefPtr<GameAppComponent> gameAppComponent;

         char **mArgv;
         int mArgc;
   };
}

#endif
