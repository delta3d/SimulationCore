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
#include <prefix/SimCorePrefix.h>
#include <StealthViewer/GMApp/PreferencesToolsConfigObject.h>
#include <StealthViewer/GMApp/StealthInputComponent.h>
#include <StealthViewer/GMApp/StealthHUD.h>
#include <SimCore/Tools/LaserRangeFinder.h>
#include <SimCore/MessageType.h>
#include <SimCore/UnitEnums.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <dtGame/gamemanager.h>
#include <dtActors/engineactorregistry.h>
#include <dtGame/exceptionenum.h>

namespace StealthGM
{
   IMPLEMENT_ENUM(PreferencesToolsConfigObject::CoordinateSystem);
   const PreferencesToolsConfigObject::CoordinateSystem PreferencesToolsConfigObject::CoordinateSystem::MGRS("MGRS");
   const PreferencesToolsConfigObject::CoordinateSystem PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ("Raw XYZ");
   const PreferencesToolsConfigObject::CoordinateSystem PreferencesToolsConfigObject::CoordinateSystem::LAT_LON("Lat Lon");

   class PrefToolImpl
   {
   public:
      PrefToolImpl()
      : mCoordinateSystem(&PreferencesToolsConfigObject::CoordinateSystem::MGRS)
      , mShowCompass360(false)
      , mShowBinocularImage(true)
      , mShowDistanceToObject(true)
      , mShowElevationOfObject(true)
      , mMagnification(7.0f)
      , mAutoAttachOnSelection(true)
      , mHighlightEntities(false)
      , mShowCallSigns(false)
      , mLengthUnit(&SimCore::UnitOfLength::METER)
      , mAngleUnit(&SimCore::UnitOfAngle::DEGREE)
      {
      }

      const PreferencesToolsConfigObject::CoordinateSystem* mCoordinateSystem;
      bool mShowCompass360;
      bool mShowBinocularImage;
      bool mShowDistanceToObject;
      bool mShowElevationOfObject;
      float mMagnification;
      bool mAutoAttachOnSelection;
      bool mHighlightEntities;
      bool mShowCallSigns;
      std::shared_ptr<SimCore::Tools::Binoculars> mBinocs;
      SimCore::UnitOfLength* mLengthUnit;
      SimCore::UnitOfAngle* mAngleUnit;
   };

