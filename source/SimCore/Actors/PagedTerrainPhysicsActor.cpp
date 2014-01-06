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
* @author Allen Danklefsen, Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/PagedTerrainPhysicsActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/logcontroller.h>
#include <osg/MatrixTransform>

#include <SimCore/CollisionGroupEnum.h>
#include <SimCore/PhysicsTypes.h>

#ifdef AGEIA_PHYSICS
#include <SimCore/ModifiedStream.h>
#include <NxCooking.h>
#else
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/trianglerecorder.h>
#include <dtPhysics/geometry.h>
#include <dtCore/transform.h>
#endif

#include <dtUtil/stringutils.h>

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string PagedTerrainPhysicsActor::DEFAULT_NAME("Terra Page Listener");

      //////////////////////////////////////////////////////////////////////
      // Geode Visitor - This visitor searches for geodes. It then adds each
      // geode separately to the physics engine. This is used when you are NOT
      // using the AgeiaCullVisitor (see RenderingSupportComponent for setting this flag).
      //////////////////////////////////////////////////////////////////////
      class GeodeTriangleVisitor : public osg::NodeVisitor
      {
      public:
         // Constructor
         GeodeTriangleVisitor(PagedTerrainPhysicsActor& landActor, std::string nodeName)
            : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
            , mLandActor(landActor)
            , mNodeName(nodeName)
            , mTempParentTransform(new osg::MatrixTransform())
         {
         }

         // Applies this visitor to a geode - add each geode as a separate physics piece.
         virtual void apply(osg::Geode& node)
         {
            osg::NodePath nodePath = getNodePath();
            osg::Matrix worldMatrix = osg::computeLocalToWorld(nodePath);

            mTempParentTransform->setMatrix(worldMatrix);
            mTempParentTransform->addChild(&node);
            mLandActor.AddTerrainNode(mTempParentTransform.get(), mNodeName);
            mTempParentTransform->removeChildren(0, mTempParentTransform->getNumChildren());
         }

      private:
         PagedTerrainPhysicsActor& mLandActor;
         std::string mNodeName;
         osg::ref_ptr<osg::MatrixTransform> mTempParentTransform;
      };


      //////////////////////////////////////////////////////////////////////
      // Triangle Visitor - used by the tiled process for the AgeiaTerrainCullVisitor
      // to pull out geode's that pass our material codes. This allows us to accept
      // a geode for the ground or a building, but ignore a bush or cactus...
      //////////////////////////////////////////////////////////////////////
      template< class T >
      class DrawableTriangleVisitor : public osg::NodeVisitor
      {
         public:
            osg::TriangleFunctor<T> mFunctor;

            // Constructor.
            DrawableTriangleVisitor(PagedTerrainPhysicsActor& landActor)
               : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
               , mLandActor(landActor)
            {
            }

            /**
            * Applies this visitor to a geode.
            * @param node the geode to visit
            */
            virtual void apply(osg::Geode& node)
            {
               int value[4];
               int iter = 0;
               for(unsigned int i=0;i<node.getNumDrawables();i++)
               {
                  osg::Drawable* d = node.getDrawable(i);

                  if(d->supports(mFunctor))
                  {
                     osg::NodePath nodePath = getNodePath();
                     mFunctor.SetMatrix(osg::computeLocalToWorld(nodePath));
                     osg::StateSet* tempStateSet = d->getStateSet();
                     osg::ref_ptr<osg::IntArray> mOurList;
                     if (tempStateSet != nullptr)
                     {
                        mOurList = dynamic_cast<osg::IntArray*>(tempStateSet->getUserData());
                     }

                     if(mOurList.valid())
                     {
                        if(mOurList->size())
                        {
                           iter = 0;
                           std::vector<int>::iterator listiter = mOurList->begin();
                           for(; listiter != mOurList->end(); ++listiter)
                           {
                              value[iter] = *listiter;
                              ++iter;
                           }

                           // if general soil or roads, then we want to gather the triangles
                           if(mLandActor.PassThisGeometry(value[0],value[1],value[2],value[3]))
                           {
                              d->accept(mFunctor);
                           }
                           // if it's a building, then we want them in this list
                           //else if(mLandActor.LoadGeomAsGroup(value[0]))
                           // what's left?  Probably vegetation, which gets ignored for now
                           //else
                              //mLandActor.DetermineHowToLoadGeometry(value[0],value[1],value[2],value[3], miniGeode.get());
                        } // end ourlist size
                     }
                     else
                        d->accept(mFunctor);
                  }
               }
            }
         private:
            PagedTerrainPhysicsActor& mLandActor;
      };

      //////////////////////////////////////////////////////////////////////
      // Node stuff
      //////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////////
         TerrainNode::TerrainNode(osg::Geode* toSet)
         : mGeodePTR(toSet)           // the group pointer of what was loaded
         , mFilledBL(false)           // filled for physics use yet?
         //, mIsBuilding(false)         // is this a building or not
         , mFlags(TILE_TODO_DISABLE)  // for flag system
         , mLastFlags(TILE_TODO_KEEP)  // for flag system, different from mFlags so it will start as "changed"
         {
            SetPhysicsObject(nullptr);
         }

         //////////////////////////////////////////////////////////////////////
         TerrainNode::~TerrainNode()
         {
            SetPhysicsObject(nullptr);
            mGeodePTR = nullptr;
         }

         /////////////////////////////////////////////////////////////////////////
         osg::Geode* TerrainNode::GetGeodePointer()
         {
            if(mGeodePTR.valid())
               return mGeodePTR.get();
            return nullptr;
         }

         /////////////////////////////////////////////////////////////////////////
         void TerrainNode::SetFlags(char value)
         {
            mFlags = value;
         }

         /////////////////////////////////////////////////////////////////////////
         char TerrainNode::GetFlags() const
         {
            return mFlags;
         }

         /////////////////////////////////////////////////////////////////////////
         void TerrainNode::SetFlagsToKeep()
         {
            SetFlags(TILE_TODO_KEEP);
         }

         /////////////////////////////////////////////////////////////////////////
         void TerrainNode::SetFlagsToLoad()
         {
            SetFlags(TILE_TODO_LOAD);
         }

         /////////////////////////////////////////////////////////////////////////
         void TerrainNode::SetFlagsToDisable()
         {
            SetFlags(TILE_TODO_DISABLE);
         }

         /////////////////////////////////////////////////////////////////////////
         void TerrainNode::SaveCurrentToLastFlags()
         {
            mLastFlags = mFlags;
         }

         /////////////////////////////////////////////////////////////////////////
         const dtCore::UniqueId& TerrainNode::GetUniqueID() const
         {
            return mUniqueID;
         }

         /////////////////////////////////////////////////////////////////////////
         void TerrainNode::SetPhysicsObject(dtPhysics::PhysicsObject* object)
         {
            mPhysicsObject = object;
         }

      //////////////////////////////////////////////////////////////////////
      // Actor stuff
      //////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////////
         PagedTerrainPhysicsActor::PagedTerrainPhysicsActor(dtGame::GameActorProxy &proxy)
            : GameActor(proxy)
            , mNumNodesLoaded(0)
            , mNumVertsLoaded(0)
         {
            mFinalizeTerrainIter = mTerrainMap.begin();
            SetName(PagedTerrainPhysicsActor::DEFAULT_NAME);
            mLoadedTerrainYet = false;
         }

         //////////////////////////////////////////////////////////////////////
         PagedTerrainPhysicsActor::~PagedTerrainPhysicsActor(void)
         {
            mTerrainMap.clear();
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActor::OnEnteredWorld()
         {
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActor::CheckGeode(osg::Geode& node, bool loadNow, const osg::Matrix& matrixForTransform)
         {
            std::map<osg::Geode*, std::shared_ptr<TerrainNode> >::iterator iter;
            TerrainNode* currentNode = nullptr;
            iter = mTerrainMap.find(&node);
            if(iter != mTerrainMap.end())
            {
               currentNode = iter->second.get();
               if(currentNode->GetFlags() != TerrainNode::TILE_TODO_LOAD)
                  currentNode->SetFlagsToKeep();
            }
            else
            {
               std::shared_ptr<TerrainNode> terrainNodeToAdd = new TerrainNode(&node);
               terrainNodeToAdd->SetFlagsToLoad();

               if(loadNow)
               {
                  terrainNodeToAdd->SetPhysicsObject(BuildTerrainAsStaticMesh( terrainNodeToAdd->GetGeodePointer(),
                     terrainNodeToAdd->GetUniqueID().ToString(), false));
                  terrainNodeToAdd->SetFilled(terrainNodeToAdd->GetPhysicsObject() != nullptr);

                  if(terrainNodeToAdd->IsFilled())
                  {
#ifdef AGEIA_PHYSICS
                     osg::Quat quaternion = matrixForTransform.getRotate();
                     terrainNodeToAdd->GetPhysicsObject()->setGlobalOrientationQuat(NxQuat(NxVec3(quaternion[0],quaternion[1],quaternion[2]), quaternion[3]));
                     terrainNodeToAdd->GetPhysicsObject()->setGlobalPosition(NxVec3(  matrixForTransform.getTrans()[0],
                        matrixForTransform.getTrans()[1],
                        matrixForTransform.getTrans()[2]));
#else
                     dtPhysics::PhysicsObject* physObject = terrainNodeToAdd->GetPhysicsObject();
                     dtCore::Transform xform;
                     xform.Set(matrixForTransform);
                     physObject->SetTransform(xform);
#endif
                     terrainNodeToAdd->SetFlagsToKeep();
                  }
                  else
                  {
                     LOG_ERROR("Could not build geometry, immediately take cover and look at it @ 0,0,0");
                  }
               }
               mTerrainMap.insert(std::make_pair(&node, terrainNodeToAdd));
            }
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActor::ClearAllTerrainPhysics()
         {
            dtPhysics::PhysicsActComp* ac;
            GetComponent(ac);
            ac->ClearAllPhysicsObjects();
            mTerrainMap.clear();
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActor::ResetTerrainIterator()
         {
            mFinalizeTerrainIter = mTerrainMap.begin();
         }

         //////////////////////////////////////////////////////////////////////
         bool PagedTerrainPhysicsActor::FinalizeTerrain(int numberOfFrames)
         {
            std::map<osg::Geode*, std::shared_ptr<TerrainNode> >::iterator removeIter;
            for(unsigned int i = 0;
               mFinalizeTerrainIter != mTerrainMap.end()
               && i < mTerrainMap.size() ;/// numberOfFrames;
               ++i)
            {
               TerrainNode* currentNode = mFinalizeTerrainIter->second.get();

               if (currentNode->GetGeodePointer() == nullptr)
               {
                  // Remove physics stuff if its loaded.
                  if (currentNode->IsFilled())
                  {
                     dtPhysics::PhysicsActComp* ac;
                     GetComponent(ac);
                     ac->RemovePhysicsObject(*currentNode->GetPhysicsObject());
                  }
                  mTerrainMap.erase(mFinalizeTerrainIter++);
                  continue;
               }
               else if (currentNode->FlagsChanged())
               {
                  if (!currentNode->IsFilled())
                  {
                     currentNode->SetFlagsToLoad();
                  }

                  dtPhysics::PhysicsObject* curPO = currentNode->GetPhysicsObject();
                  switch (currentNode->GetFlags())
                  {
                     case TerrainNode::TILE_TODO_DISABLE:
                        curPO->SetCollisionGroup(SimCore::CollisionGroup::GROUP_BULLET);
                        curPO->SetCollisionResponseEnabled(false);
                        curPO->SetTranslation(dtPhysics::VectorType(0.0f, 0.0f, -10000.0f));
                        break;
                     case TerrainNode::TILE_TODO_KEEP:
                        curPO->SetCollisionGroup(SimCore::CollisionGroup::GROUP_TERRAIN);
                        curPO->SetCollisionResponseEnabled(true);
                        curPO->SetTranslation(dtPhysics::VectorType(0.0f, 0.0f, 0.0f));
                        break;
                     case TerrainNode::TILE_TODO_LOAD:
                        currentNode->SetPhysicsObject(BuildTerrainAsStaticMesh( currentNode->GetGeodePointer(),
                                 currentNode->GetUniqueID().ToString(), false));
                        currentNode->SetFilled(currentNode->GetPhysicsObject() != nullptr);
                        break;
                  }

               }

               currentNode->SaveCurrentToLastFlags();
               currentNode->SetFlagsToDisable();

               ++mFinalizeTerrainIter;

            }

            if(mFinalizeTerrainIter ==  mTerrainMap.end() )
            {
               return false;
            }
            return true;
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActor::ReloadTerrainPhysics()
         {
            std::map<osg::Geode*, std::shared_ptr<TerrainNode> >::iterator mterrainIter;
            std::map<osg::Geode*, std::shared_ptr<TerrainNode> >::iterator removeIter;
            for(mterrainIter = mTerrainMap.begin();
               mterrainIter != mTerrainMap.end() ;
               ++mterrainIter)
            {
               TerrainNode* currentNode = mterrainIter->second.get();

               if(currentNode->GetGeodePointer() != nullptr && currentNode->IsFilled())
               {
                  dtPhysics::PhysicsActComp* ac;
                  GetComponent(ac);
                  ac->RemovePhysicsObject(currentNode->GetUniqueID().ToString());

                  currentNode->SetPhysicsObject(BuildTerrainAsStaticMesh( currentNode->GetGeodePointer(),
                     currentNode->GetUniqueID().ToString(), false));
                  currentNode->SetFilled(currentNode->GetPhysicsObject() != nullptr);
               }
            }
         }

      //////////////////////////////////////////////////////////////////////
      // Terrain tile loading
      //////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////////
         dtPhysics::PhysicsObject* PagedTerrainPhysicsActor::BuildTerrainAsStaticMesh(osg::Node* nodeToParse,
            const std::string& nameOfNode, bool buildGeodesSeparately)
         {
            if(nodeToParse == nullptr)
            {
               LOG_ALWAYS("Null nodeToParse sent into the BuildTerrainAsStaticMesh function! \
                  No action taken.");
               return nullptr;
            }

            // Some geometries are really large to load as one big mess, so we load each geode separately
            // Note, in this case, we don't have a single physics object to return...
            if (buildGeodesSeparately)
            {
               mNumNodesLoaded = 0;
               mNumVertsLoaded = 0;
               LOG_INFO("Starting to load physics geometry for terrain.");

               // For each geode it finds, it calls AddTerrainGeode();
               GeodeTriangleVisitor geodeVisitor(*this, nameOfNode);
               nodeToParse->accept(geodeVisitor);

               std::string numNodesString, numVertsString;
               dtUtil::MakeIndexString(mNumNodesLoaded, numNodesString);
               dtUtil::MakeIndexString(mNumVertsLoaded, numVertsString);
               LOG_INFO("Finished loading physics geometry. Found [" + numNodesString + "] nodes with [" + numVertsString + "] verts.");
               return nullptr;
            }
            else
            {

               DrawableTriangleVisitor<dtPhysics::TriangleRecorder> dtv(*this);
               nodeToParse->accept(dtv);

               if (!dtv.mFunctor.mVertices.empty())
               {
                  std::shared_ptr<dtPhysics::PhysicsObject> newTile = new dtPhysics::PhysicsObject(nameOfNode);
                  dtPhysics::PhysicsActComp* ac;
                  GetComponent(ac);
                  ac->AddPhysicsObject(*newTile);
                  newTile->SetName(nameOfNode);
                  newTile->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
                  newTile->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);
                  newTile->SetCollisionGroup(SimCore::CollisionGroup::GROUP_TERRAIN);
                  // We don't want this skin thickness, we want the thickness on the geometry.
                  newTile->SetSkinThickness(0.0);

                  dtPhysics::VertexData data;
                  data.mIndices = &dtv.mFunctor.mIndices.front();
                  data.mVertices = (dtPhysics::Real*)&dtv.mFunctor.mVertices.front();
                  data.mNumIndices = dtv.mFunctor.mIndices.size();
                  data.mNumVertices = dtv.mFunctor.mVertices.size();
                  dtCore::Transform xform;
                  std::shared_ptr<dtPhysics::Geometry> geom = dtPhysics::Geometry::CreateConcaveGeometry(xform, data, 0);
                  geom->SetMargin(0.06);

                  newTile->CreateFromGeometry(*geom);
                  return newTile.get();
               }
               else
               {
                  return nullptr;
               }
            }
         }

         //////////////////////////////////////////////////////////////////////
         dtPhysics::PhysicsObject* PagedTerrainPhysicsActor::AddTerrainNode(osg::Node* node,
            const std::string& nameOfNode)
         {
            mNumNodesLoaded ++;
            std::string numNodesString;
            dtUtil::MakeIndexString(mNumNodesLoaded, numNodesString);
            numNodesString = nameOfNode + " " + numNodesString;

            std::shared_ptr<dtPhysics::PhysicsObject> newTile = new dtPhysics::PhysicsObject(nameOfNode);
            dtPhysics::PhysicsActComp* ac;
            GetComponent(ac);
            ac->AddPhysicsObject(*newTile);
            newTile->SetName(nameOfNode);
            newTile->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
            newTile->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);
            newTile->SetCollisionGroup(SimCore::CollisionGroup::GROUP_TERRAIN);
            newTile->SetSkinThickness(0.06);
            newTile->CreateFromProperties(node);
            // We don't want this skin thickness, we want the thickness on the geometry.
            newTile->SetSkinThickness(0.00);

            return newTile.get();
         }

         //////////////////////////////////////////////////////////////////////
         bool PagedTerrainPhysicsActor::PassThisGeometry(int fid, int smc,
            int soilTemperatureAndPressure, int soilWaterContent)
         {
            // 957 - arid vegetation
            // 301 - commercial building
            // 401 - residential building
            // 772 - training building

            // Changed for buildings to static mesh internally
            //if(fid == 957 || fid == 301 || fid == 401 || fid == 772)
            //   return false;

            if(fid == 957)
               return false;
            return true;
         }

         ////////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActor::DetermineHowToLoadGeometry(int fid,
            int smc, int soilTemperatureAndPressure, int soilWaterContent, osg::Node* nodeToLoad)
         {
            // the case is mostly going to be is this a box or is this a tree...
         }

         ////////////////////////////////////////////////////////////////////////
         bool PagedTerrainPhysicsActor::LoadGeomAsGroup(int fid)
         {
            // 301 - commercial building
            // 401 - residential building
            // 772 - training building

            // 957 - arid vegetation
            // 902 - soil (general)
            // 251 - city roads
            // 253 - country/dirt roads

            // what we want is buildings
            if(fid == 301 || fid == 401  || fid == 772)
            {
               return true; // gets loaded as all buildings in one tri mesh
            }
            return false;
         }

      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////////
         PagedTerrainPhysicsActorProxy::PagedTerrainPhysicsActorProxy()
         {
            SetClassName("SimCore::Actors::PagedTerrainPhysicsActor");
            SetHideDTCorePhysicsProps(true);
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActorProxy::BuildActorComponents()
         {
            BaseClass::BuildActorComponents();

            if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
            {
               AddComponent(*new dtPhysics::PhysicsActComp());
            }
         }

         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActorProxy::BuildPropertyMap()
         {
            dtGame::GameActorProxy::BuildPropertyMap();
         }

         //////////////////////////////////////////////////////////////////////
         PagedTerrainPhysicsActorProxy::~PagedTerrainPhysicsActorProxy(){}
         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActorProxy::CreateDrawable()
         {
            SetDrawable(*new PagedTerrainPhysicsActor(*this));
         }
         //////////////////////////////////////////////////////////////////////
         void PagedTerrainPhysicsActorProxy::OnEnteredWorld()
         {
            dtGame::GameActorProxy::OnEnteredWorld();

            // Ignore this object from any recording
            dtGame::LogController* logController
               = dynamic_cast<dtGame::LogController*> (GetGameManager()->GetComponentByName("LogController"));

            if( logController != nullptr )
            {
               logController->RequestAddIgnoredActor( GetId() );
            }
         }

   } // namespace
} // namespace

