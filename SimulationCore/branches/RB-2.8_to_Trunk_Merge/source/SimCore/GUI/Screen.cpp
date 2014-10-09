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
#include <prefix/SimCorePrefix.h>
#include <dtCore/functor.h>
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
         : dtCore::Base(name)
         , mEnabled(false)
         , mFadeState(FADE_NONE) // -1 (fade out), 0 (no fade), 1 (fade in)
         , mFadeTimer(0.0f)
         , mTimeFadeIn(0.0f)
         , mTimeFadeOut(0.0f)
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

         if(mFadeState != FADE_NONE)
         {
            mFadeTimer -= timeDelta;
            if(mFadeTimer <= 0.0f)
            {
               mFadeTimer = 0.0f;

               if(mFadeState == FADE_IN)
               {
                  OnFadeInEnd();
               }
               else if(mFadeState == FADE_OUT)
               {
                  OnFadeOutEnd();
               }

               mFadeState = FADE_NONE;
            }
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
      void Screen::SetTimeFadeIn(float seconds)
      {
         mTimeFadeIn = seconds;
      }

      //////////////////////////////////////////////////////////////////////////
      float Screen::GetTimeFadeIn() const
      {
         return mTimeFadeIn;
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::SetTimeFadeOut(float seconds)
      {
         mTimeFadeOut = seconds;
      }
      
      //////////////////////////////////////////////////////////////////////////
      float Screen::GetTimeFadeOut() const
      {
         return mTimeFadeOut;
      }

      //////////////////////////////////////////////////////////////////////////
      Screen::FadeStateE Screen::GetFadeState() const
      {
         return mFadeState;
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnEnter()
      {
         mFadeState = FADE_IN;
         mFadeTimer = mTimeFadeIn;
         OnFadeInStart();
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnExit()
      {
         mFadeState = FADE_OUT;
         mFadeTimer = mTimeFadeOut;
         OnFadeOutStart();
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnFadeInStart()
      {
         SetEnabled(true);
         SetVisible(true);

         // OVERRIDE: to change this functionality or to start fade-in
         // animations for any child widgets.
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnFadeInEnd()
      {
         // OVERRIDE: to enabled any input widgets.
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnFadeOutStart()
      {
         // OVERRIDE: to start fade-out animations for any child widgets.
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnFadeOutEnd()
      {
         SetEnabled(false);
         SetVisible(false);

         // OVERRIDE: to change this functionality or to do additional work.
      }

      //////////////////////////////////////////////////////////////////////////
      void Screen::OnUpdate(float timeDelta)
      {
         Update(timeDelta);
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
