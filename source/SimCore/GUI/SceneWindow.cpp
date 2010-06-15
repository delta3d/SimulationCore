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
#include <dtGUI/ceguitexture.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/GUI/SceneWindow.h>

// TEMP:
#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>

#ifdef None
#undef None
#endif
#include <CEGUI.h>



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
         , mWindowUnits(-50.0, 50.0, -50.0, 50.0)
         , mCamera(new osg::Camera)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SceneWindow::~SceneWindow()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void SceneWindow::InitializeCamera( osg::Group& sceneNode,
         int textureWidth, int textureHeight  )
      {
         dtCore::RefPtr<osg::Texture2D> texture
            = GetOrCreateOSGTexture( *GetCEGUIWindow(), textureWidth, textureHeight );

         SetWindowUnits( mWindowUnits, false );

         //mCamera->setRenderOrder(osg::Camera::PRE_RENDER, 0);
         mCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         mCamera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
         mCamera->detach( osg::Camera::COLOR_BUFFER );
         mCamera->attach( osg::Camera::COLOR_BUFFER, texture.get() );
         mCamera->setNodeMask(0xFFFFFFFF);
         mCamera->setViewport(0, 0, textureWidth, textureHeight);
         mCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
         mCamera->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f) );

         sceneNode.addChild( mCamera.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Camera& SceneWindow::GetCameraNode()
      {
         return *mCamera;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Camera& SceneWindow::GetCameraNode() const
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
            mCamera->setProjectionMatrixAsPerspective( units.x(), units.y(), units.z(), units.w() );
         }
         else
         {
            // X - Left
            // Y - Right
            // Z - Bottom
            // W - Top
            mCamera->setProjectionMatrixAsOrtho2D( units.x(), units.y(), units.z(), units.w() );
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
            mCamera->setNodeMask( visible ? 0xFFFFFFFF : 0 );//dcsim::enums::NodeMaskFlagsEnum::FLAG_VISIBILITY.GetFlags() : 0x0 );
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
         mCamera->getViewMatrix().setTrans( osg::Vec3( -point.x(), -point.y(), mCamera->getViewMatrix().getTrans().z() ) );
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
         return osg::Vec2( -mCamera->getViewMatrix().getTrans().x(), -mCamera->getViewMatrix().getTrans().y() );
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Texture2D* SceneWindow::GetOrCreateOSGTexture( CEGUI::Window& widget,
         int textureWidth, int textureHeight )
      {
         // get osg-texture for the static image of the cegui-window
         if( ! widget.isPropertyPresent("Image") )
         {
            //            LOG_ERROR( " Property \"Image\" is not available for widget \"" + std::string(pWidget->getName().c_str()) + "\" \n" );
            return 0;
         }

         const CEGUI::Image* image = CEGUI::PropertyHelper::stringToImage(widget.getProperty("Image"));
         if( image == NULL )
         {
            //
            // try to create an image/texture if none present:
            //

            // generate imageset with an unique name:
            std::string imagesetName = "DynamicTexture." + std::string(widget.getName().c_str());
            while( CEGUI::ImagesetManager::getSingleton().isImagesetPresent(imagesetName) )
            {
               imagesetName = imagesetName + "X";
            }

            // create an osg(CEGUI)-Texture:
            dtGUI::CEGUITexture* texture = (dtGUI::CEGUITexture *)(CEGUI::System::getSingleton().getRenderer()->createTexture(textureWidth));

            // create cegui-imageset
            CEGUI::Imageset* imageset = CEGUI::ImagesetManager::getSingleton().createImageset(imagesetName, texture );
            imageset->defineImage("image1", CEGUI::Rect(0, 0, textureWidth, textureHeight), CEGUI::Vector2(0,0));
            widget.setProperty("Image", CEGUI::PropertyHelper::imageToString( &(imageset->getImage("image1"))));

            // apply to window
            image = CEGUI::PropertyHelper::stringToImage(widget.getProperty("Image"));

            // sth. went wrong :-((( ... cleanup
            if( image == NULL )
            {
               CEGUI::ImagesetManager::getSingleton().destroyImageset(imagesetName);
               CEGUI::System::getSingleton().getRenderer()->destroyTexture(texture);
            }
         }

         if( image == NULL )
         {
            //            LOG_ERROR( " invalid CEGUI::Window \n" );
            return 0;
         }

         dtGUI::CEGUITexture* texture = reinterpret_cast<dtGUI::CEGUITexture*>(image->getImageset()->getTexture());
         if( texture == NULL )
         {
            //LOG_ERROR(" invalid dtGUI::Texture \n");
            return 0;
         }

         texture->SetFlipHorizontal(true);
         return texture->GetOSGTexture();
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Geode* SceneWindow::CreateQuad( osg::Texture2D *tex, int renderBin )
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

   }
}
