/* -*-c++-*-
 * SimulationCore
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
 * david
 */

#ifndef VIEWWINDOWCONFIGOBJECT_H_
#define VIEWWINDOWCONFIGOBJECT_H_

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

#include <dtCore/view.h>
#include <dtCore/deltawin.h>
#include <dtUtil/functor.h>

#include <osg/Referenced>

#include <string>
#include <set>
#include <vector>

namespace dtGame
{
   class GameManager;
}

namespace StealthGM
{
   class STEALTH_GAME_EXPORT ViewWindowWrapper: public ConfigurationObjectInterface
   {
   public:

      typedef dtUtil::Functor<void, TYPELIST_1(ViewWindowWrapper&)> OperationCallback;

      ViewWindowWrapper(const std::string& name, dtCore::View& view, dtCore::DeltaWin& window);

      bool IsAddedToApplication() const;

      const std::string& GetName() const;
      void SetName(const std::string& newName);

      const std::string& GetWindowTitle() const;
      void SetWindowTitle(const std::string& newTitle);

      dtCore::Camera* GetAttachToCamera();
      void SetAttachToCamera(dtCore::Camera* camera);

      const osg::Vec3& GetAttachCameraRotation() const;
      void SetAttachCameraRotation(const osg::Vec3& hpr);

      /// Reset all the FOV stuff to defaults.
      void FOVReset();

      bool UseAspectRatioForFOV() const;
      void SetUseAspectRatioForFOV(bool useAspect);

      void SetFOVAspectRatio(float aspectRatio);
      float GetFOVAspectRatio() const;

      void SetFOVVerticalForAspect(float vertical);
      float GetFOVVerticalForAspect() const;

      void SetFOVHorizontal(float horizontal);
      float GetFOVHorizontal() const;

      void SetFOVVerticalForHorizontal(float vertical);
      float GetFOVVerticalForHorizontal() const;

      /**
       * Sets the near clipping plane
       * @param plane The new plane
       */
      void SetNearClippingPlane(double plane) { mNearClippingPlane = plane; SetIsUpdated(true); }

      /**
       * Gets the near clipping plane
       * @return mNearClippingPlane
       */
      double GetNearClippingPlane() const { return mNearClippingPlane; }

      /**
       * Sets the far clipping plane
       * @param plane The new plane
       */
      void SetFarClippingPlane(double plane) { mFarClippingPlane = plane; SetIsUpdated(true); }

      /**
       * Gets the far clipping plane
       * @return mFarClippingPlane
       */
      double GetFarClippingPlane() const { return mFarClippingPlane; }

      /// Set the scene to render.  It may not be null.
      void SetScene(dtCore::Scene& scene);
      /// @return the scene to render.
      dtCore::Scene& GetScene();

      /**
       * Applies the changes into the game manager
       */
      virtual void ApplyChanges(dtGame::GameManager& gameManager);

      /// Called when removing this view window
      virtual void OnRemove(dtGame::GameManager& gameManager);

      dtCore::View& GetView();
      const dtCore::View& GetView() const;

      dtCore::DeltaWin& GetWindow();
      const dtCore::DeltaWin& GetWindow() const;

      void SetInitCallback(OperationCallback callback);
      OperationCallback& GetInitCallback();
      void SetRemoveCallback(OperationCallback callback);
      OperationCallback& GetRemoveCallback();

   protected:
      virtual ~ViewWindowWrapper();
   private:
      std::string mTitle;
      dtCore::RefPtr<dtCore::Camera> mParentCamera;

      OperationCallback mInitCallback;
      OperationCallback mRemoveCallback;

      osg::Vec3 mAttachCameraRotation;
      bool mUseAspectRatioForFOV;
      osg::Vec2f mFOVAspectVertical;
      osg::Vec2f mFOVHorizontalVertical;
      dtCore::RefPtr<dtCore::View> mView;
      dtCore::RefPtr<dtCore::DeltaWin> mWindow;
      double mNearClippingPlane;
      double mFarClippingPlane;

      bool mAddedToApplication;
   };

   class STEALTH_GAME_EXPORT ViewWindowConfigObject: public ConfigurationObjectInterface
   {
   public:
      typedef std::map<std::string, dtCore::RefPtr<ViewWindowWrapper> > ViewWindowContainer;
      typedef std::vector<dtCore::RefPtr<ViewWindowWrapper> > ViewWindowVector;

      ViewWindowConfigObject();

      /**
       * Applies the changes into the game manager
       */
      virtual void ApplyChanges(dtGame::GameManager &gameManager);

      /**
       * Creates the main view.r
       */
      virtual void CreateMainViewWindow(dtGame::GameManager &gameManager);

      /// @return the Main View Window or throw an exception if it has not been initialized.
      ViewWindowWrapper& GetMainViewWindow();

      bool AddViewWindow(ViewWindowWrapper& vww);
      void RemoveViewWindow(ViewWindowWrapper& vww);

      /// @return the view window with the given name.
      ViewWindowWrapper* GetViewWindow(const std::string& name);

      /// Changes the name of the viewWindow in the set to match the one in the object.
      void UpdateViewName(const std::string& oldName);

      void GetAllViewWindows(std::vector<ViewWindowWrapper* >& toFill);

      /**
       * Sets the near clipping plane
       * @param plane The new plane
       */
      void SetNearClippingPlane(double plane) { mNearClippingPlane = plane; SetIsUpdated(true); }

      /**
       * Gets the near clipping plane
       * @return mNearClippingPlane
       */
      double GetNearClippingPlane() const { return mNearClippingPlane; }

      /**
       * Sets the far clipping plane
       * @param plane The new plane
       */
      void SetFarClippingPlane(double plane) { mFarClippingPlane = plane; SetIsUpdated(true); }

      /**
       * Gets the far clipping plane
       * @return mFarClippingPlane
       */
      double GetFarClippingPlane() const { return mFarClippingPlane; }

   protected:
      virtual ~ViewWindowConfigObject();
      dtCore::RefPtr<ViewWindowWrapper> mMainViewWindow;
      ViewWindowContainer mViewWindows;
      ViewWindowVector mViewsToRemove;
      double mNearClippingPlane;
      double mFarClippingPlane;
   };

}

#endif /* VIEWWINDOWCONFIGOBJECT_H_ */
