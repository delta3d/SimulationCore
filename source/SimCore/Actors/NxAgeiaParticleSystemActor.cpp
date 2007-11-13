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
* @author Allen Danklefsen, Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>
#ifdef AGEIA_PHYSICS

#include <NxAgeiaWorldComponent.h>
#include <NxAgeiaPhysicsHelper.h>

#include <SimCore/Actors/NxAgeiaParticleSystemActor.h>

#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/transformable.h>

#include <dtDAL/enginepropertytypes.h>

#include <dtGame/basemessages.h>
#include <dtGame/environmentactor.h>

#include <dtUtil/log.h>

#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/IGActor.h>

#include <osg/Group>
#include <osg/MatrixTransform>


IMPLEMENT_ENUM(NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum);
NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum::TWOD("2D");
NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum::THREED("3D");

struct particleSystemToSave
{
   float MinAngVeloc[3];
   float MaxAngVeloc[3];
   float MinVeloc[3];
   float MaxVeloc[3];
   float MinVelOT[3];
   float MaxVelOT[3];
   int   physicsType;
   float Dimensions[3];
   bool  selfInteracting;
   float MinGravity[3]; 
   float MaxGravity[3];
   int   maxParticles;
   float MinCoord[3];   
   float MaxCoord[3];
   float LengthOfStay;
   bool  InfiniteSpawn; 
};

// could be done, not done - angular velocity over time...... wouldn't be hard.

////////////////////////////////////////////////////////////////////
NxAgeiaParticleSystemActor::NxAgeiaParticleSystemActor(dtGame::GameActorProxy &proxy) : dtGame::GameActor(proxy)
, mParticleEnumForObjectType(&NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum::THREED)
, mSpawnerParticleTimer(0.0f)
, mStartingLinearVelocityScaleMin(0,0,0)
, mStartingLinearVelocityScaleMax(0,0,0)
, mStartingLinearVelocityScaleInnerConeCap(0,0,0) // NI, TODO
, mStartingPositionRandMax(0,0,0)
, mStartingPositionRandMin(0,0,0)
, mStartingAngularVelocityScaleMin(0,0,0)
, mStartingAngularVelocityScaleMax(0,0,0)
, mForceVectorMin(0,0,0)
, mForceVectorMax(0,0,0)
, mAmountOfParticlesThatHaveSpawnedTotal(0)
, mParticleFadeOutInverseDeletion(3.0f)
, mParticleFadeInAmount(0)
, mParticleLengthOfStay(60.0f)
, mStaticObjectsTimeLength(60) // this is for the particle system not each particle.... keep dont delete although it seems its used twice.
, mGravityEnabled(true)
, mApplyForces(false)
, mSelfInteracting(false)
, mAmountofParticlesWeWantSpawned(300)
, mInfiniteParticleSystem(true)
, mHitOutParticleLimitDontSpawnAnymore(false)
, mIsCurrentlyOn(false)
, mSystemsTimeTotalTimeLength(0.0f)
, mParentsWorldRelativeVelocityVector(0,0,0)
, mObjectsStayStaticWhenHit(true)
, mObjectsStayStatic(false)
{
   mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
   mPhysicsHelper->SetBaseInterfaceClass(this);

   mParticleEmitterRateMin = 0.05f; // NI
   mParticleEmitterRateMax = 0.05f; // NI
}

