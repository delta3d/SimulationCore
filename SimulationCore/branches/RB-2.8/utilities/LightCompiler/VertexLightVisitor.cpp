#include "VertexLightVisitor.h"
#include <osg/Array>
#include <osg/LineSegment>
#include <osgUtil/IntersectVisitor>
#include <osg/Math>

void VertexLightVisitor::ProcessGeometry(const osg::Matrix& mat, osg::Geometry* geom)
{
   osg::Array* vx = geom->getVertexArray();
   osg::Array* nx = geom->getNormalArray();

   if(!vx || !nx)
   {
      std::cout << "Error: vertex attributes must support vertex arrays and contain per vertex normals" << std::endl;
      return;
   }

   osg::Vec4Array* lightIndices = new osg::Vec4Array(vx->getNumElements());
   osg::Vec4Array* lightWeights = new osg::Vec4Array(vx->getNumElements());

   for(unsigned i = 0; i < vx->getNumElements(); ++i)
   {
      osg::Vec3 vert = OSGArrayToVec3(vx, i);
      osg::Vec3 norm = OSGArrayToVec3(nx, i);

      vert = vert * mat;
      norm = osg::Matrix::transform3x3(norm, mat);
      ProcessVertex(vert, norm, lightIndices->at(i), lightWeights->at(i));

      //update the "progress" bar
      if(i % 100 == 0)
      {
         std::cout << ".";
      }      
   }

   geom->setTexCoordArray(1, lightIndices);
   geom->setTexCoordArray(2, lightWeights);

   //geom->setTexCoordArray(1, lightWeights);
} 


void VertexLightVisitor::ProcessVertex(const osg::Vec3& vert, const osg::Vec3& norm, osg::Vec4& index, osg::Vec4& weight)
{

   const LightCollector::LightArray& lights = mLightCollector->GetLightArray();

   LightCollector::LightArray::const_iterator lightsBegin = lights.begin();
   LightCollector::LightArray::const_iterator lightsEnd = lights.end();

   index.set(0.0f, 0.0f, 0.0f, 0.0f);
   weight.set(0.0f, 0.0f, 0.0f, 0.0f);

   //evaluate the effect on each vert from each light
   //float lightContribRed = 0.0f;
   //float lightContribYellow = 0.0f;
   //float lightContribFire = 0.0f;
   for(unsigned lightIndex = 0; lightsBegin != lightsEnd; ++lightsBegin, ++lightIndex)
   {
      const osg::Light* currentLight = lightsBegin->get();
      float result = EvaluateLight(vert, norm, currentLight);

      //if(currentLight->getDiffuse()[1] > 0.5f)
      //{
      //   lightContribYellow += result;
      //}
      //else if (currentLight->getDiffuse()[0] > 0.5f)
      //{
      //   lightContribRed += result;
      //}
      //else //the fire lights are black 
      //{
      //   lightContribFire += result;
      //}

      //store the indices and contribution to the four lights which have the largest effect
      unsigned leastRelevant = 0;
      for(unsigned i = 1; i < 4; ++i)
      {
         if(weight[i] < weight[i - 1])
         {
            leastRelevant = i;
         }
      }

      if(result > weight[leastRelevant])
      {
         weight[leastRelevant] = result;
         index[leastRelevant] = lightIndex;
      }    
   }

   weight[0] = osg::clampBetween(weight[0], 0.0f, 1.0f);
   weight[1] = osg::clampBetween(weight[1], 0.0f, 1.0f);
   weight[2] = osg::clampBetween(weight[2], 0.0f, 1.0f);
   weight[3] = osg::clampBetween(weight[3], 0.0f, 1.0f);
}


float VertexLightVisitor::EvaluateLight(const osg::Vec3& v, const osg::Vec3& norm, const osg::Light* light)
{
   const float lightEpsilon = 0.10f;
   const float constantAtten = 0.01f;
   const float linearAtten = 0.025f;
   const float quadAtten = 0.002f;

   float resultingContribution = 0.0f;

   //const osg::Vec4& lightPos4 = light->getPosition();
   //osg::Vec3 lightPos(lightPos4[0], lightPos4[1], lightPos4[2]);

   //osg::Vec3 vecToLight = lightPos - v;
   //float distToLight = vecToLight.normalize();

   ////move the vert a little bit along the normal so we don't collide with ourselves
   //osg::Vec3 vert = v + (vecToLight * lightEpsilon);
   ////osg::Vec3 vert = v + (norm * lightEpsilon);

   //osg::ref_ptr<osg::LineSegment> lsToLight = new osg::LineSegment(vert, lightPos);



   //move the vert a little bit along the normal so we don't collide with ourselves
   osg::Vec3 vert = v + (norm * lightEpsilon);

   const osg::Vec4& lightPos4 = light->getPosition();
   osg::Vec3 lightPos(lightPos4[0], lightPos4[1], lightPos4[2]);

   osg::Vec3 vecToLight = lightPos - vert;
   float distToLight = vecToLight.normalize();

   //we use an epsilon so the light ray does not collide with the vert or light (if the light is attached to a surface)
   osg::Vec3 lightVectorEpsilon = vecToLight * 0.05;
   osg::ref_ptr<osg::LineSegment> lsToLight = new osg::LineSegment(vert + lightVectorEpsilon, lightPos - lightVectorEpsilon);



   osg::ref_ptr<osgUtil::IntersectVisitor> iv = new osgUtil::IntersectVisitor();
   iv->addLineSegment(lsToLight.get());
   mRootNode->accept(*iv);

   //if we are occluded from light return 0.0f- this is obviously only accounting for direct lighting
   if(iv->hits())
   {
      return resultingContribution;
   }

   //for now just do a simple dot product
   resultingContribution = osg::maximum(0.0f, norm * vecToLight);

   //osg::Vec3 lightDir = light->getDirection();
   //float spot = lightDir * (vert - lightPos); 
   //spot = osg::maximum(0.0f, spot);
   //resultingContribution *= spot;

   //temporarily dont use attenuation
   float atten = 1.0f / ( constantAtten + (linearAtten * distToLight) + (quadAtten * distToLight * distToLight));      
   resultingContribution *=  osg::minimum(1.0f, atten);

   return resultingContribution;
}


osg::Vec3 VertexLightVisitor::OSGArrayToVec3(osg::Array* ptr, unsigned index)
{
   osg::Vec3 vert;

   try
   {
      if(ptr->getType() == osg::Array::Vec3ArrayType)
      {
         vert = static_cast<const osg::Vec3Array*>(ptr)->at(index);
      }
      else if(ptr->getType() == osg::Array::Vec4ArrayType)
      {
         osg::Vec4 tempVert = static_cast<const osg::Vec4Array*>(ptr)->at(index);
         vert[0] = tempVert[0];
         vert[1] = tempVert[1];
         vert[2] = tempVert[2];
      }
      else
      {
         std::cout << std::endl;
         std::cout << "Unsupported vertex format" << std::endl;
      }
   } 
   catch(const std::exception& e) 
   { 
      std::cout << "Exception Thrown: " << e.what() << std::endl;
   }

   return vert;
}

