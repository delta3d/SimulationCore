#ifndef __SHADER_BUILDER_H__
#define __SHADER_BUILDER_H__

#include <osg/Node>
#include <osg/StateSet>
#include <osg/Uniform>
#include <osg/Shader>

#include <sstream>

#include "GeometryCollector.h"
#include "LightCollector.h"

class ShaderBuilder
{
public:
   ShaderBuilder(osg::Node* root, LightCollector* lc)
      : mRootNode(root)
      , mLightCollector(lc)
   {
   }

   void CreateUniforms(const std::string& name)
   {
      const LightCollector::LightArray& lights = mLightCollector->GetLightArray();

      if(lights.size() == 0)
      {
         return;
      }

      osg::ref_ptr<osg::Uniform> lightUniform = new osg::Uniform(osg::Uniform::FLOAT_VEC4, name, lights.size());

      LightCollector::LightArray::const_iterator lightsBegin = lights.begin();
      LightCollector::LightArray::const_iterator lightsEnd = lights.end();

      //evaluate the effect on each vert from each light
      for(unsigned lightIndex = 0; lightsBegin != lightsEnd; ++lightsBegin, ++lightIndex)
      {
         const osg::Light* currentLight = lightsBegin->get();
         osg::Vec4 lightValue = currentLight->getDiffuse();
         lightValue[3] = currentLight->getSpotExponent();
         lightUniform->setElement(lightIndex, lightValue);

         //-no ambient is being set in max, so we are using the
         //lightUniform->setElement(lightIndex, currentLight->getDiffuse() * currentLight->getSpotExponent());//currentLight->getAmbient()); 
         //lightUniform->setElement(lightIndex + 1, currentLight->getDiffuse());
      }

      mRootNode->getOrCreateStateSet()->addUniform(lightUniform.get());
   }

   void BindShader(const std::string& uniformName, bool useVertexColor)
   {
      std::cout << "Creating Shader." << std::endl;

      const LightCollector::LightArray& lights = mLightCollector->GetLightArray();

      std::ostringstream vertex_shader;

      vertex_shader <<                         "const int NUM_STATIC_LIGHTS = " << lights.size() << ";\n";
      vertex_shader <<                         "uniform vec4 " << uniformName << "[NUM_STATIC_LIGHTS];\n";
      //vertex_shader <<                         "uniform float redContribution;\n";
      //vertex_shader <<                         "uniform float yellowContribution;\n";
      //vertex_shader <<                         "uniform vec4 fireContribution;\n";
      //vertex_shader <<                         "uniform float fireIntensity;\n";
      vertex_shader <<                         "varying vec4 ambientContribution;\n";
      vertex_shader <<                         "varying vec4 lightContribution;\n";
      vertex_shader <<                         "void main(void)\n";
      vertex_shader <<                         "{\n";
      vertex_shader <<                         "    gl_Position = ftransform();\n";
      if(useVertexColor) vertex_shader <<      "    gl_Color = gl_FrontColor;\n";
      else vertex_shader <<                    "    gl_TexCoord[0] = gl_MultiTexCoord0;\n";
      vertex_shader <<                         "\n";
      vertex_shader <<                         "    float lightIndices[4];\n";
      vertex_shader <<                         "    float lightWeights[4];\n";
      vertex_shader <<                         "    lightIndices[0] = gl_MultiTexCoord1.x; lightIndices[1] = gl_MultiTexCoord1.y; lightIndices[2] = gl_MultiTexCoord1.z; lightIndices[3] = gl_MultiTexCoord1.w;\n";
      vertex_shader <<                         "    lightWeights[0] = gl_MultiTexCoord2.x; lightWeights[1] = gl_MultiTexCoord2.y; lightWeights[2] = gl_MultiTexCoord2.z; lightWeights[3] = gl_MultiTexCoord2.w;\n";
      vertex_shader <<                         "\n";
      vertex_shader <<                         "   ambientContribution = vec4(0.0, 0.0, 0.0, 1.0);\n";   
      vertex_shader <<                         "   lightContribution = vec4(0.0, 0.0, 0.0, 1.0);\n";    
      vertex_shader <<                         "   for(int i = 0; i < 4; ++i)\n";
      vertex_shader <<                         "      {\n";
      vertex_shader <<                         "         int lightIndex = int(lightIndices[i]);\n";
      vertex_shader <<                         "         float lightWeight = lightWeights[i];\n";
      vertex_shader <<                         "         vec3 currentLight = lightWeight * " << uniformName << "[lightIndex].xyz * " << uniformName << "[lightIndex].w;\n";
      vertex_shader <<                         "         lightContribution += 0.75 * vec4(currentLight.xyz, 0.0);\n";
      vertex_shader <<                         "         ambientContribution += 0.25 * lightWeight * vec4(currentLight.xyz, 0.0);\n";
      vertex_shader <<                         "      }\n";
      //vertex_shader <<                           "lightContribution += yellowContribution * gl_MultiTexCoord1.x * vec4(1.0, 1.0, 0.9, 1.0);\n";
      //vertex_shader <<                           "lightContribution += redContribution * gl_MultiTexCoord1.y * vec4(1.0, 0.1, 0.08, 1.0);\n";
      //vertex_shader <<                           "lightContribution += gl_MultiTexCoord1.z * (fireContribution * fireIntensity);\n";
      vertex_shader <<                           "lightContribution = clamp(lightContribution, 0.0, 1.0);\n";
      vertex_shader <<                         "}\n";
  

      std::ostringstream fragment_shader;

      fragment_shader <<                       "varying vec4 ambientContribution;\n";
      fragment_shader <<                       "varying vec4 lightContribution;\n";
      if(!useVertexColor) fragment_shader <<   "uniform sampler2D diffuseTexture;\n";    
      fragment_shader <<                       "void main(void)\n";
      fragment_shader <<                       "{\n";
      fragment_shader <<                       "\n";
      if(useVertexColor) fragment_shader <<    "    vec4 diffuseColor = gl_Color;\n";
      else fragment_shader <<                  "    vec4 diffuseColor = texture2D(diffuseTexture, gl_TexCoord[0].st);\n";
      fragment_shader <<                       "    gl_FragColor = ambientContribution + (lightContribution * diffuseColor);\n";
      fragment_shader <<                       "}\n";

      osg::ref_ptr<osg::Shader> vertShader = new osg::Shader(osg::Shader::VERTEX, vertex_shader.str());
      osg::ref_ptr<osg::Shader> fragShader = new osg::Shader(osg::Shader::FRAGMENT, fragment_shader.str());

      osg::ref_ptr<osg::Program> shaderProgram = new osg::Program();
      shaderProgram->addShader(vertShader.get());
      shaderProgram->addShader(fragShader.get());

      osg::StateSet* ss = mRootNode->getOrCreateStateSet();
      ss->setAttribute(shaderProgram.get(), osg::StateAttribute::ON);

      if(!useVertexColor)
      {
         osg::ref_ptr<osg::Uniform> texUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "diffuseTexture");
         texUniform->set(0);
         ss->addUniform(texUniform.get());
      }

      std::ofstream fragOut;
      fragOut.open("LightCompilerShader.frag");
      fragOut << fragment_shader.str();

      std::ofstream vertOut;
      vertOut.open("LightCompilerShader.vert");
      vertOut << vertex_shader.str();

      std::cout << "Shader bound to node." << std::endl;
   }


private:

   osg::ref_ptr<osg::Node> mRootNode;
   osg::ref_ptr<LightCollector> mLightCollector;
};


#endif //__SHADER_BUILDER_H__
