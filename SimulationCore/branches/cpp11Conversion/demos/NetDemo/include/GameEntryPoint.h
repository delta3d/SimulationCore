/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
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
* @author David Guthrie
*/
#ifndef RES_GAME_ENTRY_POINT
#define RES_GAME_ENTRY_POINT

#include <DemoExport.h>
#include <SimCore/BaseGameEntryPoint.h>

namespace NetDemo
{
   class NETDEMO_EXPORT GameEntryPoint : public SimCore::BaseGameEntryPoint
   {
      public:

         typedef SimCore::BaseGameEntryPoint BaseClass;

         static const std::string APP_NAME;

         GameEntryPoint();

         virtual ~GameEntryPoint();

         /**
          * Called to initialize the game application.
          * @param app the current application
          */
         virtual void Initialize(dtABC::BaseABC& app, int argc, char **argv);

         /**
          * Called after all startup related code is run.
          * @param app the current application
          */
         virtual void OnStartup(dtABC::BaseABC& app, dtGame::GameManager& gameManager);

         virtual void InitializeComponents(dtGame::GameManager& gm);

      private:
         char **mArgv;
         int mArgc;
         bool mIsServer;
   };
}

#endif
