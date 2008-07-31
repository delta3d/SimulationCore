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
* @author Allen Danklefsen
*/
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/logcontroller.h>

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


      template< class T >
      class DrawableTriangleVisitor : public osg::NodeVisitor
      {
         public:

            //osg::TriangleFunctor<T> mFunctor;
            std::vector<osg::TriangleFunctor<T> > mFunctor;
            osg::ref_ptr<osg::Geode> legeode;

            /**
            * Constructor.
            */
            DrawableTriangleVisitor(NxAgeiaTerraPageLandActor& landActor) : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
               mLandActor(landActor)
            {
               legeode = new osg::Geode();
            }

            /**
            * Applies this visitor to a geode.
            * @param node the geode to visit
            */
            virtual void apply(osg::Geode& node)
            {
               osg::TriangleFunctor<T> Temp;
               int value[4];
               int iter = 0;
               for(unsigned int i=0;i<node.getNumDrawables();i++)
               {
                  osg::Drawable* d = node.getDrawable(i);

                  if(d->supports(Temp))
                  {
                     osg::NodePath nodePath = getNodePath();
                     Temp.mMatrix = osg::computeLocalToWorld(nodePath);

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
                              d->accept(Temp);
                           }
                           // if it's a building, then we want them in this list
                           //else if(mLandActor.LoadGeomAsGroup(value[0]))
                           //{
                           //   legeode->addDrawable(d);
                           //}
                           // what's left?  Probably vegetation, which gets ignored for now
                           else
                           {
                              //dtCore::RefPtr<osg::Geode> miniGeode = new osg::Geode();
                              //miniGeode->addDrawable(d);
                              //mLandActor.DetermineHowToLoadGeometry(value[0],value[1],value[2],value[3], miniGeode.get());
                           }
                        } // end ourlist size
                     }
                     else
                        d->accept(Temp);
                  }
               }
               mFunctor.push_back(Temp);
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
         NxAgeiaTerraPageLandActor::NxAgeiaTerraPageLandActor(dtGame::GameActorProxy &proxy) : GameActor(proxy)
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
                  terrainNodeToAdd->SetFilled( ParseTerrainNode( terrainNodeToAdd->GetGeodePointer(),
                                                                  terrainNodeToAdd->GetUniqueID().ToString(),
                                                                  *terrainNodeToAdd));
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
                     mPhysicsHelper->RemovePhysicsObject(*currentNode->GetPhysicsObject());
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
                  currentNode->SetFilled( ParseTerrainNode( currentNode->GetGeodePointer(),
                                                            currentNode->GetUniqueID().ToString(),
                                                            *currentNode));

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
#ifdef AGEIA_PHYSICS
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
                  currentNode->SetFilled( ParseTerrainNode( currentNode->GetGeodePointer(),
                     currentNode->GetUniqueID().ToString(),
                     *currentNode));
               }
            }
#endif
         }

      //////////////////////////////////////////////////////////////////////
      // Terrain tile loading
      //////////////////////////////////////////////////////////////////////

