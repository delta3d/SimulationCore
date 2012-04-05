/* -*-c++-*-
* Driver Demo
* @author Bradley Anderegg
*/

#include "VehicleShield.h"

#include <dtCore/shadermanager.h>
#include <dtCore/system.h>

#include <dtDAL/project.h>

#include <dtUtil/noisetexture.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <dtUtil/matrixutil.h>

#include <osg/BlendFunc>
#include <osg/Image>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Texture3D>
#include <osg/MatrixTransform>

#include <osgDB/ReadFile>

namespace DriverDemo
{

   bool VehicleShield::mInitSuccess(false);
   dtCore::RefPtr<osg::Uniform> VehicleShield::mOffset;
   dtCore::RefPtr<osg::Uniform> VehicleShield::mShieldColor;
   dtCore::RefPtr<osg::Uniform> VehicleShield::mNoiseTexture;
   dtCore::RefPtr<osg::BlendFunc> VehicleShield::mBlendFunc;
   dtCore::RefPtr<osg::Node> VehicleShield::mForceField;
   dtCore::RefPtr<osg::Texture3D> VehicleShield::mTexture;
   dtCore::RefPtr<dtCore::ShaderProgram> VehicleShield::mShaderProgram;


   VehicleShield::VehicleShield()
      : mOSGNode(new osg::MatrixTransform)
   {
      if(!mInitSuccess) 
      {
         Init();
      }

      ConfigInstance(mOSGNode->getOrCreateStateSet(), *mOSGNode);
   }

   VehicleShield::~VehicleShield()
   {

   }

   osg::Node* VehicleShield::GetOSGNode()
   {
      return mOSGNode.get();
   }

   const osg::Node* VehicleShield::GetOSGNode() const
   {
      return mOSGNode.get();
   }

   void VehicleShield::SetTranslation(const osg::Vec3& trans)
   {
      osg::Matrix mat = mOSGNode->getMatrix();
      dtUtil::MatrixUtil::SetRow(mat, trans, 3);
      mOSGNode->setMatrix(mat);
   }

   void VehicleShield::Init()
   {
      LOG_INFO("Creating noise texture.");
      dtUtil::NoiseTexture noise3d(6, 2, 0.7, 0.5, 16, 16, 16);
      dtCore::RefPtr<osg::Image> img = noise3d.MakeNoiseTexture(GL_ALPHA);
      LOG_INFO("Finished creating noise texture.");

      dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
      mShaderProgram = sm.FindShaderPrototype("ShieldShader");
      if(!mShaderProgram.valid())
      {
         LOG_ERROR("Error Loading Shader");
         throw dtUtil::Exception("Unable to locate shader for VehicleShield", __FUNCTION__, __LINE__);
      }

      mTexture = new osg::Texture3D();
      mTexture->setImage(img.get());

      mTexture->setFilter( osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR );
      mTexture->setFilter( osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR );
      mTexture->setWrap( osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT );
      mTexture->setWrap( osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT );
      mTexture->setWrap( osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT );


      mBlendFunc = new osg::BlendFunc();
      mBlendFunc->setFunction( osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );

      mOffset = new osg::Uniform( "Offset", 1.0f);
      mShieldColor = new osg::Uniform( "ShieldColor", osg::Vec3(0.15f, 0.2f, 1.0f) );

      mNoiseTexture = new osg::Uniform( osg::Uniform::SAMPLER_3D, "NoiseTexture" );
      mNoiseTexture->set( 0 );

      dtDAL::ResourceDescriptor forceFieldResource("StaticMeshes:Sphere_ForceField.ive");
      std::string fullPath = dtDAL::Project::GetInstance().GetResourcePath(forceFieldResource);
      if(fullPath.empty())
      {
         LOG_ERROR("Error Loading StaticMesh '" + forceFieldResource.GetResourceIdentifier() + "'.");
         throw dtUtil::Exception("Unable to locate static mesh '" + forceFieldResource.GetResourceIdentifier() + "' for VehicleShield", __FUNCTION__, __LINE__);
      }

      mForceField = osgDB::readNodeFile(fullPath);

      mInitSuccess = true;
   }

   void VehicleShield::ConfigInstance(osg::StateSet* ss, osg::Group& node)
   {
      dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
      sm.AssignShaderFromPrototype(*mShaderProgram, node);

      ss->addUniform( mOffset.get() ); 
      ss->addUniform( mShieldColor.get() ); 
      ss->addUniform( mNoiseTexture.get() );
   
      ss->setAttributeAndModes(mBlendFunc.get());
      ss->setTextureAttributeAndModes(0, mTexture.get(), osg::StateAttribute::ON);

      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF );

      node.addChild(mForceField.get());
   }

   void VehicleShield::Update()
   {
      mOffset->set(float(dtCore::System::GetInstance().GetSimTimeSinceStartup()));
   }

}//namespace DriverDemo


