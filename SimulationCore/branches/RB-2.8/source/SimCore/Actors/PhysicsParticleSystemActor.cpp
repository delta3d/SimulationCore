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
 * Allen Danklefsen
 */
#include <prefix/SimCorePrefix.h>

#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/bodywrapper.h>

#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/IGActor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/transform.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/functor.h>

#include <dtGame/basemessages.h>
#include <dtGame/environmentactor.h>
#include <dtGame/messagetype.h>

#include <dtUtil/log.h>


#include <osg/Group>
#include <osg/MatrixTransform>

namespace SimCore
{
   namespace Actors
   {

      IMPLEMENT_ENUM(PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum);
      PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::TWO_D("2D");
      PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::THREE_D("3D");
      PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum::TwoDOrThreeDTypeEnum(const std::string& name)
      : dtUtil::Enumeration(name)
      {
         AddInstance(this);
      }

      // could be done, not done - angular velocity over time...... wouldn't be hard.

      ////////////////////////////////////////////////////////////////////
      PhysicsParticleSystemActor::PhysicsParticleSystemActor(dtGame::GameActorProxy& proxy) : dtGame::GameActor(proxy)
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
      }

      ////////////////////////////////////////////////////////////////////
      PhysicsParticleSystemActor::~PhysicsParticleSystemActor()
      {
         ResetParticleSystem();
      }

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActor::OnTickRemote(const dtGame::TickMessage& tickMessage){}

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         float ElapsedTime = tickMessage.GetDeltaSimTime();
         mSystemsTimeTotalTimeLength += ElapsedTime;

         if (mOurParticleList.size() > mAmountofParticlesWeWantSpawned)
         {
            mHitOutParticleLimitDontSpawnAnymore = true;
         }

