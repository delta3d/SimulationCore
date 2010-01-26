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

#ifndef SIMCORE_GUI_SCENE_WINDOW
#define SIMCORE_GUI_SCENE_WINDOW

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <osg/Camera>
#include <dtCore/refptr.h>
#include <SimCore/Export.h>
#include <SimCore/Components/BaseHUDElements.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Group;
   class Node;
   class Texture2D;
}

namespace CEGUI
{
   class Window;
}



namespace SimCore
{
   namespace gui
   {
      //////////////////////////////////////////////////////////////////////////
      // HUD SCENE WINDOW CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT SceneWindow : public SimCore::Components::HUDElement
      {
         public:
            typedef SimCore::Components::HUDElement BaseClass;

            SceneWindow( CEGUI::Window& window );

            virtual void InitializeCamera( osg::Group& sceneNode,
               int textureWidth = 256, int textureHeight = 256 );

            osg::Camera& GetCameraNode();
            const osg::Camera& GetCameraNode() const;

            void SetWindowUnits( const osg::Vec4& units, bool perspectiveMode = false );
            const osg::Vec4& GetWindowUnits() const;

            // Toggle the scene window visibility.
            // @param visible Determine whether the scene window should be visible or not.
            virtual void SetVisible( bool visible );

            bool IsPerspectiveMode() const;

            void SetViewCentered();
            void SetViewCenter(const osg::Vec2& point );
            const osg::Vec2 GetViewCenter() const;

            // Get the view dimensions in terms of window units.
            float GetViewHeight() const;
            float GetViewWidth() const;
            osg::Vec2 GetViewArea() const;

            osg::Texture2D* GetOrCreateOSGTexture( CEGUI::Window& widget,
               int textureWidth = 256, int textureHeight = 256 );
            static osg::Geode* CreateQuad( osg::Texture2D *tex, int renderBin );

         protected:
            virtual ~SceneWindow();

         private:
            bool mPerspectiveMode;
            std::string mCurrentFloor;
            osg::Vec4 mWindowUnits;
            dtCore::RefPtr<osg::Camera> mCamera;
      };
   }
}

#endif
