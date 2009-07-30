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
#ifdef AGEIA_PHYSICS
#include <NxAgeiaPhysicsHelper.h>
#include <NxAgeiaWorldComponent.h>
#else
#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/bodywrapper.h>
#endif

#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/IGActor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/transform.h>

#include <dtDAL/enginepropertytypes.h>

#include <dtGame/basemessages.h>
#include <dtGame/environmentactor.h>

#include <dtUtil/log.h>


#include <osg/Group>
#include <osg/MatrixTransform>

IMPLEMENT_ENUM(PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum);
PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::TWO_D("2D");
PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::THREE_D("3D");
PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::TwoDOrThreeDTypeEnum(const std::string &name)
: dtUtil::Enumeration(name)
{
   AddInstance(this);
}

// could be done, not done - angular velocity over time...... wouldn't be hard.

////////////////////////////////////////////////////////////////////
PhysicsParticleSystemActor::PhysicsParticleSystemActor(dtGame::GameActorProxy &proxy) : dtGame::GameActor(proxy)
, mParticleEnumForObjectType(&PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::THREE_D)
, mSpawnerParticleTimer(0.0f)
, mParticleEmitterRateMin(0.05f) // NI
, mParticleEmitterRateMax(0.05f) // NI
, mParticleLengthOfStay(60.0f)
, mStaticObjectsTimeLength(60.0f) // this is for the particle system not each particle.... keep dont delete although it seems its used twice.
, mParticleFadeInAmount(0.0f)
, mParticleFadeOutInverseDeletion(3.0f)
, mSystemsTimeTotalTimeLength(0.0f)
, mAmountofParticlesWeWantSpawned(300)
, mAmountOfParticlesThatHaveSpawnedTotal(0)
, mInfiniteParticleSystem(true)
, mObjectsStayStatic(false)
, mObjectsStayStaticWhenHit(true)
, mGravityEnabled(true)
, mApplyForces(false)
, mSelfInteracting(false)
, mHitOutParticleLimitDontSpawnAnymore(false)
, mIsCurrentlyOn(false)
, mStartingPositionRandMin(0,0,0)
, mStartingPositionRandMax(0,0,0)
, mParentsWorldRelativeVelocityVector(0,0,0)
, mStartingLinearVelocityScaleMin(0,0,0)
, mStartingLinearVelocityScaleMax(0,0,0)
, mStartingLinearVelocityScaleInnerConeCap(0,0,0) // NI, TODO
, mStartingAngularVelocityScaleMin(0,0,0)
, mStartingAngularVelocityScaleMax(0,0,0)
, mForceVectorMin(0,0,0)
, mForceVectorMax(0,0,0)
{
#ifdef AGEIA_PHYSICS
   mPhysicsHelper = new dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper(proxy);
   mPhysicsHelper->SetBaseInterfaceClass(this);
#else
   mPhysicsHelper = new dtPhysics::PhysicsHelper(proxy);
   mPhysicsHelper->SetPostPhysicsCallback(
            dtPhysics::PhysicsHelper::UpdateCallback(this, &PhysicsParticleSystemActor::PostPhysicsUpdate));
#endif
}

