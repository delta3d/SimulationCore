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

#include <StealthViewer/GMApp/ViewWindowConfigObject.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Tools/Binoculars.h>
#include <dtABC/application.h>
#include <dtCore/view.h>
#include <dtCore/camera.h>
#include <dtCore/scene.h>
#include <dtCore/deltawin.h>
#include <dtGame/gamemanager.h>
#include <dtUtil/exception.h>

#include <dtCore/observerptr.h>
#include <iostream>

namespace StealthGM
{

   //////////////////////////////////////////////////////////////////////
   ViewWindowConfigObject::ViewWindowConfigObject()
   : mNearClippingPlane(SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE)
   , mFarClippingPlane(SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE)
   {
   }

   //////////////////////////////////////////////////////////////////////
   ViewWindowConfigObject::~ViewWindowConfigObject()
   {
   }

   template <typename PairType>
   class CallUpdate
   {
   public:
      CallUpdate(ViewWindowConfigObject& vwc, dtGame::GameManager& gm)
      : mViewWindowConf(vwc)
      , mGM(gm)
      {
      }

      void operator() (PairType& pair)
      {
         pair.second->SetNearClippingPlane(mViewWindowConf.GetNearClippingPlane());
         pair.second->SetFarClippingPlane(mViewWindowConf.GetFarClippingPlane());
         pair.second->ApplyChanges(mGM);
      }

      ViewWindowConfigObject& mViewWindowConf;
      dtGame::GameManager& mGM;
   };

   template <typename ValueType>
   class CallRemove
   {
   public:
      CallRemove(dtGame::GameManager& gm)
      : mGM(gm)
      {
      }

      void operator() (ValueType& value)
      {
         value->OnRemove(mGM);
      }

      dtGame::GameManager& mGM;
   };

   //////////////////////////////////////////////////////////////////////
   void ViewWindowConfigObject::ApplyChanges(dtGame::GameManager& gameManager)
   {
      if (IsUpdated() || GetMainViewWindow().IsUpdated())
      {
         // Send the Near/Far clipping plane to the weather component
         SimCore::Components::WeatherComponent* weatherComp = NULL;
         gameManager.GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME, weatherComp);
         if(weatherComp != NULL)
         {
            weatherComp->SetNearClipPlane(GetNearClippingPlane());
            weatherComp->SetFarClipPlane(GetFarClippingPlane());
            weatherComp->UpdateFog();
         }

         /// Setting the near/far planes on the main window from the global ones.
         GetMainViewWindow().SetNearClippingPlane(GetNearClippingPlane());
         GetMainViewWindow().SetFarClippingPlane(GetFarClippingPlane());
         GetMainViewWindow().ApplyChanges(gameManager);
         SetIsUpdated(false);
      }
      std::for_each(mViewWindows.begin(), mViewWindows.end(), CallUpdate<ViewWindowContainer::value_type>(*this, gameManager));
      std::for_each(mViewsToRemove.begin(), mViewsToRemove.end(), CallRemove<ViewWindowVector::value_type>(gameManager));
      mViewsToRemove.clear();
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowConfigObject::CreateMainViewWindow(dtGame::GameManager& gameManager)
   {
      dtABC::Application& app = gameManager.GetApplication();
      mMainViewWindow = new ViewWindowWrapper("Main", *app.GetView(), *app.GetWindow());
   }

   //////////////////////////////////////////////////////////////////////////
   ViewWindowWrapper& ViewWindowConfigObject::GetMainViewWindow()
   {
      if (!mMainViewWindow.valid())
      {
         throw dtUtil::Exception("Main view is not initialized yet.", __FILE__, __LINE__);
      }

      return *mMainViewWindow;
   }