   /////////////////////////////////////////////////////////////////////////////
   PreferencesToolsConfigObject::CoordinateSystem::CoordinateSystem(const std::string& name) : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   /////////////////////////////////////////////////////////////////////////////
   PreferencesToolsConfigObject::PreferencesToolsConfigObject() :
      mPImpl(new PrefToolImpl)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   PreferencesToolsConfigObject::~PreferencesToolsConfigObject()
   {
      delete mPImpl;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetCoordinateSystem(const PreferencesToolsConfigObject::CoordinateSystem& system)
   {
      mPImpl->mCoordinateSystem = &system;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   const PreferencesToolsConfigObject::CoordinateSystem& PreferencesToolsConfigObject::GetCoordinateSystem() const
   {
      return *mPImpl->mCoordinateSystem;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetShowCompass360(bool show)
   {
      mPImpl->mShowCompass360 = show;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool PreferencesToolsConfigObject::GetShowCompass360() const
   {
      return mPImpl->mShowCompass360;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetShowBinocularImage(bool show)
   {
      mPImpl->mShowBinocularImage = show;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool PreferencesToolsConfigObject::GetShowBinocularImage() const
   {
      return mPImpl->mShowBinocularImage;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetShowDistanceToObject(bool show)
   {
      mPImpl->mShowDistanceToObject = show;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool PreferencesToolsConfigObject::GetShowDistanceToObject() const
   {
      return mPImpl->mShowDistanceToObject;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetShowElevationOfObject(bool show)
   {
      mPImpl->mShowElevationOfObject = show;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool PreferencesToolsConfigObject::GetShowElevationOfObject() const
   {
      return mPImpl->mShowElevationOfObject;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetMagnification(float factor)
   {
      mPImpl->mMagnification = factor;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   float PreferencesToolsConfigObject::GetMagnification() const
   {
      return mPImpl->mMagnification;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetAutoAttachOnSelection(bool attach)
   {
      mPImpl->mAutoAttachOnSelection = attach;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////////
   bool PreferencesToolsConfigObject::GetAutoAttachOnSelection() const
   {
      return mPImpl->mAutoAttachOnSelection;
   }

   /////////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetBinocularsTool(SimCore::Tools::Binoculars* binocs)
   {
      mPImpl->mBinocs = binocs;
   }

   /////////////////////////////////////////////////////////////////////////////
   SimCore::Tools::Binoculars* PreferencesToolsConfigObject::GetBinocularsTool()
   {
      return mPImpl->mBinocs.get();
   }

   /////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetLengthUnit(SimCore::UnitOfLength& unit)
   {
      mPImpl->mLengthUnit = &unit;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetLengthUnit(const std::string& unitName)
   {
      SimCore::UnitOfLength* uofL = SimCore::UnitOfLength::GetValueForName(unitName);
      if (uofL != nullptr)
      {
         SetLengthUnit(*uofL);
      }
   }

   /////////////////////////////////////////////////////////////////////////
   SimCore::UnitOfLength& PreferencesToolsConfigObject::GetLengthUnit() const
   {
      return *mPImpl->mLengthUnit;
   }

   /////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetAngleUnit(SimCore::UnitOfAngle& unit)
   {
      mPImpl->mAngleUnit = &unit;
      SetIsUpdated(true);
   }

   /////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::SetAngleUnit(const std::string& unitName)
   {
      SimCore::UnitOfAngle* uofA = SimCore::UnitOfAngle::GetValueForName(unitName);
      if (uofA != nullptr)
      {
         SetAngleUnit(*uofA);
      }
   }

   /////////////////////////////////////////////////////////////////////////
   SimCore::UnitOfAngle& PreferencesToolsConfigObject::GetAngleUnit() const
   {
      return *mPImpl->mAngleUnit;
   }

   /////////////////////////////////////////////////////////////////////////
   void PreferencesToolsConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      /////////////////////////////////////////////////////////////////////////////
      // Need to update the tool display every tick. Since this method is called
      // every tick, here is an easy place to implement it. This might be sort of hackish,
      // but I didn't think it was a big deal.
      // - Eddie
      /////////////////////////////////////////////////////////////////////////////
      SimCore::Tools::Binoculars* binos = GetBinocularsTool();

      if (binos == nullptr)
      {

         StealthGM::StealthInputComponent* sic =
            static_cast<StealthGM::StealthInputComponent*>(gameManager.GetComponentByName(StealthGM::StealthInputComponent::DEFAULT_NAME));

         if (sic == nullptr)
         {
            throw dtGame::GeneralGameManagerException(
               "Failed to locate the stealth input component on the Game Manager.",
               __FILE__, __LINE__);
         }

         SetBinocularsTool(static_cast<SimCore::Tools::Binoculars*>(sic->GetTool(SimCore::MessageType::BINOCULARS)));
         binos = GetBinocularsTool();
      }

      if (binos != nullptr && binos->IsEnabled())
      {
         std::vector<dtDAL::ActorProxy*> proxies;
         gameManager.FindActorsByName("Terrain", proxies);
         if (!proxies.empty())
         {
            SimCore::Actors::TerrainActorProxy* proxy =
               static_cast<SimCore::Actors::TerrainActorProxy*>(proxies[0]);

            binos->Update(*proxy->GetDrawable());
         }
      }
      ////////////////////////////////////////////////////////////////////////////////

      if (!IsUpdated())
      {
         return;
      }

      StealthHUD* hud = nullptr;
      gameManager.GetComponentByName(StealthHUD::DEFAULT_NAME, hud);

      if (hud != nullptr)
      {
         hud->SetUnitOfAngle(GetAngleUnit());
         hud->SetUnitOfLength(GetLengthUnit());

         if (*mPImpl->mCoordinateSystem == PreferencesToolsConfigObject::CoordinateSystem::MGRS)
         {
            hud->SetCoordinateSystem(CoordSystem::MGRS);
         }
         else if (*mPImpl->mCoordinateSystem == PreferencesToolsConfigObject::CoordinateSystem::RAW_XYZ)
         {
            hud->SetCoordinateSystem(CoordSystem::RAW_XYZ);
         }
         else if (*mPImpl->mCoordinateSystem == PreferencesToolsConfigObject::CoordinateSystem::LAT_LON)
         {
            hud->SetCoordinateSystem(CoordSystem::LAT_LON);
         }

         // Compass 360
         if( ! hud->HasCompass360())
         {
            hud->SetupCompass360();
         }
         hud->SetCompass360Enabled(mPImpl->mShowCompass360);
      }

      if (binos != nullptr)
      {
         binos->SetShowReticle(GetShowBinocularImage());
         binos->SetShowDistance(GetShowDistanceToObject());
         binos->SetShowElevation(GetShowElevationOfObject());
         binos->SetZoomFactor(GetMagnification());
         binos->SetUnitOfLength(GetLengthUnit());
         binos->SetUnitOfAngle(GetAngleUnit());
      }

      SetIsUpdated(false);
   }
}