////////////////////////////////////////////////////////////////////
PhysicsParticleSystemActor::~PhysicsParticleSystemActor()
{
   ResetParticleSystem();
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::TickRemote(const dtGame::Message &tickMessage){}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::TickLocal(const dtGame::Message &tickMessage)
{
   float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();
   mSystemsTimeTotalTimeLength += ElapsedTime;

   if(mOurParticleList.size() > mAmountofParticlesWeWantSpawned)
      mHitOutParticleLimitDontSpawnAnymore = true;

   while(mOurParticleList.size() > mAmountofParticlesWeWantSpawned)
   {
      mPhysicsHelper->RemovePhysicsObject(mOurParticleList.front()->GetName());
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
         dtPhysics::PhysicsObject* physXActor = (*iter)->GetPhysicsActor();
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
      mPhysicsHelper->ClearAllPhysicsObjects();
      dtPhysics::PhysicsComponent* physComponent = NULL;
      GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, physComponent);
      physComponent->UnregisterHelper(*mPhysicsHelper);
      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::ResetParticleSystem()
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
void PhysicsParticleSystemActor::RemoveParticle(PhysicsParticle& whichOne)
{
   mPhysicsHelper->RemovePhysicsObject(whichOne.GetName());

   if (whichOne.mObj->GetParent() != NULL)
   {
      whichOne.mObj->Emancipate();
   }
   else if(whichOne.mObj->GetSceneParent() != NULL)
   {
      GetGameActorProxy().GetGameManager()->GetScene().RemoveDrawable(whichOne.mObj.get());
   }
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::AddParticle()
{
   dtCore::UniqueId id;
   dtCore::RefPtr<PhysicsParticle> particle = new PhysicsParticle(id.ToString(), mParticleLengthOfStay);

   dtCore::Transform ourTransform;
   GetTransform(ourTransform);
   osg::Vec3 ourTranslation;
   osg::Vec3 xyz;
   ourTransform.GetTranslation(xyz);

   ourTranslation[0] = xyz[0];
   ourTranslation[1] = xyz[1];
   ourTranslation[2] = xyz[2];
   osg::Matrix ourRotationMatrix;
   ourTransform.GetRotation(ourRotationMatrix);

   osg::Vec3 positionRandMax;
   positionRandMax.set(mStartingPositionRandMax[0], mStartingPositionRandMax[1], mStartingPositionRandMax[2]);
   osg::Vec3 positionRandMin;
   positionRandMin.set(mStartingPositionRandMin[0], mStartingPositionRandMin[1], mStartingPositionRandMin[2]);

   positionRandMax = ourRotationMatrix.preMult(positionRandMax);
   positionRandMin = ourRotationMatrix.preMult(positionRandMin);

   ourTranslation[0] = GetRandBetweenTwoFloats(ourTranslation[0] + positionRandMax[0], ourTranslation[0] + positionRandMin[0]);
   ourTranslation[1] = GetRandBetweenTwoFloats(ourTranslation[1] + positionRandMax[1], ourTranslation[1] + positionRandMin[1]);
   ourTranslation[2] = GetRandBetweenTwoFloats(ourTranslation[2] + positionRandMax[2], ourTranslation[2] + positionRandMin[2]);

   SimCore::CollisionGroupType collisionGroupToSendIn = 0;
   if(!mSelfInteracting)
#ifdef AGEIA_PHYSICS
      collisionGroupToSendIn = mPhysicsHelper->GetCollisionGroup();
#else
      collisionGroupToSendIn = mPhysicsHelper->GetDefaultCollisionGroup();
#endif
   particle->mObj = new dtCore::Transformable(id.ToString());

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

   if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::TWO_D)
   {
   }
   else if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::THREE_D)
   {
      LoadParticleResource(*particle, referenceString);
   }

#ifdef AGEIA_PHYSICS
   NxVec3 dimensions(mPhysicsHelper->GetDimensions()[0], mPhysicsHelper->GetDimensions()[1], mPhysicsHelper->GetDimensions()[2]);
   dtPhysics::PhysicsObject* newActor = NULL;

   //////////////////////////////////////////////////////////////////////////
   // Set up the physics values for the object
   if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CUBE)
   {
      newActor = mPhysicsHelper->SetCollisionBox(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]), dimensions,
               mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::SPHERE)
   {
      // load sphere
      newActor = mPhysicsHelper->SetCollisionSphere(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
               (dimensions[0] + dimensions[1] + dimensions[2]) / 3,
               mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CAPSULE)
   {
      // load capsule
      newActor = mPhysicsHelper->SetCollisionCapsule(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
               dimensions[2], (dimensions[0] + dimensions[1]) / 2,
               mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(),
               id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::FLATPLAIN)
   {
      // load flat plain
      newActor = mPhysicsHelper->SetCollisionFlatSurface(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
               dimensions, collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), id.ToString().c_str());
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CONVEXMESH)
   {
      dtCore::Transform initialTransform, identityTransform;
      identityTransform.Set(osg::Matrix());
      GetTransform(initialTransform);
      SetTransform(identityTransform);
      // load triangle mesh
      newActor = mPhysicsHelper->SetCollisionConvexMesh(particle->mObj->GetOSGNode(),
               NxMat34(NxMat33(NxVec3(0,0,0), NxVec3(0,0,0), NxVec3(0,0,0)),
                        NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2])),
                        mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(),
                        mPhysicsHelper->GetLoadAsCached(), referenceString,
                        mPhysicsHelper->GetSceneName(), id.ToString().c_str());

      SetTransform(initialTransform);
   }
   else if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::TRIANGLEMESH)
   {
      dtCore::Transform initialTransform, identityTransform;
      identityTransform.Set(osg::Matrix());
      GetTransform(initialTransform);
      SetTransform(identityTransform);
      // load triangle mesh
      newActor = mPhysicsHelper->SetCollisionStaticMesh(particle->mObj->GetOSGNode(),
               NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
               mPhysicsHelper->GetLoadAsCached(), referenceString,
               mPhysicsHelper->GetSceneName(), id.ToString().c_str(), collisionGroupToSendIn);
      SetTransform(initialTransform);
   }
