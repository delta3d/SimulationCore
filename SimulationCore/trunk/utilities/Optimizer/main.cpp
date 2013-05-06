/**
 * \file main.cpp
 * This is the main program for LevelCompiler, a command-line utility to restructure a terrain model for faster display.
 *
 * The optimization proceeds as follows:
 * #- read the entire model (often an OpenFlight file with many external references, all of which we include).
 * #- print statistics on the input model
 * #- call the function Optimize, requesting all possible optimizations.
 * #- invoke the TileCollector, which seems mainly to eliminate all but one level of detail, then merge geodes and sort by StateSet.
 * #- rebuild the model as a tree as flat as possible
 * #- run an OcclusionQueryVisitor
 * #- call Optimize again
 * #- write the specified output file
 */
#include <iostream>

#include <dtUtil/stringutils.h>
#include <dtUtil/exception.h>
#include <dtUtil/fileutils.h>

#include <osgUtil/Optimizer>
#include "TextureAtlasBuilder.h"
#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ReaderWriter>
#include <osg/Notify>
#include <osgUtil/Simplifier>
#include <osgUtil/Statistics>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>
#include <osgViewer/GraphicsWindow>
#include <osgViewer/Version>

#include <osg/ComputeBoundsVisitor>

#include "SceneSubdivision.h"
#include "TileCollector.h"
#include "GenerateNormalsVisitor.h"


#include <sstream>
#include <vector>

using namespace LevelCompiler;

class GeodeCounter : public osg::NodeVisitor
{
public:

   GeodeCounter()
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      , mNodeCounter(0)
      , mDrawableCounter(0)
   {}

   virtual void apply(osg::Node& node)
   {
      ++mNodeCounter;
      traverse(node);
   }


   /**
   * Visits the specified geode.
   *
   * @param node the geode to visit
   */
   virtual void apply(osg::Geode& node)
   {
      mDrawableCounter += node.getNumDrawables();
      traverse(node);
   }

   unsigned mNodeCounter;
   unsigned mDrawableCounter;

};

class LODPlacer : public osg::NodeVisitor
{
public:

   float mSampleRatio;
   std::vector<osg::Geode*> mGeodes;

   LODPlacer(float sampleRatio)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      , mSampleRatio(sampleRatio)
   {}

   virtual void apply(osg::Node& node)
   {
      traverse(node);
   }


   /**
   * Visits the specified geode.
   *
   * @param node the geode to visit
   */
   virtual void apply(osg::Geode& node)
   {
      mGeodes.push_back(&node);
   }

   void createLODs()
   {
      for(unsigned int geodeCounter = 0; geodeCounter < mGeodes.size(); ++geodeCounter)
      {
         osg::Geode& node = *mGeodes[geodeCounter];

         for(unsigned int i = 0; i < node.getNumParents(); ++i)
         {
            osg::Group* parent = node.getParent(i);
            osg::LOD* lod = dynamic_cast<osg::LOD*>(parent);

            if(lod == NULL)
            {
               osg::ref_ptr<osg::LOD> lodNode = new osg::LOD();

               unsigned int count = 0;
               for(;count < parent->getNumChildren(); ++count)
               {
                  parent->getChild(count)->dirtyBound();

                  lodNode->addChild(parent->getChild(count), 0.0f, parent->getChild(count)->getBound().radius() + mSampleRatio);
               }

               for(count = 0; count < parent->getNumParents(); ++count)
               {
                  parent->getParent(count)->addChild(lodNode.get());
                  parent->getParent(count)->removeChild(parent);
               }
            }
         }
      }
   }

};


#include <osg/OcclusionQueryNode>


// NodeVisitors and utility functions for OcclusionQueryNode

// Use this visitor to insert OcclusionQueryNodes (OQNs) in the
//   visited subgraph. Only one OQN will test any particular node
//   (no nesting). See also OcclusionQueryNonFlatVisitor.
class OcclusionQueryVisitor : public osg::NodeVisitor
{
public:
   OcclusionQueryVisitor();
   virtual ~OcclusionQueryVisitor();

   // Specify the vertex count threshold for performing occlusion
   //   query tests. Nodes in the scene graph whose total child geometry
   //   contains fewer vertices than the specified threshold will
   //   never be tested, just drawn. (In fact, they will br treated as
   //   potential occluders and rendered first in front-to-back order.)
   void setOccluderThreshold( int vertices );
   int getOccluderThreshold() const;

   virtual void apply( osg::OcclusionQueryNode& oqn );
   virtual void apply( osg::Group& group );
   virtual void apply( osg::Geode& geode );

protected:
   void addOQN( osg::Node& node );

