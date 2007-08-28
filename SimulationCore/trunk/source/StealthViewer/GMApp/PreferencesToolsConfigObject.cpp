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
#include <prefix/dvteprefix-src.h>
#include <StealthGM/PreferencesToolsConfigObject.h>
#include <StealthGM/StealthInputComponent.h>
#include <StealthGM/StealthHUD.h>
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

   PreferencesToolsConfigObject::PreferencesToolsConfigObject() :
      mCoordinateSystem(&PreferencesToolsConfigObject::CoordinateSystem::MGRS),
      mShowBinocularImage(true),
      mShowDistanceToObject(true),
      mShowElevationOfObject(true),
      mMagnification(7.0f),
      mAutoAttachOnSelection(false),
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

   void PreferencesToolsConfigObject::SetCoordinateSystem(const std::string &system)
   {
      for(unsigned int i = 0; i < PreferencesToolsConfigObject::CoordinateSystem::Enumerate().size(); i++)
      {
         dtUtil::Enumeration *current = PreferencesToolsConfigObject::CoordinateSystem::Enumerate()[i];

         if(current->GetName() == system)
         {
            SetCoordinateSystem(static_cast<PreferencesToolsConfigObject::CoordinateSystem&>(*current));
         }
      }
   }
}
