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
#include <SimCore/GUI/AnimationController.h>

namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // ANIMATION CONTROLLER BASE CODE
      //////////////////////////////////////////////////////////////////////////
      AnimationControllerBase::AnimationControllerBase()
         : mEnabled(false)
         , mReversed(false)
         , mSwingRepeat(false)
         , mPaused(false)
         , mRepeatCount(0)
         , mInterpRatio(0.0f)
         , mInterpTimeStart(0.0f)
         , mInterpTime(0.0f)
         , mInterpTimeLimit(1.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      AnimationControllerBase::~AnimationControllerBase()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetRepeatCount( int repeats )
      {
         mRepeatCount = repeats;
      }

      //////////////////////////////////////////////////////////////////////////
      int AnimationControllerBase::GetRepeatCount() const
      {
         return mRepeatCount;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetExecutionCallback( AnimCallback* callback )
      {
         AnimCallback defaultCallback;
         mExecutionFunc = callback != NULL ? *callback : defaultCallback;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::HasExecutionCallback() const
      {
         return mExecutionFunc.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetStartCallback( AnimCallback* callback )
      {
         AnimCallback defaultCallback;
         mStartFunc = callback != NULL ? *callback : defaultCallback;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::HasStartCallback() const
      {
         return mStartFunc.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetEndCallback( AnimCallback* callback )
      {
         AnimCallback defaultCallback;
         mEndFunc = callback != NULL ? *callback : defaultCallback;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::HasEndCallback() const
      {
         return mEndFunc.valid();
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetEnabled( bool enable )
      {
         mEnabled = enable;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::IsEnabled() const
      {
         return mEnabled;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetPaused( bool paused )
      {
         mPaused = paused;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::IsPaused() const
      {
         return mPaused;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetRepeatSwinging( bool swingRepeat )
      {
         mSwingRepeat = swingRepeat;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::IsRepeatSwinging() const
      {
         return mSwingRepeat;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetReversed( bool reversed )
      {
         mReversed = reversed;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::IsReversed() const
      {
         return mReversed;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::IsAtStart() const
      {
         return mInterpRatio <= 0.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      bool AnimationControllerBase::IsAtEnd() const
      {
         return mInterpRatio >= 1.0f;
      }
      
      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetToStart()
      {
         SetInterpolatedTarget( 0.0f );
         mInterpTime = mInterpTimeLimit;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetToEnd()
      {
         SetInterpolatedTarget( 1.0f );
         mInterpTime = mInterpTimeLimit;
      }

      //////////////////////////////////////////////////////////////////////////
      float AnimationControllerBase::GetInterpolationRatio() const
      {
         return mInterpRatio;
      }

      //////////////////////////////////////////////////////////////////////////
      float AnimationControllerBase::GetCurrAnimTime() const
      {
         return mInterpTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetDelayTime( float delayTime )
      {
         mInterpTime = delayTime;
      }

      //////////////////////////////////////////////////////////////////////////
      float AnimationControllerBase::GetDelayTime() const
      {
         return mInterpTimeStart;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::SetTimeLimit( float timeLimit )
      {
         mInterpTimeLimit = timeLimit;
      }

      //////////////////////////////////////////////////////////////////////////
      float AnimationControllerBase::GetTimeLimit() const
      {
         return mInterpTimeLimit;
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::Execute( float timeLimit, float delay, bool reversed )
      {
         mInterpTime = 0.0f;
         mInterpTimeStart = delay;
         mInterpTimeLimit = timeLimit;
         SetReversed( reversed );
         SetEnabled( true );

         if( mExecutionFunc.valid() )
         {
            // Notify something that the animation has been initiated.
            mExecutionFunc( *this );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void AnimationControllerBase::Execute(bool reversed)
      {
         Execute(mInterpTimeLimit, mInterpTimeStart, reversed);
      }

   }
}