   // When an OQR creates all OQNs and each OQN shares the same OQC,
   //   these methods are used to uniquely name all OQNs. Handy
   //   for debugging.
   std::string getNextOQNName();
   int getNameIdx() const { return _nameIdx; }

   osg::ref_ptr<osg::StateSet> _state;
   osg::ref_ptr<osg::StateSet> _debugState;

   unsigned int _nameIdx;

   int _occluderThreshold;
};

unsigned int countGeometryVertices( osg::Geometry* geom )
{
   if (!geom->getVertexArray())
      return 0;

   // TBD This will eventually iterate over the PrimitiveSets and total the
   //   number of vertices actually used. But for now, it just returns the
   //   size of the vertex array.

   return geom->getVertexArray()->getNumElements();
}

class VertexCounter : public osg::NodeVisitor
{
public:
   VertexCounter( int limit )
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
      _limit( limit ),
      _total( 0 ) {}
   ~VertexCounter() {}

   int getTotal() { return _total; }
   bool exceeded() const { return _total > _limit; }
   void reset() { _total = 0; }

   virtual void apply( osg::Node& node )
   {
      // Check for early abort. If out total already exceeds the
      //   max number of vertices, no need to traverse further.
      if (exceeded())
         return;
      traverse( node );
   }

   virtual void apply( osg::Geode& geode )
   {
      // Possible early abort.
      if (exceeded())
         return;

      unsigned int i;
      for( i = 0; i < geode.getNumDrawables(); i++ )
      {
         osg::Geometry* geom = dynamic_cast<osg::Geometry *>(geode.getDrawable(i));
         if( !geom )
            continue;

         _total += countGeometryVertices( geom );

         if (_total > _limit)
            break;
      }
   }

protected:
   int _limit;
   int _total;
};



OcclusionQueryVisitor::OcclusionQueryVisitor()
: osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ),
_nameIdx( 0 ),
_occluderThreshold( 5000 )
{
   // Create a dummy OcclusionQueryNode just so we can get its state.
   // We'll then share that state between all OQNs we add to the visited scene graph.
   osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode;

   _state = oqn->getQueryStateSet();
   _debugState = oqn->getDebugStateSet();
}

OcclusionQueryVisitor::~OcclusionQueryVisitor()
{
   osg::notify( osg::INFO ) <<
      "osgOQ: OcclusionQueryVisitor: Added " << getNameIdx() <<
      " OQNodes." << std::endl;
}

void
OcclusionQueryVisitor::setOccluderThreshold( int vertices )
{
   _occluderThreshold = vertices;
}
int
OcclusionQueryVisitor::getOccluderThreshold() const
{
   return _occluderThreshold;
}

void
OcclusionQueryVisitor::apply( osg::OcclusionQueryNode& oqn )
{
   // A subgraph is already under osgOQ control.
   // Don't traverse further.
   return;
}

void
OcclusionQueryVisitor::apply( osg::Group& group )
{
   if (group.getNumParents() == 0)
   {
      // Can't add an OQN above a root node.
      traverse( group );
      return;
   }

   int preTraverseOQNCount = getNameIdx();
   traverse( group );

   if (getNameIdx() > preTraverseOQNCount)
      // A least one OQN was added below the current node.
      //   Don't add one here to avoid hierarchical nesting.
      return;

   // There are no OQNs below this group. If the vertex
   //   count exceeds the threshold, add an OQN here.
   addOQN( group );
}

void
OcclusionQueryVisitor::apply( osg::Geode& geode )
{
   if (geode.getNumParents() == 0)
   {
      // Can't add an OQN above a root node.
      traverse( geode );
      return;
   }

   addOQN( geode );
}

void
OcclusionQueryVisitor::addOQN( osg::Node& node )
{
   VertexCounter vc( _occluderThreshold );
   node.accept( vc );
   if (vc.exceeded())
   {
      // Insert OQN(s) above this node.
      unsigned int np = node.getNumParents();
      while (np--)
      {
         osg::Group* parent = dynamic_cast<osg::Group*>( node.getParent( np ) );
         if (parent != NULL)
         {
            osg::ref_ptr<osg::OcclusionQueryNode> oqn = new osg::OcclusionQueryNode();
            oqn->addChild( &node );
            parent->replaceChild( &node, oqn.get() );

            oqn->setName( getNextOQNName() );
            // Set all OQNs to use the same query StateSets (instead of multiple copies
            //   of the same StateSet) for efficiency.
            oqn->setQueryStateSet( _state.get() );
            oqn->setDebugStateSet( _debugState.get() );
         }
      }
   }
}

