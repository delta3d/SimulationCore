#ifndef __TILE_COLLECTOR_H__
#define __TILE_COLLECTOR_H__

#include "Primitives.h"
#include "Util.h"

#include <osg/NodeVisitor>
#include <osg/Transform>
#include <osg/Geode>
#include <osg/Drawable>
#include <osg/TriangleFunctor>
#include <osgUtil/Optimizer>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/ComputeBoundsVisitor>

#include "Simplifier.h"

#include <string>
#include <map>
#include <set>
#include <stack>
#include <dtUtil/stringutils.h>

namespace LevelCompiler
{

   void SimplifyNode(osg::Node* n)
   {
   /*   static int simpCount = 0;
      ++simpCount;
      std::cout << "Simplify Geometry pass num " << simpCount << "." << std::endl;

      if(simpCount != 36 && simpCount != 77 && simpCount != 181 && simpCount != 248 && simpCount != 728 && simpCount != 811 &&
         simpCount != 53 && simpCount != 224 && simpCount != 249)
      {*/
         Simplifier simple;
         simple.setSmoothing(true);
         simple.setSampleRatio(0.5f);
         n->accept(simple);

       /*  osgDB::writeNodeFile(*n ,std::string("./Mesh/simpleGeode") + dtUtil::ToString(simpCount)+ std::string(".ive"));
      }
      else
      {
         osgDB::writeNodeFile(*n ,std::string("./Mesh/badGeode") + dtUtil::ToString(simpCount)+ std::string(".ive"));
      }*/
      
   }



class TileCollector: public osg::NodeVisitor
{
public:

   typedef std::vector<osg::ref_ptr<osg::Geode> > GeodeArray;
   struct CollapsedDrawable
   {      
      osg::ref_ptr<osg::LOD> mParent;
      GeodeArray mGeodeArray;
      TriangleArray* mTriangles;   
      osg::ref_ptr<osg::ComputeBoundsVisitor> mBounds;
   };

   
   typedef osg::ref_ptr<osg::StateSet> StateSetRefPtr;
   typedef std::map<StateSetRefPtr, CollapsedDrawable*, CompareStateSet<StateSetRefPtr> > DrawableMapping;
   DrawableMapping mDrawableMap;

   float mCombineDistance;
   bool mRemoveAlpha;
   osg::ref_ptr<osg::Node> mRootNode;   
   osg::ref_ptr<osg::Group> mComputedTree;   
   osg::ref_ptr<osg::Group> mCurrentNode;   
   osg::ref_ptr<osg::Billboard> mCurrentBB;

   typedef std::vector< osg::ref_ptr<osg::Node> > GroupArray;
   GroupArray mIgnoreList;

   typedef std::vector<osg::ref_ptr<osg::Billboard> > BillboardArray;
   BillboardArray mBillboardNodes;

   typedef std::set<std::string> StringSet;
   StringSet mIgnoreNodeNameSet;
   StringSet mIgnoreSubstringSet;

public:
   TileCollector(osg::Node* rootNode, bool removeAlpha, float combineDistance, const StringSet& ignoreNameList, const StringSet& ignoreSubstrList)
      : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
      , mCombineDistance(combineDistance)
      , mRemoveAlpha(removeAlpha)
      , mRootNode(rootNode)
      , mComputedTree(new osg::Group())
      , mIgnoreNodeNameSet(ignoreNameList.begin(), ignoreNameList.end())
      , mIgnoreSubstringSet(ignoreSubstrList.begin(), ignoreSubstrList.end())
   {
      mCurrentNode = mComputedTree.get();
      mRootNode->accept(*this);
   }

   bool CheckIgnoreNode(osg::Node* node)
   {
      bool shouldIgnore = mIgnoreNodeNameSet.find(node->getName()) != mIgnoreNodeNameSet.end();

      //check substring list
      if(!shouldIgnore)
      {
         StringSet::iterator iter = mIgnoreSubstringSet.begin();
         StringSet::iterator iterEnd = mIgnoreSubstringSet.end();

         for(;iter != iterEnd && !shouldIgnore; ++iter)
         {
            if(node->getName().find(*iter) != std::string::npos)
            {
               shouldIgnore = true;
            }
         }
      }


      if(shouldIgnore)
      {
         std::cout << "Ignoring node with name: " << node->getName() << std::endl;
         mIgnoreList.push_back(node);
      }

      return shouldIgnore;
   }

   virtual void apply(osg::Node& node)
   {
      if(!CheckIgnoreNode(&node))
      {
         traverse(node);      
      }
   }

