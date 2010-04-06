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

#ifndef SIMCORE_DEFAULT_ANIMATION_CONTROLLERS_H
#define SIMCORE_DEFAULT_ANIMATION_CONTROLLERS_H



////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <SimCore/GUI/AnimationController.h>



namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // VEC2 INTERPOLATOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT Vec2Interpolator
      {
         public:
            static void GetInterpolatedTarget( float interpolationRatio,
               const osg::Vec2& startTarget, const osg::Vec2& endTarget,
               osg::Vec2& outInterpolatedTarget );

         private:
            Vec2Interpolator(){}
            virtual ~Vec2Interpolator(){}
      };



      //////////////////////////////////////////////////////////////////////////
      // BOUNDS INTERPOLATOR CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BoundsInterpolator
      {
         public:
            static void GetInterpolatedTarget( float interpolationRatio,
               const osg::Vec4& startTarget, const osg::Vec4& endTarget,
               osg::Vec4& outInterpolatedTarget );

         private:
            BoundsInterpolator(){}
            virtual ~BoundsInterpolator(){}
      };



      //////////////////////////////////////////////////////////////////////////
      // VEC2 (POSITION & SIZE) CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT Vec2Controller : public AnimationController<osg::Vec2> 
      {
         public:
            typedef AnimationController<osg::Vec2> BaseClass;

            Vec2Controller( AnimCallbackSetVec2* callback = NULL );

         protected:
            virtual ~Vec2Controller();
      };



      //////////////////////////////////////////////////////////////////////////
      // POSITION CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT PositionController : public Vec2Controller
      {
         public:
            typedef Vec2Controller BaseClass;

            PositionController( AnimCallbackSetVec2* callback = NULL );

         protected:
            virtual ~PositionController();
      };



      //////////////////////////////////////////////////////////////////////////
      // SIZE CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT SizeController : public Vec2Controller
      {
         public:
            typedef Vec2Controller BaseClass;

            SizeController( AnimCallbackSetVec2* callback = NULL );

         protected:
            virtual ~SizeController();
      };



      //////////////////////////////////////////////////////////////////////////
      // BOUNDS CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BoundsController : public AnimationController<osg::Vec4> 
      {
         public:
            typedef AnimationController<osg::Vec4> BaseClass;

            BoundsController( AnimCallbackSetVec4* callback = NULL );

         protected:
            virtual ~BoundsController();
      };

   }
}

#endif