std::string
OcclusionQueryVisitor::getNextOQNName()
{
   std::ostringstream ostr;
   ostr << "OQNode_" << _nameIdx++;
   return ostr.str();
}


//used by the compress texture visitor
class MyGraphicsContext {
public:
   MyGraphicsContext()
   {
      osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
      traits->x = 0;
      traits->y = 0;
      traits->width = 1;
      traits->height = 1;
      traits->windowDecoration = false;
      traits->doubleBuffer = false;
      traits->sharedContext = 0;
      traits->pbuffer = true;

      _gc = osg::GraphicsContext::createGraphicsContext(traits.get());

      if (!_gc)
      {
         osg::notify(osg::NOTICE)<<"Failed to create pbuffer, failing back to normal graphics window."<<std::endl;

         traits->pbuffer = false;
         _gc = osg::GraphicsContext::createGraphicsContext(traits.get());
      }

      if (_gc.valid()) 
      {
         _gc->realize();
         _gc->makeCurrent();
         if (dynamic_cast<osgViewer::GraphicsWindow*>(_gc.get()))
         {
            std::cout<<"Realized graphics window for OpenGL operations."<<std::endl;
         }
         else
         {
            std::cout<<"Realized pbuffer for OpenGL operations."<<std::endl;
         }
      }
   }

   bool valid() const { return _gc.valid() && _gc->isRealized(); }

private:
   osg::ref_ptr<osg::GraphicsContext> _gc;
};

/////////////////////////////////////////////////////
class EmptyDrawableDeleter : public osg::NodeVisitor
{
public:

    EmptyDrawableDeleter()
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
   {}

   /**
   * Visits the specified geode.
   *
   * @param node the geode to visit
   */
   virtual void apply(osg::Geode& node)
   {
      for (unsigned i = 0; i < node.getNumDrawables();)
      {
          osg::Geometry* geom = dynamic_cast<osg::Geometry*>(node.getDrawable(i));
          if (geom != NULL)
          {
              for (unsigned j = 0; j < geom->getNumPrimitiveSets();)
              {
                  // If it an index array AND it has a zero size, then delete it.  It does nothing.
                  osg::DrawElements* ps = dynamic_cast<osg::DrawElements*>(geom->getPrimitiveSet(j));
                  if (ps != NULL && ps->getTotalDataSize() == 0)
                  {
                      geom->removePrimitiveSet(j, 1);
                      std::cout << "Removing Primitive set." << std::endl;
                  }
                  else
                  {
                      ++j;
                  }
              }

              if (geom->getNumPrimitiveSets() == 0)
              {
                  node.removeDrawable(geom);
                  std::cout << "Removing Drawable." << std::endl;
              }
              else
              {
                  ++i;
              }
          }
          else
          {
              ++i;
          }
      }

      if (node.getNumDrawables() == 0)
      {
          std::cout << "Now have empty geode." << std::endl;
      }

      traverse(node);
   }

};

/////////////////////////////////////////////////////////////////////
class CompressTexturesVisitor : public osg::NodeVisitor
{
public:

   CompressTexturesVisitor(osg::Texture::InternalFormatMode internalFormatMode):
      osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
         _internalFormatMode(internalFormatMode) {}

      virtual void apply(osg::Node& node)
      {
         if (node.getStateSet()) apply(*node.getStateSet());
         traverse(node);
      }

      virtual void apply(osg::Geode& node)
      {
         if (node.getStateSet()) apply(*node.getStateSet());

         for(unsigned int i=0;i<node.getNumDrawables();++i)
         {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet()) apply(*drawable->getStateSet());
         }

