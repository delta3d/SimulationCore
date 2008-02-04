/* 
* Delta3D Open Source Game and Simulation Engine 
* Copyright (C) 2004-2008 MOVES Institute 
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
* @author Bradley Anderegg, modded by allen danklefsen
*/

#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/oceanwater.h>
#include <osg/Group>
#include <osg/Drawable>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/PrimitiveSet>
#include <osgDB/ReadFile>   
#include <osg/Uniform>
#include <osg/BlendFunc>
#include <dtCore/system.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/noiseutility.h>
#include <dtUtil/log.h>
#include <osgDB/FileUtils>
#include <assert.h>

#include <dtDAL/exceptionenum.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/functor.h>

#include <dtCore/shadermanager.h>
#include <dtCore/shaderparameter.h>
#include <dtCore/shaderparamfloat.h>

#include <NxAgeiaWorldComponent.h>

#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      OceanWater::OceanWater(dtGame::GameActorProxy& proxy) : dtGame::GameActor(proxy)
         , mWaterHeight(0.0f)
         , mElapsedTime(0.0f)
         , mDeltaTime(0.0f)
         , mWaterSpeed(1.0f/20.f)
      {
#ifdef AGEIA_PHYSICS
         mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
         mPhysicsHelper->SetBaseInterfaceClass(this);
#endif
         mCenter.set(0.0f, 0.0f);
         mSize.set(100.0f, 100.0f);
         mResolution.set(50.0f, 50.0f);
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWater::OnEnteredWorld()
      {
         GameActor::OnEnteredWorld();

#ifdef AGEIA_PHYSICS

         // make it in a different group

         // load box collision
         
         // make it kinematic

         mPhysicsHelper->SetIsKinematic(true);
         mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CUBE);
         mPhysicsHelper->SetDimensions(osg::Vec3(mSize[0], mSize[1], 1));
         mPhysicsHelper->SetCollisionGroup(23);
         mPhysicsHelper->SetAgeiaMass(5000);

         NxMat34 sendInMatrix;
         sendInMatrix.id();
         mPhysicsHelper->InitializePrimitive(NULL, sendInMatrix);
         
         mPhysicsHelper->SetAgeiaUserData(mPhysicsHelper.get());
         mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_PRE_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);

         dtGame::GMComponent *comp = 
            GetGameActorProxy().GetGameManager()->GetComponentByName(dtAgeiaPhysX::NxAgeiaWorldComponent::DEFAULT_NAME);
         if(comp != NULL)
         {
            static_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(comp)->RegisterAgeiaHelper(*mPhysicsHelper.get());
         }