#else
   dtCore::RefPtr<dtPhysics::PhysicsObject> newObject = new dtPhysics::PhysicsObject(id.ToString());

   //////////////////////////////////////////////////////////////////////////
   // Set up the physics values for the object


   newObject->SetMass(mPhysicsHelper->GetMass());
   newObject->SetExtents(mPhysicsHelper->GetDimensions());
   newObject->SetCollisionGroup(collisionGroupToSendIn);
   newObject->SetPrimitiveType(mPhysicsHelper->GetDefaultPrimitiveType());
   newObject->CreateFromProperties(particle->mObj->GetOSGNode());

   dtCore::Transform xform;
   xform.SetTranslation(ourTranslation);
   newObject->SetTransform(xform);

#endif

   //////////////////////////////////////////////////////////////////////////
   // Set up emitter values on the particle...

   osg::Vec4 linearVelocities;
   linearVelocities[0] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[0], mStartingLinearVelocityScaleMin[0]);
   linearVelocities[1] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[1], mStartingLinearVelocityScaleMin[1]);
   linearVelocities[2] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[2], mStartingLinearVelocityScaleMin[2]);


   linearVelocities[0] += mParentsWorldRelativeVelocityVector[0];
   linearVelocities[1] += mParentsWorldRelativeVelocityVector[1];
   linearVelocities[2] += mParentsWorldRelativeVelocityVector[2];

#ifdef AGEIA_PHYSICS
   NxVec3 vRandVec(linearVelocities[0], linearVelocities[1], linearVelocities[2]);
   newActor->setLinearVelocity(vRandVec);

   vRandVec.set(  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[0], mStartingAngularVelocityScaleMin[0]),
            GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[1], mStartingAngularVelocityScaleMin[1]),
            GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[2], mStartingAngularVelocityScaleMin[2]));
   newActor->setAngularVelocity(vRandVec);
   if(!mGravityEnabled)
      newActor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);

   // add to our list for updating and such....
   particle->SetPhysicsObject(newActor);

   newActor->userData = mPhysicsHelper.get();
#else
   osg::Vec3 vRandVec(linearVelocities[0], linearVelocities[1], linearVelocities[2]);
   newObject->SetLinearVelocity(vRandVec);

   vRandVec.set(  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[0], mStartingAngularVelocityScaleMin[0]),
            GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[1], mStartingAngularVelocityScaleMin[1]),
            GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[2], mStartingAngularVelocityScaleMin[2]));
   newObject->SetAngularVelocity(vRandVec);
   particle->SetPhysicsObject(newObject.get());

   if (!mObjectsStayStaticWhenHit)
   {
      newObject->SetNotifyCollisions(true);
   }

   //TODO turn off gravity
#endif

   GetGameActorProxy().GetGameManager()->GetScene().AddDrawable(particle->mObj.get());
   ++mAmountOfParticlesThatHaveSpawnedTotal;

   mOurParticleList.push_back(particle);
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::LoadParticleResource(PhysicsParticle &particle,
         const std::string &resourceFile)
{
   // LOAD Object File - most of the time, it will be in the cache
   dtCore::RefPtr<osg::Node> cachedOriginalNode;
   dtCore::RefPtr<osg::Node> copiedNode;
   if (!SimCore::Actors::IGActor::LoadFileStatic(resourceFile, cachedOriginalNode, copiedNode, true))
   {
      /*throw dtUtil::Exception(dtGame::ExceptionEnum::INVALID_PARAMETER,
      std::string("Physics Particle System - mesh could not be loaded: ") + resourceFile, __FILE__, __LINE__);*/
      LOG_WARNING(std::string("Physics Particle System - mesh could not be loaded: ") + resourceFile);
   }

   // Add the child to our transformable's group node
   osg::Group* g = particle.mObj->GetOSGNode()->asGroup();
   g->addChild(copiedNode.get());
   // hold a reference to the cached original to force it to stay in the cache
   g->setUserData(cachedOriginalNode.get());

   // old way
   //particle->mObj->LoadFile(referenceString);
}

