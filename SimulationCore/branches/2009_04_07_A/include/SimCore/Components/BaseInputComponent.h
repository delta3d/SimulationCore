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
#ifndef _BASE_INPUT_COMPONENT_H_
#define _BASE_INPUT_COMPONENT_H_

#include <dtGame/gmcomponent.h>

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
      class SIMCORE_EXPORT BaseMouseListener : public dtCore::MouseListener
      {
         public:

            /// Constructor
            BaseMouseListener(dtGame::GMComponent &inputComp);

            /**
             *  Called when a button is pressed.
             * @param mouse the source of the event
             * @param button the button pressed
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsible for using this
             * return value or not.
             */
            virtual bool HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button);

            /**
             *  Called when a button is released.
             * @param mouse the source of the event
             * @param button the button released
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsible for using this
             * return value or not.
             */
            virtual bool HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button);

            /**
             *  Called when a button is clicked.
             * @param mouse the source of the event
             * @param button the button clicked
             * @param clickCount the click count
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsbile for using this
             * return value or not
             */
            virtual bool HandleButtonClicked(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button, int clickCount);

            /**
             *  Called when the mouse pointer is moved.
             * @param mouse the source of the event
             * @param x the x coordinate
             * @param y the y coordinate
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsbile for using this
             * return value or not.
             */
            virtual bool HandleMouseMoved(const dtCore::Mouse* mouse, float x, float y);

            /**
             * Called when the mouse pointer is dragged.
             * @param mouse the source of the event
             * @param x the x coordinate
             * @param y the y coordinate
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsbile for using this
             * return value or not.
             */
            virtual bool HandleMouseDragged(const dtCore::Mouse* mouse, float x, float y);

            /**
             *  Called when the mouse is scrolled.
             * @param mouse the source of the event
             * @param delta the scroll delta (+1 for up one, -1 for down one)
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsbile for using this
             * return value or not.
             */
            virtual bool HandleMouseScrolled(const dtCore::Mouse* mouse, int delta);

         protected:

            /// Destructor
            virtual ~BaseMouseListener();

         private:

            dtCore::RefPtr<dtGame::GMComponent> mInputComp;
      };

      class SIMCORE_EXPORT BaseKeyboardListener : public dtCore::KeyboardListener
      {
         public:

            /// Constructor
            BaseKeyboardListener(dtGame::GMComponent &inputComp);

            /**
             * KeyboardListener override
             * Called when a key is pressed.
             *
             * @param keyboard the source of the event
             * @param key the key pressed
             */
            virtual bool HandleKeyPressed(const dtCore::Keyboard* keyboard, int key);

            /**
             * Called when a key is released.
             *
             * @param keyboard the source of the event
             * @param key the key released
             * @return true if this KeyboardListener handled the event. The
             * Keyboard calling this function is responsbile for using this
             * return value or not.
             */
            virtual bool HandleKeyReleased(const dtCore::Keyboard* keyboard, int key);

            /**
             * Called when a key is typed.
             *
             * @param keyboard the source of the event
             * @param key the key typed
             * @return true if this KeyboardListener handled the event. The
             * Keyboard calling this function is responsbile for using this
             * return value or not.
             */
            virtual bool HandleKeyTyped(const dtCore::Keyboard* keyboard, int key) { return false; }

         protected:

            /// Destructor
            virtual ~BaseKeyboardListener();

         private:

            dtCore::RefPtr<dtGame::GMComponent> mInputComp;
      };

      class SIMCORE_EXPORT BaseInputComponent : public dtGame::GMComponent
      {
         public:

            static const std::string DEFAULT_NAME;

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
             * KeyboardListener override
             * Called when a key is released.
             *
             * @param keyboard the source of the event
             * @param key the key released
             * @return true if this KeyboardListener handled the event. The
             * Keyboard calling this function is responsbile for using this
             * return value or not.
             */
            virtual bool HandleKeyReleased(const dtCore::Keyboard* keyboard, int key) { return false; }

            /**
             * MouseListener override
             * Called when a button is pressed.
             *
             * @param mouse the source of the event
             * @param button the button pressed
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsible for using this
             * return value or not.
             */
            virtual bool HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button) { return false; }

            /**
             * MouseListener override
             * Called when a button is released.
             *
             * @param mouse the source of the event
             * @param button the button released
             * @return true if this MouseListener handled the event. The
             * Mouse calling this function is responsible for using this
             * return value or not.
             */
            virtual bool HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button) { return false; }

            /**
             * Sets the mouse listener and keyboard listener on this input component
             */
            void SetListeners();

            const dtUtil::Coordinates& GetCoordinateConverter() const { return mCoordinateConverter; }
            void SetCoordinateConverter(const dtUtil::Coordinates& converter) { mCoordinateConverter = converter; }

            const std::string& GetFoName() const { return mFoName; }
            void SetFoName(const std::string& name) { mFoName = name; }

            /**
             * Gets the mouse listener of this input component
             * @return mMouseListener
             */
            BaseMouseListener* GetMouseListener() { return mMouseListener.get(); }

            /**
             * Gets the keyboard listener of this input component
             * @return mKeyboardListener
             */
            BaseKeyboardListener* GetKeyboardListener() { return mKeyboardListener.get(); }

            float GetEntityMagnification() const { return mEntityMagnification; }
            void SetEntityMagnification(float newMag);

            /**
             * This method increments (or decrements) the current sim time.  Adjusts the time of day
             * which changes the sun position, light value, moon, and the rest. Typically, linked to a
             * UI or hotkey or something.
             */
            void IncrementTime(const int minuteStep);

            virtual void OnAddedToGM();

            // provide an access for the stealth actor. This is used by the Stealth Viewer to check the status of the stealth camera.
            // DO NOT HOLD ONTO THIS MEMORY
            SimCore::Actors::StealthActor* GetStealthActor();
            void SetStealthActor(SimCore::Actors::StealthActor* stealth);


         protected:
            //Cycles the weather type based on the predefined types in the basic environment actor
            void ChangeWeatherType();
            //Increases/decreases the Level of Detail scale by 10%
            void ChangeLODScale(bool down);

            dtUtil::Log& GetLogger();

            /// Destructor
            virtual ~BaseInputComponent();

            dtCore::RefPtr<SimCore::AttachedMotionModel> mAttachedMM;
            dtCore::RefPtr<dtDAL::ActorProxy> mTerrainActor;
            dtUtil::Coordinates mCoordinateConverter;
         private:
            dtCore::RefPtr<BaseKeyboardListener> mKeyboardListener;
            dtCore::RefPtr<BaseMouseListener> mMouseListener;
            std::string mFoName;
            dtCore::RefPtr<SimCore::Actors::StealthActor> mStealthActor;
            float mEntityMagnification;
            std::string mTerrainActorName;
            dtUtil::Log* mLogger;
      };
   }
}

#endif


