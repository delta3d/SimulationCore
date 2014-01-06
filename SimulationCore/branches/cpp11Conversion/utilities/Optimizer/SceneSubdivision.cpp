
#include "SceneSubdivision.h"
#include <osgUtil/Simplifier>
#include <osgDB/WriteFile>
#include <iostream>
#include <dtUtil/stringutils.h>

namespace LevelCompiler
{

   void SpatializeGroupsVisitor::apply(osg::Group& group)
   {
      if (isOperationPermissibleForObject(&group))
      {
         mGroupsToDivide.insert(&group);
      }
   
      traverse(group);
   }

   void SpatializeGroupsVisitor::apply(osg::Geode& geode)
   {
      if (isOperationPermissibleForObject(&geode))
      {
         mGeodesToDivide.insert(&geode);
      }
 
      traverse(geode);
   }

   //void SpatializeGroupsVisitor::apply(osg::LOD& node)
   //{
   //   if(isOperationPermissibleForObject(&node))
   //   {
   //      mLODsToCombine.insert(&node);
   //      mGeodesToDivide.insert(&node);
   //   }

   //   traverse(node);
   //}

   bool SpatializeGroupsVisitor::divide(unsigned int maxNumTreesPerCell)
   {
      bool divided = false;
      for(GroupsToDivideList::iterator itr=mGroupsToDivide.begin();
         itr!=mGroupsToDivide.end();
         ++itr)
      {
         if (divide(*itr,maxNumTreesPerCell)) divided = true;
      }

      for(GeodesToDivideList::iterator geode_itr=mGeodesToDivide.begin();
         geode_itr!=mGeodesToDivide.end();
         ++geode_itr)
      {
         if (divide(*geode_itr,maxNumTreesPerCell)) divided = true;
      }

      for(LODsToCombineList::iterator lod_itr = mLODsToCombine.begin(); lod_itr != mLODsToCombine.end(); ++lod_itr)
      {
         combine(*lod_itr, maxNumTreesPerCell);
      }

      return divided;
   }

   bool SpatializeGroupsVisitor::divide(osg::Group* group, unsigned int maxNumTreesPerCell)
   {
      if (group->getNumChildren()<=maxNumTreesPerCell)
      {   
         return false;
      }

      // create the original box.
      osg::BoundingBox bb;
      unsigned int i;
      for(i=0;i<group->getNumChildren();++i)
      {
         bb.expandBy(group->getChild(i)->getBound().center());
      }

      float radius = bb.radius();
      float divide_distance = radius*0.7f;
      bool xAxis = (bb.xMax()-bb.xMin())>divide_distance;
      bool yAxis = (bb.yMax()-bb.yMin())>divide_distance;
      bool zAxis = (bb.zMax()-bb.zMin())>divide_distance;

      //OSG_NOTIFY(osg::INFO)<<"Dividing "<<group->className()<<"  num children = "<<group->getNumChildren()<<"  xAxis="<<xAxis<<"  yAxis="<<yAxis<<"   zAxis="<<zAxis<<std::endl;

      if (!xAxis && !yAxis && !zAxis)
      {
//         OSG_NOTIFY(osg::INFO)<<"  No axis to divide, stopping division."<<std::endl;
         return false;
      }

      unsigned int numChildrenOnEntry = group->getNumChildren();

      typedef std::pair< osg::BoundingBox, osg::ref_ptr<osg::Group> > BoxGroupPair;
      typedef std::vector< BoxGroupPair > Boxes;
      Boxes boxes;
      boxes.push_back( BoxGroupPair(bb,new osg::Group) );

      // divide up on each axis    
      if (xAxis)
      {
         unsigned int numCellsToDivide=boxes.size();
         for(unsigned int i=0;i<numCellsToDivide;++i)
         {
            osg::BoundingBox& orig_cell = boxes[i].first;
            osg::BoundingBox new_cell = orig_cell;

            float xCenter = (orig_cell.xMin()+orig_cell.xMax())*0.5f;
            orig_cell.xMax() = xCenter;
            new_cell.xMin() = xCenter;

            boxes.push_back(BoxGroupPair(new_cell,new osg::Group));
         }
      }

      if (yAxis)
      {
         unsigned int numCellsToDivide=boxes.size();
         for(unsigned int i=0;i<numCellsToDivide;++i)
         {
            osg::BoundingBox& orig_cell = boxes[i].first;
            osg::BoundingBox new_cell = orig_cell;

            float yCenter = (orig_cell.yMin()+orig_cell.yMax())*0.5f;
            orig_cell.yMax() = yCenter;
            new_cell.yMin() = yCenter;

            boxes.push_back(BoxGroupPair(new_cell,new osg::Group));
         }
      }

      if (zAxis)
      {
         unsigned int numCellsToDivide=boxes.size();
         for(unsigned int i=0;i<numCellsToDivide;++i)
         {
            osg::BoundingBox& orig_cell = boxes[i].first;
            osg::BoundingBox new_cell = orig_cell;

            float zCenter = (orig_cell.zMin()+orig_cell.zMax())*0.5f;
            orig_cell.zMax() = zCenter;
            new_cell.zMin() = zCenter;

            boxes.push_back(BoxGroupPair(new_cell,new osg::Group));
         }
      }


      // create the groups to drop the children into


      // bin each child into associated bb group
      typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
      NodeList unassignedList;
      for(i=0;i<group->getNumChildren();++i)
      {
         bool assigned = false;
         osg::Vec3 center = group->getChild(i)->getBound().center();
         for(Boxes::iterator itr=boxes.begin();
            itr!=boxes.end() && !assigned;
            ++itr)
         {
            if (itr->first.contains(center))
            {
               // move child from main group into bb group.
               (itr->second)->addChild(group->getChild(i));
               assigned = true;
            }
         }
         if (!assigned)
         {
            unassignedList.push_back(group->getChild(i));
         }
      }


      // now transfer nodes across, by :
      //      first removing from the original group,
      //      add in the bb groups
      //      add then the unassigned children.


      // first removing from the original group,
      group->removeChildren(0,group->getNumChildren());

      // add in the bb groups
      typedef std::vector< osg::ref_ptr<osg::Group> > GroupList;
      GroupList groupsToDivideList;
      for(Boxes::iterator itr=boxes.begin();
         itr!=boxes.end();
         ++itr)
      {
         // move child from main group into bb group.
         osg::Group* bb_group = (itr->second).get();
         if (bb_group->getNumChildren()>0)
         {
            if (bb_group->getNumChildren()==1)
            {
               group->addChild(bb_group->getChild(0));
            }
            else
            {
               group->addChild(bb_group);
               if (bb_group->getNumChildren()>maxNumTreesPerCell)
               {
                  groupsToDivideList.push_back(bb_group);
               }
            }
         }
      }


      // add then the unassigned children.
      for(NodeList::iterator nitr=unassignedList.begin();
         nitr!=unassignedList.end();
         ++nitr)
      {
         group->addChild(nitr->get());
      }

      // now call divide on all groups that require it.
      for(GroupList::iterator gitr=groupsToDivideList.begin();
         gitr!=groupsToDivideList.end();
         ++gitr)
      {
         divide(gitr->get(),maxNumTreesPerCell);
      }

      return (numChildrenOnEntry<group->getNumChildren());

   }

