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

namespace SimCore
{
   class UnitOfLength;
   class UnitOfAngle;

   namespace Tools
   {
      class Binoculars;
   }
}

namespace StealthGM
{
   class PrefToolImpl;

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

               CoordinateSystem(const std::string& name);
         };


         /// Constructor
         PreferencesToolsConfigObject();

         /**
          * Overridden base class method to apply the changes made to this class to the
          * game manager
          */
         virtual void ApplyChanges(dtGame::GameManager& gameManager);

         /**
          * Sets the coordinate system
          * @param system The system to use
          */
         void SetCoordinateSystem(const CoordinateSystem& system);

         /**
          * Gets the coordinate system
          * @return mCoordinateSystem
          */
         const CoordinateSystem& GetCoordinateSystem() const;

         /**
          * Sets whether to render the 360 compass
          * @param show True to show, false to hide
          */
         void SetShowCompass360(bool show);

         /**
          * Returns true if we are showing the 360 compass
          * @return mShowCompass360
          */
         bool GetShowCompass360() const;

         /**
          * Sets whether to render the binocular reticle
          * @param show True to show, false to hide
          */
         void SetShowBinocularImage(bool show);

         /**
          * Returns true if we are showing the binocular image
          * @return mShowBinocularImage
          */
         bool GetShowBinocularImage() const;

         /**
          * Sets whether to show the distance to an object
          * @param show True to show, false to hide
          */
         void SetShowDistanceToObject(bool show);

         /**
          * Returns true if we are showing the the object distance
          * @return mShowBinocularImage
          */
         bool GetShowDistanceToObject() const;

         /**
          * Sets whether to show the distance to an object
          * @param show True to show, false to hide
          */
         void SetShowElevationOfObject(bool show);

         /**
          * Returns true if we are showing the the object distance
          * @return mShowBinocularImage
          */
         bool GetShowElevationOfObject() const;

         /**
          * Sets the magnification
          * @param factor The zoom factor
          */
         void SetMagnification(float factor);

         /**
          * Returns the magnification
          * @return mMagnification
          */
         float GetMagnification() const;

         /**
          * Sets auto attaching on selection
          * @param attach True to auto attach, false to not
          */
         void SetAutoAttachOnSelection(bool attach);

         /**
          * Returns true if we are auto attaching
          * @return mAutoAttachOnSelection
          */
         bool GetAutoAttachOnSelection() const;

         void SetBinocularsTool(SimCore::Tools::Binoculars* binocs);
         SimCore::Tools::Binoculars* GetBinocularsTool();

         void SetLengthUnit(SimCore::UnitOfLength& unit);
         void SetLengthUnit(const std::string& unitName);
         SimCore::UnitOfLength& GetLengthUnit() const;

         void SetAngleUnit(SimCore::UnitOfAngle& unit);
         void SetAngleUnit(const std::string& unitName);
         SimCore::UnitOfAngle& GetAngleUnit() const;
      protected:

         /// Destructor
         virtual ~PreferencesToolsConfigObject();

      private:

         PrefToolImpl* mPImpl;
   };
}
#endif
