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
#include <osg/Vec4>
#include <SimCore/GUI/DefaultAnimationControllers.h>


namespace SimCore
{
   namespace GUI
   {
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
         : BaseClass(dtUtil::MakeFunctor(&VecInterpolator<osg::Vec2>::GetInterpolatedTarget))
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
      // VEC3 (COLOR - RGB) CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      Vec3Controller::Vec3Controller( AnimCallbackSetVec3* callback )
         : BaseClass(dtUtil::MakeFunctor(&VecInterpolator<osg::Vec3>::GetInterpolatedTarget))
      {
         if( callback != NULL )
         {
            SetTargetSetCallback( callback );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      Vec3Controller::~Vec3Controller()
      {
      }



      //////////////////////////////////////////////////////////////////////////
      // VEC4 (COLOR - RGBA) CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      Vec4Controller::Vec4Controller( AnimCallbackSetVec4* callback )
         : BaseClass(dtUtil::MakeFunctor(&VecInterpolator<osg::Vec4>::GetInterpolatedTarget))
      {
         if( callback != NULL )
         {
            SetTargetSetCallback( callback );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      Vec4Controller::~Vec4Controller()
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