         traverse(node);
      }

      virtual void apply(osg::StateSet& stateset)
      {
         // search for the existence of any texture object attributes
         for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
         {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            if (texture)
            {
               _textureSet.insert(texture);
            }
         }
      }

      void compress()
      {
         MyGraphicsContext context;
         if (!context.valid())
         {
            osg::notify(osg::NOTICE)<<"Error: Unable to create graphis context, cannot run texture compression."<< std::endl;
            return;
         }

         osg::ref_ptr<osg::State> state = new osg::State;

         for(TextureSet::iterator itr=_textureSet.begin();
            itr!=_textureSet.end();
            ++itr)
         {
            osg::Texture* texture = const_cast<osg::Texture*>(itr->get());

            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
            osg::Texture3D* texture3D = dynamic_cast<osg::Texture3D*>(texture);

            osg::ref_ptr<osg::Image> image = texture2D ? texture2D->getImage() : (texture3D ? texture3D->getImage() : 0);
            if (image.valid() &&
               (image->getPixelFormat()==GL_RGB || image->getPixelFormat()==GL_RGBA) &&
               (image->s()>=32 && image->t()>=32))
            {
               texture->setInternalFormatMode(_internalFormatMode);

               // need to disable the unref after apply, other the image could go out of scope.
               bool unrefImageDataAfterApply = texture->getUnRefImageDataAfterApply();
               texture->setUnRefImageDataAfterApply(false);

               // get OpenGL driver to create texture from image.
               texture->apply(*state);

               // restore the original setting
               texture->setUnRefImageDataAfterApply(unrefImageDataAfterApply);

               image->readImageFromCurrentTexture(0,true);

               texture->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);
            }
         }
      }

      typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
      TextureSet                          _textureSet;
      osg::Texture::InternalFormatMode    _internalFormatMode;

};


/////////////////////////////////////////////////////////////////////
void FlattenTransforms(osg::Node* n)
{
   osgUtil::Optimizer opt;
   opt.optimize(n, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS);
}

/////////////////////////////////////////////////////////////////////
void SimplifyNode(osg::Node* n, float sampleRatio, float maxError, bool smoothing, bool triStrip)
{
   std::cout << "Beginning simplify pass with sample ratio " << sampleRatio << "." << std::endl;

   Simplifier simple;
   simple.setSmoothing(smoothing);
   simple.setDoTriStrip(triStrip);
   simple.setSampleRatio(sampleRatio);
   simple.setMaximumError(maxError);
   n->accept(simple);

   std::cout << "Simplify successful." << std::endl;
}

/////////////////////////////////////////////////////////////////////
bool gCreateAtlas = false;
void Optimize(osg::Node* n)
{
   osgUtil::Optimizer opt;
   opt.optimize(n, osgUtil::Optimizer::STATIC_OBJECT_DETECTION);
   opt.optimize(n, osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS);
   opt.optimize(n, osgUtil::Optimizer::REMOVE_REDUNDANT_NODES);
   opt.optimize(n, osgUtil::Optimizer::SHARE_DUPLICATE_STATE);

   opt.optimize(n, osgUtil::Optimizer::TESSELLATE_GEOMETRY);

   opt.optimize(n, osgUtil::Optimizer::MERGE_GEODES);
   opt.optimize(n, osgUtil::Optimizer::CHECK_GEOMETRY);
   opt.optimize(n, osgUtil::Optimizer::MERGE_GEOMETRY);
   opt.optimize(n, osgUtil::Optimizer::COMBINE_ADJACENT_LODS);
   opt.optimize(n, osgUtil::Optimizer::TESSELLATE_GEOMETRY);

   if(gCreateAtlas)
   {
      // traverse the scene collecting textures into texture atlas.
      TextureAtlasVisitor tav(&opt);
      osg::notify(osg::INFO) << "Building texture atlases" << std::endl;
      n->accept(tav);
      tav.optimize();

      //opt.optimize(n, osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER);
   }

   opt.optimize(n, osgUtil::Optimizer::FLATTEN_BILLBOARDS);

   //opt.optimize(n, osgUtil::Optimizer::SPATIALIZE_GROUPS);
   //opt.optimize(n, osgUtil::Optimizer::ALL_OPTIMIZATIONS&~(TEXTURE_ATLAS_BUILDER | osgUtil::Optimizer::SPATIALIZE_GROUPS));
}


/////////////////////////////////////////////////////////////////////
void PrintNodeCount(osg::Node* n)
{
   int numNodes = 0;
   int numDrawables = 0;

   GeodeCounter gc;
   n->accept(gc);

   numNodes = gc.mNodeCounter;
   numDrawables = gc.mDrawableCounter;;

   std::cout << "Number of nodes: "  << numNodes << std::endl;
   std::cout << "Number of drawable objects: "  << numDrawables << std::endl;

}


/////////////////////////////////////////////////////////////////////
void PrintStats(osg::Node* n)
{
   //print out stats
   osgUtil::StatsVisitor statVis;
   n->accept(statVis);
   statVis.print(std::cout);
}


/////////////////////////////////////////////////////////////////////
void CompressTextures(osg::Node* n)
{         
   CompressTexturesVisitor ctv(osg::Texture::USE_ARB_COMPRESSION);
   n->accept(ctv);
   ctv.compress();  
}

