/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>

#include <SimCore/Actors/IGActor.h>

#include <dtUtil/nodecollector.h>
#include <dtUtil/fileutils.h>
#include <dtCore/particlesystem.h>
#include <dtCore/scene.h>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoninghelper.h>

#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/VisibilityOptions.h>

#include <osg/Node>
#include <osg/MatrixTransform>
#include <osgSim/DOFTransform>

#include <osgDB/ReadFile>
#include <osgDB/Registry>

#include <osgUtil/CullVisitor>

namespace SimCore
{
   namespace Actors
   {
      class HideNodeCallback : public osg::NodeCallback
      {
          public:

             /**
              * Constructor.
              *
              * @param terrain the owning InfiniteTerrain object
              */
             HideNodeCallback()
             {}

             virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
             {
                // We're hiding the node.  The point is to NOT traverse.
                // traverse(node, nv);
             }
      };
      static std::shared_ptr<HideNodeCallback> HIDE_NODE_CALLBACK(new HideNodeCallback);

      ///////////////////////////////////////////////////////////////////////////
      IGActor::IGActor(dtGame::GameActorProxy &proxy)
      : GameActor(proxy)
      , mIsVisible(true)
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      IGActor::~IGActor()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      bool IGActor::LoadFile(const std::string& fileName, osg::ref_ptr<osg::Node>& originalFile,
         osg::ref_ptr<osg::Node>& copiedFile, bool useCache , bool loadTerrainMaterialsOn )
      {
         return IGActor::LoadFileStatic(fileName, originalFile,
            copiedFile, useCache, loadTerrainMaterialsOn);
      }

      ///////////////////////////////////////////////////////////////////////////
      bool IGActor::LoadFileStatic(const std::string& fileName, osg::ref_ptr<osg::Node>& originalFile,
         osg::ref_ptr<osg::Node>& copiedFile, bool useCache , bool loadTerrainMaterialsOn )
      {
         //Log::GetInstance()->LogMessage(Log::LOG_DEBUG, __FUNCTION__,
         //   "Loading '%s'", filename.c_str());

         // Setup appropriate options
         osgDB::ReaderWriter::Options* curOptions = osgDB::Registry::instance()->getOptions();
         osg::ref_ptr<osgDB::ReaderWriter::Options> options =  curOptions ?
            static_cast<osgDB::ReaderWriter::Options*>(curOptions->clone(osg::CopyOp::SHALLOW_COPY)) :
         new osgDB::ReaderWriter::Options;


         if (useCache)
         {
            options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);
         }
         else
         {
            options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_NONE);
         }

         if (loadTerrainMaterialsOn)
         {
            options->setOptionString("loadMaterialsToStateSet");
         }
         
         originalFile = dtUtil::FileUtils::GetInstance().ReadNode(fileName, options.get());

         if (originalFile.valid())
         {
            if (useCache)
            {
               copiedFile = static_cast<osg::Node*>(originalFile->clone(osg::CopyOp(COPY_OPS_SHARED_GEOMETRY) ));
            }
            return true;
         }
         else
         {
//            Log::GetInstance()->LogMessage(Log::LOG_WARNING, __FUNCTION__,
//               "Can't load '%s'", mFilename.c_str() );
            return false;
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void IGActor::RegisterParticleSystem( dtCore::ParticleSystem& particles,
               const SimCore::Components::ParticleInfoAttributeFlags* attributes )
      {
         // Register the particle systems with the particle manager component
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
         if( gm == nullptr ) { return; }

         SimCore::Components::ParticleManagerComponent* comp;
         gm->GetComponentByName(SimCore::Components::ParticleManagerComponent::DEFAULT_NAME, comp);

         if( comp != nullptr )
         {
            comp->Register(particles, attributes);
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void IGActor::UnregisterParticleSystem( dtCore::ParticleSystem& particles )
      {
         // Register the particle systems with the particle manager component
         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
         if( gm == nullptr ) { return; }

         SimCore::Components::ParticleManagerComponent* comp;
         gm->GetComponentByName(SimCore::Components::ParticleManagerComponent::DEFAULT_NAME, comp);

         if( comp != nullptr )
         {
            comp->Unregister(particles);
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      bool IGActor::AddChild(dtCore::DeltaDrawable* child, const std::string& nodeName)
      {
         if (nodeName.empty())
         {
            return BaseClass::AddChild(child);
         }

         if (child->GetOSGNode()->getNumParents() > 0)
         {
            return false;
         }

         bool result = dtCore::DeltaDrawable::AddChild(child);
         if (result)
         {
             osg::Group* group = nullptr;

             if (GetNodeCollector() == nullptr)
             {
                LoadNodeCollector();
             }

             dtUtil::NodeCollector* nc = GetNodeCollector();

             group = nc->GetMatrixTransform(nodeName);

             if (group == nullptr)
             {
                group = nc->GetDOFTransform(nodeName);
             }

             if (group != nullptr)
             {
                group->addChild(child->GetOSGNode());
             }
             else
             {
                GetOSGNode()->asGroup()->addChild(child->GetOSGNode());
             }
         }

         return result;
      }

      ////////////////////////////////////////////////////////////////////////////
      void IGActor::RemoveChild(dtCore::DeltaDrawable* child)
      {
         osg::Node* node = child->GetOSGNode();

         if (!GetMatrixNode()->removeChild(node))
         {
            osg::Node::ParentList parents = node->getParents();
            for (size_t i = 0; i != parents.size(); ++i)
            {
               parents[i]->removeChild(node);
            }
         }
         dtCore::DeltaDrawable::RemoveChild(child);
      }

      ////////////////////////////////////////////////////////////////////////////
      bool IGActor::ShouldBeVisible(const SimCore::VisibilityOptions&)
      {
         return true;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool IGActor::IsVisible() const
      {
         return mIsVisible;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void IGActor::SetVisible(bool visible)
      {
         mIsVisible = visible;
         SetNodeVisible(visible, *GetOSGNode());
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void IGActor::SetNodeVisible(bool visible, osg::Node& nodeToUse)
      {
         if (visible)
         {
            nodeToUse.setCullCallback(nullptr);
         }
         else
         {
            nodeToUse.setCullCallback(HIDE_NODE_CALLBACK.get());
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void IGActor::LoadNodeCollector()
      {
         mNodeCollector = new dtUtil::NodeCollector(GetOSGNode(),
                  dtUtil::NodeCollector::AllNodeTypes);
         dtGame::DeadReckoningHelper* drAC = nullptr;
         GetComponent(drAC);
         if (drAC != nullptr)
         {
            drAC->SetNodeCollector(*mNodeCollector);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void IGActor::SetNodeCollector(dtUtil::NodeCollector* newNC)
      {
         mNodeCollector = newNC;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      dtUtil::NodeCollector* IGActor::GetNodeCollector()
      {
         return mNodeCollector.get();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      const dtUtil::NodeCollector* IGActor::GetNodeCollector() const
      {
         return mNodeCollector.get();
      }

   }
}
