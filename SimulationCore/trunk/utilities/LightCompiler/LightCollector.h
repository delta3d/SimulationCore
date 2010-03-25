#ifndef __LIGHT_COLLECTOR_H__
#define __LIGHT_COLLECTOR_H__



#include <osg/NodeVisitor>
#include <osg/LightSource>
#include <osg/Light>
#include <osg/Group>
#include <osg/Transform>

#include <osg/Switch>
#include <vector>



class LightCollector: public osg::NodeVisitor
{

public:
   typedef std::vector< osg::ref_ptr<osg::Light> > LightArray;
   typedef std::pair<osg::ref_ptr<osg::Group>, osg::ref_ptr<osg::LightSource> > LightSourcePair;
   typedef std::vector<LightSourcePair> LightSources;

public:
   LightCollector(osg::Node* rootNode, bool removeLights)
      : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
      , mRootNode(rootNode)
      , mRemoveLights(removeLights)
   {
      mRootNode->accept(*this);

      if(mRemoveLights) RemoveLights();
   }

   void RemoveLights()
   {
      LightSources::iterator iter = mLightsToRemove.begin();
      LightSources::iterator end = mLightsToRemove.end();

      for(;iter != end; ++iter)
      {
         if((*iter).first.valid())
         {
            osg::Group* grp = (*iter).first.get();
            if(grp != NULL && grp->getParent(0) != NULL)
            {
               grp->getParent(0)->removeChild(grp);
            }
            else if(grp != NULL)
            {
               (*iter).first->removeChild((*iter).second.get());
            }

            osg::Light* light = (*iter).second->getLight();
            mRootNode->getOrCreateStateSet()->setAssociatedModes(light, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
         }
      }
   }


   const LightArray& GetLightArray() const
   {
      return mLights;
   }

   virtual void apply(osg::LightSource& lightSource)
   { 
      osg::NodePath nodePath = getNodePath();
      osg::Matrix mat = osg::computeLocalToWorld(nodePath);  

      osg::Light* light = lightSource.getLight();

      if(light != NULL)
      {
         light->setLightNum(mLights.size());
         light->setPosition(osg::Vec4(mat(3,0), mat(3,1), mat(3,2), mat(3,3)));
         light->setDirection(osg::Vec3(-mat(2,0), -mat(2,1), -mat(2,2)));
         mLights.push_back(light);
         //std::cout << "Light Pos: " << light->getPosition()[0] << ", " << light->getPosition()[1] << ", " << light->getPosition()[2] << std::endl;
      }

      traverse(lightSource);

      if(mRemoveLights)
      {
         mLightsToRemove.push_back(LightSourcePair(lightSource.getParent(0), &lightSource));
      }

      //add the light name to the description array on the root node which will map back to the light index data
      mRootNode->addDescription(light->getName());
   }

   virtual void apply(osg::Switch& switchNode)
   {
      if(strstr(switchNode.getName().c_str(), "light") != NULL)
      {
         switchNode.setAllChildrenOff();
         std::cout << "Turning off node: " << switchNode.getName() << std::endl;
      }
   }

private:

   osg::ref_ptr<osg::Node> mRootNode;
   LightArray mLights;
   LightSources mLightsToRemove;
   bool mRemoveLights;

};


#endif //__LIGHT_COLLECTOR_H__
