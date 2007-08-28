/*
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2005, BMH Associates, Inc.
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
* @author William E. Johnson II
*/
#include <prefix/dvteprefix-src.h>
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
      bool BaseKeyboardListener::HandleKeyPressed(const dtCore::Keyboard* keyboard,
         Producer::KeyboardKey key,
         Producer::KeyCharacter character)
      {
         return static_cast<BaseInputComponent&>(*mInputComp).HandleKeyPressed(keyboard, key, character);
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseKeyboardListener::HandleKeyReleased(const dtCore::Keyboard* keyboard,
         Producer::KeyboardKey key,
         Producer::KeyCharacter character)
      {
         return static_cast<BaseInputComponent&>(*mInputComp).HandleKeyReleased(keyboard, key, character);
      }

      const std::string &BaseInputComponent::DEFAULT_NAME = "Input Component";

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
         dtCore::DeltaWin* win = GetGameManager()->GetApplication().GetWindow();
         win->GetMouse()->AddMouseListener(mMouseListener.get());
         win->GetKeyboard()->AddKeyboardListener(mKeyboardListener.get());
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::ChangeWeatherType()
      {
         dtActors::BasicEnvironmentActor *envActor = dynamic_cast<dtActors::BasicEnvironmentActor*>(GetGameManager()->GetEnvironmentActor());
         if(envActor == NULL)
         {
            LOG_ERROR("The dynamic cast to an EnvironmentActor failed. Can't change weather.");
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
         float originalLODScale = GetGameManager()->GetApplication().GetCamera()->GetSceneHandler()->GetSceneView()->getLODScale();
         float newZoom = originalLODScale;

         if (down)
            newZoom /= 1.1f;
         else
            newZoom *= 1.1f;

         if (osg::equivalent(newZoom, 1.0f, 0.01f))
            newZoom = 1.0f;

         mLogger->LogMessage(dtUtil::Log::LOG_ALWAYS, __FUNCTION__, __LINE__,  "Setting the Level of Detail Scale to %f", newZoom);

         GetGameManager()->GetApplication().GetCamera()->GetSceneHandler()->GetSceneView()->setLODScale(newZoom);
      }

      ////////////////////////////////////////////////////////////////////
      bool BaseInputComponent::HandleKeyPressed(const dtCore::Keyboard* keyboard,
         Producer::KeyboardKey key,
         Producer::KeyCharacter character)
      {
         bool handled = true;
         switch(key)
         {
         case Producer::Key_X:
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
#ifndef __APPLE__               
               if (app.GetKeyboard()->GetKeyState(Producer::Key_Alt_L) ||
                  app.GetKeyboard()->GetKeyState(Producer::Key_Alt_R))
#endif
                  app.Quit();
               break;
            }
         case Producer::Key_Escape:
            {
               dtABC::Application& app = GetGameManager()->GetApplication();
               app.GetWindow()->SetFullScreenMode(!app.GetWindow()->GetFullScreenMode());
               break;
            }

         case Producer::Key_Page_Up:
            {
               SetEntityMagnification(mEntityMagnification * 2.0f);
               break;
            }

         case Producer::Key_Page_Down:
            {
               SetEntityMagnification(mEntityMagnification / 2.0f);
               break;
            }

         case Producer::Key_F12:
            {
               ChangeWeatherType();
               break;
            }

         case Producer::Key_Home:
            {
               ChangeLODScale(true);
               break;
            }

         case Producer::Key_End:
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
         enum TimeSetBy{ eNONE = 0, eWEATHER_COMP, eENVIRONMENT_ACTOR};

         TimeSetBy set = eNONE;
         int year =0 ,month=0 ,day=0 ,hour=0 ,minute=0 ,sec=0 ;
        
         dtGame::EnvironmentActor* envActor = 0;
         dtGame::EnvironmentActorProxy *ap = GetGameManager()->GetEnvironmentActor();
         SimCore::Components::WeatherComponent* weatherComp = dynamic_cast<SimCore::Components::WeatherComponent*>(GetGameManager()->GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME));
         SimCore::Actors::DayTimeActor* dayTimeActor = 0;        
         
         if( weatherComp != NULL ) 
         { 
            SimCore::Actors::DayTimeActorProxy* dayTimeActorProxy = weatherComp->GetDayTimeActor();
            if(dayTimeActorProxy != NULL)
            {
               dayTimeActor = dynamic_cast<SimCore::Actors::DayTimeActor*>(dayTimeActorProxy->GetActor());   
               if(dayTimeActor != NULL)
               {
                  set = eWEATHER_COMP;

                  sec = dayTimeActor->GetSecond();
                  minute = dayTimeActor->GetMinute(); 
                  hour = dayTimeActor->GetHour() - dayTimeActor->GetPrimeMeridianHourOffset();
                  day = dayTimeActor->GetDay();
                  month = dayTimeActor->GetMonth();
                  year = dayTimeActor->GetYear()+1900;

                  // Ensure the hour value is valid after the compensation for
                  // the Prime Meridian offset.
                  if( hour < 0 ) { hour += 24; }
                  hour %= 24;
               }
            }
         }         
         else if(ap != NULL)
         {           
            dtGame::EnvironmentActorProxy *ap = GetGameManager()->GetEnvironmentActor();
            envActor = dynamic_cast<dtGame::EnvironmentActor*>(&ap->GetGameActor());     
            if(envActor)
            {
               set = eENVIRONMENT_ACTOR;
               envActor->GetTimeAndDate(year,month,day,hour,minute,sec);
            }
         }

         if(set != eNONE)
         {            
            // hack to work around an issue that sometimes, an HLA message will come in
            // to set the time to something nutty, preventing us from changing the time up and down.
            if(year >= 2038) 
               year = 2030;
            else if(year <= 1970) 
               year = 1971;

            minute += minuteStep;
            if(minute <= 0)
            {
               minute += 60;
               hour--;
               if(hour < 0)
                  hour = 23;
            }

            if(minute >= 60)
            {
               minute -= 60;
               hour++;
               if(hour >= 24)
                  hour = 0;
            }

            std::ostringstream oss;
            oss << "Setting Time " << hour << ":" << minute << ":" << sec << std::endl;
            LOG_DEBUG(oss.str());        

            if(set == eWEATHER_COMP)
            {
               dayTimeActor->SetYear(year-1900);
               dayTimeActor->SetMonth(month);
               dayTimeActor->SetDay(day);
               dayTimeActor->SetHour(hour);
               dayTimeActor->SetMinute(minute);
               dayTimeActor->SetSecond(sec);

               weatherComp->UpdateDayTime();
            }
            else if(set == eENVIRONMENT_ACTOR)
            {
               envActor->SetTimeAndDate(year,month,day,hour,minute,sec);
            }
         }

      }

      void BaseInputComponent::OnAddedToGM()
      {
         SetListeners();
      }
   }
}