//class BlendVisitor : public osg::NodeVisitor
//{
//public:
//   BlendVisitor(float pBlend): osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), mBlendValue(pBlend)
//   {
//
//   }
//
//   virtual void apply(osg::Geode& geode)
//   {
//      osg::StateSet* ss = geode.getOrCreateStateSet();
//      osg::BlendFunc* bf = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, mBlendValue, 1.0f);
//      ss->setMode(GL_BLEND, osg::StateAttribute::ON);
//      ss->setAttributeAndModes(bf, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
//      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
//      osg::Material* mat = dynamic_cast<osg::Material*>(ss->getAttribute(osg::StateAttribute::MATERIAL));
//      if(mat)
//      {
//         mat->setAlpha(osg::Material::FRONT_AND_BACK, mBlendValue);
//      }
//
//      /*unsigned pNumDrawables = geode.getNumDrawables();
//      for(unsigned i = 0; i < pNumDrawables; ++i)
//      {
//      osg::Drawable* draw = geode.getDrawable(i);
//      osg::Material* mat = dynamic_cast<osg::Material*>(draw->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
//      if(mat)
//      {
//      mat->setAlpha(osg::Material::FRONT_AND_BACK, mBlendValue);
//      }
//      }*/
//   }
//
//private:
//   float mBlendValue;
//};

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::OnEnteredWorld()
{
   // this way we dont turn off defaults to the scene.....
   // your particle system may not work the way you wanted if this
   // wasnt set for w/e reason.

   dtPhysics::PhysicsComponent* component = NULL;
   GetGameActorProxy().GetGameManager()->GetComponentByName(dtPhysics::PhysicsComponent::DEFAULT_NAME, component);

   if (component != NULL)
   {
      component->RegisterHelper(*mPhysicsHelper);
   }
   else
   {
      LOGN_ERROR("PhysicsParticleSystem.cpp", "Unable to find a physics component with the default name in the GameManager.");
   }

#ifdef AGEIA_PHYSICS
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
   {
      mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE);
   }
   else
   {
      mPhysicsHelper->SetAgeiaFlags(dtAgeiaPhysX::AGEIA_FLAGS_POST_UPDATE | dtAgeiaPhysX::AGEIA_FLAGS_GET_COLLISION_REPORT);
   }
#else
   if(mPhysicsHelper->GetDefaultCollisionGroup() == 0)
   {
      LOG_WARNING("You need to set your collision group to something other than 0 for the particle system, its going to give you an issue and not act correctly.");
   }

#endif

}