   bool SpatializeGroupsVisitor::divide(osg::Geode* geode, unsigned int maxNumTreesPerCell)
   {

      if (geode->getNumDrawables()<=maxNumTreesPerCell) return false;

      // create the original box.
      osg::BoundingBox bb;
      unsigned int i;
      for(i=0; i<geode->getNumDrawables(); ++i)
      {
         bb.expandBy(geode->getDrawable(i)->getBound().center());
      }

      float radius = bb.radius();
      float divide_distance = radius*0.7f;
      bool xAxis = (bb.xMax()-bb.xMin())>divide_distance;
      bool yAxis = (bb.yMax()-bb.yMin())>divide_distance;
      bool zAxis = (bb.zMax()-bb.zMin())>divide_distance;

//      OSG_NOTIFY(osg::INFO)<<"INFO "<<geode->className()<<"  num drawables = "<<geode->getNumDrawables()<<"  xAxis="<<xAxis<<"  yAxis="<<yAxis<<"   zAxis="<<zAxis<<std::endl;

      if (!xAxis && !yAxis && !zAxis)
      {
//         OSG_NOTIFY(osg::INFO)<<"  No axis to divide, stopping division."<<std::endl;
         return false;
      }

      osg::Node::ParentList parents = geode->getParents();
      if (parents.empty()) 
      {
//         OSG_NOTIFY(osg::INFO)<<"  Cannot perform spatialize on root Geode, add a Group above it to allow subdivision."<<std::endl;
         return false;
      }

      osg::ref_ptr<osg::Group> group = new osg::Group;
      group->setName(geode->getName());
      group->setStateSet(geode->getStateSet());
      for(i=0; i<geode->getNumDrawables(); ++i)
      {
         osg::Geode* newGeode = new osg::Geode;
         newGeode->addDrawable(geode->getDrawable(i));
         group->addChild(newGeode);
      }

      divide(group.get(), maxNumTreesPerCell);

      // keep reference around to prevent it being deleted.
      osg::ref_ptr<osg::Geode> keepRefGeode = geode;

      for(osg::Node::ParentList::iterator itr = parents.begin();
         itr != parents.end();
         ++itr)
      {
         (*itr)->replaceChild(geode, group.get());
      }

      return true;
   }

   void SpatializeGroupsVisitor::combine(osg::LOD* lod, unsigned int maxNumTreesPerCell)
   {
      //osg::Node::ParentList parents = lod->getParents();
      //if(!parents.empty()) 
      //{
      //   for(osg::Node::ParentList::iterator itr = parents.begin(); itr != parents.end(); ++itr)
      //   {
      //      osg::Group* parent = (*iter)(node).getParent(i);
      //      osg::LOD* lod = dynamic_cast<osg::LOD*>(parent);

      //      if(lod == nullptr)
      //      {
      //         osg::ref_ptr<osg::LOD> lodNode = new osg::LOD();

      //         unsigned int count = 0;
      //         for(;count < parent->getNumChildren(); ++count)
      //         {
      //            parent->getChild(count)->dirtyBound();

      //            lodNode->addChild(parent->getChild(count), 0.0f, parent->getChild(count)->getBound().radius() + mSampleRatio);
      //         }

      //         for(count = 0; count < parent->getNumParents(); ++count)
      //         {
      //            parent->getParent(count)->addChild(lodNode.get());
      //            parent->getParent(count)->removeChild(parent);
      //         }
      //      }
      //   }
      //}

   }

}//namespace LevelCompiler
