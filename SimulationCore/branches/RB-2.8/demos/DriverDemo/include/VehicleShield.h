/* -*-c++-*-
* Driver Demo - VehicleShield (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
* 
* @author Bradley Anderegg
*/

#ifndef DELTA_VehicleShield
#define DELTA_VehicleShield

#include "DriverExport.h"

#include <dtCore/refptr.h>
#include <dtCore/deltadrawable.h>

namespace osg
{
   class Node;
   class Group;
   class MatrixTransform;
   class Uniform;
   class StateSet;
   class Texture3D;
   class BlendFunc;
}

namespace dtCore
{
   class ShaderProgram;
}

namespace DriverDemo
{

   class DRIVER_DEMO_EXPORT VehicleShield: public dtCore::DeltaDrawable
   {
   public:

      VehicleShield();

      void Update();

      /*virtual*/ osg::Node* GetOSGNode();
      /*virtual*/ const osg::Node* GetOSGNode() const;

      void SetTranslation(const osg::Vec3&);

   protected:
      ~VehicleShield();

   private:

      dtCore::RefPtr<osg::MatrixTransform> mOSGNode;

      static void Init();
      static void ConfigInstance(osg::StateSet*, osg::Group&);

      static bool mInitSuccess;
      
      static dtCore::RefPtr<osg::Uniform> mOffset;
      static dtCore::RefPtr<osg::Uniform> mShieldColor;
      static dtCore::RefPtr<osg::Uniform> mNoiseTexture;
      static dtCore::RefPtr<osg::Texture3D> mTexture;
      static dtCore::RefPtr<osg::BlendFunc> mBlendFunc;
      static dtCore::RefPtr<dtCore::ShaderProgram> mShaderProgram;
      static dtCore::RefPtr<osg::Node> mForceField;

   };

} //namespace DriverDemo

#endif //DELTA_VehicleShield