#ifdef AGEIA_PHYSICS
////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit)
{
   if (!mObjectsStayStaticWhenHit)
   {
      std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
      for(;iter!= mOurParticleList.end(); ++iter)
      {
         if (&ourSelf == (*iter)->GetPhysicsObject())
            //if(&ourSelf == mPhysicsHelper->GetPhysXObject((*iter)->GetName().c_str()))
         {
            (*iter)->FlagToDelete();
            return;
         }
      }
   }
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::AgeiaPostPhysicsUpdate()
{
   std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
   for(;iter!= mOurParticleList.end(); ++iter)
   {
      if((*iter)->ShouldBeRemoved() == false)
      {
         //dtPhysics::PhysicsObject* physXActor = mPhysicsHelper->GetPhysXObject((*iter)->GetName().c_str());
         dtPhysics::PhysicsObject* physXActor = (*iter)->GetPhysicsObject();
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
#else

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActor::PostPhysicsUpdate()
{
   std::vector<dtPhysics::CollisionContact> contacts;
   std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter, iterEnd;
   iter = mOurParticleList.begin();
   iterEnd = mOurParticleList.end();
   for(;iter != iterEnd; ++iter)
   {
      PhysicsParticle* particle = (iter->get());
      contacts.clear();

      if (!mObjectsStayStaticWhenHit)
      {
         dtPhysics::PhysicsWorld::GetInstance().GetContacts(*particle->GetPhysicsObject(), contacts);
      }

      if (!contacts.empty())
         //if(&ourSelf == mPhysicsHelper->GetPhysXObject((*iter)->GetName().c_str()))
      {
         particle->FlagToDelete();
      }
      else if (!particle->ShouldBeRemoved())
      {
         dtPhysics::PhysicsObject* physObj = particle->GetPhysicsObject();
         if (physObj->IsActive())
         {
            dtCore::Transform xform;
            physObj->GetTransform(xform);
            particle->mObj->SetTransform(xform);
         }
      }
   }
}

#endif
////////////////////////////////////////////////////////////////////
// Actor Proxy Below here
////////////////////////////////////////////////////////////////////
PhysicsParticleSystemActorProxy::PhysicsParticleSystemActorProxy()
{
   SetClassName("PhysicsParticleSystemActor");
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActorProxy::BuildPropertyMap()
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

   PhysicsParticleSystemActor& actor = static_cast<PhysicsParticleSystemActor&>(GetGameActor());

   std::vector<dtCore::RefPtr<dtDAL::ActorProperty> >  toFillIn;
   actor.GetPhysicsHelper().BuildPropertyMap(toFillIn);
   for(unsigned int i = 0 ; i < toFillIn.size(); ++i)
      AddProperty(toFillIn[i].get());

   AddProperty(new dtDAL::EnumActorProperty<PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum>("TwoDOrThreeDTypeEnum", "TwoDOrThreeDTypeEnum",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetTwoDOrThreeDTypeEnum),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetTwoDOrThreeDTypeEnum),
            "Holds a Type Enum property", EMMITER_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleEmitterRateMax", "ParticleEmitterRateMax",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetParticleEmitterRateMax),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetParticleEmitterRateMax),
            "", EMMITER_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleEmitterRateMin", "ParticleEmitterRateMin",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetParticleEmitterRateMin),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetParticleEmitterRateMin),
            "", EMMITER_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleLengthofStay", "ParticleLengthofStay",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetParticleLengthofStay),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetParticleLengthofStay),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("EmitterTimeUntilDeletion", "EmitterTimeUntilDeletion",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetStaticObjectsLifeTime),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetStaticObjectsLifeTime),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleFadeInTime", "ParticleFadeInTime",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetParticleFadeInTime),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetParticleFadeInTime),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("ObjectsStayStaticWhenHit", "ObjectsStayStaticWhenHit",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetObjectsStayStaticWhenHit),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetObjectsStayStaticWhenHit),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::FloatActorProperty("ParticleFadeOutInverseDeletion", "ParticleFadeOutInverseDeletion",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetParticleFadeOutInverseDeletion),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetParticleFadeOutInverseDeletion),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::IntActorProperty("NumberOfParticlesWeWantSpawned", "NumberOfParticlesWeWantSpawned",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetNumberOfParticlesWeWantSpawned),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetNumberOfParticlesWeWantSpawned),
            "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("ThisAsAnInfiniteParticleSystem", "ThisAsAnInfiniteParticleSystem",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetThisAsAnInfiniteParticleSystem),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetThisAsAnInfiniteParticleSystem),
            "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("Does Particle System Delete Itself", "Does Particle System Delete Itself",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetObjectToStatic),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetObjectToStatic),
            "Does Particle System Delete Itself", PARTICLE_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("GravityEnabledOnParticleSystem", "GravityEnabledOnParticleSystem",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetGravityEnabledOnParticleSystem),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetGravityEnabledOnParticleSystem),
            "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("ToApplyForcesToParticlesEveryFrame", "ToApplyForcesToParticlesEveryFrame",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetToApplyForcesToParticlesEveryFrame),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetToApplyForcesToParticlesEveryFrame),
            "", EMMITER_GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("CollideWithSelf", "CollideWithSelf",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetCollideWithSelf),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetCollideWithSelf),
            "", EMMITER_GROUP));

   /* AddProperty(new dtDAL::IntActorProperty("ParticleCollisionGroup", "ParticleCollisionGroup",
      dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetParticleCollisionGroup),
      dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetParticleCollisionGroup),
      "", EMMITER_GROUP));*/

   AddProperty(new dtDAL::Vec3ActorProperty("StartingPositionMin", "StartingPositionMin",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetStartingPositionMin),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetStartingPositionMin),
            "", PARTICLE_GROUP ));

   AddProperty(new dtDAL::Vec3ActorProperty("StartingPositionMax", "StartingPositionMax",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetStartingPositionMax),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetStartingPositionMax),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("LinearVelocityStartMin", "LinearVelocityStartMin",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetLinearVelocityStartMin),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetLinearVelocityStartMin),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("LinearVelocityStartMax", "LinearVelocityStartMax",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetLinearVelocityStartMax),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetLinearVelocityStartMax),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("EmitterNoZoneEmitterCone", "EmitterNoZoneEmitterCone",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetEmitterNoZoneEmitteerCone),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetEmitterNoZoneEmitteerCone),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("AngularVelocityStartMin", "AngularVelocityStartMin",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetAngularVelocityStartMin),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetAngularVelocityStartMin),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("AngularVelocityStartMax", "AngularVelocityStartMax",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetAngularVelocityStartMax),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetAngularVelocityStartMax),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("OverTimeForceVecMin", "OverTimeForceVecMin",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetOverTimeForceVecMin),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetOverTimeForceVecMin),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::Vec3ActorProperty("OverTimeForceVecMax", "OverTimeForceVecMax",
            dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetOverTimeForceVecMax),
            dtDAL::MakeFunctorRet(actor, &PhysicsParticleSystemActor::GetOverTimeForceVecMax),
            "", PARTICLE_GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "ObjectToUse1", "ObjectToUse1", dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetFileToLoadOne),
            "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "ObjectToUse2", "ObjectToUse2", dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetFileToLoadTwo),
            "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "ObjectToUse3", "ObjectToUse3", dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetFileToLoadThree),
            "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "ObjectToUse4", "ObjectToUse4", dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetFileToLoadFour),
            "The static mesh resource that defines the geometry", GROUP));

   AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
            "ObjectToUse5", "ObjectToUse5", dtDAL::MakeFunctor(actor, &PhysicsParticleSystemActor::SetFileToLoadFive),
            "The static mesh resource that defines the geometry", GROUP));
}