         while(mOurParticleList.size() > mAmountofParticlesWeWantSpawned)
         {
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

         ParticleList::iterator iter = mOurParticleList.begin();
         for(;iter!= mOurParticleList.end();)
         {
            (*iter)->UpdateTime(ElapsedTime);
            if((*iter)->ShouldBeRemoved())
            {
               RemoveParticle(*(*iter));
               ParticleList::iterator toDelete = iter;
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
            mPhysicsActComp->ClearAllPhysicsObjects();
            GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
         }
      }

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActor::ResetParticleSystem()
      {
         ParticleList::iterator i, iend;
         i = mOurParticleList.begin();
         iend = mOurParticleList.end();
         for(; i != iend; ++i)
         {
            PhysicsParticle& particle = **i;
            if (particle.GetPhysicsObject()->GetBaseBodyWrapper() != NULL)
            {
               LOG_ERROR("During a Reset, got a particle that is not yet cleaned up by the physics helper.");
            }
            RemoveParticle(particle);
         }

         mOurParticleList.clear();

         mSpawnerParticleTimer = 0.0f;
         mHitOutParticleLimitDontSpawnAnymore = false;
      }

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActor::RemoveParticle(PhysicsParticle& whichOne)
      {
         mPhysicsActComp->RemovePhysicsObject(*whichOne.GetPhysicsObject());
         whichOne.GetPhysicsObject()->CleanUp();

         if (whichOne.mObj->GetParent() != NULL)
         {
            whichOne.mObj->Emancipate();
         }
         else if(whichOne.mObj->GetSceneParent() != NULL)
         {
            RemoveChild(whichOne.mObj.get());
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
         if (!mSelfInteracting)
         {
            collisionGroupToSendIn = mPhysicsActComp->GetDefaultCollisionGroup();
         }

         particle->mObj = new dtCore::Transformable(id.ToString());

         int numPaths = 0;
         // Note this expects you to have 1 - 5 loaded correctly.
         for(int i = 0 ; i < 5; i++)
         {
            if(!mPathOfFileToLoad[i].empty())
            {
               ++numPaths;
            }
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

         dtCore::RefPtr<dtPhysics::PhysicsObject> newObject = new dtPhysics::PhysicsObject(id.ToString());

         //////////////////////////////////////////////////////////////////////////
         // Set up the physics values for the object


         dtPhysics::Real skinWidth = 0.10f;
         newObject->SetMass(mPhysicsActComp->GetMass());
         dtPhysics::VectorType dimensions = mPhysicsActComp->GetDimensions();
         for (unsigned i = 0; i < 3; ++i)
         {
            dimensions[i] += skinWidth;
         }
         newObject->SetExtents(dimensions);
         newObject->SetCollisionGroup(collisionGroupToSendIn);
         newObject->SetPrimitiveType(mPhysicsActComp->GetDefaultPrimitiveType());
         newObject->SetActivationLinearVelocityThreshold(dtPhysics::Real(1.0));
         newObject->SetActivationAngularVelocityThreshold(dtPhysics::Real(3.0));
         newObject->SetActivationTimeThreshold(dtPhysics::Real(1.0));
         newObject->SetLinearDamping(dtPhysics::Real(0.1));
         newObject->SetAngularDamping(dtPhysics::Real(0.4));
         newObject->SetSkinThickness(dtPhysics::Real(0.04));
         newObject->CreateFromProperties(particle->mObj->GetOSGNode());
         newObject->SetSkinThickness(dtPhysics::Real(0.0));
         osg::Vec3 moment = newObject->GetMomentOfInertia();
         newObject->SetMomentOfInertia(moment * 2.0f);

         dtCore::Transform xform;
         xform.SetTranslation(ourTranslation);
         newObject->SetTransform(xform);

         //////////////////////////////////////////////////////////////////////////
         // Set up emitter values on the particle...

         osg::Vec4 linearVelocities;
         linearVelocities[0] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[0], mStartingLinearVelocityScaleMin[0]);
         linearVelocities[1] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[1], mStartingLinearVelocityScaleMin[1]);
         linearVelocities[2] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[2], mStartingLinearVelocityScaleMin[2]);


         linearVelocities[0] += mParentsWorldRelativeVelocityVector[0];
         linearVelocities[1] += mParentsWorldRelativeVelocityVector[1];
         linearVelocities[2] += mParentsWorldRelativeVelocityVector[2];

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

         AddChild(particle->mObj.get());
         mPhysicsActComp->AddPhysicsObject(*newObject);

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

         GetComponent(mPhysicsActComp);

         mPhysicsActComp->SetPostPhysicsCallback(
                  dtPhysics::PhysicsActComp::UpdateCallback(this, &PhysicsParticleSystemActor::PostPhysicsUpdate));

         if (mPhysicsActComp->GetDefaultCollisionGroup() == 0)
         {
            LOG_WARNING("You need to set your collision group to something other than 0 for the particle system, its going to give you an issue and not act correctly.");
         }

      }

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActor::PostPhysicsUpdate()
      {
         std::vector<dtPhysics::CollisionContact> contacts;
         ParticleList::iterator iter, iterEnd;
         iter = mOurParticleList.begin();
         iterEnd = mOurParticleList.end();
         for(;iter != iterEnd; ++iter)
         {
            PhysicsParticle* particle = (iter->get());
            contacts.clear();

            if (!mObjectsStayStaticWhenHit && !particle->IsFlaggedToDelete())
            {
               dtPhysics::PhysicsWorld::GetInstance().GetContacts(*particle->GetPhysicsObject(), contacts);
            }

            if (!contacts.empty())
               //if(&ourSelf == mPhysicsActComp->GetPhysXObject((*iter)->GetName().c_str()))
            {
               particle->FlagToDelete();
               // Don't need any more collsions reports.
               particle->GetPhysicsObject()->SetNotifyCollisions(false);
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

         PhysicsParticleSystemActor* actor = NULL;
         GetActor(actor);

         AddProperty(new dtDAL::EnumActorProperty<PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum>("TwoDOrThreeDTypeEnum", "TwoDOrThreeDTypeEnum",
                  dtDAL::EnumActorProperty<PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum>::SetFuncType(actor, &PhysicsParticleSystemActor::SetTwoDOrThreeDTypeEnum),
                  dtDAL::EnumActorProperty<PhysicsParticleSystemActor::TwoDOrThreeDTypeEnum>::GetFuncType(actor, &PhysicsParticleSystemActor::GetTwoDOrThreeDTypeEnum),
                  "Holds a Type Enum property", EMMITER_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("ParticleEmitterRateMax", "ParticleEmitterRateMax",
                  dtDAL::FloatActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetParticleEmitterRateMax),
                  dtDAL::FloatActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetParticleEmitterRateMax),
                  "", EMMITER_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("ParticleEmitterRateMin", "ParticleEmitterRateMin",
                  dtDAL::FloatActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetParticleEmitterRateMin),
                  dtDAL::FloatActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetParticleEmitterRateMin),
                  "", EMMITER_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("ParticleLengthofStay", "ParticleLengthofStay",
                  dtDAL::FloatActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetParticleLengthofStay),
                  dtDAL::FloatActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetParticleLengthofStay),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("EmitterTimeUntilDeletion", "EmitterTimeUntilDeletion",
                  dtDAL::FloatActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetStaticObjectsLifeTime),
                  dtDAL::FloatActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetStaticObjectsLifeTime),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("ParticleFadeInTime", "ParticleFadeInTime",
                  dtDAL::FloatActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetParticleFadeInTime),
                  dtDAL::FloatActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetParticleFadeInTime),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("ObjectsStayStaticWhenHit", "ObjectsStayStaticWhenHit",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetObjectsStayStaticWhenHit),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetObjectsStayStaticWhenHit),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::FloatActorProperty("ParticleFadeOutInverseDeletion", "ParticleFadeOutInverseDeletion",
                  dtDAL::FloatActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetParticleFadeOutInverseDeletion),
                  dtDAL::FloatActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetParticleFadeOutInverseDeletion),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::IntActorProperty("NumberOfParticlesWeWantSpawned", "NumberOfParticlesWeWantSpawned",
                  dtDAL::IntActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetNumberOfParticlesWeWantSpawned),
                  dtDAL::IntActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetNumberOfParticlesWeWantSpawned),
                  "", EMMITER_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("ThisAsAnInfiniteParticleSystem", "ThisAsAnInfiniteParticleSystem",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetThisAsAnInfiniteParticleSystem),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetThisAsAnInfiniteParticleSystem),
                  "", EMMITER_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("Does Particle System Delete Itself", "Does Particle System Delete Itself",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetObjectToStatic),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetObjectToStatic),
                  "Does Particle System Delete Itself", PARTICLE_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("GravityEnabledOnParticleSystem", "GravityEnabledOnParticleSystem",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetGravityEnabledOnParticleSystem),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetGravityEnabledOnParticleSystem),
                  "", EMMITER_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("ToApplyForcesToParticlesEveryFrame", "ToApplyForcesToParticlesEveryFrame",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetToApplyForcesToParticlesEveryFrame),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetToApplyForcesToParticlesEveryFrame),
                  "", EMMITER_GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("CollideWithSelf", "CollideWithSelf",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetCollideWithSelf),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetCollideWithSelf),
                  "", EMMITER_GROUP));

         /* AddProperty(new dtDAL::IntActorProperty("ParticleCollisionGroup", "ParticleCollisionGroup",
      dtDAL::IntActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetParticleCollisionGroup),
      dtDAL::IntActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetParticleCollisionGroup),
      "", EMMITER_GROUP));*/

         AddProperty(new dtDAL::Vec3ActorProperty("StartingPositionMin", "StartingPositionMin",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetStartingPositionMin),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetStartingPositionMin),
                  "", PARTICLE_GROUP ));

         AddProperty(new dtDAL::Vec3ActorProperty("StartingPositionMax", "StartingPositionMax",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetStartingPositionMax),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetStartingPositionMax),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("LinearVelocityStartMin", "LinearVelocityStartMin",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetLinearVelocityStartMin),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetLinearVelocityStartMin),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("LinearVelocityStartMax", "LinearVelocityStartMax",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetLinearVelocityStartMax),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetLinearVelocityStartMax),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("EmitterNoZoneEmitterCone", "EmitterNoZoneEmitterCone",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetEmitterNoZoneEmitteerCone),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetEmitterNoZoneEmitteerCone),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("AngularVelocityStartMin", "AngularVelocityStartMin",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetAngularVelocityStartMin),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetAngularVelocityStartMin),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("AngularVelocityStartMax", "AngularVelocityStartMax",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetAngularVelocityStartMax),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetAngularVelocityStartMax),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("OverTimeForceVecMin", "OverTimeForceVecMin",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetOverTimeForceVecMin),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetOverTimeForceVecMin),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::Vec3ActorProperty("OverTimeForceVecMax", "OverTimeForceVecMax",
                  dtDAL::Vec3ActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetOverTimeForceVecMax),
                  dtDAL::Vec3ActorProperty::GetFuncType(actor, &PhysicsParticleSystemActor::GetOverTimeForceVecMax),
                  "", PARTICLE_GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
                  "ObjectToUse1", "ObjectToUse1", dtDAL::ResourceActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetFileToLoadOne),
                  "The static mesh resource that defines the geometry", GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
                  "ObjectToUse2", "ObjectToUse2", dtDAL::ResourceActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetFileToLoadTwo),
                  "The static mesh resource that defines the geometry", GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
                  "ObjectToUse3", "ObjectToUse3", dtDAL::ResourceActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetFileToLoadThree),
                  "The static mesh resource that defines the geometry", GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
                  "ObjectToUse4", "ObjectToUse4", dtDAL::ResourceActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetFileToLoadFour),
                  "The static mesh resource that defines the geometry", GROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::STATIC_MESH,
                  "ObjectToUse5", "ObjectToUse5", dtDAL::ResourceActorProperty::SetFuncType(actor, &PhysicsParticleSystemActor::SetFileToLoadFive),
                  "The static mesh resource that defines the geometry", GROUP));
      }

      ////////////////////////////////////////////////////////////////////
      PhysicsParticleSystemActorProxy::~PhysicsParticleSystemActorProxy(){}
      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActorProxy::CreateDrawable()
      {
         SetDrawable(*new PhysicsParticleSystemActor(*this));
      }

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActorProxy::OnEnteredWorld()
      {
         dtGame::GameActorProxy::OnEnteredWorld();
         if (IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }
         else
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         }
      }

      ////////////////////////////////////////////////////////////////////
      void PhysicsParticleSystemActorProxy::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();
         if (!HasComponent(dtPhysics::PhysicsActComp::TYPE))
         {
            dtCore::RefPtr<dtPhysics::PhysicsActComp> physAC = new dtPhysics::PhysicsActComp();
            physAC->SetDefaultCollisionGroup(SimCore::CollisionGroup::GROUP_PARTICLE);
            AddComponent(*physAC);
         }
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

      //////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::ActorProperty> PhysicsParticleSystemActorProxy::GetDeprecatedProperty(const std::string& name)
      {
         dtCore::RefPtr<dtDAL::ActorProperty> depProp = BaseClass::GetDeprecatedProperty(name);

         if (!depProp.valid())
         {
            PhysicsParticleSystemActor* actor = NULL;
            GetActor(actor);
            depProp = actor->GetPhysicsActComp().GetDeprecatedProperty(name);
         }
         return depProp;
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
      bool PhysicsParticle::IsFlaggedToDelete()
      {
         return mNeedsToBeDeleted;
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
   }
}
