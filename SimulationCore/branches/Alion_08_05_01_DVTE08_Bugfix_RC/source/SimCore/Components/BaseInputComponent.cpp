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
* @author William E. Johnson II
*/
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Components/BaseInputComponent.h>
#include <SimCore/AttachedMotionModel.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/StealthActor.h>
#include <dtABC/application.h>
#include <dtCore/logicalinputdevice.h>
#include <dtCore/deltawin.h>
#include <dtCore/scene.h>
#include <dtCore/system.h>
#include <dtCore/camera.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/coordinates.h>

#include <dtActors/basicenvironmentactorproxy.h>

using dtCore::RefPtr;
using SimCore::MessageType;

namespace SimCore
{
   namespace Components
   {
      ////////////////////////////////////////////////////////////////////
      BaseMouseListener::BaseMouseListener(dtGame::GMComponent &inputComp) : mInputComp(&inputComp)
      {
      }

      ////////////////////////////////////////////////////////////////////
      BaseMouseListener::~BaseMouseListener()
      {
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseMouseListener::HandleButtonPressed(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
      {
         return static_cast<BaseInputComponent&>(*mInputComp).HandleButtonPressed(mouse, button);
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseMouseListener::HandleButtonReleased(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button)
      {
         return static_cast<BaseInputComponent&>(*mInputComp).HandleButtonReleased(mouse, button);
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseMouseListener::HandleButtonClicked(const dtCore::Mouse* mouse, dtCore::Mouse::MouseButton button, int clickCount)
      {
         return false;
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseMouseListener::HandleMouseMoved(const dtCore::Mouse* mouse, float x, float y)
      {
         return false;
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseMouseListener::HandleMouseDragged(const dtCore::Mouse* mouse, float x, float y)
      {
         return false;
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseMouseListener::HandleMouseScrolled(const dtCore::Mouse* mouse, int delta)
      {
         return false;
      }

      ////////////////////////////////////////////////////////////////////
      BaseKeyboardListener::BaseKeyboardListener(dtGame::GMComponent &inputComp) : mInputComp(&inputComp)
      {
      }

      ////////////////////////////////////////////////////////////////////
      BaseKeyboardListener::~BaseKeyboardListener()
      {
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseKeyboardListener::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
      {
         return static_cast<BaseInputComponent&>(*mInputComp).HandleKeyPressed(keyboard, key);
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseKeyboardListener::HandleKeyReleased(const dtCore::Keyboard* keyboard, int key)
      {
         return static_cast<BaseInputComponent&>(*mInputComp).HandleKeyReleased(keyboard, key);
      }

      const std::string BaseInputComponent::DEFAULT_NAME = "Input Component";

      ////////////////////////////////////////////////////////////////////
      BaseInputComponent::BaseInputComponent(const std::string &name) :
      dtGame::GMComponent(name),
         mEntityMagnification(1.0f)
      {
         mLogger = &dtUtil::Log::GetInstance("BaseInputComponent.cpp");
         mMouseListener = new BaseMouseListener(*this);
         mKeyboardListener = new BaseKeyboardListener(*this);
      }

      ////////////////////////////////////////////////////////////////////
      BaseInputComponent::~BaseInputComponent()
      {
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::SetEntityMagnification(float newMag)
      {  
         mEntityMagnification = newMag;
         dtUtil::Clamp(mEntityMagnification, 1.0f, 8.0f);
         RefPtr<dtGame::Message> msg = GetGameManager()->GetMessageFactory().CreateMessage(MessageType::MAGNIFICATION);
         MagnificationMessage &magMsg = static_cast<MagnificationMessage&>(*msg);
         magMsg.SetMagnification(mEntityMagnification);
         magMsg.SetSource(*new dtGame::MachineInfo);
         GetGameManager()->SendMessage(magMsg);
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::SetListeners()
      {
         //enable the keyboard input.
         //dtCore::DeltaWin* win = GetGameManager()->GetApplication().GetWindow();
         //win->GetMouse()->AddMouseListener(mMouseListener.get());
         //win->GetKeyboard()->AddKeyboardListener(mKeyboardListener.get());
         dtABC::Application &app = GetGameManager()->GetApplication();
         app.GetMouse()->AddMouseListener(mMouseListener.get());
         app.GetKeyboard()->AddKeyboardListener(mKeyboardListener.get());
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::ChangeWeatherType()
      {
         dtActors::BasicEnvironmentActor *envActor = dynamic_cast<dtActors::BasicEnvironmentActor*>(GetGameManager()->GetEnvironmentActor());
         if(envActor == NULL)
         {
            LOG_ERROR("The dynamic cast to an EnvironmentActor failed. Can't change weather.");
            return;
         }

         if (envActor->GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CLEAR)
         {
            envActor->SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FAIR);
         } 
         else if (envActor->GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FAIR)
         {
            envActor->SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FOGGY);
         } 
         else if (envActor->GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_FOGGY)
         {
            envActor->SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_RAINY);
         } 
         else if (envActor->GetWeatherTheme() == dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_RAINY)
         {
            envActor->SetWeatherTheme(dtActors::BasicEnvironmentActor::WeatherThemeEnum::THEME_CLEAR);
         } 
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::ChangeLODScale(bool down)
      {
         float originalLODScale = GetGameManager()->GetApplication().GetCamera()->GetOSGCamera()->getLODScale();
         float newZoom = originalLODScale;

         if (down)
            newZoom /= 1.1f;
         else
            newZoom *= 1.1f;

         if (osg::equivalent(newZoom, 1.0f, 0.01f))
            newZoom = 1.0f;

         mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,  "Setting the Level of Detail Scale to %f", newZoom);

         GetGameManager()->GetApplication().GetCamera()->GetOSGCamera()->setLODScale(newZoom);
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseInputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard, int key)
      {
         bool handled = true;
         switch(key)
         {
         case 'x':
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
#ifndef __APPLE__               
               if (app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_L) ||
                   app.GetKeyboard()->GetKeyState(osgGA::GUIEventAdapter::KEY_Alt_R))
#endif
                  app.Quit();
               break;
            }
         case osgGA::GUIEventAdapter::KEY_Escape:
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
               app.GetWindow()->SetFullScreenMode(!app.GetWindow()->GetFullScreenMode());
               break;
            }

         // Shrink the entities (helps visibility of things far away)
         case ',':  
         case '<':
         //case osgGA::GUIEventAdapter::KEY_Page_Down:
            {
               SetEntityMagnification(mEntityMagnification / 2.0f);
               break;
            }

         // Enlarge the entities (helps visibility of things far away)
         case '.':  
         case '>':
         //case osgGA::GUIEventAdapter::KEY_Page_Up:
            {
               SetEntityMagnification(mEntityMagnification * 2.0f);
               break;
            }

         case osgGA::GUIEventAdapter::KEY_F12:
            {
               ChangeWeatherType();
               break;
            }

         case osgGA::GUIEventAdapter::KEY_Home:
            {
               ChangeLODScale(true);
               break;
            }

         case osgGA::GUIEventAdapter::KEY_End:
            {
               ChangeLODScale(false);
               break;
            }

         default:
            // Implemented to get rid of warnings in Linux
            handled = false;
            break;
         }
         return handled;
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::IncrementTime(const int minuteStep)
      {
         dtCore::System::GetInstance().SetSimulationClockTime(            //multiply by 60 to get into seconds and 1 million to get micro seconds
            dtCore::System::GetInstance().GetSimulationClockTime() + dtCore::Timer_t(((minuteStep * 60) * 1000000)));
      }

      void BaseInputComponent::OnAddedToGM()
      {
         SetListeners();
      }
   }
}

