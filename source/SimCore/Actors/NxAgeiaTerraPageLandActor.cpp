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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/logcontroller.h>
#include <osg/MatrixTransform>

#ifdef AGEIA_PHYSICS

#include <NxAgeiaWorldComponent.h>
#include <SimCore/ModifiedStream.h>
#include <SimCore/CollisionGroupEnum.h>
#include <NxCooking.h>
#else
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/trianglerecorder.h>
#include <dtCore/transform.h>
#endif

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      const std::string NxAgeiaTerraPageLandActor::DEFAULT_NAME("Terra Page Listener");

      //////////////////////////////////////////////////////////////////////
      // Geode Visitor - This visitor searches for geodes. It then adds each 
      // geode separately to the physics engine. This is used when you are NOT 
      // using the AgeiaCullVisitor (see RenderingSupportComponent for setting this flag).
      //////////////////////////////////////////////////////////////////////
      class GeodeTriangleVisitor : public osg::NodeVisitor
      {
      public:
         // Constructor
         GeodeTriangleVisitor(NxAgeiaTerraPageLandActor& landActor, std::string nodeName) 
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
         std::string mNodeName;
         NxAgeiaTerraPageLandActor& mLandActor;
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
            DrawableTriangleVisitor(NxAgeiaTerraPageLandActor& landActor) 
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
                     mFunctor.mMatrix = osg::computeLocalToWorld(nodePath);

                     osg::StateSet* tempStateSet = d->getStateSet();
                     osg::ref_ptr<osg::IntArray> mOurList;
                     if(tempStateSet != NULL)
                        mOurList = dynamic_cast<osg::IntArray*>(tempStateSet->getUserData());

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
            NxAgeiaTerraPageLandActor& mLandActor;
      };

      //////////////////////////////////////////////////////////////////////
      // Node stuff
      //////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////////
         TerrainNode::TerrainNode(osg::Geode* toSet) :
            mGeodePTR(toSet)           // the group pointer of what was loaded
          , mFilledBL(false)           // filled for physics use yet?
          //, mIsBuilding(false)         // is this a building or not
          , mFlags(TILE_TODO_DISABLE)  // for flag system
         {
            SetPhysicsObject(NULL);
         }

         //////////////////////////////////////////////////////////////////////
         TerrainNode::~TerrainNode()
         {
            SetPhysicsObject(NULL);
            mGeodePTR = NULL;
         }

      //////////////////////////////////////////////////////////////////////
      // Actor stuff
      //////////////////////////////////////////////////////////////////////

         //////////////////////////////////////////////////////////////////////
         NxAgeiaTerraPageLandActor::NxAgeiaTerraPageLandActor(dtGame::GameActorProxy &proxy) 
            : GameActor(proxy)
            , mNumNodesLoaded(0)
            , mNumVertsLoaded(0)
         {
            mFinalizeTerrainIter = mTerrainMap.begin();
            SetName(NxAgeiaTerraPageLandActor::DEFAULT_NAME);
#ifdef AGEIA_PHYSICS
            mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
            mPhysicsHelper->SetBaseInterfaceClass(this);
            mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
#else
            mPhysicsHelper = new dtPhysics::PhysicsHelper(proxy);
            //mPhysicsHelper->SetFlags(dtPhysics::PhysicsHelper::FLAGS_POST_UPDATE);
#endif
            mLoadedTerrainYet = false;
         }

         //////////////////////////////////////////////////////////////////////
         NxAgeiaTerraPageLandActor::~NxAgeiaTerraPageLandActor(void)
         {
#ifdef AGEIA_PHYSICS
            mPhysicsHelper->ReleaseAllPhysXObjects();
#endif
            mTerrainMap.clear();
         }

         //////////////////////////////////////////////////////////////////////
         void NxAgeiaTerraPageLandActor::OnEnteredWorld()
         {
#ifdef AGEIA_PHYSICS
            dtAgeiaPhysX::NxAgeiaWorldComponent* worldComponent;
            GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent", worldComponent);
            if (worldComponent != NULL)
               worldComponent->RegisterAgeiaHelper(*mPhysicsHelper);
            mPhysicsHelper->SetAgeiaUserData(mPhysicsHelper.get());
#else
            dtPhysics::PhysicsComponent* physicsComponent;
            GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physicsComponent);
            if (physicsComponent != NULL)
               physicsComponent->RegisterHelper(*mPhysicsHelper);
