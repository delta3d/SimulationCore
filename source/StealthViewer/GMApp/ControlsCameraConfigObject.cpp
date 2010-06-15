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
#include <StealthViewer/GMApp/ControlsCameraConfigObject.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <dtUtil/coordinates.h>
#include <dtActors/coordinateconfigactor.h>
#include <dtActors/engineactorregistry.h>
#include <dtGame/gamemanager.h>
#include <dtGame/messagefactory.h>

namespace StealthGM
{
   ////////////////////////////////////////////////////////////
   ControlsCameraConfigObject::ControlsCameraConfigObject():
      mWarp(false),
      mCoordValid(false),
      mStealthActorId("")
   {
   }

   ////////////////////////////////////////////////////////////
   ControlsCameraConfigObject::~ControlsCameraConfigObject()
   {
   }

   ////////////////////////////////////////////////////////////
   void ControlsCameraConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      if(!IsUpdated())
         return;

      SetIsUpdated(false);
      if (mWarp)
      {
         if (!mStealthActorId.ToString().empty())
         {
            mWarp = false;
            dtCore::RefPtr<SimCore::StealthActorUpdatedMessage> wtpMsg;
            gameManager.GetMessageFactory().CreateMessage(SimCore::MessageType::REQUEST_WARP_TO_POSITION, wtpMsg);
            wtpMsg->SetTranslation(mWarpToPosition);
            wtpMsg->SetAboutActorId(mStealthActorId);
            gameManager.SendMessage(*wtpMsg);
         }
         else
         {
            LOG_ERROR("Unable to Warp Stealth Actor, none was found.");
         }
      }
   }

   ////////////////////////////////////////////////////////////
   bool FindCoordinatesObject(dtGame::GameManager& gm, dtUtil::Coordinates& coord)
   {
      dtActors::CoordinateConfigActorProxy* coordConfigActorProxy = NULL;
      gm.FindActorByType(*dtActors::EngineActorRegistry::COORDINATE_CONFIG_ACTOR_TYPE, coordConfigActorProxy);
      if (coordConfigActorProxy != NULL)
      {
         dtActors::CoordinateConfigActor* coordConfigActor = NULL;
         coordConfigActorProxy->GetActor(coordConfigActor);
         coord = coordConfigActor->GetCoordinates();
      }

      return true;
   }

   ////////////////////////////////////////////////////////////
   void ControlsCameraConfigObject::Reset(dtGame::GameManager& gameManager)
   {
      mCoordValid = FindCoordinatesObject(gameManager, mCoord);

      dtGame::GameActorProxy* StealthActor = NULL;
      gameManager.FindActorByType(*SimCore::Actors::EntityActorRegistry::STEALTH_ACTOR_TYPE, StealthActor);
      if (StealthActor != NULL)
      {
         mStealthActorId = StealthActor->GetId();
      }
      else
      {
         mStealthActorId = "";
      }
   }

   ////////////////////////////////////////////////////////////
   void ControlsCameraConfigObject::WarpToPosition(double latitude, double longitude, double elevation)
   {
      dtUtil::Coordinates coordCopy = mCoord;
      coordCopy.SetIncomingCoordinateType(dtUtil::IncomingCoordinateType::GEODETIC);
      mWarpToPosition = coordCopy.ConvertToLocalTranslation(osg::Vec3d(latitude, longitude, elevation));

      SetIsUpdated(true);
      mWarp = true;
   }

   ////////////////////////////////////////////////////////////
   void ControlsCameraConfigObject::WarpToPosition(const std::string& MGRS, double elevation)
   {
      dtUtil::Coordinates coordCopy = mCoord;
      mWarpToPosition = coordCopy.ConvertMGRSToXYZ(MGRS);
      mWarpToPosition.z() = elevation;

      SetIsUpdated(true);
      mWarp = true;
   }

   ////////////////////////////////////////////////////////////
   void ControlsCameraConfigObject::WarpToPosition(const osg::Vec3d& xyzPosition)
   {
      mWarpToPosition = xyzPosition;
      SetIsUpdated(true);
      mWarp = true;
   }

   ////////////////////////////////////////////////////////////
   bool ControlsCameraConfigObject::IsAboutToWarp() const
   {
      return mWarp;
   }

   ////////////////////////////////////////////////////////////
   bool ControlsCameraConfigObject::IsCoordConfigValid() const
   {
      return mCoordValid;
   }

   ////////////////////////////////////////////////////////////
   const dtUtil::Coordinates& ControlsCameraConfigObject::GetCoordinates() const
   {
      return mCoord;
   }

   ////////////////////////////////////////////////////////////
   const osg::Vec3d& ControlsCameraConfigObject::GetWarpToPosition() const
   {
      return mWarpToPosition;
   }

   ////////////////////////////////////////////////////////////
   const dtCore::UniqueId& ControlsCameraConfigObject::GetStealthActorId() const
   {
      return mStealthActorId;
   }

   void ControlsCameraConfigObject::SetWarpPositionCountToSave(int positions)
   {
      mWarpPositionsToSave = positions;
   }

   int ControlsCameraConfigObject::GetWarpPositionCountToSave() const
   {
      return mWarpPositionsToSave;
   }
}
