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
#ifndef _CONTROLS_CAMERA_CONFIG_OBJECT_H_
#define _CONTROLS_CAMERA_CONFIG_OBJECT_H_

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>
#include <dtUtil/coordinates.h>
#include <dtCore/uniqueid.h>
#include <osg/Vec3d>
#include <string>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT ControlsCameraConfigObject : public ConfigurationObjectInterface
   {
      public:

         /// Constructor
         ControlsCameraConfigObject();

         /**
          * Applies the changes into the game manager
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

         /**
          * Reloads the coordinate config.
          */
         virtual void Reset(dtGame::GameManager &gameManager);

         void WarpToPosition(double latitude, double longitude, double elevation);
         void WarpToPosition(const std::string& MGRS, double elevation);
         void WarpToPosition(const osg::Vec3d& xyzPosition);

         bool IsAboutToWarp() const;
         bool IsCoordConfigValid() const;
         const dtUtil::Coordinates& GetCoordinates() const;
         const osg::Vec3d& GetWarpToPosition() const;
         const dtCore::UniqueId& GetStealthActorId() const;

         /// Sets the number of warp to history position to save.
         void SetWarpPositionCountToSave(int positions);
         /// @return the number of warp to history position to save.
         int GetWarpPositionCountToSave() const;
      protected:

         /// Destructor
         virtual ~ControlsCameraConfigObject();

         bool mWarp;
         bool mCoordValid;
         dtUtil::Coordinates mCoord;
         osg::Vec3d mWarpToPosition;
         dtCore::UniqueId mStealthActorId;
         //std::deque<osg::Vec3d> mXYZWarpPositions;
         int mWarpPositionsToSave;
   };
}

#endif