#endif
         }

         //////////////////////////////////////////////////////////////////////
         void NxAgeiaTerraPageLandActor::CheckGeode(osg::Geode& node, bool loadNow, const osg::Matrix& matrixForTransform)
         {
            std::map<osg::Geode*, dtCore::RefPtr<TerrainNode> >::iterator iter;
            TerrainNode* currentNode = NULL;
            iter = mTerrainMap.find(&node);
            if(iter != mTerrainMap.end())
            {
               currentNode = iter->second.get();
               if(currentNode->GetFlags() != TerrainNode::TILE_TODO_LOAD)
                  currentNode->SetFlagsToKeep();
            }
            else
            {
               dtCore::RefPtr<TerrainNode> terrainNodeToAdd = new TerrainNode(&node);
               terrainNodeToAdd->SetFlagsToLoad();

               if(loadNow)
               {
                  terrainNodeToAdd->SetPhysicsObject(BuildTerrainAsStaticMesh( terrainNodeToAdd->GetGeodePointer(),
                     terrainNodeToAdd->GetUniqueID().ToString()));
                  terrainNodeToAdd->SetFilled(terrainNodeToAdd->GetPhysicsObject() != NULL);

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
         void NxAgeiaTerraPageLandActor::ClearAllTerrainPhysics()
         {
#ifdef AGEIA_PHYSICS
            mPhysicsHelper->ReleaseAllPhysXObjects();
#else
            mPhysicsHelper->ClearAllPhysicsObjects();
#endif
            mTerrainMap.clear();
         }

         //////////////////////////////////////////////////////////////////////
         void NxAgeiaTerraPageLandActor::ResetTerrainIterator()
         {
            mFinalizeTerrainIter = mTerrainMap.begin();
         }

         //////////////////////////////////////////////////////////////////////
         bool NxAgeiaTerraPageLandActor::FinalizeTerrain(int amountOfFrames)
         {
            std::map<osg::Geode*, dtCore::RefPtr<TerrainNode> >::iterator removeIter;
            for(unsigned int i = 0;
               mFinalizeTerrainIter != mTerrainMap.end()
               && i < mTerrainMap.size() / amountOfFrames + 1;
               ++i)
            {
               TerrainNode* currentNode = mFinalizeTerrainIter->second.get();

               if(currentNode->GetGeodePointer() == NULL)
               {
                  // Remove ageia stuff if its loaded.
                  if(currentNode->IsFilled())
                  {
                     mPhysicsHelper->RemovePhysicsObject(currentNode->GetUniqueID().ToString());
                  }
                  mTerrainMap.erase(mFinalizeTerrainIter++);
                  continue;
               }
               else if(currentNode->GetFlags() == TerrainNode::TILE_TODO_DISABLE
                     &&currentNode->IsFilled())
               {
#ifdef AGEIA_PHYSICS
                  mPhysicsHelper->TurnOffCollision(currentNode->GetPhysicsObject());
#else
                  //TODO make this work for a terrain
                  currentNode->GetPhysicsObject()->SetActive(false);
#endif
               }
               else if(currentNode->GetFlags() == TerrainNode::TILE_TODO_KEEP
                     &&currentNode->IsFilled())
               {
#ifdef AGEIA_PHYSICS
                  mPhysicsHelper->TurnOnCollision(currentNode->GetPhysicsObject());
#else
                  //TODO make this work for a terrain
                  currentNode->GetPhysicsObject()->SetActive(true);
#endif
               }
               else if(currentNode->GetFlags() == TerrainNode::TILE_TODO_LOAD
                     &&currentNode->GetGeodePointer() != NULL)
               {
                  // load the tile to ageia
                  currentNode->SetPhysicsObject(BuildTerrainAsStaticMesh( currentNode->GetGeodePointer(),
                     currentNode->GetUniqueID().ToString()));
                  currentNode->SetFilled(currentNode->GetPhysicsObject() != NULL);
               }

               if(currentNode->IsFilled() == false)
                  currentNode->SetFlagsToLoad();
               else
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
         void NxAgeiaTerraPageLandActor::ReloadTerrainPhysics()
         {
            std::map<osg::Geode*, dtCore::RefPtr<TerrainNode> >::iterator mterrainIter;
            std::map<osg::Geode*, dtCore::RefPtr<TerrainNode> >::iterator removeIter;
            for(mterrainIter = mTerrainMap.begin();
               mterrainIter != mTerrainMap.end() ;
               ++mterrainIter)
            {
               TerrainNode* currentNode = mterrainIter->second.get();

               if(currentNode->GetGeodePointer() != NULL && currentNode->IsFilled())
               {
                  mPhysicsHelper->RemovePhysicsObject(currentNode->GetUniqueID().ToString());

                  currentNode->SetPhysicsObject(BuildTerrainAsStaticMesh( currentNode->GetGeodePointer(),
                     currentNode->GetUniqueID().ToString()));
                  currentNode->SetFilled(currentNode->GetPhysicsObject() != NULL);
               }
            }
         }

      //////////////////////////////////////////////////////////////////////
      // Terrain tile loading
      //////////////////////////////////////////////////////////////////////

#ifdef AGEIA_PHYSICS
         //////////////////////////////////////////////////////////////////////
         NxActor* NxAgeiaTerraPageLandActor::BuildTerrainAsStaticMesh(osg::Node* nodeToParse,
            const std::string& nameOfNode, bool buildGeodesSeparately)
         {
            dtAgeiaPhysX::NxAgeiaWorldComponent* worldComponent =  NULL;
            GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent", worldComponent);

            if(worldComponent == NULL)
            {
               LOG_ERROR("worldComponent Is not initialized, make sure a new one \
                         was made before loading a map in, or setting physics objects");
               return NULL;
            }

            mLoadedTerrainYet = true;

            // Some geometries are really large to load as one big mess, so we load each geode separately
            // Note, in this case, we don't have a single physics object to return...
            if (buildGeodesSeparately)
            {
               mNumNodesLoaded = 0;
               mNumVertsLoaded = 0;
               LOG_ALWAYS("Starting to load physics geometry for terrain.");

               // For each geode it finds, it calls AddTerrainGeode();
               GeodeTriangleVisitor geodeVisitor(*this, nameOfNode);
               nodeToParse->accept(geodeVisitor);

               std::string numNodesString, numVertsString;
               dtUtil::MakeIndexString(mNumNodesLoaded, numNodesString);
               dtUtil::MakeIndexString(mNumVertsLoaded, numVertsString);
               LOG_ALWAYS("Finished loading physics geometry. Found [" + numNodesString + "] nodes with [" + numVertsString + "] verts.");
               return NULL;
            }

            // For normal geometries, we go through the node and find all children and check the drawables
            // for material codes and everything. Then we treat all the triangles as one big soup. 
            // For large pieces, the physics engine could choke here.
            else
            {
               // MOST of this is copied from the NxAgeiaPhysicsHelper::SetCollisionStaticMesh
               // It could be refactored massively if we could replace the visitor.
               DrawableTriangleVisitor<dtAgeiaPhysX::TriangleRecorder> mv(*this);
               nodeToParse->accept(mv);

               // our visitor should be all filled out now.
               int vertSize = mv.mFunctor.mVertices.size();
               int facesSize = mv.mFunctor.mTriangles.size()*3;
               mNumVertsLoaded = vertSize;
               if (vertSize <= 0 || facesSize <= 0)
               {
                  LOG_WARNING("Physics will ignore the geode with 0 verts.");
                  return NULL;
               }

               // It's odd, but the verts and triangles match the perfect memory 
               // footprint of what we need for PhysX. We just need to C cast it.
               NxVec3* Verts = (NxVec3*) &mv.mFunctor.mVertices.front(); 
               NxU32* Faces = (NxU32*) &mv.mFunctor.mTriangles.front();

               // Build physical model
               NxTriangleMeshDesc heightfieldDesc;
               heightfieldDesc.numVertices        = vertSize;
               heightfieldDesc.numTriangles       = facesSize / 3;
               heightfieldDesc.pointStrideBytes   = sizeof(NxVec3);
               heightfieldDesc.triangleStrideBytes= 3*sizeof(NxU32);
               heightfieldDesc.points             = Verts;
               heightfieldDesc.triangles          = Faces;
               // The next 2 lines are to make this into a height field
               //heightfieldDesc.heightFieldVerticalAxis   = NX_Z;
               //heightfieldDesc.heightFieldVerticalExtent = -3000;
               heightfieldDesc.flags              = 0;

               NxTriangleMeshShapeDesc heightfieldShapeDesc;
               // makes the sharp angles be smoothed, so that wheels will roll over better
               //heightfieldShapeDesc.meshFlags = NX_MESH_SMOOTH_SPHERE_COLLISIONS;
               heightfieldShapeDesc.name = nameOfNode.c_str();
               NxCookingInterface *gCooking = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
               gCooking->NxInitCooking();

               SimCore::MMemoryWriteBuffer buf;
               bool status = gCooking->NxCookTriangleMesh(heightfieldDesc, buf);
               if(status == false)
               {
                  std::stringstream ss;
                  ss << "Cooking - Failed to build mesh. Actor[" + nameOfNode + "], NumVerts[" << vertSize << "].";
                  LOG_ERROR(ss.str());
                  return NULL;
               }

               NxActorDesc     actorDesc;
               heightfieldShapeDesc.meshData = worldComponent->GetPhysicsSDK().createTriangleMesh(SimCore::MMemoryReadBuffer(buf.data));
               actorDesc.shapes.pushBack(&heightfieldShapeDesc);
               actorDesc.userData = (void *) mPhysicsHelper.get();
               actorDesc.group = SimCore::CollisionGroup::GROUP_TERRAIN;

               NxActor *actor = worldComponent->GetPhysicsScene(std::string("Default")).createActor(actorDesc);
               mPhysicsHelper->AddPhysXObject(*actor, nameOfNode.c_str());
               gCooking->NxCloseCooking();
               return actor;
            } //  !buildGeodesSeparately
         }

         //////////////////////////////////////////////////////////////////////
         NxActor* NxAgeiaTerraPageLandActor::AddTerrainNode(osg::Node* node,
            const std::string& nameOfNode)
         {
            dtAgeiaPhysX::NxAgeiaWorldComponent* worldComponent =  NULL;
            GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent", worldComponent);
            if(worldComponent == NULL)
            {
               LOG_ERROR("Critical Error! Physics Component does not exist.");
               return NULL;
            }

            mNumNodesLoaded ++;
            std::string numNodesString;
            dtUtil::MakeIndexString(mNumNodesLoaded, numNodesString);
            numNodesString = nameOfNode + " " + numNodesString;

            // Create our physics object 
            NxActor* actor = NULL;
            actor = mPhysicsHelper->SetCollisionStaticMesh(node, NxVec3(0.0f, 0.0f, 0.0f), false, 
               numNodesString, dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_SCENE_NAME, 
               nameOfNode, SimCore::CollisionGroup::GROUP_TERRAIN);
            mNumVertsLoaded += mPhysicsHelper->GetNumVertsOnLastGeometry();
            //NxActor* actor = mPhysicsHelper->SetCollisionMeshHeightField(node, SimCore::CollisionGroup::GROUP_TERRAIN, 
            //   dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_SCENE_NAME, numNodesString,0);

            return actor;
         }

#else
         //////////////////////////////////////////////////////////////////////
         dtPhysics::PhysicsObject* NxAgeiaTerraPageLandActor::BuildTerrainAsStaticMesh(osg::Node* nodeToParse,
            const std::string& nameOfNode)
         {
            if(nodeToParse == NULL)
            {
               LOG_ALWAYS("Null nodeToParse sent into the BuildTerrainAsStaticMesh function! \
                  No action taken.");
               return NULL;
            }

            dtCore::RefPtr<dtPhysics::PhysicsObject> newTile = new dtPhysics::PhysicsObject(nameOfNode);
            mPhysicsHelper->AddPhysicsObject(*newTile);
            newTile->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
            newTile->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);
            newTile->CreateFromProperties(nodeToParse);
            return newTile.get();
         }
#endif

         //////////////////////////////////////////////////////////////////////
         bool NxAgeiaTerraPageLandActor::PassThisGeometry(int fid, int smc,
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
         void NxAgeiaTerraPageLandActor::DetermineHowToLoadGeometry(int fid,
            int smc, int soilTemperatureAndPressure, int soilWaterContent, osg::Node* nodeToLoad)
         {
            // the case is mostly going to be is this a box or is this a tree...
         }

         ////////////////////////////////////////////////////////////////////////
         bool NxAgeiaTerraPageLandActor::LoadGeomAsGroup(int fid)
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
         NxAgeiaTerraPageLandActorProxy::NxAgeiaTerraPageLandActorProxy()
         {
            SetClassName("NxAgeiaTerraPageLand");
         }

         //////////////////////////////////////////////////////////////////////
         void NxAgeiaTerraPageLandActorProxy::BuildPropertyMap()
         {
            dtGame::GameActorProxy::BuildPropertyMap();
            NxAgeiaTerraPageLandActor* actor = NULL;
            GetActor(actor);
            std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
            toFillIn.reserve(15U);
            actor->GetPhysicsHelper()->BuildPropertyMap(toFillIn);
            for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
            {
               AddProperty(toFillIn[i].get());
            }
         }

         //////////////////////////////////////////////////////////////////////
         NxAgeiaTerraPageLandActorProxy::~NxAgeiaTerraPageLandActorProxy(){}
         //////////////////////////////////////////////////////////////////////
         void NxAgeiaTerraPageLandActorProxy::CreateActor()
         {
            SetActor(*new NxAgeiaTerraPageLandActor(*this));
         }
         //////////////////////////////////////////////////////////////////////
         void NxAgeiaTerraPageLandActorProxy::OnEnteredWorld()
         {
            dtGame::GameActorProxy::OnEnteredWorld();

            // Ignore this object from any recording
            dtGame::LogController* logController
               = dynamic_cast<dtGame::LogController*> (GetGameManager()->GetComponentByName("LogController"));

            if( logController != NULL )
            {
               logController->RequestAddIgnoredActor( GetId() );
            }
         }

   } // namespace
} // namespace

