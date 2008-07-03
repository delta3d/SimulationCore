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
#include <prefix/SimCorePrefix-src.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <StealthViewer/GMApp/StealthHUD.h>
#include <SimCore/Tools/LaserRangeFinder.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <dtGame/gamemanager.h>
#include <dtActors/engineactorregistry.h>

namespace StealthGM
{
   IMPLEMENT_ENUM(PreferencesToolsConfigObject::CoordinateSystem);
   const PreferencesToolsConfigObject::CoordinateSystem PreferencesToolsConfigObject::CoordinateSystem::MGRS("MGRS");
   const PreferencesToolsConfigObject::CoordinateSystem PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ("Raw XYZ");
   const PreferencesToolsConfigObject::CoordinateSystem PreferencesToolsConfigObject::CoordinateSystem::LAT_LON("Lat Lon");

   PreferencesToolsConfigObject::CoordinateSystem::CoordinateSystem(const std::string &name) : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   PreferencesToolsConfigObject::PreferencesToolsConfigObject() :
      mCoordinateSystem(&PreferencesToolsConfigObject::CoordinateSystem::MGRS),
      mShowBinocularImage(true),
      mShowDistanceToObject(true),
      mShowElevationOfObject(true),
      mMagnification(7.0f),
      mAutoAttachOnSelection(true),
      mHighlightEntities(false),
      mShowCallSigns(false)
   {

   }

   PreferencesToolsConfigObject::~PreferencesToolsConfigObject()
   {

   }

   void PreferencesToolsConfigObject::ApplyChanges(dtGame::GameManager &gameManager)
   {
      /////////////////////////////////////////////////////////////////////////////
      // Need to update the tool display every tick. Since this method is called
      // every tick, here is an easy place to implement it. This might be sort of hackish,
      // but I didn't think it was a big deal.
      // - Eddie
      /////////////////////////////////////////////////////////////////////////////
      StealthGM::StealthInputComponent *sic =
         static_cast<StealthGM::StealthInputComponent*>(gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME));

      if(sic == NULL)
      {
         throw dtUtil::Exception(dtGame::ExceptionEnum::GENERAL_GAMEMANAGER_EXCEPTION,
            "Failed to locate the stealth input component on the Game Manager.",
            __FILE__, __LINE__);
      }

      SimCore::Tools::Binoculars *binos =
         static_cast<SimCore::Tools::Binoculars*>(sic->GetTool(SimCore::MessageType::BINOCULARS));
      if(binos != NULL)
      {
         std::vector<dtDAL::ActorProxy*> proxies;
         gameManager.FindActorsByType(*SimCore::Actors::EntityActorRegistry::TERRAIN_ACTOR_TYPE, proxies);
         if(!proxies.empty())
         {
            SimCore::Actors::TerrainActorProxy *proxy =
               static_cast<SimCore::Actors::TerrainActorProxy*>(proxies[0]);

            binos->Update(*proxy->GetActor());

            binos->SetShowReticle(GetShowBinocularImage());
            binos->SetShowDistance(GetShowDistanceToObject());
            binos->SetShowElevation(GetShowElevationOfObject());
            binos->SetZoomFactor(GetMagnification());
         }
      }
      ////////////////////////////////////////////////////////////////////////////////

      if(!IsUpdated())
         return;

      StealthHUD *hud = static_cast<StealthHUD*>(gameManager.GetComponentByName(StealthHUD::DEFAULT_NAME));
      if(hud != NULL)
      {
         if(*mCoordinateSystem == PreferencesToolsConfigObject::CoordinateSystem::MGRS)
         {
            hud->SetCoordinateSystem(CoordSystem::MGRS);
         }
         else if(*mCoordinateSystem == PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ)
         {
            hud->SetCoordinateSystem(CoordSystem::RAW_XYZ);
         }
         else if(*mCoordinateSystem == PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
         {
            hud->SetCoordinateSystem(CoordSystem::LAT_LON);
         }
         else
         {

         }
      }

      SetIsUpdated(false);
   }
}
