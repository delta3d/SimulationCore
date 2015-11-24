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
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/PreferencesGeneralConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>

#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <dtGame/messagefactory.h>
#include <dtGame/gamemanager.h>

#include <dtCore/camera.h>

#include <dtABC/application.h>

#include <dtUtil/version.h>

namespace StealthGM
{
   ////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(PreferencesGeneralConfigObject::AttachMode);
   const PreferencesGeneralConfigObject::AttachMode PreferencesGeneralConfigObject::AttachMode::FIRST_PERSON("First Person");
   const PreferencesGeneralConfigObject::AttachMode PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON("Third Person");
   PreferencesGeneralConfigObject::AttachMode::AttachMode(const std::string& name) : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   ////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(PreferencesGeneralConfigObject::PerformanceMode);
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BEST_GRAPHICS("Best Graphics");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BETTER_GRAPHICS("Better Graphics");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::DEFAULT("Default");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BETTER_SPEED("Better Speed");
   const PreferencesGeneralConfigObject::PerformanceMode PreferencesGeneralConfigObject::PerformanceMode::BEST_SPEED("Best Speed");
   PreferencesGeneralConfigObject::PerformanceMode::PerformanceMode(const std::string& name) : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   ////////////////////////////////////////////////////////////////
   PreferencesGeneralConfigObject::PreferencesGeneralConfigObject()
   : mAutoReconnect(false)
   , mAutoReconnectTimeout(30)
   , mAttachMode(&PreferencesGeneralConfigObject::AttachMode::THIRD_PERSON)
   , mEnableCameraCollision(true)
   , mPerformanceMode(&PreferencesGeneralConfigObject::PerformanceMode::DEFAULT)
   , mLODScale(1.0f)
   , mShowAdvancedOptions(false)
   , mAttachActorId("")
   , mReconnectOnStartup(true)
   , mAutoRefreshEntityInfo(true)
   , mDetachFromActor(false)
   , mInputComponent(NULL)
   , mShouldAutoAttachToEntity(false)
   {
   }

   PreferencesGeneralConfigObject::~PreferencesGeneralConfigObject()
   {

   }

   void PreferencesGeneralConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      if (GetShouldAutoAttachToEntity() && !IsStealthActorCurrentlyAttached()
               && !GetAutoAttachEntityCallsign().empty())
      {
         dtCore::ActorProxy* proxy = NULL;
         gameManager.FindActorByName(GetAutoAttachEntityCallsign(), proxy);
         if (proxy != NULL)
         {
            AttachToActor(proxy->GetId());
         }
      }


      if(!IsUpdated())
         return;

      dtGame::GMComponent* component =
         gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME);
      mInputComponent = static_cast<StealthInputComponent*>(component);

      // Update attach mode
      mInputComponent->ChangeMotionModels(GetAttachMode() == AttachMode::FIRST_PERSON);

      // Update performance

      // Update motion model
      mInputComponent->EnableCameraCollision(GetEnableCameraCollision());

      // Update rendering options
      unsigned numViews = gameManager.GetApplication().GetNumberOfViews();

      for (unsigned i = 0; i < numViews; ++i)
      {
         dtCore::View* view = gameManager.GetApplication().GetView(i);
         view->GetCamera()->SetLODScale(GetLODScale());
      }
      // Updated the LOD scale

      AttachOrDetach(gameManager);

      SetIsUpdated(false);
   }

   void PreferencesGeneralConfigObject::AttachOrDetach(dtGame::GameManager& gameManager)
   {
      if(!mAttachActorId.ToString().empty() && mInputComponent->GetStealthActor() != NULL)
      {
         if(mAttachActorId == mInputComponent->GetStealthActor()->GetUniqueId())
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
            ataMsg.SetAttachToActor(mAttachActorId);
            ataMsg.SetAttachPointNodeName(GetAttachPointNodeName());
            ataMsg.SetInitialAttachRotationHPR(GetInitialAttachRotationHPR());

            gameManager.SendMessage(ataMsg);
         }

         mAttachActorId = "";
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

   }

   //////////////////////////////////////////////////////////////////////////
   void PreferencesGeneralConfigObject::Reattach()
   {
      if (mInputComponent.valid() && mInputComponent->GetStealthActor() != NULL)
      {
         if (mInputComponent->GetStealthActor()->IsAttachedToActor())
         {
            AttachToActor(mInputComponent->GetStealthActor()->GetParent()->GetUniqueId());
         }
      }
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

   //////////////////////////////////////////////////////////////////////////
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

   //////////////////////////////////////////////////////////////////////////
   void PreferencesGeneralConfigObject::SetPerformanceMode(const std::string &mode)
   {
      PreferencesGeneralConfigObject::PerformanceMode* modeVal =
         PreferencesGeneralConfigObject::PerformanceMode::GetValueForName(mode);

      if (modeVal != NULL)
      {
         SetPerformanceMode(*modeVal);
      }
      else
      {
         LOGN_ERROR("PreferencesGeneralConfigObject.cpp", "Unable to set the performance mode to \"" + mode
                  + "\", no such mode exists.");
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void PreferencesGeneralConfigObject::SetShouldAutoAttachToEntity(bool shouldAttach)
   {
      mShouldAutoAttachToEntity = shouldAttach;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   bool PreferencesGeneralConfigObject::GetShouldAutoAttachToEntity() const
   {
      return mShouldAutoAttachToEntity;
   }

   //////////////////////////////////////////////////////////////////////////
   void PreferencesGeneralConfigObject::SetAutoAttachEntityCallsign(const std::string& callsign)
   {
      mAutoAttachEntityCallsign = callsign;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& PreferencesGeneralConfigObject::GetAutoAttachEntityCallsign() const
   {
      return mAutoAttachEntityCallsign;
   }

   //////////////////////////////////////////////////////////////////////////
   void PreferencesGeneralConfigObject::SetAttachPointNodeName(const std::string& name)
   {
      mAttachPointNodeName = name;
      SetIsUpdated(true);
      Reattach();
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& PreferencesGeneralConfigObject::GetAttachPointNodeName() const
   {
      return mAttachPointNodeName;
   }

   //////////////////////////////////////////////////////////////////////////
   void PreferencesGeneralConfigObject::SetInitialAttachRotationHPR(const osg::Vec3& hpr)
   {
      mInitialAttachRotationHPR = hpr;
      SetIsUpdated(true);
      Reattach();
   }

   //////////////////////////////////////////////////////////////////////////
   const osg::Vec3& PreferencesGeneralConfigObject::GetInitialAttachRotationHPR() const
   {
      return mInitialAttachRotationHPR;
   }

}
