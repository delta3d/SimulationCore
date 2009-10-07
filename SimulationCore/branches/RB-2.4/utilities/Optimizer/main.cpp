
#include <sstream>

#include <iostream>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>


#include <osgUtil/Optimizer>


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


int main(int argc, char** argv)
{ 
   osg::ArgumentParser parser(&argc, argv);

   parser.getApplicationUsage()->setApplicationName(parser.getApplicationName());
   parser.getApplicationUsage()->setCommandLineUsage(parser.getApplicationName()+" [options] filename ...");
   parser.getApplicationUsage()->addCommandLineOption("-h or --help", "Display command line options");
   parser.getApplicationUsage()->addCommandLineOption("--verbose", "Output debug info.");
   parser.getApplicationUsage()->addCommandLineOption("--optimize", "Run the optimizer over the mesh.");
   parser.getApplicationUsage()->addCommandLineOption("--useOccluders", "Create occlusion query nodes.");

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
   
   int verbose = 1;
   int optimize = 1;
   int useOccluders = 1;
   parser.read("--verbose", verbose);
   parser.read("--optimize", optimize);
   parser.read("--useOccluders", useOccluders);


   std::cout << "Loading File '"<< parser[1] << "'." << std::endl;

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


   if(verbose) std::cout << "Beginning optimization." << std::endl;

   osg::Timer_t start_tick = osg::Timer::instance()->tick();

   if(optimize)
   {
      if(verbose) std::cout << "Starting OSG Optimizer." << std::endl;
      osgUtil::Optimizer opt;
      opt.optimize(node, osgUtil::Optimizer::ALL_OPTIMIZATIONS ^ osgUtil::Optimizer::TRISTRIP_GEOMETRY ^ osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS);// ^ osgUtil::Optimizer::TEXTURE_ATLAS_BUILDER) ^ osgUtil::Optimizer::TRISTRIP_GEOMETRY);
      if(verbose) std::cout << "OSG Optimizer completed." << std::endl;
   }

   if(useOccluders)
   {
      if(verbose) std::cout << "Starting Occlusion Visitor." << std::endl;
      
      OcclusionQueryVisitor oqv;
      node->accept( oqv );

      if(verbose) std::cout << "Occlusion Visitor completed." << std::endl;
   }

   std::cout << "Optimization complete, saving file '" << parser[2] << "." << std::endl;

   //save file back out
   osgDB::writeNodeFile(*node, parser[2]);

   osg::Timer_t end_tick = osg::Timer::instance()->tick();

   std::cout << "Process Completed in approximately: " << osg::Timer::instance()->delta_s(start_tick, end_tick) << " seconds." << std::endl;
   
   return 0;
}


