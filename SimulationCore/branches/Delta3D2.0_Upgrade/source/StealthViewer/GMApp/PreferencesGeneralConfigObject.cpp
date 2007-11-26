/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
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
      mAutoRefreshEntityInfo(false)
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

      StealthInputComponent *inputComponent = 
         static_cast<StealthInputComponent*>(component);

      // Update attach mode
      inputComponent->ChangeMotionModels(GetAttachMode() == AttachMode::FIRST_PERSON);

      // Update performance

      // Update motion model
      inputComponent->EnableCameraCollision(GetEnableCameraCollision());

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
         camera->SetPerspective(camera->GetHorizontalFov(), camera->GetVerticalFov(), 
            GetNearClippingPlane(), GetFarClippingPlane());
      }

      // Updated the LOD scale
      camera->GetOSGCamera()->setLODScale(GetLODScale());

      if(mAttachProxy != NULL)
      {
         if(mAttachProxy->GetId() == inputComponent->GetStealthActor().GetUniqueId())
         {
            LOG_ERROR("The stealth actor cannot attach to itself.");
         }
         else
         {
            dtCore::RefPtr<dtGame::Message> msg = 
               gameManager.GetMessageFactory().CreateMessage(SimCore::MessageType::ATTACH_TO_ACTOR);

            SimCore::AttachToActorMessage &ataMsg = 
               static_cast<SimCore::AttachToActorMessage&>(*msg);

            ataMsg.SetAboutActorId(inputComponent->GetStealthActor().GetUniqueId());
            ataMsg.SetAttachToActor(mAttachProxy->GetId());

            gameManager.SendMessage(ataMsg);
         }

         mAttachProxy = NULL;
      }

      SetIsUpdated(false);
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