////////////////////////////////////////////////////////////////////
PhysicsParticleSystemActorProxy::~PhysicsParticleSystemActorProxy(){}
////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActorProxy::CreateActor()
{
   SetActor(*new PhysicsParticleSystemActor(*this));
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActorProxy::OnEnteredWorld()
{
   dtGame::GameActorProxy::OnEnteredWorld();
   if (IsRemote())
      RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
   else
      RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
}

////////////////////////////////////////////////////////////////////
void PhysicsParticleSystemActorProxy::OnRemovedFromWorld()
{
   dtGame::GameActorProxy::OnRemovedFromWorld();
   PhysicsParticleSystemActor* actor = NULL;
   GetActor(actor);

   // Clear all existing particles
   if( actor != NULL )
   {
      actor->ResetParticleSystem();
   }
}

//////////////////////////////////////////////////////////////////
float PhysicsParticleSystemActor::GetRandBetweenTwoFloats(float max, float min)
{
   if(min > max)
      min = max;

   if(max == min)
      return max;

   int Max = (int)(max * 100);
   int Min = (int)(min * 100);

   int result = (rand() % (Max - Min + 1) + Min);
   float Result = (float)result / 100;
   return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
PhysicsParticle::PhysicsParticle(const std::string& name, float ParticleLengthOfTimeOut, float InverseDeletionAlphaTime, float alphaInTime)
{
   mSpawnTimer = 0;
   mParticleLengthOfTimeOut = ParticleLengthOfTimeOut;
   mInverseDeletionAlphaTime = InverseDeletionAlphaTime;
   mAlphaInStartTime = alphaInTime;
   mBeenHit = false;
   mName = name;
   mNeedsToBeDeleted = false;
   mPhysicsObject = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
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
      ss->setAttributeAndModes(bf, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
      ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);*/
   //}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsParticle::UpdateTime(float elapsedTime) {mSpawnTimer += elapsedTime;}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool PhysicsParticle::ShouldBeRemoved()
{
   if(mSpawnTimer > mParticleLengthOfTimeOut || mNeedsToBeDeleted == true)
      return true;
   return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsParticle::HitReceived()
{
   if(mBeenHit == false)
   {
      mSpawnTimer = mParticleLengthOfTimeOut - mInverseDeletionAlphaTime;
   }
   mBeenHit = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
void PhysicsParticle::FlagToDelete()
{
   mNeedsToBeDeleted = true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
const std::string& PhysicsParticle::GetName() const
{
   return mName;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
dtPhysics::PhysicsObject* PhysicsParticle::GetPhysicsObject()
{
#ifdef AGEIA_PHYSICS
   return mPhysicsObject;
#else
   return mPhysicsObject.get();
#endif
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void PhysicsParticle::SetPhysicsObject(dtPhysics::PhysicsObject* physObj)
{
   mPhysicsObject = physObj;
}
