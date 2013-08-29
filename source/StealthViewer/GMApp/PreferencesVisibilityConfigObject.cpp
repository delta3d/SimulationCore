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
 * @author David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
#include <StealthViewer/GMApp/StealthHUD.h>
#include <SimCore/Components/ViewerMessageProcessor.h>

#include <SimCore/Actors/BattlefieldGraphicsActor.h>

namespace StealthGM
{

   ////////////////////////////////////////////////////////////////////
   PreferencesVisibilityConfigObject::PreferencesVisibilityConfigObject()
   : mBFGCloseTops(SimCore::Actors::BattlefieldGraphicsActorProxy::GetGlobalEnableTopGeometry())
   , mEntityOptions(new SimCore::VisibilityOptions)
   {
   }

   ////////////////////////////////////////////////////////////////////
   PreferencesVisibilityConfigObject::~PreferencesVisibilityConfigObject()
   {
   }

   ////////////////////////////////////////////////////////////////////
   DT_IMPLEMENT_ACCESSOR_GETTER(PreferencesVisibilityConfigObject, bool, BFGCloseTops)

   ////////////////////////////////////////////////////////////////////
   void PreferencesVisibilityConfigObject::SetBFGCloseTops(bool closeTops)
   {
      SetIsUpdated(true);
      mBFGCloseTops = closeTops;
   }

   ////////////////////////////////////////////////////////////////////
   void PreferencesVisibilityConfigObject::SetLabelOptions(const SimCore::Components::LabelOptions& options)
   {
      SetIsUpdated(true);
      mLabelOptions = options;
   }

   ////////////////////////////////////////////////////////////////////
   const SimCore::Components::LabelOptions& PreferencesVisibilityConfigObject::GetLabelOptions() const
   {
      return mLabelOptions;
   }

   ////////////////////////////////////////////////////////////////////
   SimCore::Components::LabelOptions& PreferencesVisibilityConfigObject::GetLabelOptions()
   {
      return mLabelOptions;
   }

   ////////////////////////////////////////////////////////////////////
   void PreferencesVisibilityConfigObject::SetEntityOptions(SimCore::VisibilityOptions& options)
   {
      SetIsUpdated(true);
      mEntityOptions = &options;
   }

   ////////////////////////////////////////////////////////////////////
   const SimCore::VisibilityOptions& PreferencesVisibilityConfigObject::GetEntityOptions() const
   {
      return *mEntityOptions;
   }

   ////////////////////////////////////////////////////////////////////
   SimCore::VisibilityOptions& PreferencesVisibilityConfigObject::GetEntityOptions()
   {
      return *mEntityOptions;
   }

   ////////////////////////////////////////////////////////////////////
   void PreferencesVisibilityConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      if (IsUpdated())
      {
         StealthGM::StealthHUD* hud = NULL;
         gameManager.GetComponentByName(StealthGM::StealthHUD::DEFAULT_NAME, hud);
         if (hud != NULL)
         {
            SimCore::Components::LabelManager& lm = hud->GetLabelManager();
            lm.SetOptions(GetLabelOptions());
         }
         else
         {
            LOG_ERROR("Attempted to update the label manager options, but on StealthHUD component could be found");
         }

         SimCore::Components::ViewerMessageProcessor* vmp = NULL;
         gameManager.GetComponentByName(SimCore::Components::ViewerMessageProcessor::DEFAULT_NAME, vmp);
         if (vmp != NULL)
         {
            vmp->SetVisibilityOptions(*mEntityOptions);
         }
         else
         {
            LOG_ERROR("Attempted to update the visibility options, but on ViewerMessageProcessor component could be found");
         }

         SetIsUpdated(false);

         if (SimCore::Actors::BattlefieldGraphicsActorProxy::GetGlobalEnableTopGeometry() != mBFGCloseTops)
         {
            SimCore::Actors::BattlefieldGraphicsActorProxy::SetGlobalEnableTopGeometry(mBFGCloseTops, gameManager);
         }
      }
   }

}