   virtual void apply(osg::LOD& lod)
   {  
      if(!CheckIgnoreNode(&lod))
      {
         for ( unsigned int i = 0; i < lod.getNumChildren(); ++i)
         {
            osg::Node* n = lod.getChild(i);

            //drop all but the highest lod- this is making the assumption
            //that we have the bandwitdth to render the highest detail for
            //all models in the scene, once scene is reconstructed we can 
            //auto lod down if need be, otherwise we will need to revisit this
            //std::cout << "Min Range: " << lod.getMinRange(i) << "   Max Range: " << lod.getMaxRange(i) << std::endl;
            if(lod.getMinRange(i) < 1.0f)
            {
               //this is basically the same as calling traverse(lod)
               n->accept(*this);
            }
         }
      }
   }

   virtual void apply(osg::Geode& geode)
   {
      if(!CheckIgnoreNode(&geode))
      {
         //simplify
         //SimplifyNode(&geode);

         //then flatten
         Util::FlattenGeodeTransformAndState(getNodePath());
               
         //store them by stateset
         osg::StateSet* ss = geode.getOrCreateStateSet();

         // Turn on mipmapping on ALL textures in the terrain.
         for (unsigned i = 0; i < 16; ++i)
         {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(i, osg::Texture::TEXTURE));

            if (texture != NULL)
            {
               texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
               texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
               if (texture->getMaxAnisotropy() < 4.0f)
               {
                  texture->setMaxAnisotropy(4.0f);
               }
               texture->setUseHardwareMipMapGeneration(true);
            }
         }


         //if this flag is set we will skip all objects with alpha 
         if(mRemoveAlpha)
         {
            osg::BlendFunc* bf = dynamic_cast<osg::BlendFunc*>(ss->getAttribute(osg::StateAttribute::BLENDFUNC));
            if(bf != NULL || ss->getRenderingHint() == osg::StateSet::TRANSPARENT_BIN)
            {
               //skip this drawable
               return;
            } 
         }

         DrawableMapping::iterator iter = mDrawableMap.find(ss);
         
         CollapsedDrawable* cd = NULL;      

         osg::ref_ptr<osg::ComputeBoundsVisitor> cbv = new osg::ComputeBoundsVisitor();
         geode.accept(*cbv);

         if(iter == mDrawableMap.end())
         {
            //initialize a new collapsed drawable
            cd = new CollapsedDrawable();
            cd->mTriangles = new TriangleArray();
            cd->mParent = new osg::LOD();         
            cd->mBounds = new osg::ComputeBoundsVisitor();
            
            cd->mGeodeArray.push_back(&geode);
            cd->mBounds->apply(geode);
      
            mDrawableMap.insert(std::make_pair(ss, cd));
         }
         else
         {
            cd = (*iter).second;
                     
            float distance = (cbv->getBoundingBox().center() - cd->mBounds->getBoundingBox().center()).length();
            if(distance < mCombineDistance)
            {
               cd->mBounds->apply(geode);

               for ( unsigned int i = 0; i < geode.getNumDrawables(); ++i )
               {
                  osg::Drawable* d = geode.getDrawable(i);
                  cd->mGeodeArray.front()->addDrawable(d);            
               }

               geode.removeDrawables(0, geode.getNumDrawables());
            }
            else
            {
               cd->mBounds->apply(geode);
               cd->mGeodeArray.push_back(&geode);
            }
         }
         
      }
   }

   virtual void apply(osg::Billboard& node)
   {
      if(!CheckIgnoreNode(&node))
      {
         //std::cout << "Billboard node- children: " << node.getNumDrawables() << /*"  Axis: " << node.getAxis() << "  Norm: " << node.getNormal() <<*/ "  Num Positions: " << node.getPositionList().size() << std::endl;
         //if(!mCurrentBB.valid())
         {
            //Util::FlattenGeodeTransformAndState(getNodePath());
            mCurrentBB = &node;         
            mBillboardNodes.push_back(&node);

            ////this is an attempt to correct the billboard nodes in the database that don't appear to export/import properly         
            //node.setAxis(osg::Vec3(0.0f, 0.0f, 1.0f));

            //osg::Matrix mat = osg::computeLocalToWorld(getNodePath());

            //for(int i = 0; i < node.getNumDrawables(); ++i)
            //{
            //   osg::Drawable* d = node.getDrawable(i);
            //   node.setPosition(i, node.getPosition(i) + mat.getTrans());            
            //}
         }
         //else
         //{
         //   for(int i = 0; i < node.getNumDrawables(); ++i)
         //   {
         //      osg::Drawable* d = node.getDrawable(i);
         //      int childNum = mCurrentBB->getNumDrawables();
         //      mCurrentBB->addDrawable(d);            
         //      mCurrentBB->setPosition(childNum, node.getPosition(i));
         //   }
         //}
      }
   }

};

}//namespace LevelCompiler

#endif //__TILE_COLLECTOR_H__