#ifdef AGEIA_PHYSICS
         //////////////////////////////////////////////////////////////////////
         bool NxAgeiaTerraPageLandActor::ParseTerrainNode(osg::Geode* nodeToParse,
            const std::string& nameOfNode, TerrainNode& terrainNode)
         {
            dtAgeiaPhysX::NxAgeiaWorldComponent* worldComponent =  NULL;
            GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent", worldComponent);

            if(worldComponent == NULL)
            {
               LOG_ERROR("worldComponent Is not initialized, make sure a new one \
                         was made before loading a map in, or setting physics objects");
               return false;
            }

            mLoadedTerrainYet = true;
            dtCore::RefPtr<DrawableTriangleVisitor<dtAgeiaPhysX::TriangleRecorder> > mv =
               new DrawableTriangleVisitor<dtAgeiaPhysX::TriangleRecorder>(*this);
            nodeToParse->accept(*mv.get());

            // our visitor should be all filled out now.
            if(mv->mFunctor.size())
            {
               //printf("Size = %d\n" ,mv->mFunctor.size());
               int sizeofVerts = 0;
               int sizeofFaces = 0;

               std::vector<osg::TriangleFunctor<dtAgeiaPhysX::TriangleRecorder> >::iterator funcIter = mv->mFunctor.begin();
               for(; funcIter != mv->mFunctor.end() ; ++funcIter)
               {
                  sizeofVerts += (*funcIter).mVertices.size();
                  sizeofFaces += (*funcIter).mTriangles.size()*3;
               }
               NxVec3*         gHeightfieldVerts = new NxVec3[sizeofVerts];
               NxU32*          gHeightfieldFaces = new NxU32[sizeofFaces];

               memset((void*)&gHeightfieldVerts[0], 0, sizeof(NxVec3) * sizeofVerts);
               memset((void*)&gHeightfieldFaces[0], 0, sizeof(NxU32) * sizeofFaces);

               int offset1 = 0;
               int offset2 = 0;
               int sizevarz = 0;
               std::vector<dtAgeiaPhysX::StridedVertex>::iterator   vertIter;
               std::vector<dtAgeiaPhysX::StridedTriangle>::iterator trngIter;
               for(funcIter = mv->mFunctor.begin(); funcIter != mv->mFunctor.end();
                   ++funcIter)
               {
                  for(vertIter = (*funcIter).mVertices.begin();
                      vertIter != (*funcIter).mVertices.end(); ++vertIter)
                  {
                     gHeightfieldVerts[offset1] = (*vertIter).Vertex;
                     offset1++;
                  }

                  for(trngIter = (*funcIter).mTriangles.begin();
                      trngIter != (*funcIter).mTriangles.end(); ++trngIter)
                  {
                     gHeightfieldFaces[offset2]     = (*trngIter).Indices[0] + sizevarz;
                     gHeightfieldFaces[offset2 + 1] = (*trngIter).Indices[1] + sizevarz;
                     gHeightfieldFaces[offset2 + 2] = (*trngIter).Indices[2] + sizevarz;
                     offset2 += 3;
                  }
                  sizevarz = offset1;
               }

               // Put this line back in to do debugging of terrain loading
               //LOG_ALWAYS(std::string("Found terrain node[") + nameOfNode + std::string("] with [") +
               //  dtUtil::ToString(sizeofVerts) + "] verts and [" + dtUtil::ToString(sizeofFaces) + "] triangles");

               // Build physical model
               NxTriangleMeshDesc heightfieldDesc;
               heightfieldDesc.numVertices        = sizeofVerts;
               heightfieldDesc.numTriangles       = sizeofFaces / 3;
               heightfieldDesc.pointStrideBytes   = sizeof(NxVec3);
               heightfieldDesc.triangleStrideBytes= 3*sizeof(NxU32);
               heightfieldDesc.points             = gHeightfieldVerts;
               heightfieldDesc.triangles          = gHeightfieldFaces;
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
                  delete [] gHeightfieldVerts;
                  delete [] gHeightfieldFaces;
                  return false;
               }

               NxActorDesc     actorDesc;
               heightfieldShapeDesc.meshData = worldComponent->GetPhysicsSDK().createTriangleMesh(SimCore::MMemoryReadBuffer(buf.data));
               actorDesc.shapes.pushBack(&heightfieldShapeDesc);
               actorDesc.userData = (void *) mPhysicsHelper.get();
               actorDesc.group = SimCore::CollisionGroup::GROUP_TERRAIN;

               delete [] gHeightfieldVerts;
               delete [] gHeightfieldFaces;

               NxActor *actor = worldComponent->GetPhysicsScene(std::string("Default")).createActor(actorDesc);
               terrainNode.SetPhysicsObject(actor);

               mPhysicsHelper->AddPhysXObject(*actor, nameOfNode.c_str());
               gCooking->NxCloseCooking();
               return true;
            }
            else
            {
               LOG_WARNING("Terrain tile not loaded in, this can happen for several reasons. 1. Bad Terrain. 2. Bad OSG_txp Dlls. 3. Material on this terrain was set to not be loaded in.");
               return false;
            }
            return false;
         }
#else
         //////////////////////////////////////////////////////////////////////
         bool NxAgeiaTerraPageLandActor::ParseTerrainNode(osg::Geode* nodeToParse,
            const std::string& nameOfNode, TerrainNode& terrainNode)
         {
            if(nodeToParse == NULL)
            {
               LOG_ALWAYS("Null nodeToParse sent into the ParseTerrainNode function! \
                  No action taken.");
               return false;
            }

            dtCore::RefPtr<dtPhysics::PhysicsObject> newTile = new dtPhysics::PhysicsObject(nameOfNode);
            mPhysicsHelper->AddPhysicsObject(*newTile);
            terrainNode.SetPhysicsObject(newTile.get());
            newTile->SetMechanicsType(dtPhysics::MechanicsType::STATIC);
            newTile->SetPrimitiveType(dtPhysics::PrimitiveType::TERRAIN_MESH);
            newTile->CreateFromProperties(nodeToParse);
            return true;
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

