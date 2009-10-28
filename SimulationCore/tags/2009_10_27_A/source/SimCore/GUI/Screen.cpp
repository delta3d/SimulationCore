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
 * Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtDAL/functor.h>
#include <dtUtil/log.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/GUI/Screen.h>
#include <SimCore/GUI/DefaultAnimationControllers.h>
#include <SimCore/GUI/SceneWindow.h>



namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // SCREEN CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string Screen::DEFAULT_NAME("SimCore::GUI::Screen");

      //////////////////////////////////////////////////////////////////////////
      const std::string Screen::ANIM_TYPE_BOUNDS("BOUNDS");
      const std::string Screen::ANIM_TYPE_MOTION("MOTION");
      const std::string Screen::ANIM_TYPE_SIZE("SIZE");

      //////////////////////////////////////////////////////////////////////////
      Screen::Screen( const std::string& name )
         : dtCore::Base(name),
         mEnabled(false)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      Screen::~Screen()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::Reset()
      {
         // Reset all animations
         AnimControlMap::iterator curControl = mAnimControls.begin();
         AnimControlMap::iterator endControlList = mAnimControls.end();
         for( ; curControl != endControlList; ++curControl )
         {
            curControl->second->SetToStart();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool Screen::Update( float timeDelta )
      {
         if( ! IsEnabled() )
         {
            return false;
         }

         AnimControlMap::iterator curControl = mAnimControls.begin();
         AnimControlMap::iterator endControlList = mAnimControls.end();
         for( ; curControl != endControlList; ++curControl )
         {
            curControl->second->Update( timeDelta );
         }

         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::SetEnabled( bool enabled )
      {
         mEnabled = enabled;
      }

      //////////////////////////////////////////////////////////////////////////
      bool Screen::IsEnabled() const
      {
         return mEnabled;
      }

      //////////////////////////////////////////////////////////////////////////
      bool Screen::CreateAnimationControl( const std::string& animType,
         SimCore::Components::HUDElement& widget,
         dtCore::RefPtr<AnimControl>& outControl )
      {
         using namespace SimCore::Components;

         // Make a controller based on the specified animation type and element type.
         if( animType == ANIM_TYPE_MOTION )
         {
            AnimCallbackSetVec2 functor(&widget, &HUDElement::SetPositionByVec);
            dtCore::RefPtr<PositionController> control = new PositionController(&functor);

            osg::Vec2 position;
            widget.GetPosition( position );
            control->SetStartTarget( position );
            control->SetEndTarget( position );
            
            outControl = control.get();
         }
         else if( animType == ANIM_TYPE_BOUNDS )
         {
            AnimCallbackSetVec4 functor(&widget, &HUDElement::SetBoundsByVec);
            dtCore::RefPtr<BoundsController> control = new BoundsController(&functor);

            osg::Vec4 bounds;
            widget.GetBounds( bounds );
            control->SetStartTarget( bounds );
            control->SetEndTarget( bounds );
            
            outControl = control.get();
         }
         else if( animType == ANIM_TYPE_SIZE )
         {
            AnimCallbackSetVec2 functor(&widget, &HUDElement::SetSizeByVec);
            dtCore::RefPtr<SizeController> control = new SizeController(&functor);

            osg::Vec2 dimensions;
            widget.GetSize( dimensions );
            control->SetStartTarget( dimensions );
            control->SetEndTarget( dimensions );

            outControl = control.get();
         }

         return outControl.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      AnimControl* Screen::AddAnimationControl( const std::string& animType,
         SimCore::Components::HUDElement& widget )
      {
         dtCore::RefPtr<AnimControl> newControl;
         if( CreateAnimationControl( animType, widget, newControl ) )
         {
            const std::string& name = widget.GetName();

            if( ! AddAnimationControl( animType, name, *newControl ) )
            {
               LOG_WARNING( ("Could not add  " + animType
                  + " Animation Controller because it already exists: \""
                  + name + "\"") );
            }
         }
         return newControl.get();
      }

      //////////////////////////////////////////////////////////////////////////
      bool Screen::AddAnimationControl(
         const std::string& animType, const std::string name, AnimControl& contoller )
      {
         return mAnimControls.insert( 
            std::make_pair( animType+name, &contoller ) ).second;
      }

      //////////////////////////////////////////////////////////////////////////
      AnimControl* Screen::GetAnimationControl(
         const std::string& animType, const std::string name )
      {
         AnimControlMap::iterator foundIter = mAnimControls.find( animType+name );
         return foundIter != mAnimControls.end() ? foundIter->second.get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const AnimControl* Screen::GetAnimationControl(
         const std::string& animType, const std::string name ) const
      {
         AnimControlMap::const_iterator foundIter = mAnimControls.find( animType+name );
         return foundIter != mAnimControls.end() ? foundIter->second.get() : NULL;
      }

   }
}
