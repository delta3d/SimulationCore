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
* Eddie Johnson, David Guthrie, Curtiss Murphy
*/
#ifndef _BASE_INPUT_COMPONENT_H_
#define _BASE_INPUT_COMPONENT_H_

#include <dtGame/gmcomponent.h>
#include <dtGame/baseinputcomponent.h>

#include <dtUtil/coordinates.h>

#include <SimCore/Export.h>

#include <dtCore/keyboard.h>
#include <dtCore/mouse.h>

#include <SimCore/Actors/StealthActor.h>

namespace dtUtil
{
   class Log;
}

namespace SimCore
{
   class AttachedMotionModel;

  namespace Components
   {

      class SIMCORE_EXPORT BaseInputComponent : public dtGame::BaseInputComponent //public dtGame::GMComponent
      {
         public:
            typedef dtGame::BaseInputComponent BaseClass;

            static const std::string DEFAULT_NAME;

            enum TestWeatherMode { TEST_WEATHER_CLEAR = 0, TEST_WEATHER_CLOUDY, 
               TEST_WEATHER_RAIN_LIGHT, TEST_WEATHER_RAIN_HEAVY, 
               TEST_WEATHER_SNOW_LIGHT, TEST_WEATHER_SNOW_HEAVY };

            /// Constructor
            BaseInputComponent(const std::string &name = DEFAULT_NAME);

            /**
             * KeyboardListener override
             * Called when a key is pressed.
             *
             * @param keyboard the source of the event
             * @param key the key pressed
             */
            virtual bool HandleKeyPressed(const dtCore::Keyboard* keyboard, int key);

            /**
             * Sets the mouse listener and keyboard listener on this input component
             */
            //void SetListeners();

            const dtUtil::Coordinates& GetCoordinateConverter() const { return mCoordinateConverter; }
            void SetCoordinateConverter(const dtUtil::Coordinates& converter) { mCoordinateConverter = converter; }

            const std::string& GetFoName() const { return mFoName; }
            void SetFoName(const std::string& name) { mFoName = name; }

            float GetEntityMagnification() const { return mEntityMagnification; }
            void SetEntityMagnification(float newMag);

            /**
             * This method increments (or decrements) the current sim time.  Adjusts the time of day
             * which changes the sun position, light value, moon, and the rest. Typically, linked to a
             * UI or hotkey or something.
             */
            void IncrementTime(const int minuteStep);

            virtual void OnAddedToGM();

            virtual void ProcessMessage(const dtGame::Message& msg);

            // provide an access for the stealth actor. This is used by the Stealth Viewer to check the status of the stealth camera.
            // DO NOT HOLD ONTO THIS MEMORY
            SimCore::Actors::StealthActor* GetStealthActor();
            void SetStealthActor(SimCore::Actors::StealthActor* stealth);

            /**
             * Moves to the next on-screen system timing statistics if dev mode is enabled via
             * config option.  This is a helper method that exists so apps can easily map a key to this.
             */
            void SetNextStatisticsIfDevMode();

            /**
             * Moves to the next physics debug draw mode if dev mode is enabled via
             * config option.  This is a helper method that exists so apps can easily map a key to this.
             */
            void SetNextPhysicsDebugDrawIfDevMode();

            /**
             * Reloads and and reassigns all shaders from disk if dev mode is enabled via
             * config option.  This is a helper method that exists so apps can easily map a key to this.
             */
            void ReloadShadersIfDevMode();

            /** 
             * Basic test behavior for weather. Calling this multiple times cycles 
             * through states of TestWeatherMode.
             */
            void ToggleWeatherStateIfDevMode();

         protected:
            //Increases/decreases the Level of Detail scale by 10%
            void ChangeLODScale(bool down);

            dtUtil::Log& GetLogger();

            /// Destructor
            virtual ~BaseInputComponent();

            dtCore::RefPtr<SimCore::AttachedMotionModel> mAttachedMM;
            dtCore::RefPtr<dtDAL::BaseActorObject> mTerrainActor;
            dtUtil::Coordinates mCoordinateConverter;
         private:
            std::string mFoName;
            dtCore::ObserverPtr<SimCore::Actors::StealthActor> mStealthActor;
            float mEntityMagnification;
            std::string mTerrainActorName;
            dtUtil::Log* mLogger;
            TestWeatherMode mTestWeatherMode;
      };
   }
}

#endif
