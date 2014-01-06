/* -*-c++-*-
* Driver Demo - GameAppComponent (.cpp & .h) - Using 'The MIT License'
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

#ifndef _DRIVER_GAME_APP_COMPONENT_
#define _DRIVER_GAME_APP_COMPONENT_

// project includes needed
#include <SimCore/Components/BaseGameAppComponent.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>
#include <dtUtil/refcountedbase.h>
#include <osg/ArgumentParser>

namespace dtGame
{
   class GameManager;
   class TickMessage;
   class ActorUpdateMessage;
}

namespace SimCore
{
   namespace Actors
   {
      //class Platform;
      class StealthActor;
   }
}


namespace DriverDemo
{
   class DriverInputComponent;

   ///////////////////////////////////////////////////////
   //    The Component
   ///////////////////////////////////////////////////////
   class GameAppComponent : public SimCore::Components::BaseGameAppComponent
   {
      public:
         static const std::string DEFAULT_NAME;
         static const std::string APPLICATION_NAME;

         static const std::string CMD_LINE_STARTING_POSITION;
         static const std::string CMD_LINE_HAS_NIGHTVISION;
         static const std::string CMD_LINE_MACHINE_ID;
         static const std::string CMD_LINE_VEHICLE_PROTOTYPE_NAME;
         static const std::string CMD_LINE_WEAPON;
         static const std::string CMD_LINE_START_HEADING;

         static const int  LOG_TIME_AMOUNT = 20; // for logging

         /// Constructor
         GameAppComponent(const std::string& name = DEFAULT_NAME);

         /**
         * Processes messages sent from the Game Manager
         * @param The message to process
         * @see dtGame::GameManager
         */
         virtual void ProcessMessage(const dtGame::Message& msg);

         /// initializes command line to parser and sets the options in the command
         /// line component.
         virtual void InitializeCommandLineOptionsAndRead(osg::ArgumentParser* parser);

         /// inits the player for the application
         void InitializePlayer();

         /// inits the vhiecle for app - Uses "Hover_Vehicle" as a default name. Pass in to command line to change.
         SimCore::Actors::BasePhysicsVehicleActor* CreateNewVehicle();

      protected:
         /// Destructor
         virtual ~GameAppComponent(void);

      private:
         std::shared_ptr<SimCore::Actors::StealthActor> mStealth;

         // if this is set to true, we are waiting for a vehicle to join the network
         // with the name of the group sent in.
         bool mWaitForVehicle;
   };
} // namespace
#endif
