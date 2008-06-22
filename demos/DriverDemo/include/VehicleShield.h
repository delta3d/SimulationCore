/* -*-c++-*-
* Driver Demo
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

