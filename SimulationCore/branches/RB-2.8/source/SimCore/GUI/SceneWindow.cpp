/*
 * Copyright, 2007, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osg/Camera>
#include <osg/PositionAttitudeTransform>
#include <dtUtil/mathdefines.h>
#include <SimCore/GUI/SceneWindow.h>
#include <dtCore/transform.h>
#include <osg/Texture2D>
#include <dtGUI/gui.h>

// TEMP:
#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>

#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>

#include <dtCore/scene.h>
#include <dtCore/view.h>



namespace SimCore
{
   namespace gui
   {
      //////////////////////////////////////////////////////////////////////////
      // HUD SCENE WINDOW CODE
      //////////////////////////////////////////////////////////////////////////
      SceneWindow::SceneWindow( CEGUI::Window& window )
         : BaseClass(window)
         , mPerspectiveMode(false)
         , mLastVisibilityMask(1)
         , mWindowUnits(-50.0, 50.0, -50.0, 50.0)
         , mCamera(NULL)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SceneWindow::~SceneWindow()
      {
      }

      //////////////////////////////////////////////////////////////////////////
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
      void SceneWindow::InitializeCamera(
#else
      void SceneWindow::InitializeCamera( dtGUI::GUI& mainGUI,
#endif
         int textureWidth, int textureHeight )
      {
         CEGUI::Window* w = GetCEGUIWindow();
         dtCore::RefPtr<osg::Texture2D> rttTex;// = mainGUI.CreateRenderTargetTexture(*w, NULL, "RTT", "RTTImage");
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         GetOrCreateOSGTexture(rttTex, *w, textureWidth, textureHeight);
#else
         GetOrCreateOSGTexture(rttTex, mainGUI, *w, textureWidth, textureHeight);
#endif
         //osg::Vec2 viewDims(w->getPixelSize().d_width, w->getPixelSize().d_height);
         osg::Vec2 viewDims(textureWidth, textureHeight);
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         mCamera = new dtCore::Camera();

         //mCamera->setRenderOrder(osg::Camera::PRE_RENDER, 0);
         mCamera->GetOSGCamera()->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         mCamera->GetOSGCamera()->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
         mCamera->GetOSGCamera()->detach( osg::Camera::COLOR_BUFFER );
         mCamera->GetOSGCamera()->attach( osg::Camera::COLOR_BUFFER, rttTex.get() );
         mCamera->GetOSGCamera()->setNodeMask(0xFFFFFFFF);
         mCamera->GetOSGCamera()->setViewport(0, 0, textureWidth, textureHeight);
         mCamera->GetOSGCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         mCamera->GetOSGCamera()->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f) );
#else
         mCamera = mainGUI.CreateCameraForRenderTargetTexture(*rttTex, viewDims);
#endif

         dtCore::Transform xform;
         xform.SetRotation(0.0, -90.0f, 0.0f);
         xform.SetTranslation(0.0, 0.0, 1.0f);
         mCamera->SetTransform(xform);

         if (mSceneNode.valid())
         {
            // Reset the set scene node if it was set before call this init function.
            SetSceneNode(mSceneNode);
         }

         SetWindowUnits( mWindowUnits, false );
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::Camera& SceneWindow::GetCamera()
      {
         return *mCamera;
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::Camera& SceneWindow::GetCamera() const
      {
         return *mCamera;
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::SetWindowUnits( const osg::Vec4& units, bool perspectiveMode )
      {
         mWindowUnits = units;
         mPerspectiveMode = perspectiveMode;

         if( mPerspectiveMode )
         {
            // X - Field-Of-View
            // Y - Aspect Ratio (Width/Height)
            // Z - Near
            // W - Far
            mCamera->SetPerspectiveParams( units.x(), units.y(), 0.01f, 1000.0f);//units.z(), units.w() );
         }
         else
         {
            // X - Left
            // Y - Right
            // Z - Bottom
            // W - Top
            mCamera->GetOSGCamera()->setProjectionMatrixAsOrtho( units.x(), units.y(), units.z(), units.w(), 0.01f, 1000.0f);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec4& SceneWindow::GetWindowUnits() const
      {
         return mWindowUnits;
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::SetVisible( bool visible )
      {
         BaseClass::SetVisible( visible );

         if( mCamera.valid() )
         {
            osg::Camera& cam = *mCamera->GetOSGCamera();
            if(cam.getNodeMask() != 0)
            {
               mLastVisibilityMask = cam.getNodeMask(); 
            }

            cam.setNodeMask( visible ? mLastVisibilityMask : 0 );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool SceneWindow::IsPerspectiveMode() const
      {
         return mPerspectiveMode;
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::SetViewCentered()
      {
         osg::Vec2 zeroVec;
         SetViewCenter( zeroVec );
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::SetViewCenter(const osg::Vec2& point )
      {
         osg::Vec3 pos;
         dtCore::Transform xform;
         mCamera->GetTransform(xform);
         xform.GetTranslation(pos);
         pos.set(point.x(), point.y(), pos.z());
         xform.SetTranslation(pos);
         mCamera->SetTransform(xform);
      }

      //////////////////////////////////////////////////////////////////////////
      float SceneWindow::GetViewHeight() const
      {
         return dtUtil::Abs( mWindowUnits.z() - mWindowUnits.w() );
      }
      
      //////////////////////////////////////////////////////////////////////////
      float SceneWindow::GetViewWidth() const
      {
         return dtUtil::Abs( mWindowUnits.x() - mWindowUnits.y() );
      }
      
      //////////////////////////////////////////////////////////////////////////
      osg::Vec2 SceneWindow::GetViewArea() const
      {
         return osg::Vec2( GetViewWidth(), GetViewHeight() );
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec2 SceneWindow::GetViewCenter() const
      {
         osg::Vec3 pos;
         dtCore::Transform xform;
         mCamera->GetTransform(xform);
         xform.GetTranslation(pos);
         return osg::Vec2(pos.x(), pos.y());
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::GetOrCreateOSGTexture(dtCore::RefPtr<osg::Texture2D>& outTexture,
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
         CEGUI::Window& widget,
#else
         dtGUI::GUI& mainGUI, CEGUI::Window& widget,
#endif
         int textureWidth, int textureHeight)
      {
         // Determine if an image already exists for the widget.
         const CEGUI::Image* image = NULL;
         if(widget.isPropertyPresent("Image"))
         {
            image = CEGUI::PropertyHelper::stringToImage(widget.getProperty("Image"));
         }

         if( image == NULL )
         {
            // Generate an image set with a unique name.
            std::string imagesetName = "RenderTargetTexture." + std::string(widget.getName().c_str());
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
            while( CEGUI::ImagesetManager::getSingleton().isImagesetPresent(imagesetName) )
#else
            while( CEGUI::ImagesetManager::getSingleton().isDefined(imagesetName) )
#endif
            {
               imagesetName = imagesetName + "X";
            }

            // Create and assign the texture to the widget.
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
            dtGUI::CEGUITexture* texture = (dtGUI::CEGUITexture *)(CEGUI::System::getSingleton().getRenderer()->createTexture(textureWidth));
            outTexture = texture->GetOSGTexture();
#else
            osg::Vec2 texSize(textureWidth, textureHeight);
            outTexture = mainGUI.CreateRenderTargetTexture(widget, &texSize, imagesetName);
#endif
         }
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Geode* SceneWindow::CreateQuad( osg::Texture2D* tex, int renderBin )
      {
         osg::Geometry* geo = new osg::Geometry;
         geo->setUseDisplayList( false );

         osg::Vec4Array* colors = new osg::Vec4Array;
         colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
         geo->setColorArray(colors);
         geo->setColorBinding(osg::Geometry::BIND_OVERALL);

         osg::Vec3Array *vx = new osg::Vec3Array;
         vx->push_back(osg::Vec3(-10, -10, 0));
         vx->push_back(osg::Vec3(10, -10, 0));
         vx->push_back(osg::Vec3(10, 10, 0 ));
         vx->push_back(osg::Vec3(-10, 10, 0));
         geo->setVertexArray(vx);

         osg::Vec3Array *nx = new osg::Vec3Array;
         nx->push_back(osg::Vec3(0, 0, 1));
         geo->setNormalArray(nx);

         osg::Vec2Array *tx = new osg::Vec2Array;
         tx->push_back(osg::Vec2(0, 0));
         tx->push_back(osg::Vec2(1, 0));
         tx->push_back(osg::Vec2(1, 1));
         tx->push_back(osg::Vec2(0, 1));
         geo->setTexCoordArray(0, tx);

         if(tex != NULL)
         {
            geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
         }

         geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
         osg::Geode *geode = new osg::Geode;
         geode->addDrawable(geo);
         geode->setCullingActive(false);

         osg::StateSet* ss = geode->getOrCreateStateSet();
         ss->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
         ss->setMode( GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
         ss->setRenderBinDetails( renderBin, "RenderBin" );

         return geode;
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::SetSceneNode(osg::Group* node)
      {
         // Don't change this code to check for the node being the same as mSceneNode
         // becaues it will break the init camera function.

         if (mSceneNode .valid() && mCamera.valid())
         {
            mCamera->GetOSGCamera()->removeChild(mSceneNode);
         }

         mSceneNode = node;

         if (mSceneNode.valid() && mCamera.valid())
         {
            mCamera->GetOSGCamera()->addChild(mSceneNode);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Group* SceneWindow::GetSceneNode()
      {
         return mSceneNode;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Group* SceneWindow::GetSceneNode() const
      {
         return mSceneNode;
      }

   }
}