////////////////////////////////////////////////////////////////////
NxAgeiaParticleSystemActor::~NxAgeiaParticleSystemActor()
{
   ResetParticleSystem(); 
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::LoadPhysicsParticleFile(const std::string& fileString)
{
   if(fileString.empty())
   {
      LOG_ERROR("File not found in LoadPhysicsParticleFile, nothing loaded");
      return;
   }

   particleSystemToSave particle;
   FILE* file = fopen(fileString.c_str(), "rb");
   fread((void*)&particle, sizeof(particleSystemToSave),1, file);
   fclose(file);

   float Dimensions[3];
   for(int i = 0 ; i < 3; i++)
   {
      //MinGravity[i] =particle.MinGravity[i]  ;
      Dimensions[i] =particle.Dimensions[i]  ;
      //MaxGravity[i] =particle.MaxGravity[i]  ;
      mStartingAngularVelocityScaleMin[i]=particle.MinAngVeloc[i] ;
      mStartingAngularVelocityScaleMax[i]=particle.MaxAngVeloc[i] ;
      mStartingLinearVelocityScaleMin[i]   =particle.MinVeloc[i]    ;
      mStartingLinearVelocityScaleMax[i]   =particle.MaxVeloc[i]    ;
      mForceVectorMin[i]   =particle.MinVelOT[i]    ;
      mForceVectorMax[i]   =particle.MaxVelOT[i]    ;
      mStartingPositionRandMin[i]   =particle.MinCoord[i]    ;   
      mStartingPositionRandMax[i]   =particle.MaxCoord[i]    ;
   }

   mPhysicsHelper->SetDimensions(osg::Vec3(Dimensions[0], Dimensions[1], Dimensions[2]));

   mAmountofParticlesWeWantSpawned   = particle.maxParticles   ;
   mParticleLengthOfStay   = particle.LengthOfStay   ;
   mInfiniteParticleSystem  = particle.InfiniteSpawn  ;
   mSelfInteracting= particle.selfInteracting ;

   switch(particle.physicsType)
   {
   default:
   case 1:
      mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CUBE);
      break;

   case 2:
      mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::SPHERE);
         break;

   case 0:
      mPhysicsHelper->SetPhysicsModelTypeEnum(dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CAPSULE);
         break;
   }
}
////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::TickRemote(const dtGame::Message &tickMessage){}
////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::TickLocal(const dtGame::Message &tickMessage)
{
   float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
   mSystemsTimeTotalTimeLength += ElapsedTime;

   if(mOurParticleList.size() > mAmountofParticlesWeWantSpawned)
      mHitOutParticleLimitDontSpawnAnymore = true;

   while(mOurParticleList.size() > mAmountofParticlesWeWantSpawned)
   {
      mPhysicsHelper->ReleasePhysXObject(mOurParticleList.front()->GetName().c_str());
      RemoveParticle(*mOurParticleList.front());
      mOurParticleList.pop_front();
   }

   if(mIsCurrentlyOn)
   {
      if(mInfiniteParticleSystem == true || (mInfiniteParticleSystem == false &&  mHitOutParticleLimitDontSpawnAnymore == false))
      {
         mSpawnerParticleTimer -= ElapsedTime;
         while(mSpawnerParticleTimer < 0.0f)
         {
            AddParticle();
            mSpawnerParticleTimer += mParticleEmitterRateMin;
            if(mOurParticleList.size() >= mAmountofParticlesWeWantSpawned)
            {
               mSpawnerParticleTimer = 0.0f;
            }
         }
      }
   }

   std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
   for(;iter!= mOurParticleList.end();)
   {
      // CURT HACK FOR IPT 2 - DISABLED FORCE ON MUNITIONS
     /* if(false && mApplyForces)
      {
         NxActor* physXActor = (*iter)->GetPhysicsActor();
         physXActor->addForce(
            NxVec3(  GetRandBetweenTwoFloats(mForceVectorMax[0], mForceVectorMin[0]), 
                     GetRandBetweenTwoFloats(mForceVectorMax[1], mForceVectorMin[1]),
                     GetRandBetweenTwoFloats(mForceVectorMax[2], mForceVectorMin[2])));
      }*/

      (*iter)->UpdateTime(ElapsedTime);
      if((*iter)->ShouldBeRemoved())
      {
         RemoveParticle(*(*iter));
         std::list<dtCore::RefPtr<PhysicsParticle> >::iterator toDelete = iter;
         ++iter;
         mOurParticleList.erase(toDelete);
         continue;
      }   
      else
      {
         ++iter;
      }
      //(*iter)->UpdateAlphaAmount();
   }

   // TODO Sync up inverse alpha delete time for all Particles if we using line below.
   if(mSystemsTimeTotalTimeLength > mStaticObjectsTimeLength && mObjectsStayStatic == true)
   {
      ResetParticleSystem();
      mPhysicsHelper->ReleaseAllPhysXObjects();
      dtAgeiaPhysX::NxAgeiaWorldComponent *worldComponent = dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"));
      worldComponent->UnRegisterAgeiaHelper(*mPhysicsHelper.get());
      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::ResetParticleSystem()
{
   while(!mOurParticleList.empty())
   {
      RemoveParticle(*mOurParticleList.front());
      mOurParticleList.pop_front();
   }
   mSpawnerParticleTimer = 0.0f;
   mHitOutParticleLimitDontSpawnAnymore = false;
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::RemoveParticle(PhysicsParticle& whichOne)
{
   mPhysicsHelper->ReleasePhysXObject(whichOne.GetName().c_str());

   if(whichOne.mObj->GetSceneParent() != NULL)
   {
      GetGameActorProxy().GetGameManager()->GetScene().RemoveDrawable(whichOne.mObj.get());
   }
   else
      whichOne.mObj->Emancipate();
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::AddParticle()
{
   dtCore::UniqueId _id;
   dtCore::RefPtr<PhysicsParticle> _particle = new PhysicsParticle(_id.ToString(), mParticleLengthOfStay);
   NxActor* newActor = NULL;

   dtCore::Transform ourTransform;
   ourTransform.Set(GetMatrixNode()->getMatrix());
   osg::Vec4 ourTranslation;
   ourTranslation[0] = ourTransform.GetTranslation()[0];
   ourTranslation[1] = ourTransform.GetTranslation()[1];
   ourTranslation[2] = ourTransform.GetTranslation()[2];
   osg::Matrix ourRotationMatrix = ourTransform.GetRotation();
   NxVec3 dimensions(mPhysicsHelper->GetDimensions()[0], mPhysicsHelper->GetDimensions()[1], mPhysicsHelper->GetDimensions()[2]);

   osg::Vec4 positionRandMax;
   positionRandMax.set(mStartingPositionRandMax[0], mStartingPositionRandMax[1], mStartingPositionRandMax[2], 0);
   osg::Vec4 positionRandMin;
   positionRandMin.set(mStartingPositionRandMin[0], mStartingPositionRandMin[1], mStartingPositionRandMin[2], 0);
   
   positionRandMax = ourRotationMatrix.preMult(positionRandMax);
   positionRandMin = ourRotationMatrix.preMult(positionRandMin);
 
   ourTranslation[0] = GetRandBetweenTwoFloats(ourTranslation[0] + positionRandMax[0], ourTranslation[0] + positionRandMin[0]);
   ourTranslation[1] = GetRandBetweenTwoFloats(ourTranslation[1] + positionRandMax[1], ourTranslation[1] + positionRandMin[1]); 
   ourTranslation[2] = GetRandBetweenTwoFloats(ourTranslation[2] + positionRandMax[2], ourTranslation[2] + positionRandMin[2]); 

   NxCollisionGroup collisionGroupToSendIn = 0;
   if(!mSelfInteracting)
      collisionGroupToSendIn = mPhysicsHelper->GetCollisionGroup();

   //_particle->mObj = new dtCore::Object(_id.ToString().c_str());
   _particle->mObj = new dtCore::Transformable(_id.ToString().c_str());

   int numPaths = 0;
   // Note this expects you to have 1 - 5 loaded correctly.
   for(int i = 0 ; i < 5; i++)
   {
      if(!mPathOfFileToLoad[i].empty())
         ++numPaths;
   }

   if( numPaths <= 0 )
   {
      LOG_WARNING("No file paths set for loading physics particle models");
      return;
   }
   std::string referenceString = mPathOfFileToLoad[rand() % numPaths];

   if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::TWOD)
   {
   }
   else if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::THREED)
   {
      LoadParticleResource(*_particle.get(), referenceString);
   }

   //////////////////////////////////////////////////////////////////////////
   // Set up the physics values for the object
   if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CUBE)
   {
      newActor = mPhysicsHelper->SetCollisionBox(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]), dimensions, 
                                       mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), _id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::SPHERE)
   {
      // load sphere
      newActor = mPhysicsHelper->SetCollisionSphere(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
                                          (dimensions[0] + dimensions[1] + dimensions[2]) / 3, 
                                          mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), _id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CAPSULE)
   {
      // load capsule
      newActor = mPhysicsHelper->SetCollisionCapsule(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]), 
                                          dimensions[2], (dimensions[0] + dimensions[1]) / 2, 
                                          mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(),
                                          _id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::FLATPLAIN)
   {
      // load flat plain            
      newActor = mPhysicsHelper->SetCollisionFlatSurface(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
                                             dimensions, collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), _id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CONVEXMESH)
   {
      dtCore::Transform initialTransform, identityTransform;
      identityTransform.Set(osg::Matrix());
      GetTransform(initialTransform);
      SetTransform(identityTransform);
      // load triangle mesh
      newActor = mPhysicsHelper->SetCollisionConvexMesh(_particle->mObj->GetOSGNode(), 
		         NxMat34(NxMat33(NxVec3(0,0,0), NxVec3(0,0,0), NxVec3(0,0,0)), 
               NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2])), 
                 mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), 
				 mPhysicsHelper->GetLoadAsCached(), referenceString, 
				 mPhysicsHelper->GetSceneName(), _id.ToString().c_str());
      
	  SetTransform(initialTransform);
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::TRIANGLEMESH)
   {
      dtCore::Transform initialTransform, identityTransform;
      identityTransform.Set(osg::Matrix());
      GetTransform(initialTransform);
      SetTransform(identityTransform);
      // load triangle mesh
      newActor = mPhysicsHelper->SetCollisionStaticMesh(_particle->mObj->GetOSGNode(), 
		         NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]), 
                 mPhysicsHelper->GetLoadAsCached(), referenceString, 
				 mPhysicsHelper->GetSceneName(), _id.ToString().c_str(), collisionGroupToSendIn);

      SetTransform(initialTransform);
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Set up emitter values on the particle...
   //pNewActor =  mPhysicsHelper->GetPhysXObject(_id.ToString().c_str());
   
   osg::Vec4 linearVelocities;
   linearVelocities[0] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[0], mStartingLinearVelocityScaleMin[0]);
   linearVelocities[1] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[1], mStartingLinearVelocityScaleMin[1]);
   linearVelocities[2] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[2], mStartingLinearVelocityScaleMin[2]);
   
   //linearVelocities = ourRotationMatrix.preMult(linearVelocities);

   linearVelocities[0] += mParentsWorldRelativeVelocityVector[0];
   linearVelocities[1] += mParentsWorldRelativeVelocityVector[1];
   linearVelocities[2] += mParentsWorldRelativeVelocityVector[2];

   //cone capping
   //for(int i = 0 ; i < 3; i++)
   //{
   //   if(linearVelocities[i] > -mStartingLinearVelocityScaleInnerConeCap[i] && linearVelocities[i] < mStartingLinearVelocityScaleInnerConeCap[i])
   //   {
   //      float diffone = (linearVelocities[i] - -mStartingLinearVelocityScaleInnerConeCap[i]) * (linearVelocities[i] - -mStartingLinearVelocityScaleInnerConeCap[i]);
   //      float difftwo = (linearVelocities[i] - mStartingLinearVelocityScaleInnerConeCap[i]) * (linearVelocities[i] - mStartingLinearVelocityScaleInnerConeCap[i]);
   //      
   //      if(diffone > difftwo)
   //      {
   //         linearVelocities[i] = -mStartingLinearVelocityScaleInnerConeCap[i];
   //      }
   //      else if(difftwo > diffone)
   //      {
   //         linearVelocities[i] = mStartingLinearVelocityScaleInnerConeCap[i];
   //      }
   //   }
   //}

   NxVec3 vRandVec(linearVelocities[0], linearVelocities[1], linearVelocities[2]);
   newActor->setLinearVelocity(vRandVec);

   vRandVec.set(  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[0], mStartingAngularVelocityScaleMin[0]), 
                  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[1], mStartingAngularVelocityScaleMin[1]), 
                  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[2], mStartingAngularVelocityScaleMin[2]));
   newActor->setAngularVelocity(vRandVec);
   if(!mGravityEnabled) 
      newActor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
   
   GetGameActorProxy().GetGameManager()->GetScene().AddDrawable(_particle->mObj.get());

   ++mAmountOfParticlesThatHaveSpawnedTotal;

   newActor->userData = mPhysicsHelper.get();

   // add to our list for updating and such....
   _particle->SetPhysicsActor(newActor);
   mOurParticleList.push_back(_particle);
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::LoadParticleResource(PhysicsParticle &particle, 
                                                      const std::string &resourceFile)
{
   // LOAD Object File - most of the time, it will be in the cache
   dtCore::RefPtr<osg::Node> cachedOriginalNode;
   dtCore::RefPtr<osg::Node> copiedNode;
   if (!SimCore::Actors::IGActor::LoadFileStatic(resourceFile, cachedOriginalNode, copiedNode, true))
      throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER, 
      std::string("Physics Particle System - mesh could not be loaded: ") + resourceFile, __FILE__, __LINE__);

   // Add the child to our transformable's group node
   osg::Group* g = particle.mObj->GetOSGNode()->asGroup();
   g->addChild(copiedNode.get());
   // hold a reference to the cached original to force it to stay in the cache
   g->setUserData(cachedOriginalNode.get()); 

   // old way
   //_particle->mObj->LoadFile(referenceString);
}