#endif

         mGeode = new osg::Geode();

         dtCore::Transform ourtransform;
         ourtransform.SetTranslation(osg::Vec3( (-mSize[0] / 2.0) + mCenter[0], (-mSize[1] / 2.0) + mCenter[1], mWaterHeight));
         SetTransform(ourtransform);

         CreateGeometry();

         GetOSGNode()->asGroup()->addChild(mGeode.get());

         if (!GetShaderGroup().empty())
            SetShaderGroup(GetShaderGroup());
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWater::TickLocal(const dtGame::Message &tickMessage)
      {
         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         Update(ElapsedTime);
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWater::TickRemote(const dtGame::Message &tickMessage)
      {
         float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
         Update(ElapsedTime);
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWater::Update(double dt)
      {  
         float mWaterUpdateSpeed = 2.0f;
         mElapsedTime += dt;
         mDeltaTime += dt; 

         dtCore::ShaderProgram *shaderProgram = 
            dtCore::ShaderManager::GetInstance().GetShaderInstanceForNode(GetOSGNode());

         if (shaderProgram != NULL)
         {
            dtCore::ShaderParamFloat* angle = dynamic_cast<dtCore::ShaderParamFloat*>(shaderProgram->FindParameter("currentAngle"));
            dtCore::ShaderParamFloat* texIncX = dynamic_cast<dtCore::ShaderParamFloat*>(shaderProgram->FindParameter("texIncX"));
            dtCore::ShaderParamFloat* texIncY = dynamic_cast<dtCore::ShaderParamFloat*>(shaderProgram->FindParameter("texIncY"));
            dtCore::ShaderParamFloat* texIncPrevX = dynamic_cast<dtCore::ShaderParamFloat*>(shaderProgram->FindParameter("texIncPrevX"));
            dtCore::ShaderParamFloat* texIncPrevY = dynamic_cast<dtCore::ShaderParamFloat*>(shaderProgram->FindParameter("texIncPrevY"));
            dtCore::ShaderParamFloat* blendValue = dynamic_cast<dtCore::ShaderParamFloat*>(shaderProgram->FindParameter("blend"));
            
            if(angle && texIncPrevX && texIncPrevY && texIncX && texIncY && blendValue)
            {
               angle->SetValue(mElapsedTime);

               if(mDeltaTime > mWaterUpdateSpeed)
               {
                  texIncPrevX->SetValue(mTexShift[0]);
                  texIncPrevY->SetValue(mTexShift[1]);
                  
                  mTexShift.set(mElapsedTime * mWaterSpeed, mElapsedTime * mWaterSpeed);

                  texIncX->SetValue(mTexShift[0]);
                  texIncY->SetValue(mTexShift[1]);

                  blendValue->SetValue(0.0f);
                  mDeltaTime = 0.0f;
               }
               else
               {
                  blendValue->SetValue(mDeltaTime / mWaterUpdateSpeed);
               }
            }
         }
      }

      
      //////////////////////////////////////////////////////////////////////////
      //void OceanWater::CreateNodes()
      //{
         //mNode = new osg::Group();
         
         //mXform = new osg::PositionAttitudeTransform();
        

         //mXform->setPosition(osg::Vec3( (-mSize[0] / 2.0) + mCenter[0], (-mSize[1] / 2.0) + mCenter[1], mWaterHeight ));
         //mXform->addChild(mGeode.get());

         //osg::Group* grp = new osg::Group;
         //mGeode->setName("Water Surface");
        
         //grp->addChild(mXform.get());
         //mNode = mXform.get();

         //GetOSGNode()->asGroup()->addChild(mGeode.get());
      //}

      //////////////////////////////////////////////////////////////////////////
      void OceanWater::CreateGeometry()
      {
         mGeode->removeDrawable(mGeometry.get());

         mGeometry = new osg::Geometry();

         //the resolution is the number of verts to span the size
         //so we must at least have 2
         if(mResolution[0] < 2.0f) mResolution[0] = 2.0f;
         if(mResolution[1] < 2.0f) mResolution[1] = 2.0f;

         //since resolution must be an integer, we enforce this here
         int resX = int(mResolution[0]);
         int resY = int(mResolution[1]);
         mResolution[0] = resX;
         mResolution[1] = resY;

         //calculate num verts and num indices
         int numVerts = resX * resY;
         int numIndices = (resX - 1) * (resY - 1) * 6;

         //inc is the distance between the verts
         osg::Vec2 inc;
         inc[0] = mSize[0] / mResolution[0];
         inc[1] = mSize[1] / mResolution[1];

         //srand(394823);
         dtUtil::Noise1f noise1;   

         //lets make the geometry
         dtCore::RefPtr<osg::Vec3Array> pVerts = new osg::Vec3Array(numVerts);
         dtCore::RefPtr<osg::Vec2Array> pTexCoords1 = new osg::Vec2Array(numVerts);
         dtCore::RefPtr<osg::IntArray> pIndices = new osg::IntArray(numIndices);

         for(int i = 0; i < resX; ++i)
         {
            for(int j = 0; j < resY; ++j)
            {
               osg::Vec2 pTex(float(i) / mResolution[0], float(j) / mResolution[1]);

               (*pVerts)[(i * resX) + j ].set( i * inc[0], j * inc[1], 0);
               (*pTexCoords1)[(i * resX) + j ] = pTex;
            }
         }

         int counter = 0;

         for(int i = 0; i < resX - 1; ++i)
         {
            for(int j = 0; j < resY - 1; ++j)
            {
               (*pIndices)[counter] = (i * resX) + j;
               (*pIndices)[counter + 1] = ((i + 1) * resX) + j;
               (*pIndices)[counter + 2] = ((i + 1) * resX) + (j + 1);

               (*pIndices)[counter + 3] = ((i + 1) * resX) + (j + 1);
               (*pIndices)[counter + 4] = (i * resX) + (j + 1);
               (*pIndices)[counter + 5] = (i * resX) + j;

               counter += 6;
            }
         }
         
         mGeometry->setVertexArray(pVerts.get());
         mGeometry->setTexCoordArray(0, pTexCoords1.get());
         
         mGeometry->setVertexIndices(pIndices.get());
         mGeometry->setTexCoordIndices(0, pIndices.get());

         mGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, numIndices));
         mGeode->addDrawable(mGeometry.get());
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWaterActorProxy::CreateActor()
      {
         SetActor(*new OceanWater(*this));
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWaterActorProxy::BuildPropertyMap()
      {
         const std::string GROUPNAME = "OceanWater";

         GameActorProxy::BuildPropertyMap();

         OceanWater  &actor = static_cast<OceanWater &>(GetGameActor());

         AddProperty(new dtDAL::FloatActorProperty("Water Height","Water Height",
            dtDAL::MakeFunctor(actor,&OceanWater::SetHeight),
            dtDAL::MakeFunctorRet(actor,&OceanWater::GetHeight),
            "Sets the height of the water.", GROUPNAME));

         AddProperty(new dtDAL::Vec2ActorProperty("Water Size","Water Size",
            dtDAL::MakeFunctor(actor,&OceanWater::SetSize),
            dtDAL::MakeFunctorRet(actor,&OceanWater::GetSize),
            "Sets the size of the water.", GROUPNAME));

         AddProperty(new dtDAL::Vec2ActorProperty("Mesh Resolution","Mesh Resolution",
            dtDAL::MakeFunctor(actor,&OceanWater::SetResolution),
            dtDAL::MakeFunctorRet(actor,&OceanWater::GetResolution),
            "Sets the number of verts per column contained by the mesh.", GROUPNAME));

         AddProperty(new dtDAL::Vec2ActorProperty("Center","Center",
            dtDAL::MakeFunctor(actor,&OceanWater::SetCenter),
            dtDAL::MakeFunctorRet(actor,&OceanWater::GetCenter),
            "Sets the center of the water.", GROUPNAME)); 
      }

      //////////////////////////////////////////////////////////////////////////
      void OceanWaterActorProxy::OnEnteredWorld()
      {
         RegisterForMessages(dtGame::MessageType::INFO_GAME_EVENT);

         if (IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE, 
                  dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }
         else
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, 
                  dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         }

         GameActorProxy::OnEnteredWorld();
      }
   } //namespace
}//namespace