/////////////////////////////////////////////////////////////////////
/** \brief A functor which tests if a character is part of a newline.
* This "predicate" needed to have 'state', the locale member.
*/
class IsEOL : public std::unary_function<char, bool>
{
public:
   bool operator()(char c) const { return c == '\n' || c == '\r'; }

private:
   std::locale mLocale;
};


/////////////////////////////////////////////////////////////////////
void ReadIgnoreProplist(const std::string& path, std::vector<std::string>& resultingList)
{
   dtUtil::FileInfo fi = dtUtil::FileUtils::GetInstance().GetFileInfo(path);
   if (fi.fileType != dtUtil::REGULAR_FILE)
   {
      throw dtUtil::Exception("Unable to open file \"" + path + "\" for a list of properties to ignore when copying from the base map.",
         __FILE__, __LINE__);
   }

   char* c = new char[fi.size + 1];
   memset(c, 0, fi.size + 1);

   std::ifstream ifs(path.c_str(), std::ifstream::in);
   ifs.read(c, fi.size);
   if (ifs.bad())
   {
      delete[] c;
      throw dtUtil::Exception("Unable to read from file \"" + path + "\" for a list of properties to ignore when copying from the base map.",
         __FILE__, __LINE__);

   }

   std::string data(c);

   dtUtil::StringTokenizer<IsEOL>::tokenize(resultingList, data);
}


/////////////////////////////////////////////////////////////////////
void ReadCmdOption(bool& var, const std::string& varName, osg::ArgumentParser& parser)
{
   int varInt = var;
   parser.read(varName, varInt);

   var = ( varInt != 0 );
   if(var)
   {
      std::cout << "Found option " << varName << "." << std::endl;
   }
   else
   {
      std::cout << "option " << varName << " not found." << std::endl;
   }
}


void ReadVariable(float& var, const std::string& varName, osg::ArgumentParser& parser)
{   
   bool foundVar = parser.read(varName, var);
  
   if(foundVar)
   {
      std::cout << "Found option " << varName << ": " << var << "." << std::endl;
   }
   else
   {
      std::cout << "Option " << varName << " not found, default is: " << var << "." << std::endl;
   }
}

osg::BoundingBox ComputeBound(osg::Node* node)
{
   osg::ComputeBoundsVisitor bbv;
   node->accept(bbv);
   return bbv.getBoundingBox();
}

