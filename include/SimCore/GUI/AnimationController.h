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

#ifndef SIMCORE_ANIMATION_CONTROLLER_H
#define SIMCORE_ANIMATION_CONTROLLER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtUtil/refcountedbase.h>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <dtCore/base.h>
#include <dtUtil/functor.h>
#include <dtUtil/log.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/typetraits.h>



namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // ANIMATION FUNCTOR TYPE DEFINITIONS
      //////////////////////////////////////////////////////////////////////////
      class AnimationControllerBase;
      typedef dtUtil::Functor<void,TYPELIST_1(AnimationControllerBase&) > AnimCallback;
      typedef dtUtil::Functor<void,TYPELIST_1(float) > AnimCallbackSetFloat;
      typedef dtUtil::Functor<void,TYPELIST_1(const osg::Vec2&) > AnimCallbackSetVec2;
      typedef dtUtil::Functor<void,TYPELIST_1(const osg::Vec3&) > AnimCallbackSetVec3;
      typedef dtUtil::Functor<void,TYPELIST_1(const osg::Vec4&) > AnimCallbackSetVec4;



      //////////////////////////////////////////////////////////////////////////
      // ANIMATION CONTROLLER BASE CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT AnimationControllerBase : public std::enable_shared_from_this
      {
         public:
            AnimationControllerBase();

            /**
             * Set the number of times the animation should replay.
             */
            void SetRepeatCount( int repeats );
            int GetRepeatCount() const;

            /**
             * Determine if the animation has finished.
             */
            void SetEnabled( bool enable );
            bool IsEnabled() const;

            /**
             * Determine if the animation should be paused.
             */
            void SetPaused( bool paused );
            bool IsPaused() const;

            /**
             * Determine if the animation should swing back and forth on repeats.
             */
            void SetRepeatSwinging( bool swingRepeat );
            bool IsRepeatSwinging() const;

            /**
             * Determine if the animation occurs backward starting from the end target
             * and transitioning to the start target.
             */
            void SetReversed( bool reversed );
            bool IsReversed() const;

            /**
             * Determine if the animation has NOT started and is waiting at the beginning.
             */
            bool IsAtStart() const;
            bool IsAtEnd() const;

            /**
             * Set the current animation transition to the start target.
             */
            void SetToStart();

            /**
             * Set the current animation transition to the end target.
             */
            void SetToEnd();

            /**
             * Get the current transition point of the animation.
             * @return a value between 0.0 and 1.0; 0.0 being at the start target
             *         and 1.0 at the end target.
             */
            float GetInterpolationRatio() const;

            /**
             * @return the current time (seconds) in the animation cycle, relative to this controller,
             *         ranging from 0.0 to time limit.
             */
            float GetCurrAnimTime() const;

            /**
             * Set the time that the animation should wait after execution before starting.
             */
            void SetDelayTime( float delayTime );

            /**
             * @return the delay time set on execution, measured in seconds.
             */
            float GetDelayTime() const;

            /**
             * @param timeLimit Seconds to complete the animation transition.
             */
            void SetTimeLimit( float timeLimit );

            /**
             * @return the total time over which the animation should occur, including the delay time.
             */
            float GetTimeLimit() const;

            /**
             * Set the function to be called just as the animation is executed (before delay time).
             */
            void SetExecutionCallback( AnimCallback* callback );
            bool HasExecutionCallback() const;

            /**
             * Set the function to be called just before the animation starts (after the delay time).
             */
            void SetStartCallback( AnimCallback* callback );
            bool HasStartCallback() const;

            /**
             * Set the function to be called just after the animation has ended.
             */
            void SetEndCallback( AnimCallback* callback );
            bool HasEndCallback() const;

            /**
             * Set the current transition point of the animation explicitly.
             * NOTE: This function is automatically called in Update.
             * This should be overidden in sub-classes.
             * @param interpolationRatio Value between 0 and 1 that represents the current
             *        transition point in the animation cycle.
             */
            virtual void SetInterpolatedTarget( float interpolationRatio )
            {
            }

            /**
             * Update the animation based on time
             * @param timeDelta time in seconds that the animation should transition
             *        from the current transition point.
             */
            virtual void Update( float timeDelta )
            {
            }

            /**
             * Start the transition animation by setting or resetting states necessary for animation.
             * @param timeLimit Total time over which the transition should animate.
             * @param delay Time to wait before starting the transition animation.
             * @param reversed Determine if the animation should start from the end target to the start target.
             */
            virtual void Execute( float timeLimit, float delay = 0.0f, bool reversed = false );

            /**
             * Start the transition animation by setting or resetting states necessary for animation.
             * This version of the method uses the times previously set on this controller.
             * @param reversed Determine if the animation should start from the end target to the start target.
             */
            virtual void Execute(bool reversed = false);

         protected:
            virtual ~AnimationControllerBase();

            bool mEnabled;
            bool mReversed;
            bool mSwingRepeat;
            bool mPaused;
            int mRepeatCount;
            float mInterpRatio;     // Current transition between start and end positions ( 0.0 to 1.0 ).
            float mInterpTimeStart; // Current interpolation time delay
            float mInterpTime;      // Current interpolation time
            float mInterpTimeLimit; // Interpolation time limit from the instant interpolation begins (after the delay).
            AnimCallback mExecutionFunc;
            AnimCallback mStartFunc;
            AnimCallback mEndFunc;
      };



