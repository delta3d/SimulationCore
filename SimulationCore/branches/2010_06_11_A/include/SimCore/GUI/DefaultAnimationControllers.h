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
      // VEC INTERPOLATOR CODE - for vec 2, 3 & 4
      //////////////////////////////////////////////////////////////////////////
      template<typename T_VEC>
      class SIMCORE_EXPORT VecInterpolator
      {
         public:
            static void GetInterpolatedTarget( float interpolationRatio,
               const T_VEC& startTarget, const T_VEC& endTarget,
               T_VEC& outInterpolatedTarget )
            {
               T_VEC normal(endTarget - startTarget);
               float displacement = normal.length() * interpolationRatio;

               normal.normalize();
               normal *= displacement;
               outInterpolatedTarget = normal + startTarget;
            }

         private:
            VecInterpolator(){}
            virtual ~VecInterpolator(){}
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
      // VEC3 (COLOR - RGB) CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT Vec3Controller : public AnimationController<osg::Vec3> 
      {
         public:
            typedef AnimationController<osg::Vec3> BaseClass;

            Vec3Controller( AnimCallbackSetVec3* callback = NULL );

         protected:
            virtual ~Vec3Controller();
      };



      //////////////////////////////////////////////////////////////////////////
      // VEC4 (COLOR - RGBA) CONTROLLER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT Vec4Controller : public AnimationController<osg::Vec4> 
      {
         public:
            typedef AnimationController<osg::Vec4> BaseClass;

            Vec4Controller( AnimCallbackSetVec4* callback = NULL );

         protected:
            virtual ~Vec4Controller();
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



      //////////////////////////////////////////////////////////////////////////
      // TYPEDEFS - Simple Controllers
      //////////////////////////////////////////////////////////////////////////
      typedef Vec2Controller PositionController;
      typedef Vec2Controller SizeController;
      typedef Vec3Controller Color3Controller;
      typedef Vec4Controller Color4Controller;
   }
}

#endif
