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
#include <osg/vec4>
#include <SimCore/GUI/DefaultAnimationControllers.h>


namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // VEC2 INTERPOLATOR CODE
      //////////////////////////////////////////////////////////////////////////
      void Vec2Interpolator::GetInterpolatedTarget( float interpolationRatio,
         const osg::Vec2& startTarget, const osg::Vec2& endTarget,
         osg::Vec2& outInterpolatedTarget )
      {
         osg::Vec2 normal( endTarget - startTarget );
         float displacement = normal.length() * interpolationRatio;

         normal.normalize();
         normal *= displacement;
         outInterpolatedTarget = normal + startTarget;
      }



      //////////////////////////////////////////////////////////////////////////
      // BOUNDS INTERPOLATOR CODE
      //////////////////////////////////////////////////////////////////////////
      void BoundsInterpolator::GetInterpolatedTarget( float interpolationRatio,
         const osg::Vec4& startTarget, const osg::Vec4& endTarget,
         osg::Vec4& outInterpolatedTarget )
      {
         // Get position & size differences
         osg::Vec4 diff( endTarget - startTarget );

         // Interpolate the current position.
         osg::Vec2 posDiff( diff.x(), diff.y() );
         float displacement = posDiff.length() * interpolationRatio;
         posDiff.normalize();
         posDiff *= displacement;
         posDiff += osg::Vec2( startTarget.x(), startTarget.y() );

         // Interpolate the current size.
         diff.x() = 0.0f;
         diff.y() = 0.0f;
         displacement = diff.length() * interpolationRatio;
         diff.normalize();
         diff *= displacement;
         diff += startTarget;
         diff.x() = posDiff.x();
         diff.y() = posDiff.y();

         outInterpolatedTarget = diff;
      }



      //////////////////////////////////////////////////////////////////////////
      // VEC2 (POSITION & SIZE) CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      Vec2Controller::Vec2Controller( AnimCallbackSetVec2* callback )
         : BaseClass(dtUtil::MakeFunctor(&Vec2Interpolator::GetInterpolatedTarget))
      {
         if( callback != NULL )
         {
            SetTargetSetCallback( callback );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      Vec2Controller::~Vec2Controller()
      {
      }



      //////////////////////////////////////////////////////////////////////////
      // POSITION CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      PositionController::PositionController( AnimCallbackSetVec2* callback )
         : BaseClass(callback)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      PositionController::~PositionController()
      {
      }



      //////////////////////////////////////////////////////////////////////////
      // SIZE CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      SizeController::SizeController( AnimCallbackSetVec2* callback )
         : BaseClass(callback)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SizeController::~SizeController()
      {
      }



      //////////////////////////////////////////////////////////////////////////
      // BOUNDS CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      BoundsController::BoundsController( AnimCallbackSetVec4* callback )
         : BaseClass(dtUtil::MakeFunctor(&BoundsInterpolator::GetInterpolatedTarget))
      {
         if( callback != NULL )
         {
            SetTargetSetCallback( callback );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      BoundsController::~BoundsController()
      {
      }

   }
}