   //////////////////////////////////////////////////////////////////////////
   bool ViewWindowConfigObject::AddViewWindow(ViewWindowWrapper& vww)
   {
      return mViewWindows.insert(std::make_pair(vww.GetName(), dtCore::RefPtr<ViewWindowWrapper>(&vww))).second;
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowConfigObject::RemoveViewWindow(ViewWindowWrapper& vww)
   {
      ViewWindowContainer::iterator i = mViewWindows.find(vww.GetName());
      if (i != mViewWindows.end())
      {
         //erase it if the name matches and it's the same pointer value.
         if (i->second == &vww)
         {
            mViewsToRemove.push_back(&vww);
            mViewWindows.erase(i);

         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   ViewWindowWrapper* ViewWindowConfigObject::GetViewWindow(const std::string& name)
   {
      ViewWindowContainer::iterator i = mViewWindows.find(name);
      if (i != mViewWindows.end())
      {
         return i->second.get();
      }
      return NULL;
   }

   //////////////////////////////////////////////////////////////////////////
   template <typename PairType, typename ContainerType>
   class PushBackSecond
   {
   public:
      PushBackSecond(ContainerType& container)
      : mContainer(container)
      {
      }

      void operator()(PairType& value)
      {
         mContainer.push_back(value.second.get());
      }

      ContainerType& mContainer;
   };

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowConfigObject::GetAllViewWindows(std::vector<ViewWindowWrapper*>& toFill)
   {
      std::for_each(mViewWindows.begin(), mViewWindows.end(), PushBackSecond<ViewWindowContainer::value_type, std::vector<ViewWindowWrapper*> >(toFill));
   }

   //////////////////////////////////////////////////////////////////////////
   ViewWindowWrapper::ViewWindowWrapper(const std::string& name, dtCore::View& view, dtCore::DeltaWin& window)
   : mView(&view)
   , mWindow(&window)
   , mNearClippingPlane(SimCore::Tools::Binoculars::NEAR_CLIPPING_PLANE)
   , mFarClippingPlane(SimCore::Tools::Binoculars::FAR_CLIPPING_PLANE)
   , mAddedToApplication(false)
   {
      view.SetName(name);
      FOVReset();
   }

   //////////////////////////////////////////////////////////////////////////
   ViewWindowWrapper::~ViewWindowWrapper()
   {

   }

   //////////////////////////////////////////////////////////////////////////
   bool ViewWindowWrapper::IsAddedToApplication() const
   {
      return mAddedToApplication;
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& ViewWindowWrapper::GetName() const
   {
      return mView->GetName();
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetName(const std::string& newName)
   {
      mView->SetName(newName);
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& ViewWindowWrapper::GetWindowTitle() const
   {
      return mTitle;
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetWindowTitle(const std::string& newTitle)
   {
      mTitle = newTitle;
   }

   //////////////////////////////////////////////////////////////////////////
   dtCore::Camera* ViewWindowWrapper::GetAttachToCamera()
   {
      return mParentCamera.get();
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetAttachToCamera(dtCore::Camera* camera)
   {
      mParentCamera = camera;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   const osg::Vec3& ViewWindowWrapper::GetAttachCameraRotation() const
   {
      return mAttachCameraRotation;
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetAttachCameraRotation(const osg::Vec3& hpr)
   {
      mAttachCameraRotation = hpr;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::FOVReset()
   {
      mUseAspectRatioForFOV = true;
      mFOVAspectVertical.set(1.6, 60.0);
      mFOVHorizontalVertical.set(96.0, 60.0);
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetUseAspectRatioForFOV(bool useAspect)
   {
      mUseAspectRatioForFOV = useAspect;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   bool ViewWindowWrapper::UseAspectRatioForFOV() const
   {
      return mUseAspectRatioForFOV;
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetFOVAspectRatio(float aspectRatio)
   {
      mFOVAspectVertical[0] = aspectRatio;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   float ViewWindowWrapper::GetFOVAspectRatio() const
   {
      return mFOVAspectVertical[0];
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetFOVVerticalForAspect(float vertical)
   {
      mFOVAspectVertical[1] = vertical;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   float ViewWindowWrapper::GetFOVVerticalForAspect() const
   {
      return mFOVAspectVertical[1];
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetFOVHorizontal(float horizontal)
   {
      mFOVHorizontalVertical[0] = horizontal;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   float ViewWindowWrapper::GetFOVHorizontal() const
   {
      return mFOVHorizontalVertical[0];
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetFOVVerticalForHorizontal(float vertical)
   {
      mFOVHorizontalVertical[1] = vertical;
      SetIsUpdated(true);
   }

   //////////////////////////////////////////////////////////////////////////
   float ViewWindowWrapper::GetFOVVerticalForHorizontal() const
   {
      return mFOVHorizontalVertical[1];
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::ApplyChanges(dtGame::GameManager& gameManager)
   {
      if (!mAddedToApplication)
      {
         dtABC::Application& app = gameManager.GetApplication();
         dtCore::RefPtr<dtCore::Camera> cam = mView->GetCamera();
         if (!cam.valid())
         {
            cam = new dtCore::Camera(mView->GetName());
            /// only set the mask if we create the camera.
            osg::Camera* osgCam = cam->GetOSGCamera();
            osgCam->setCullMask(SimCore::Components::RenderingSupportComponent::ADDITIONAL_CAMERA_CULL_MASK);

            mView->SetCamera(cam.get());
         }

         /// Setting the view first here is required.
         dtCore::View* mainView = app.GetView();
         mView->SetDatabasePager(mainView->GetDatabasePager());

         if (mView->GetScene() == NULL)
         {
            mView->SetScene(app.GetScene());
         }

         cam->SetWindow(mWindow.get());

         if (!app.ContainsView(*mView))
         {
            app.AddView(*mView);
         }

         mAddedToApplication = true;

         if (mInitCallback.valid())
         {
            mInitCallback(*this);
         }
      }

      dtCore::Camera* camera = mView->GetCamera();
      if (camera != NULL)
      {
         // since we set the perspective,
         if (mUseAspectRatioForFOV)
         {
            camera->SetPerspectiveParams(mFOVAspectVertical[1], mFOVAspectVertical[0],
                     GetNearClippingPlane(), GetFarClippingPlane());
         }
         else
         {
            camera->SetPerspectiveParams(mFOVHorizontalVertical[1], mFOVHorizontalVertical[0]/mFOVHorizontalVertical[1],
                     GetNearClippingPlane(), GetFarClippingPlane());
         }

         if (mParentCamera.valid())
         {
            if (camera->GetParent() != mParentCamera.get())
            {
               camera->Emancipate();
               mParentCamera->AddChild(camera);
            }
            dtCore::Transform xform;
            camera->GetTransform(xform, dtCore::Transformable::REL_CS);
            xform.SetRotation(mAttachCameraRotation);
            camera->SetTransform(xform, dtCore::Transformable::REL_CS);
         }

         camera->SetName(GetName());
      }
      mWindow->SetName(GetName());
      mWindow->SetWindowTitle(GetWindowTitle());

      SetIsUpdated(false);
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::OnRemove(dtGame::GameManager& gameManager)
   {
      GetWindow().GetOsgViewerGraphicsWindow()->getEventQueue()->closeWindow();
      dtCore::View& view = GetView();
      gameManager.GetApplication().RemoveView(view);

      if (view.GetCamera() != NULL)
      {
         dtCore::Camera& camera = *view.GetCamera();
         camera.Emancipate();
         if (camera.GetSceneParent() != NULL)
         {
            camera.GetSceneParent()->RemoveDrawable(&camera);
         }
         camera.SetWindow(NULL);
      }

      view.SetCamera(NULL);

      if (GetRemoveCallback().valid())
      {
         GetRemoveCallback()(*this);
      }

      mAddedToApplication = false;
   }

   //////////////////////////////////////////////////////////////////////////
   dtCore::View& ViewWindowWrapper::GetView()
   {
      return *mView;
   }

   //////////////////////////////////////////////////////////////////////////
   const dtCore::View& ViewWindowWrapper::GetView() const
   {
      return *mView;

   }

   //////////////////////////////////////////////////////////////////////////
   dtCore::DeltaWin& ViewWindowWrapper::GetWindow()
   {
      return *mWindow;
   }

   //////////////////////////////////////////////////////////////////////////
   const dtCore::DeltaWin& ViewWindowWrapper::GetWindow() const
   {
      return *mWindow;
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetInitCallback(ViewWindowWrapper::OperationCallback callback)
   {
      mInitCallback = callback;
   }

   //////////////////////////////////////////////////////////////////////////
   ViewWindowWrapper::OperationCallback& ViewWindowWrapper::GetInitCallback()
   {
      return mInitCallback;
   }

   //////////////////////////////////////////////////////////////////////////
   void ViewWindowWrapper::SetRemoveCallback(ViewWindowWrapper::OperationCallback callback)
   {
      mRemoveCallback = callback;
   }

   //////////////////////////////////////////////////////////////////////////
   ViewWindowWrapper::OperationCallback& ViewWindowWrapper::GetRemoveCallback()
   {
      return mRemoveCallback;
   }

}
