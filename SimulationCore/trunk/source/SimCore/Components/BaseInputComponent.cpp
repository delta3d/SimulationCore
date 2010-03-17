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
#include <prefix/SimCorePrefix.h>
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
#include <dtGame/messagefactory.h>

using dtCore::RefPtr;
using SimCore::MessageType;

namespace SimCore
{
   namespace Components
   {

      const std::string BaseInputComponent::DEFAULT_NAME = "Input Component";

      ////////////////////////////////////////////////////////////////////
      BaseInputComponent::BaseInputComponent(const std::string& name)
         : dtGame::BaseInputComponent(name)
      , mEntityMagnification(1.0f)
      {
         mLogger = &dtUtil::Log::GetInstance("BaseInputComponent.cpp");
         //mMouseListener = new BaseMouseListener(*this);
         //mKeyboardListener = new BaseKeyboardListener(*this);
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
      dtUtil::Log& BaseInputComponent::GetLogger()
      {
         return *mLogger;
      }

      ////////////////////////////////////////////////////////////////////
      SimCore::Actors::StealthActor* BaseInputComponent::GetStealthActor()
      {
         return mStealthActor.get();
      }

      ////////////////////////////////////////////////////////////////////
      void BaseInputComponent::SetStealthActor(SimCore::Actors::StealthActor* stealth)
      {
         mStealthActor = stealth;
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

         case osgGA::GUIEventAdapter::KEY_Page_Up:
            {
               SetEntityMagnification(mEntityMagnification * 2.0f);
               break;
            }

         case osgGA::GUIEventAdapter::KEY_Page_Down:
            {
               SetEntityMagnification(mEntityMagnification / 2.0f);
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
         BaseClass::OnAddedToGM();
         //SetListeners();
      }

      void BaseInputComponent::ProcessMessage(const dtGame::Message& msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOAD_BEGIN)
         {
            SetStealthActor(NULL);
         }
      }
   }
}

