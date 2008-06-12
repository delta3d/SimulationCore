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
 * @author Curtiss Murphy
 */
#include <prefix/SimCorePrefix-src.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>

#include <SimCore/Tools/Binoculars.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/WeatherComponent.h>

#include <dtGame/gamemanager.h>

#include <dtCore/camera.h>

#include <dtABC/application.h>

#include <dtUtil/version.h>
 
namespace StealthGM
{
   IMPLEMENT_ENUM(PreferencesGeneralConfigObject::AttachMode);
   const PreferencesGeneralConfigObject::AttachMode PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON("First Person");
   const PreferencesGeneralConfigObject::AttachMode PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON("Third Person");

   IMPLEMENT_ENUM(PreferencesGeneralConfigObject::PerformanceMode);
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS("Best Graphics");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BETTER_GRAPHICS("Better Graphics");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::DEFAULT("Default");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BETTER_SPEED("Better Speed");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BEST_SPEED("Best Speed");

   PreferencesGeneralConfigObject::PreferencesGeneralConfigObject() :
      mAttachMode(&PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON), 
      mEnableCameraCollision(true), 
      mPerformanceMode(&PreferencesGeneralConfigObject::PerformanceMode::DEFAULT), 
      mLODScale(1.0f), 
      mNearClippingPlane(SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE),
      mFarClippingPlane(SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE),
      mShowAdvancedOptions(false), 
      mAttachProxy(NULL), 
      mReconnectOnStartup(true), 
      mAutoRefreshEntityInfo(true),
      mDetachFromActor(false),
      mInputComponent(NULL)
   {

   }

   PreferencesGeneralConfigObject::~PreferencesGeneralConfigObject()
   {

   }

   void PreferencesGeneralConfigObject::ApplyChanges(dtGame::GameManager &gameManager)
   {
      if(!IsUpdated())
         return;

      dtGame::GMComponent *component = 
         gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME);
      mInputComponent = static_cast<StealthInputComponent*>(component);

      // Update attach mode
      mInputComponent->ChangeMotionModels(GetAttachMode() == AttachMode::FIRST_PERSON);

      // Update performance

      // Update motion model
      mInputComponent->EnableCameraCollision(GetEnableCameraCollision());

      // Update rendering options
      dtCore::Camera *camera = gameManager.GetApplication().GetCamera();
      
      // Send the Near/Far clipping plane to the weather component
      dtGame::GMComponent *weatherGMComp = gameManager.GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME);
      if(weatherGMComp != NULL)
      {
         SimCore::Components::WeatherComponent &weatherComp = static_cast<SimCore::Components::WeatherComponent&>(*weatherGMComp);
         weatherComp.SetNearClipPlane(GetNearClippingPlane());
         weatherComp.SetFarClipPlane(GetFarClippingPlane());
         weatherComp.UpdateFog();
      }
      // no weather component, so we update the clip planes by hand
      else
      {
         camera->SetPerspective(camera->GetVerticalFov(), camera->GetAspectRatio(), 
            GetNearClippingPlane(), GetFarClippingPlane());
      }

      // Updated the LOD scale
      camera->GetOSGCamera()->setLODScale(GetLODScale());

      if(mAttachProxy != NULL && mInputComponent->GetStealthActor() != NULL)
      {
         if(mAttachProxy->GetId() == mInputComponent->GetStealthActor()->GetUniqueId())
         {
            LOG_ERROR("The stealth actor cannot attach to itself.");
         }
         else
         {
            dtCore::RefPtr<dtGame::Message> msg = 
               gameManager.GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);

            SimCore::AttachToActorMessage &ataMsg = 
               static_cast<SimCore::AttachToActorMessage&>(*msg);

            ataMsg.SetAboutActorId(mInputComponent->GetStealthActor()->GetUniqueId());
            ataMsg.SetAttachToActor(mAttachProxy->GetId());

            gameManager.SendMessage(ataMsg);
         }

         mAttachProxy = NULL;
      }

      // DETACH - Send an attach message with no actor
      if (mDetachFromActor && mInputComponent->GetStealthActor() != NULL) 
      {
         dtCore::RefPtr<dtGame::Message> msg = 
            gameManager.GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);
         SimCore::AttachToActorMessage &ataMsg = static_cast<SimCore::AttachToActorMessage&>(*msg);
         ataMsg.SetAboutActorId(mInputComponent->GetStealthActor()->GetUniqueId());
         ataMsg.SetAttachToActor(dtCore::UniqueId(""));

         gameManager.SendMessage(ataMsg);

         mDetachFromActor = false;
      }

      SetIsUpdated(false);
   }

   //////////////////////////////////////////////////////////////////////////
   bool PreferencesGeneralConfigObject::IsStealthActorCurrentlyAttached()
   {
      bool result = false;

      if (mInputComponent.valid() && mInputComponent->GetStealthActor() != NULL)
      {
         result = mInputComponent->GetStealthActor()->IsAttachedToActor();
      }

      return result;
   }

   void PreferencesGeneralConfigObject::SetAttachMode(const std::string &mode)
   {
      for(unsigned int i = 0; i < PreferencesGeneralConfigObject::AttachMode::Enumerate().size(); i++)
      {
         dtUtil::Enumeration *current = PreferencesGeneralConfigObject::AttachMode::Enumerate()[i];

         if(current->GetName() == mode)
         {
            SetAttachMode(
               static_cast<const PreferencesGeneralConfigObject::AttachMode&>(*current));
         }
      }
   }

   void PreferencesGeneralConfigObject::SetPerformanceMode(const std::string &mode)
   {
      for(unsigned int i = 0; i < PreferencesGeneralConfigObject::PerformanceMode::Enumerate().size(); i++)
      {
         dtUtil::Enumeration *current =
            PreferencesGeneralConfigObject::PerformanceMode::Enumerate()[i];

         if(current->GetName() == mode)
         {
            SetPerformanceMode(
               static_cast<const PreferencesGeneralConfigObject::PerformanceMode&>(*current));
         }
      }
   }
}