int main(int argc, char** argv)
{

   osg::Timer_t start_tick = osg::Timer::instance()->tick();

   osg::ArgumentParser parser(&argc, argv);

   parser.getApplicationUsage()->setApplicationName(parser.getApplicationName());
   parser.getApplicationUsage()->setCommandLineUsage(parser.getApplicationName()+" [options] filename ...");
   parser.getApplicationUsage()->addCommandLineOption("-h or --help", "Display command line options");
   parser.getApplicationUsage()->addCommandLineOption("--removeAlpha", "Setting this option true removes all geometry with alpha from the scene.");
   parser.getApplicationUsage()->addCommandLineOption("--removeBillboards", "Setting this option true removes all osg::BillboardNodes from the scene.");
   parser.getApplicationUsage()->addCommandLineOption("--regenerateNormals", "Setting this option will compute normals for the geometry, existing normals will be overwritten.");
   parser.getApplicationUsage()->addCommandLineOption("--computeNormals", "Setting this option will compute normals for the geometry which does not have existing normals.");
   parser.getApplicationUsage()->addCommandLineOption("--createAtlas", "Setting this option will create a texture atlas on the geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--compressTextures", "Setting this option will compress the textures using ARB compression.");
   parser.getApplicationUsage()->addCommandLineOption("--createOccluders", "Setting this option will attempt to create occluder geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--occluderVertCount", "This option specifies the numberr of verts to use for an occluder geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--runOptimizer", "This option is mainly for debugging and does not run the optimizer.");
   parser.getApplicationUsage()->addCommandLineOption("--simplify", "This option will run the Simplifier over the geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--sampleRatio", "The sample ratio for the Simplifier.");
   parser.getApplicationUsage()->addCommandLineOption("--maxError", "The maximum error allowed by the Simplifier.");
   parser.getApplicationUsage()->addCommandLineOption("--smoothing", "Specifies whether the Simplifier will smooth geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--triStrip", "Specifies whether or not to combine triangles into triangle strips.");
   parser.getApplicationUsage()->addCommandLineOption("--subdivide", "This option will subdivide the geometry after compile.");
   parser.getApplicationUsage()->addCommandLineOption("--combineGeodes", "This option will flatten the geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--combineDistance", "The distance to allow combining geometry.");
   parser.getApplicationUsage()->addCommandLineOption("--createLODs", "Setting this option will add LODs to the model.");
   parser.getApplicationUsage()->addCommandLineOption("--lodMidDistance", "The distance to switch to a lower resolution mesh.");
   parser.getApplicationUsage()->addCommandLineOption("--lodMaxDistance", "The far distance the lods will render out to.");
   parser.getApplicationUsage()->addCommandLineOption("--lodBillboardDistance", "The distance to render the billboards out to.");
   parser.getApplicationUsage()->addCommandLineOption("--ignoreNames", "This parameter specifies a file with a list of node names to ignore.");
   parser.getApplicationUsage()->addCommandLineOption("--ignoreSubstrings", "This parameter specifies a file with a list of substring names to ignore.");


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

   bool simplify = false;
   float sampleRatio = 0.5f;
   float maxError = 1.0f;
   bool smoothing = true;
   bool triStrip = false;

   ReadCmdOption(simplify, "--simplify", parser);
   ReadVariable(sampleRatio, "--sampleRatio", parser);
   ReadVariable(maxError, "--maxError", parser);
   ReadCmdOption(smoothing, "--smoothing", parser);
   ReadCmdOption(triStrip, "--triStrip", parser);

   bool subdivide = true;
   ReadCmdOption(subdivide, "--subdivide", parser);   

   bool compressTextures = true;
   ReadCmdOption(compressTextures, "--compressTextures", parser);   
   
   ReadCmdOption(gCreateAtlas, "--createAtlas", parser);    

   bool runOptimizer = true;
   ReadCmdOption(runOptimizer, "--runOptimizer", parser);   

   bool combineGeodes = true;
   ReadCmdOption(combineGeodes, "--combineGeodes", parser);   

   float combineDistance = 500.0f;
   ReadVariable(combineDistance, "--combineDistance", parser);

   bool createLODs = true;
   ReadCmdOption(createLODs, "--createLODs", parser);

   float lodMidDistance = 100.0f;
   ReadVariable(lodMidDistance, "--lodMidDistance", parser);

   float lodMaxDistance = 1000.0f;
   ReadVariable(lodMaxDistance, "--lodMaxDistance", parser);

   float lodBillboardDistance = 1000.0f;
   ReadVariable(lodBillboardDistance, "--lodBillboardDistance", parser);

   bool removeAlpha = false;
   ReadCmdOption(removeAlpha, "--removeAlpha", parser);

   bool removeBillboards = false;
   ReadCmdOption(removeBillboards, "--removeBillboards", parser);

   bool createOccluders = false;
   ReadCmdOption(createOccluders, "--createOccluders", parser);   

   float occluderVertCount = 2500.0f;
   ReadVariable(occluderVertCount, "--occluderVertCount", parser);


   //read ignore list
   std::string ignoreFile;
   std::set<std::string> ignoreSet;
   bool foundVar = parser.read("--ignoreNames", ignoreFile);

   if(foundVar)
   {
      std::cout << "Found option " << "--ignoreNames" << ": " << ignoreFile << "." << std::endl;

      if (!ignoreFile.empty())
      {
         std::vector<std::string> ignoreList;
         ReadIgnoreProplist(ignoreFile, ignoreList); 

         ignoreSet.insert(ignoreList.begin(), ignoreList.end());

         for(unsigned i = 0; i < ignoreList.size(); ++i)
         {
            std::cout << "Found node name to ignore: " << ignoreList[i] << std::endl;
         }
      }
   }
   else
   {
      std::cout << "Option " << "--ignoreNames" << " not found." << std::endl;
   }


   //read ignore list
   std::set<std::string> ignoreSubStringSet;
   foundVar = parser.read("--ignoreSubstrings", ignoreFile);

   if(foundVar)
   {
      std::cout << "Found option " << "--ignoreSubstrings" << ": " << ignoreFile << "." << std::endl;

      if (!ignoreFile.empty())
      {
         std::vector<std::string> ignoreList;
         ReadIgnoreProplist(ignoreFile, ignoreList); 

         ignoreSubStringSet.insert(ignoreList.begin(), ignoreList.end());

         for(unsigned i = 0; i < ignoreList.size(); ++i)
         {
            std::cout << "Found substring to ignore: " << ignoreList[i] << std::endl;
         }
      }
   }
   else
   {
      std::cout << "Option " << "--ignoreSubstrings" << " not found." << std::endl;
   }



   std::cout << std::endl << std::endl << "Loading file: " << parser[1] << "." << std::endl << std::endl;



   //the first argument is the file to load, the second will be the new file to save back out
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(parser[1]);

  if(!node)
   {
      std::cout << "Error loading filename '" << parser[1] << "'." << std::endl;
      parser.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
      return 1;
   }


   //this will write out the original
   //else
   //{
   //   osgDB::writeNodeFile(*node, parser[2]);
   //   return true;
   //}

   if (node->getName().empty())
   {
      node->setName(parser[1]);
   }

   std::cout << "Scene Loaded Successfully." << std::endl;

   PrintNodeCount(node);
   PrintStats(node);

   std::cout << "Initial optimize pass." << std::endl;

   if(runOptimizer)
   {
      Optimize(node);
   }

   osg::setNotifyLevel(osg::ALWAYS);

   int regenerateNormals = 0;
   //by default we will compute missing normals
   int computeNormals = 1;
   parser.read("--regenerateNormals", regenerateNormals);
   parser.read("--computeNormals", computeNormals);

   if(regenerateNormals != 0  || computeNormals != 0)
   {
      if(regenerateNormals != 0)
      {
         std::cout << "Found option: regenerateNormals, regenerating all normals." << std::endl;
      }
      else
      {
         std::cout << "Found option: computeNormals, computing normals where not present." << std::endl;
      }
      GenerateNormalsVisitor gnv(computeNormals!=0, regenerateNormals!=0);
      node->accept(gnv);
   }

   std::cout << "Beginning level compile." << std::endl;

   osg::Group* sceneRoot = new osg::Group();
   sceneRoot->setName("Terrain Root Node");

   if(combineGeodes)
   {
      TileCollector tc(node, removeAlpha, combineDistance, ignoreSet, ignoreSubStringSet);

      std::cout << std::endl << "Rebuilding Tree"  << std::endl;

      TileCollector::DrawableMapping::iterator iter = tc.mDrawableMap.begin();
      TileCollector::DrawableMapping::iterator iterEnd = tc.mDrawableMap.end();

      for(;iter != iterEnd; ++iter)
      {
         TileCollector::GeodeArray::iterator geodeIter = (*iter).second->mGeodeArray.begin();
         TileCollector::GeodeArray::iterator geodeIterEnd = (*iter).second->mGeodeArray.end();
         
         osg::LOD* lodNode = (*iter).second->mParent.get();
         osg::ComputeBoundsVisitor& bounds = *(*iter).second->mBounds;

         for(;geodeIter != geodeIterEnd; ++geodeIter)
         {
            osg::ref_ptr<osg::Geode> g = (*geodeIter);

            if(createLODs)
            {              
               //lodNode->addChild(g.get(), 0.0f, bounds.getBoundingBox().radius() + lodMaxDistance);

               //std::cout << "LOD Radius: " << bounds.getBoundingBox().radius() + lodMaxDistance << std::endl;

               osg::ref_ptr<osg::LOD> lodNode = new osg::LOD();

               osg::BoundingBox bb = ComputeBound(g.get());

               lodNode->setCenterMode(osg::LOD::USER_DEFINED_CENTER);
               lodNode->setCenter(bb.center());               

               //remove all previous parents
               for( unsigned int i = 0; i < g->getNumParents(); ++i )
               {
                  g->getParent(i)->removeChild(g.get());
               }

               lodNode->addChild(g.get(), 0.0f, bb.radius() + lodMaxDistance);

               //std::cout << "LOD Radius: " << bounds.getBoundingBox().radius() + lodMaxDistance << std::endl;

               sceneRoot->addChild(lodNode.get());
            }
            else
            {
               //remove all previous parents
               for ( unsigned int i = 0; i < g->getNumParents(); ++i )
               {
                  g->getParent(i)->removeChild(g.get());
               }

               sceneRoot->addChild(g.get());
            }
         }

         //if(createLODs)
         //{
         //   lodNode->setCenterMode(osg::LOD::USER_DEFINED_CENTER);
         //   lodNode->setCenter(bounds.getBoundingBox().center());

         //   std::cout << "Box radius: " << bounds.getBoundingBox().radius() << ", with center: " << bounds.getBoundingBox().center()[0] << ", " << bounds.getBoundingBox().center()[1] << ", " << bounds.getBoundingBox().center()[2] << std::endl;

         //   sceneRoot->addChild(lodNode);
         //}
      }

      TileCollector::BillboardArray::iterator billIter = tc.mBillboardNodes.begin();
      TileCollector::BillboardArray::iterator billIterEnd = tc.mBillboardNodes.end();

      osg::Group* bbParent = new osg::Group();

      for(;billIter != billIterEnd; ++billIter)
      {
         osg::ref_ptr<osg::Billboard> g = *billIter;

         if(!removeBillboards)
         {
            if(createLODs)
            {
               osg::ref_ptr<osg::LOD> lodNode = new osg::LOD();

               osg::BoundingBox bb = ComputeBound(g.get());

               lodNode->setCenterMode(osg::LOD::USER_DEFINED_CENTER);
               lodNode->setCenter(bb.center());               

               for ( unsigned int i = 0; i < g->getNumParents(); ++i )
                  //for ( unsigned int i = 0; i < tc.mCurrentBB->getNumParents(); ++i )
               {
                  g->getParent(i)->removeChild(g.get());
               }

               lodNode->addChild(g.get(), 0.0f, bb.radius() + lodBillboardDistance);
               bbParent->addChild(lodNode.get());
            }
            else
            {
               bbParent->addChild(g.get());
            }
         }
      }
      
      sceneRoot->addChild(bbParent);

      //add in all the ignored nodes
      osg::Group* ignoreNodeParent = new osg::Group();

      TileCollector::GroupArray::iterator iterTL = tc.mIgnoreList.begin();
      TileCollector::GroupArray::iterator iterTLEnd = tc.mIgnoreList.end();

      for(;iterTL != iterTLEnd; ++iterTL)
      {
         osg::ref_ptr<osg::Node> n = (*iterTL);

         //remove all previous parents
         for( unsigned int i = 0; i < n->getNumParents(); ++i )
         {
            n->getParent(i)->removeChild(n.get());
         }

         ignoreNodeParent->addChild(n.get());

      }

      sceneRoot->addChild(ignoreNodeParent);
   }
   else
   {
      sceneRoot->addChild(node);
   }

   //if we did not have the option to combine geodes 
   //but we still want lods, we have to do it differently
   if(createLODs && !combineGeodes)
   {
      LODPlacer* placer = new LODPlacer(lodMaxDistance);
      sceneRoot->accept(*placer);
      placer->createLODs();
   }

   //if we created lods clean them up
   if(createLODs)
   {
      //this combine nearby lods
      osgUtil::Optimizer opt;
      opt.optimize(sceneRoot, osgUtil::Optimizer::COMBINE_ADJACENT_LODS);
   }

   if (simplify)
   {
      SimplifyNode(sceneRoot, sampleRatio, maxError, smoothing, triStrip);
   }

   {
       EmptyDrawableDeleter edd;
       sceneRoot->accept(edd);
   }

   if(subdivide)
   {
      std::cout << "Subdividing geometry" << std::endl;
      osgUtil::Optimizer opt;
      SpatializeGroupsVisitor* ss = new SpatializeGroupsVisitor(&opt);
      sceneRoot->accept(*ss);
      ss->divide(8);

      /*sceneRoot->accept(*ss);
      ss->divide(4);*/

      /*osgUtil::Optimizer opt;
      opt.optimize(sceneRoot, osgUtil::Optimizer::SPATIALIZE_GROUPS);*/

      std::cout << "Subdivision Finished" << std::endl;
   }

   if(createOccluders)
   {
      std::cout << "Creating Occlusion Query Nodes." << std::endl;

      OcclusionQueryVisitor oqv;
      oqv.setOccluderThreshold(occluderVertCount);
      node->accept( oqv );

      std::cout << "Occlusion Query visitor completed." << std::endl;
   }

   if(compressTextures)
   {
      std::cout << "Compressing textures." << std::endl;
      CompressTextures(sceneRoot);
      std::cout << "Compressing textures completed." << std::endl;
   }
  
   if(runOptimizer)
   {
      std::cout << "Final optimize pass." << std::endl;

      Optimize(sceneRoot);
   }

   //start the save process
   std::string fileNameOut = parser[2];

   std::cout << "Scene successfully compiled." << std::endl;

   PrintNodeCount(sceneRoot);
   PrintStats(sceneRoot);

   std::cout << "Saving result to: " << fileNameOut << std::endl;

   //save file back out
   osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
   //options->setOptionString("OutputTextureFiles");
   //options->setOptionString("noWriteExternalReferenceFiles");
   options->setOptionString("inlineExternalReferencesInIVEFile");
   osgDB::writeNodeFile(*sceneRoot, fileNameOut, options);

   osg::Timer_t end_tick = osg::Timer::instance()->tick();

   std::cout << "Process Completed in approximately: " << osg::Timer::instance()->delta_s(start_tick, end_tick) << " seconds." << std::endl;

   return 0;
}