////////////////////////////////////////////////////////////////////
void PhysicsParticle::UpdateAlphaAmount()
{
   //if(mParticleLengthOfTimeOut - mInverseDeletionAlphaTime < mSpawnTimer)
   //{
   //   float alphaAmount = 1.0f * ((mParticleLengthOfTimeOut - mSpawnTimer) / mParticleLengthOfTimeOut);
      
      /*osg::ref_ptr<BlendVisitor> aVis = new BlendVisitor(alphaAmount);
      mObj->GetOSGNode()->accept(*aVis);*/

      /*osg::StateSet* ss = mObj->GetOSGNode()->getOrCreateStateSet();
      osg::BlendFunc* bf = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, alphaAmount, 1.0f);
      ss->setMode(GL_BLEND, osg::StateAttribute::ON);
      ss->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::ON);
      ss->setAttributeAndModes(bf, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);*/
   //}
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::OnEnteredWorld()
{
   dtAgeiaPhysX::NxAgeiaWorldComponent *component = dynamic_cast<dtAgeiaPhysX::NxAgeiaWorldComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("NxAgeiaWorldComponent"));
   component->RegisterAgeiaHelper(*mPhysicsHelper.get());

   // this way we dont turn off defaults to the scene.....
   // your particle system may not work the way you wanted if this 
   // wasnt set for w/e reason.
   if(mPhysicsHelper->GetCollisionGroup() != 0)
   {
      /*
      NxScene& nxScene = component->GetPhysicsScene(std::string("Default"));
      if(mSelfInteracting == false)
      {
         nxScene.setGroupCollisionFlag(mPhysicsHelper->GetCollisionGroup(), mPhysicsHelper->GetCollisionGroup(), false);
         //nxScene.setGroupCollisionFlag(0, mPhysicsHelper->GetCollisionGroup(), false);
      }
      // listen for contacts or not
      if (mObjectsStayStaticWhenHit)
         // turn off contact reports
         nxScene.setActorGroupPairFlags(mPhysicsHelper->GetCollisionGroup(), 0,  NX_IGNORE_PAIR);
      else 
         // only enable collision flags if the objects should be deleted when they collide with something
         nxScene.setActorGroupPairFlags(mPhysicsHelper->GetCollisionGroup(), 0,  NX_NOTIFY_ON_START_TOUCH | NX_NOTIFY_ON_TOUCH | NX_NOTIFY_ON_END_TOUCH);
      */ 
   }
   else
   {
      LOG_WARNING("You need to set your collision group to something other than 0 for the particle system, its going to give you an issue and not act correctly.");
   }

   if (mObjectsStayStaticWhenHit)
      mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
   else 
      mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_GET_COLLISION_REPORT);
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit)
{
   if(mObjectsStayStaticWhenHit == false)
   {
      std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
      for(;iter!= mOurParticleList.end(); ++iter)
      {
         if (&ourSelf == (*iter)->GetPhysicsActor())
         //if(&ourSelf == mPhysicsHelper->GetPhysXObject((*iter)->GetName().c_str()))
         {
            (*iter)->FlagToDelete();
            return;
         }
      }
   }
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActor::AgeiaPostPhysicsUpdate()
{
   std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
   for(;iter!= mOurParticleList.end(); ++iter)
   {
      if((*iter)->ShouldBeRemoved() == false)
      {
         //NxActor* physXActor = mPhysicsHelper->GetPhysXObject((*iter)->GetName().c_str());
         NxActor* physXActor = (*iter)->GetPhysicsActor();
         if(!physXActor->isSleeping())
         {
            float glmat[16];
            physXActor->getGlobalOrientation().getColumnMajorStride4(glmat);

            //clear the elements we don't need:
            glmat[3]  = glmat[7]  = glmat[11] = 0.0f;
            glmat[12] = physXActor->getGlobalPosition()[0];
            glmat[13] = physXActor->getGlobalPosition()[1];
            glmat[14] = physXActor->getGlobalPosition()[2];
            glmat[15] = 1.0f;

            (*iter)->mObj->GetMatrixNode()->setMatrix(osg::Matrix(glmat));
         }
      }
   }
}

////////////////////////////////////////////////////////////////////
// Actor Proxy Below here
////////////////////////////////////////////////////////////////////
NxAgeiaParticleSystemActorProxy::NxAgeiaParticleSystemActorProxy()
{
   SetClassName("NxAgeiaParticleSystemActor");
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActorProxy::BuildPropertyMap()
{
   const std::string GROUP = "NxAgeiaParticleSystem";
   const std::string EMMITER_GROUP = "Emitter Properties";
   const std::string PARTICLE_GROUP = "Particle Properties";
   
   dtGame::GameActorProxy::BuildPropertyMap();

   //RemoveProperty("Enable Dynamics");
   //RemoveProperty("Mass");
   //RemoveProperty("Center of Gravity");
   //RemoveProperty("Collision Box");
   //RemoveProperty("Collision Length");
   //RemoveProperty("Collision Radius");
   //RemoveProperty("Collision Type");
   //RemoveProperty("Show Collision Geometry");
   //RemoveProperty("Render Proxy Node");
   //RemoveProperty("Normal Rescaling");
   //RemoveProperty("Rotation");
   //RemoveProperty("Translation");
   //RemoveProperty("Scale");

   NxAgeiaParticleSystemActor &actor = static_cast<NxAgeiaParticleSystemActor&>(GetGameActor());

   std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
   actor.mPhysicsHelper->BuildPropertyMap(toFillIn);
   for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
      AddProperty(toFillIn[i].get());

   AddProperty(new dtDAL::EnumActorProperty<NxAgeiaParticleSystemActor::TwoDOrThreeDTypeEnum>("TwoDOrThreeDTypeEnum", "TwoDOrThreeDTypeEnum",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetTwoDOrThreeDTypeEnum),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetTwoDOrThreeDTypeEnum),
      "Holds a Type Enum property", EMMITER_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleEmitterRateMax", "ParticleEmitterRateMax",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetParticleEmitterRateMax),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetParticleEmitterRateMax),
      "", EMMITER_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleEmitterRateMin", "ParticleEmitterRateMin",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetParticleEmitterRateMin),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetParticleEmitterRateMin),
      "", EMMITER_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleLengthofStay", "ParticleLengthofStay",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetParticleLengthofStay),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetParticleLengthofStay),
      "", PARTICLE_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("EmitterTimeUntilDeletion", "EmitterTimeUntilDeletion",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetStaticObjectsLifeTime),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetStaticObjectsLifeTime),
      "", PARTICLE_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleFadeInTime", "ParticleFadeInTime",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetParticleFadeInTime),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetParticleFadeInTime),
      "", PARTICLE_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("ObjectsStayStaticWhenHit", "ObjectsStayStaticWhenHit",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetObjectsStayStaticWhenHit),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetObjectsStayStaticWhenHit),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::FloatActorProperty("ParticleFadeOutInverseDeletion", "ParticleFadeOutInverseDeletion",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetParticleFadeOutInverseDeletion),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetParticleFadeOutInverseDeletion),
      "", PARTICLE_GROUP));

   AddProperty(new dtDAL::IntActorProperty("NumberOfParticlesWeWantSpawned", "NumberOfParticlesWeWantSpawned",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetNumberOfParticlesWeWantSpawned),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetNumberOfParticlesWeWantSpawned),
      "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("ThisAsAnInfiniteParticleSystem", "ThisAsAnInfiniteParticleSystem",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetThisAsAnInfiniteParticleSystem),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetThisAsAnInfiniteParticleSystem),
      "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("Does Particle System Delete Itself", "Does Particle System Delete Itself",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetObjectToStatic),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetObjectToStatic),
      "Does Particle System Delete Itself", PARTICLE_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("GravityEnabledOnParticleSystem", "GravityEnabledOnParticleSystem",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetGravityEnabledOnParticleSystem),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetGravityEnabledOnParticleSystem),
      "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("ToApplyForcesToParticlesEveryFrame", "ToApplyForcesToParticlesEveryFrame",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetToApplyForcesToParticlesEveryFrame),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetToApplyForcesToParticlesEveryFrame),
      "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("CollideWithSelf", "CollideWithSelf",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetCollideWithSelf),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetCollideWithSelf),
      "", EMMITER_GROUP));

  /* AddProperty(new dtDAL::IntActorProperty("ParticleCollisionGroup", "ParticleCollisionGroup",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetParticleCollisionGroup),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetParticleCollisionGroup),
      "", EMMITER_GROUP));*/

   AddProperty(new dtDAL::Vec3ActorProperty("StartingPositionMin", "StartingPositionMin",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetStartingPositionMin),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetStartingPositionMin),
      "", PARTICLE_GROUP ));
   
   AddProperty(new dtDAL::Vec3ActorProperty("StartingPositionMax", "StartingPositionMax",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetStartingPositionMax),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetStartingPositionMax),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::Vec3ActorProperty("LinearVelocityStartMin", "LinearVelocityStartMin",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetLinearVelocityStartMin),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetLinearVelocityStartMin),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::Vec3ActorProperty("LinearVelocityStartMax", "LinearVelocityStartMax",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetLinearVelocityStartMax),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetLinearVelocityStartMax),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::Vec3ActorProperty("EmitterNoZoneEmitterCone", "EmitterNoZoneEmitterCone",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetEmitterNoZoneEmitteerCone),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetEmitterNoZoneEmitteerCone),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::Vec3ActorProperty("AngularVelocityStartMin", "AngularVelocityStartMin",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetAngularVelocityStartMin),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetAngularVelocityStartMin),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::Vec3ActorProperty("AngularVelocityStartMax", "AngularVelocityStartMax",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetAngularVelocityStartMax),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetAngularVelocityStartMax),
      "", PARTICLE_GROUP));
   
   AddProperty(new dtDAL::Vec3ActorProperty("OverTimeForceVecMin", "OverTimeForceVecMin",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetOverTimeForceVecMin),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetOverTimeForceVecMin),
      "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("OverTimeForceVecMax", "OverTimeForceVecMax",
      dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetOverTimeForceVecMax),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaParticleSystemActor::GetOverTimeForceVecMax),
      "", PARTICLE_GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
      "ObjectToUse1", "ObjectToUse1", dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetFileToLoadOne),
      "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
      "ObjectToUse2", "ObjectToUse2", dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetFileToLoadTwo),
      "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
      "ObjectToUse3", "ObjectToUse3", dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetFileToLoadThree),
      "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
      "ObjectToUse4", "ObjectToUse4", dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetFileToLoadFour),
      "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
      "ObjectToUse5", "ObjectToUse5", dtDAL::MakeFunctor(actor, &NxAgeiaParticleSystemActor::SetFileToLoadFive),
      "The static mesh resource that defines the geometry", GROUP));
}

////////////////////////////////////////////////////////////////////
NxAgeiaParticleSystemActorProxy::~NxAgeiaParticleSystemActorProxy(){}
////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActorProxy::CreateActor()
{
   SetActor(*new NxAgeiaParticleSystemActor(*this));
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActorProxy::OnEnteredWorld()
{
   dtGame::GameActorProxy::OnEnteredWorld();
   if (IsRemote())
      RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
   else
      RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
}

////////////////////////////////////////////////////////////////////
void NxAgeiaParticleSystemActorProxy::OnRemovedFromWorld()
{
   dtGame::GameActorProxy::OnRemovedFromWorld();
   NxAgeiaParticleSystemActor* actor = static_cast<NxAgeiaParticleSystemActor*>(GetActor());

   // Clear all existing particles
   if( actor != NULL ) { actor->ResetParticleSystem(); }
}
#endif
