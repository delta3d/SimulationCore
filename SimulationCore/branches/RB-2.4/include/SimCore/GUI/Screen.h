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
#ifndef SIMCORE_SCREEN_H
#define SIMCORE_SCREEN_H



////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtCore/base.h>
#include <dtCore/refptr.h>
#include <SimCore/Export.h>
#include <SimCore/GUI/AnimationController.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Components
   {
      class HUDElement;
      class HUDGroup;
   }
}

namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // TYPE DEFINITIONS
      //////////////////////////////////////////////////////////////////////////
      typedef AnimationControllerBase AnimControl;
      typedef std::map<std::string, dtCore::RefPtr<AnimControl> > AnimControlMap;



      //////////////////////////////////////////////////////////////////////////
      // SCREEN LISTENER INTERFACE CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ScreenListener
      {
         public:
            /**
             * Callback methods for determining when an animation controller
             * has executed completed or ended.
             */
            virtual void OnAnimationExecuted( AnimControl& controller ) = 0;
            virtual void OnAnimationStarted( AnimControl& controller ) = 0;
            virtual void OnAnimationCompleted( AnimControl& controller ) = 0;
      };



      //////////////////////////////////////////////////////////////////////////
      // SCREEN CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT Screen : public dtCore::Base
      {
         public:
            static const std::string DEFAULT_NAME;
            static const std::string ANIM_TYPE_BOUNDS;
            static const std::string ANIM_TYPE_MOTION;
            static const std::string ANIM_TYPE_SIZE;

            enum FadeStateE
            {
               FADE_OUT = -1,
               FADE_NONE = 0,
               FADE_IN = 1
            };

            Screen( const std::string& name );

            /**
             * Perform the initial setup of the object.
             * @param root Delta object that acts as the root attach point
             *        for the screen drawables. It is up to the implementation
             *        to cast the root and handle attaching elements to it.
             */
            virtual void Setup(SimCore::Components::HUDGroup* root = NULL) = 0;

            /**
             * Reset all animations and members to the original default values.
             */
            virtual void Reset();

            /**
             * Update the screen object's animations of its self or its sub-elements.
             * @param timeDelta Time in seconds between now and the previous update.
             * @return TRUE if updating occurred or FALSE if disabled.
             */
            virtual bool Update( float timeDelta );

            /**
             * Set the visibility of the screen.
             * @param visible TRUE if the screen should be made visible.
             */
            virtual void SetVisible( bool visible ) {}
            virtual bool IsVisible() const { return false; }

            /**
             * Set whether the screen should be updated or not.
             * @param enabled TRUE to perform updates, FALSE to prevent updates.
             */
            void SetEnabled( bool enabled );
            bool IsEnabled() const;

            /**
             * Set the amount of time that it takes for the screen to completely
             * transition in.
             */
            void SetTimeFadeIn(float seconds);
            float GetTimeFadeIn() const;

            /**
             * Set the amount of time that it takes for the screen to completely
             * transition out.
             */
            void SetTimeFadeOut(float seconds);
            float GetTimeFadeOut() const;

            /**
             * Get the current fade state of this screen.
             */
            FadeStateE GetFadeState() const;

            /**
             * To be called when the screen is entered, usually when an associated
             * game state is entered. Override this to handle screen setup
             * This method should signal the start of transitioning in of elements.
             */
            virtual void OnEnter();

            /**
             * To be called when the screen is entered, usually when an associated
             * game state is entered. Override this to handle screen shutdown.
             * This method should signal the start of transitioning out of elements.
             */
            virtual void OnExit();

            /**
             * To be called when the screen begins transitioning in.
             * This method should signal widgets to begin fade-in animations.
             */
            virtual void OnFadeInStart();

            /**
             * To be called when the screen elements finish transitioning in.
             * This method should signal the implemented screen to enable widgets for input.
             */
            virtual void OnFadeInEnd();

            /**
             * To be called when the screen begins transitioning out.
             * This method should signal widgets to begin fade-out animations.
             */
            virtual void OnFadeOutStart();

            /**
             * To be called when the screen elements finish transitioning out.
             * This method should signal the implemented screen to hide itself.
             */
            virtual void OnFadeOutEnd();

            /**
             * To be called when the screen is being updated by a game state.
             */
            void OnUpdate(float timeDelta);

            /**
             * Convenience method for setting all animation controls to their
             * start targets.
             */
            void SetAnimControlsToStart();
            
            /**
             * Convenience method for setting all animation controls to their
             * end targets.
             */
            void SetAnimControlsToEnd();

         protected:
            virtual ~Screen();

            bool CreateAnimationControl( const std::string& animType,
               SimCore::Components::HUDElement& widget,
               dtCore::RefPtr<AnimControl>& outControl );

            AnimControl* AddAnimationControl( const std::string& animType, SimCore::Components::HUDElement& widget );

            bool AddAnimationControl( const std::string& animType, const std::string name,
               AnimControl& contoller );

            AnimControl* GetAnimationControl( const std::string& animType, const std::string name );
            const AnimControl* GetAnimationControl( const std::string& animType, const std::string name ) const;

         private:
            bool mEnabled;
            FadeStateE mFadeState;
            float mFadeTimer;
            float mTimeFadeIn;
            float mTimeFadeOut;
            AnimControlMap mAnimControls;
      };
   }
}

#endif
