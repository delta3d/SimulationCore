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
#ifndef _PREFERENCES_TOOLS_CONFIG_OBJECT_H_
#define _PREFERENCES_TOOLS_CONFIG_OBJECT_H_

#include <dtUtil/enumeration.h>

#include <StealthGM/ConfigurationObjectInterface.h>
#include <StealthGM/Export.h>

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

               CoordinateSystem(const std::string &name) : dtUtil::Enumeration(name)
               {
                  AddInstance(this);
               }
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
          * Set overload
          */
         void SetCoordinateSystem(const std::string &system);

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
