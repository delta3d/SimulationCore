
#include "Util.h"

#include <iostream>

namespace LevelCompiler
{


   void Util::FlattenGeodeTransformAndState(osg::NodePath& path)
   {  
      osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet;

      stateSet->clear();

      osg::Matrix matrix;

      for(osg::NodePath::iterator it = path.begin();
         it != path.end();
         it++)
      {
         if((*it)->getStateSet() != nullptr)
         {
            stateSet->merge(*(*it)->getStateSet());
         }

         osg::Transform* transform = dynamic_cast<osg::Transform*>(*it);

         if(transform != nullptr)
         {
            transform->computeLocalToWorldMatrix(matrix, nullptr);
         }
      }

      osg::Geode* geode = dynamic_cast<osg::Geode*>(path.back());

      geode->setStateSet(stateSet.get());

      unsigned int i, j;

      for( i = 0; i < geode->getNumDrawables(); i++ )
      {
         if( osg::Geometry* geometry = geode->getDrawable(i)->asGeometry() )
         {
            if( osg::Vec3Array* vertexArray = dynamic_cast< osg::Vec3Array* >( geometry->getVertexArray() ) )
            {
               for( j = 0; j < vertexArray->size(); j++ )
               {
                  (*vertexArray)[j] = (*vertexArray)[j]*matrix;
               }
            }

            if( osg::Vec3Array* v3a = dynamic_cast< osg::Vec3Array* >( geometry->getNormalArray() ) )
            {
               for( j = 0; j < v3a->size(); j++ )
               {
                  (*v3a)[j] = osg::Matrix::transform3x3((*v3a)[j], matrix);
               }
            }
            //else if(osg::Vec4Array* v4a = dynamic_cast< osg::Vec4Array* >(geometry->getNormalArray()) )
            //{
            //   for( j = 0; j < v4a->size(); j++ )
            //   {
            //      osg::Vec3 norm = osg::Matrix::transform3x3(osg::Vec3((*v4a)[j][0], (*v4a)[j][1], (*v4a)[j][2]), matrix);
            //      
            //      (*v4a)[j][0] = norm[0];
            //      (*v4a)[j][1] = norm[1];
            //      (*v4a)[j][2] = norm[2];
            //      (*v4a)[j][3] = 0.0f;
            //   }
            //}
            else
            {
               std::cout << "Found geode without normal array" << std::endl;
            }
         }
      }
   }


}//namespace LevelCompiler