////////////////////////////////////////////////////////////////////////////////
// Here begins the template class.
// Define the template parameters with a single symbol for easier readability.
#define MY_TEMPLATE_TYPE template <typename T_Target>
#define MY_TYPES T_Target

      //////////////////////////////////////////////////////////////////////////
      // ANIMATION CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      class AnimationController : public AnimationControllerBase
      {
         /**
          * This functor type allows the animation controller to use an
          * arbitrary method/function to handle interpolation between
          * start and end targets.
          * @param [0][ratio] Linear value between 0.0 and 1.0 that determines
          *        the resulting interpolation within the specified start
          *        and end target range.
          * @param [1][startTarget] The minimum/start value in the value range.
          * @param [2][endTarget] The maximum/end value in the value range.
          * @param [3][outInterpolatedTarget] Variable for capturing the resulting
          *        interpolated value.
          */
         typedef dtUtil::Functor<void,TYPELIST_4(float, const T_Target&, const T_Target&, T_Target& ) > AnimCallbackInterpolate;

         /**
          * This functor allows setting the interpolated value on any object or
          * variable that can be set via a method/function. This prevents the
          * Animation Controller from having to maintain a specific and direct
          * reference to an object; in some cases a simple primitive needs to
          * be animated and thus an object reference is undesirable.
          * @param [0][newTargetValue] The newly interpolated value computed in
          *        the Animation Controller.
          */
         typedef dtUtil::Functor<void,TYPELIST_1(const T_Target&) > AnimCallbackSetTarget;

         public:
            typedef AnimationControllerBase BaseClass;

            AnimationController( AnimCallbackInterpolate interpolator );

            /**
             * Set the start target value from which the controller should interpolate.
             */
            void SetStartTarget( const T_Target& startTarget );
            const T_Target& GetStartTarget() const;

            /**
             * Set the end target value to which the controller should interpolate.
             */
            void SetEndTarget( const T_Target& endTarget );
            const T_Target& GetEndTarget() const;

            /**
             * Get the current interpolated value set from the last call to Update.
             */
            const T_Target& GetCurrentTarget() const;

            /**
             * Set the method/function that performs the interpolation between the
             * start and end targets.
             */
            void SetInterpolateCallback( AnimCallbackInterpolate& callback );

            /**
             * Set the method/function that will receive the interpolated value and
             * that will set the value on whatever variable or object property that
             * is under animation control.
             */
            void SetTargetSetCallback( const AnimCallbackSetTarget* callback );
            bool HasTargtSetCallback() const;

            /**
             * Set the current transition point of the animation explicitly.
             * NOTE: This function is automatically called in Update.
             * This should be overidden in sub-classes.
             * @param interpolationRatio Value between 0 and 1 that represents the current
             *        transition point in the animation cycle.
             */
            virtual void SetInterpolatedTarget( float interpolationRatio );

            /**
             * Update the animation based on time
             * @param timeDelta time in seconds that the animation should transition
             *        from the current transition point.
             */
            virtual void Update( float timeDelta );

         protected:
            virtual ~AnimationController();

         private:
            AnimCallbackSetTarget mTargetSetFunc;
            AnimCallbackInterpolate mInterpolatorFunc;
            T_Target mTargetStart;
            T_Target mTargetEnd;
            T_Target mTargetCurrent;
      };



      //////////////////////////////////////////////////////////////////////////
      // IMPLEMENTATION CODE
      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      AnimationController<MY_TYPES >::AnimationController(
         typename AnimationController<MY_TYPES >::AnimCallbackInterpolate interpolatorCallback )
         : AnimationController<MY_TYPES >::BaseClass()
      {
         SetInterpolateCallback( interpolatorCallback );
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      AnimationController<MY_TYPES >::~AnimationController()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      void AnimationController<MY_TYPES >::SetStartTarget( const T_Target& startTarget )
      {
         mTargetStart = startTarget;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      const T_Target& AnimationController<MY_TYPES >::GetStartTarget() const
      {
         return mTargetStart;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      void AnimationController<MY_TYPES >::SetEndTarget( const T_Target& endTarget )
      {
         mTargetEnd = endTarget;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      const T_Target& AnimationController<MY_TYPES >::GetEndTarget() const
      {
         return mTargetEnd;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      const T_Target& AnimationController<MY_TYPES >::GetCurrentTarget() const
      {
         return mTargetEnd;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      void AnimationController<MY_TYPES >::SetInterpolateCallback(
         typename AnimationController<T_Target>::AnimCallbackInterpolate& callback )
      {
         // The assigned callback MUST be valid
         if( ! callback.valid() )
         {
            LOG_ERROR( "Animation controller MUST have a valid callback assigned." );
            return;
         }
         mInterpolatorFunc = callback;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      void AnimationController<MY_TYPES >::SetTargetSetCallback(
         const typename AnimationController<T_Target>::AnimCallbackSetTarget* callback )
      {
         AnimCallbackSetTarget defaultCallback;
         mTargetSetFunc = callback != nullptr ? *callback : defaultCallback;;
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
         bool AnimationController<MY_TYPES >::HasTargtSetCallback() const
      {
         return mTargetSetFunc.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      void AnimationController<MY_TYPES >::SetInterpolatedTarget( float interpolationRatio )
      {
         mInterpRatio = interpolationRatio;
         mInterpolatorFunc( interpolationRatio, mTargetStart, mTargetEnd, mTargetCurrent );

         if( mTargetSetFunc.valid() )
         {
            mTargetSetFunc( mTargetCurrent );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      MY_TEMPLATE_TYPE
      void AnimationController<MY_TYPES >::Update( float timeDelta )
      {
         // Do not do anything if the animation is over or has been paused.
         if( ! mEnabled || mPaused )
         {
            return;
         }

         if( ( mInterpTimeStart == 0.0f && mInterpTime == 0.0f )
            || ( mInterpTime < mInterpTimeStart && mInterpTime + timeDelta >= mInterpTimeStart )
            )
         {
            if( mStartFunc.valid() )
            {
               // Notify something that the animation has started.
               mStartFunc( *this );
            }
         }

         // Step the time tracker forward
         mInterpTime += timeDelta;

         // Find the transition period.
         float interpRatio = mInterpTimeLimit;

         if( interpRatio != 0.0f )
         {
            // Determine how much of a transition has occurred after the delay.
            interpRatio = (mInterpTime-mInterpTimeStart)/interpRatio;
         }

         // Clamp the ratio to 0 or 1 ( 1 if not repeating )
         if( mRepeatCount != 0 )
         {
            // Get the decimal part of the number.
            if( interpRatio > 1.0f )
            {
               interpRatio = interpRatio - int(interpRatio);

               // If swing repeating, the ratio overage should be subtracted
               // from a full transition interpolation (which is 1).
               if( mSwingRepeat )
               {
                  interpRatio = 1.0f - interpRatio;
               }
            }

            if( interpRatio < 0.0f )
            {
               interpRatio = 0.0f;
            }
         }
         else
         {
            dtUtil::Clamp( interpRatio, 0.0f, 1.0f );
         }

         // Invert the resulting ratio if in reverse mode.
         if( mReversed )
         {
            interpRatio = 1.0f - interpRatio;
         }

         SetInterpolatedTarget( interpRatio );

         if( mInterpTime >= mInterpTimeLimit + mInterpTimeStart )
         {
            // Is this the last animation cycle?
            if( mRepeatCount == 0 )
            {
               if( mEndFunc.valid() )
               {
                  // Notify something that the animation is done.
                  mEndFunc( *this );
               }

               SetEnabled( false );
               mSwingRepeat = false;
            }
            else // ...continue repeating
            {
               // If not 0 or lower... (-1 is infinite looping)
               if( mRepeatCount > 0 )
               {
                  --mRepeatCount;
               }

               // Reset the time to the beginning of the animation cycle, just after the time delay.
               mInterpTime = mInterpTimeStart;
            }

            if( mSwingRepeat )
            {
               mReversed = ! mReversed;
            }
         }
      }

////////////////////////////////////////////////////////////////////////////////
// Ensure these macros do not intrude and cause havoc somewhere else.
#undef MY_TEMPLATE_TYPE
#undef MY_TYPES

   }
}

#endif
