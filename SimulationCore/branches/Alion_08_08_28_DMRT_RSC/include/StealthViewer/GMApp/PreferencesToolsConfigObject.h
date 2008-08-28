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
#ifndef _PREFERENCES_TOOLS_CONFIG_OBJECT_H_
#define _PREFERENCES_TOOLS_CONFIG_OBJECT_H_

#include <dtUtil/enumeration.h>

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT PreferencesToolsConfigObject : public ConfigurationObjectInterface
   {
      public:

         class STEALTH_GAME_EXPORT CoordinateSystem : public dtUtil::Enumeration
         {
            DECLARE_ENUM(CoordinateSystem);

            public:

               static const CoordinateSystem MGRS;
               static const CoordinateSystem RAW_XYZ;
               static const CoordinateSystem LAT_LON;

            private:

               CoordinateSystem(const std::string &name);
         };


         /// Constructor
         PreferencesToolsConfigObject();

         /**
          * Overridden base class method to apply the changes made to this class to the
          * game manager
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

         /**
          * Sets the coordinate system
          * @param system The system to use
          */
         void SetCoordinateSystem(const CoordinateSystem &system) { mCoordinateSystem = &system; SetIsUpdated(true); }

         /**
          * Gets the coordinate system
          * @return mCoordinateSystem
          */
         const CoordinateSystem& GetCoordinateSystem() const { return *mCoordinateSystem; }

         /**
          * Sets whether to render the binocular reticle
          * @param show True to show, false to hide
          */
         void SetShowBinocularImage(bool show) { mShowBinocularImage = show; SetIsUpdated(true); }

         /**
          * Returns true if we are showing the binocular image
          * @return mShowBinocularImage
          */
         bool GetShowBinocularImage() const { return mShowBinocularImage; }

         /**
          * Sets whether to show the distance to an object
          * @param show True to show, false to hide
          */
         void SetShowDistanceToObject(bool show) { mShowDistanceToObject = show; SetIsUpdated(true); }

         /**
          * Returns true if we are showing the the object distance
          * @return mShowBinocularImage
          */
         bool GetShowDistanceToObject() const { return mShowDistanceToObject; }

         /**
          * Sets whether to show the distance to an object
          * @param show True to show, false to hide
          */
         void SetShowElevationOfObject(bool show) { mShowElevationOfObject = show; SetIsUpdated(true); }

         /**
          * Returns true if we are showing the the object distance
          * @return mShowBinocularImage
          */
         bool GetShowElevationOfObject() const { return mShowElevationOfObject; }

         /**
          * Sets the magnification
          * @param factor The zoom factor
          */
         void SetMagnification(float factor) { mMagnification = factor; SetIsUpdated(true); }

         /**
          * Returns the magnification
          * @return mMagnification
          */
         float GetMagnification() const { return mMagnification; }

         /**
          * Sets auto attaching on selection
          * @param attach True to auto attach, false to not
          */
         void SetAutoAttachOnSelection(bool attach) { mAutoAttachOnSelection = attach; SetIsUpdated(true); }

         /**
          * Returns true if we are auto attaching
          * @return mAutoAttachOnSelection
          */
         bool GetAutoAttachOnSelection() const { return mAutoAttachOnSelection; }

      protected:

         /// Destructor
         virtual ~PreferencesToolsConfigObject();

      private:

         const CoordinateSystem *mCoordinateSystem;
         bool mShowBinocularImage;
         bool mShowDistanceToObject;
         bool mShowElevationOfObject;
         float mMagnification;
         bool mAutoAttachOnSelection;
         bool mHighlightEntities;
         bool mShowCallSigns;
   };
}
#endif
