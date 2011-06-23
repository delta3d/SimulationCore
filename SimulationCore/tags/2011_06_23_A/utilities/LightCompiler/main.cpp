

#include <iostream>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "LightCollector.h"
#include "VertexLightVisitor.h"
#include "ShaderBuilder.h"

int main(int argc, char** argv)
{ 
   osg::ArgumentParser parser(&argc, argv);

   parser.getApplicationUsage()->setApplicationName(parser.getApplicationName());
   parser.getApplicationUsage()->setCommandLineUsage(parser.getApplicationName()+" [options] filename ...");
   parser.getApplicationUsage()->addCommandLineOption("-h or --help", "Display command line options");
   parser.getApplicationUsage()->addCommandLineOption("--verbose", "Output debug info.");
   parser.getApplicationUsage()->addCommandLineOption("--bindShader", "Creates a shader and binds it to the node.");
   parser.getApplicationUsage()->addCommandLineOption("--uniformName", "Sets the name of the uniform to use for the light array.");
   parser.getApplicationUsage()->addCommandLineOption("--useVertexColor", "Uses the vertex color instead of the texture, if option bindShader is true.");
   parser.getApplicationUsage()->addCommandLineOption("--removeLights", "This flag will remove the osg::Lights in the output file.");
   parser.getApplicationUsage()->addCommandLineOption("--lightEpsilon", "This is a floating point number representing the distance to move each vert towards each light when testing for an LOS, default value is 0.1");

   //the first two arguments are reserved
   if (parser.argc()<=1 || parser.isOption(1) || parser.isOption(2))
   {
      parser.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
      return 1;
   }
   else if (parser.errors())
   {
      parser.writeErrorMessages(std::cout);
      return 1;
   }
   else if(parser.read("-h") || parser.read("--help") || parser.read("-?") || parser.read("--?"))
   {
      parser.getApplicationUsage()->write(std::cout);
      return 1;
   }
   
   int verbose = 0;
   parser.read("--verbose", verbose);

   int bindShader = 0;
   parser.read("--bindShader", bindShader);

   std::string uniformName("LightArray");
   parser.read("--uniformName", uniformName);

   int useVertexColor = 0;
   parser.read("--useVertexColor", useVertexColor);

   int removeLights = 0;
   parser.read("--removeLights", removeLights);

   int bakeLights = 1;
   parser.read("--bakeLights", bakeLights);


   float lightEpsilon = 0.1f;
   parser.read("--lightEpsilon", lightEpsilon);


   //the first argument is the file to load, the second will be the new file to save back out
   osg::Node* node = osgDB::readNodeFile(parser[1]);

   if(!node)
   {
      std::cout << "Error loading filename '" << parser[1] << "'." << std::endl;
      parser.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
      return 1;
   }

   if (node->getName().empty())
   {
      node->setName(parser[1]);
   }


   std::cout << "Beginning light precomputation." << std::endl;
   
   osg::Timer_t start_tick = osg::Timer::instance()->tick();


   osg::ref_ptr<LightCollector> lc = new LightCollector(node, removeLights);

   if(verbose)
   {
      std::cout << "Found " << lc->GetLightArray().size() << " lights." << std::endl;
   }

   if(bakeLights)
   {
      osg::ref_ptr<VertexLightVisitor> lv = new VertexLightVisitor(node, lc.get(), lightEpsilon);
   }

   //if(verbose)
   //{
   //   std::cout << "Found " << lv->GetGeodeCount() << " Geodes, and " << lv->GetDrawableCount() << " Drawables." << std::endl;
   //}

   //if(1)//verbose)
   //{
   //   std::cout << "Processed vertex data." << std::endl;
   //}

   //if(1)//verbose)
   //{
   //   std::cout << "Creating light uniforms." << std::endl;
   //}

   ShaderBuilder sb(node, lc.get());
   sb.CreateUniforms(uniformName);
   
   if(bindShader)
   {
      sb.BindShader(uniformName, useVertexColor);
   }

   //save file back out
   osgDB::writeNodeFile(*node, parser[2]);

   osg::Timer_t end_tick = osg::Timer::instance()->tick();

   std::cout << "Process Completed in approximately: " << osg::Timer::instance()->delta_s(start_tick, end_tick) << " seconds." << std::endl;
   
   return 0;
}


