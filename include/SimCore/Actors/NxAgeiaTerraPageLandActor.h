/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/
#ifndef _NX_AGEIA_TERRA_LAND_
#define _NX_AGEIA_TERRA_LAND_

#ifdef AGEIA_PHYSICS
   #include <NxAgeiaPrimitivePhysicsHelper.h>
   #include <Stream.h>
   #include <PhysicsGlobals.h>
 
   #include <osgDB/Registry>
   #include <osg/Geode>
   #include <osg/MatrixTransform>
   #include <osg/TriangleFunctor>
   
   #include <map>
#endif 

#include <dtGame/gameactor.h>
#include <SimCore/Export.h>

#ifdef AGEIA_PHYSICS

//namespace SimCore
//{
   //namespace Actors
   //{
      ////////////////////////////////////////////////////
      // Export symbol not needed, this should not be used
      // by external libraries
      class TerrainNode : public osg::Referenced
      {
         public:
            TerrainNode(osg::Geode* toSet);

         protected:
            ~TerrainNode();

         public:
            ///////////////////////////////////
            enum TILE_TODO
            {
               TILE_TODO_DISABLE = 0,
               TILE_TODO_KEEP,
               TILE_TODO_LOAD
            };

            /////////////////////////////////////////////////////////////////////////
            bool  IsFilled() const {return mFilledBL;}
            void  SetFilled(bool value) {mFilledBL = value;}

            /////////////////////////////////////////////////////////////////////////
            osg::Geode* GetGeodePointer() 
            {
               if(mGeodePTR.valid())
                  return mGeodePTR.get();
               return NULL;
            }

            /////////////////////////////////////////////////////////////////////////
            void SetFlags(char value)  {mFlags = value;}
            char GetFlags() const {return mFlags;}
            
            /////////////////////////////////////////////////////////////////////////
            void SetFlagsToKeep()    {mFlags = TILE_TODO_KEEP;}
            void SetFlagsToLoad()    {mFlags = TILE_TODO_LOAD;}
            void SetFlagsToDisable() {mFlags = TILE_TODO_DISABLE;}

            /////////////////////////////////////////////////////////////////////////
            const dtCore::UniqueId& GetUniqueID() const {return mUniqueID;}

            /////////////////////////////////////////////////////////////////////////
            void SetActor(NxActor* actor) {mActor = actor;}
            NxActor* GetActor() {return mActor;}

         private:
            NxActor*                         mActor;
            osg::observer_ptr<osg::Geode>    mGeodePTR;
            bool                             mFilledBL;
            char                             mFlags;
            dtCore::UniqueId                 mUniqueID;
      };

      class NxAgeiaTerraPageLandActor;
      //////////////////////////////////////////////////////////////////////
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

      ////////////////////////////////////////////////////////////////////
      //class NxAgeiaTerraPageListener;
      class SIMCORE_EXPORT NxAgeiaTerraPageLandActor : public dtGame::GameActor, 
                                                    public dtAgeiaPhysX::NxAgeiaPhysicsInterface
      {
         public:
            static const std::string& DEFAULT_NAME;
            /// Constructor
            NxAgeiaTerraPageLandActor(dtGame::GameActorProxy &proxy);

         protected:
            /// Destructor
            virtual ~NxAgeiaTerraPageLandActor(void);

            // internally called functions when a terrain tile is loaded into the system
            bool ParseTerrainNode(osg::Geode* nodeToParse, const std::string& nameOfNode, TerrainNode& terrainNode);
            
         public:
            
            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            // public accessor to get the variable
            dtAgeiaPhysX::NxAgeiaPhysicsHelper* GetPhysicsHelper() const 
            {return mPhysicsHelper.get();}

            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate(){}

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate(){}

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, 
               NxActor& ourSelf, NxActor& whatWeHit) {}

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, const NxActor& whatWeHit){}

            // determine if we want to use these hard coded materials and load to physics
            bool PassThisGeometry(int fid, int smc, int soilTemperatureAndPressure, 
               int soilWaterContent);

            // should this be a group or just heightfield.
            void DetermineHowToLoadGeometry(int fid, int smc, 
               int soilTemperatureAndPressure, int soilWaterContent, osg::Node* nodeToLoad);

            // should we load the geom as a group (buildings)
            bool LoadGeomAsGroup(int fid);

            // for the Hmmwv ground sim actor
            bool HasSomethingBeenLoaded() {return mLoadedTerrainYet;}

            // geode 
            void CheckGeode(osg::Geode& node, bool loadNow, const osg::Matrix& matrixForTransform);

            // reset terrain iterator so we can start at the beginning
            // and subdivide the work being done.
            void ResetTerrainIterator();

            // called to act on the flags.
            bool FinalizeTerrain(int amountOfFrames);

         private:

            // shouldnt be called, only for debugging purposes.
            // reloads all terrain to physics during runtimne
            void ReloadTerrainPhysics();

            // for the hmmwv sim
            bool mLoadedTerrainYet;

            // our map nodes
            std::map<osg::Geode*, dtCore::RefPtr<TerrainNode> > mTerrainMap;

            std::map<osg::Geode*, dtCore::RefPtr<TerrainNode> >::iterator mFinalizeTerrainIter;

            // our physics helper
            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPhysicsHelper> mPhysicsHelper;
      };

      #else
      //////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT NxAgeiaTerraPageLandActor : public dtGame::GameActor
      {
         public:
            static const std::string& DEFAULT_NAME;
           
            /// Constructor
            NxAgeiaTerraPageLandActor(dtGame::GameActorProxy &proxy);

            // geode 
            void CheckGeode(osg::Geode& node, bool loadNow, const osg::Matrix& matrixForTransform){};
            bool FinalizeTerrain(int number){return false;}
            void ResetTerrainIterator() {}

         protected:
            /// Destructor
            virtual ~NxAgeiaTerraPageLandActor(void);
      };
      #endif

   ///////////////////////////////////////////////////////////////////////////
   // Proxy
   ///////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT NxAgeiaTerraPageLandActorProxy : public dtGame::GameActorProxy
   {
      public:
         NxAgeiaTerraPageLandActorProxy();
         virtual void BuildPropertyMap();

      protected:
         virtual ~NxAgeiaTerraPageLandActorProxy();
         void CreateActor();
         virtual void OnEnteredWorld();
   };

   //} // namespace
//} // namespace

#endif
