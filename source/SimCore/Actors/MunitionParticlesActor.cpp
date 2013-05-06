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

#include <SimCore/Actors/MunitionParticlesActor.h>

#include <SimCore/Actors/VolumetricLine.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/WeaponActor.h>
#include <SimCore/CollisionGroupEnum.h>

#include <dtDAL/enginepropertytypes.h>

#include <dtGame/basemessages.h>

#include <dtCore/shadermanager.h>
#include <dtCore/uniqueid.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtUtil/mathdefines.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/BlendFunc>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/PrimitiveSet>

#include <dtPhysics/physicscomponent.h>
#include <dtPhysics/palutil.h>
#include <pal/palCollision.h>

using namespace SimCore::CollisionGroup;

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////////////////////////////
      //MunitionsPhysicsParticle
      //////////////////////////////////////////////////////////////////////////////////////////////////


      ///////////////////////////////////////////////////////////////////////////////////////////////////
      // This class is used to prevent self collision.
      //////////////////////////////////////////////////////////////////////////////////////////////////////
      class MunitionRaycastReport : public palRayHitCallback
      {
      public:
         /////////////////////////////////////////////////////////////////////////////////////////////
         MunitionRaycastReport(dtCore::DeltaDrawable* ownerActor)
         : mGotAHit(false)
         , mOwnerActor(ownerActor)
         , mClosestHitsObject(NULL)
         {
         }

         /////////////////////////////////////////////////////////////////////////////////////////////
         virtual ~MunitionRaycastReport(){}

         /////////////////////////////////////////////////////////////////////////////////////////////
         virtual Float AddHit(palRayHit& hit)
         {
            dtPhysics::PhysicsObject* physObject = reinterpret_cast<dtPhysics::PhysicsObject*>(hit.m_pBody->GetUserData());

            dtPhysics::PhysicsActComp* physicsHelper = NULL;
            if (physObject != NULL)
            {
               physicsHelper = dynamic_cast<dtPhysics::PhysicsActComp*>(physObject->GetUserData());
            }

            dtCore::DeltaDrawable* hitTarget = NULL;

            if(physicsHelper != NULL)
            {
               // null checked up above in the return
               physicsHelper->GetOwner(hitTarget);
            }

            // We don't want to hit ourselves.  So, if we don't have a 'self' owner, then we take
            // whatever hit we get.  Otherwise, we check the owner drawables
            if (mOwnerActor == NULL || hitTarget != mOwnerActor
                     // So we dont want to return false if collision is off, this onHit is called for
                     // every hit along the line, and returning false tells it to stop the raycast
                     // report, its amazing how rereading the sdk can help so much :(
                     &&  physObject->IsCollisionResponseEnabled())
            {
               if (!mGotAHit || mClosestHit.m_fDistance > hit.m_fDistance)
               {
                  mClosestHitsObject = physObject;
                  mGotAHit = true;
                  mClosestHit = hit;
               }
            }

            return mClosestHit.m_fDistance;
         }

      public:
         bool mGotAHit;
         palRayHit mClosestHit;
         dtCore::DeltaDrawable *mOwnerActor;
         dtPhysics::PhysicsObject* mClosestHitsObject;
      };

      MunitionsPhysicsParticle::MunitionsPhysicsParticle(SimCore::Components::RenderingSupportComponent* renderComp, const std::string& name, float ParticleLengthOfTimeOut, float InverseDeletionAlphaTime, float alphaInTime)
      : PhysicsParticle(name, ParticleLengthOfTimeOut, InverseDeletionAlphaTime, alphaInTime)
      , mIsTracer(renderComp!=NULL)
      , mLastPosition()
      , mDynamicLight()
      {
         if( mIsTracer )
         {
            SimCore::Components::RenderingSupportComponent::DynamicLight* dl =
               renderComp->AddDynamicLightByPrototypeName("Light-Tracer");
            //SimCore::Components::RenderingSupportComponent::DynamicLight* dl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
            //dl->mColor.set(1.0f, 0.2f, 0.2f);
            //dl->mAttenuation.set(0.1, 0.05, 0.0002);
            //dl->mIntensity = 1.0f;
            mDynamicLight = new dtCore::Transformable(name + " - Dynamic Light");
            dl->mTarget = mDynamicLight.get();
            //dl->mFlicker = true;
            //dl->mFlickerScale = 0.1f;
            //dl->mAutoDeleteLightOnTargetNull = true;
            //renderComp->AddDynamicLight(dl);
         }
      }

      MunitionsPhysicsParticle::~MunitionsPhysicsParticle()
      {
      }

      void MunitionsPhysicsParticle::SetLastPosition(const osg::Vec3& value)
      {
         mLastPosition = value;

         if( mIsTracer && mDynamicLight.valid() )
         {
            osg::Matrix mat;
            mat(3, 0) = value[0];
            mat(3, 1) = value[1];
            mat(3, 2) = value[2];
            mDynamicLight->GetMatrixNode()->setMatrix(mat);
         }
      }



      ////////////////////////////////////////////////////////////////////
      MunitionParticlesActor::MunitionParticlesActor(dtGame::GameActorProxy& proxy)
      : PhysicsParticleSystemActor(proxy)
      , mUseTracers(false)
      , mCurrentTracerRoundNumber(0)
      , mFrequencyOfTracers(10)
      {
      }

      ////////////////////////////////////////////////////////////////////
      MunitionParticlesActor::~MunitionParticlesActor()
      {
      }

      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         float ElapsedTime = tickMessage.GetDeltaSimTime();

         ParticleList::iterator iter = mOurParticleList.begin();
         for (;iter!= mOurParticleList.end();)
         {
            MunitionsPhysicsParticle* munitionsParticle = dynamic_cast<MunitionsPhysicsParticle*>((*iter).get());

   //         if (object == NULL)
   //            continue;

            (*iter)->UpdateTime(ElapsedTime);

            if ((*iter)->ShouldBeRemoved())
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

            ResolveISectorCollision(*munitionsParticle);
         }
      }
      ////////////////////////////////////////////////////////////////////
      bool MunitionParticlesActor::ResolveISectorCollision(MunitionsPhysicsParticle& particleToCheck)
      {
         dtPhysics::PhysicsObject* physicsObject = particleToCheck.GetPhysicsObject();
         if (physicsObject != NULL && mWeapon.valid())
         {
            osg::Vec3 lastPosition = particleToCheck.GetLastPosition();

            dtCore::Transform xform;
            physicsObject->GetTransform(xform);
            osg::Vec3 currentPosition;
            xform.GetTranslation(currentPosition);

            if (!dtUtil::Equivalent(currentPosition, lastPosition, 1e-5f))
            {
               dtPhysics::RayCast ray;

               ray.SetOrigin(lastPosition);
               osg::Vec3 dirVec = currentPosition - lastPosition;
               ray.SetDirection(dirVec);

               MunitionRaycastReport report(mWeapon.valid() ? mWeapon->GetOwner() : NULL);

               // CR: Create the bit mask once rather than every time the method is called.
               static const dtPhysics::CollisionGroupFilter GROUPS_FLAGS =
                  (1 << GROUP_TERRAIN)
                  | (1 << GROUP_WATER)
                  | (1 << GROUP_VEHICLE_GROUND)
                  | (1 << GROUP_VEHICLE_WATER)
                  | (1 << GROUP_HUMAN_LOCAL)
                  | (1 << GROUP_HUMAN_REMOTE);

               // TODO fix collision groups.
               ray.SetCollisionGroupFilter(GROUPS_FLAGS);

               dtPhysics::PhysicsWorld::GetInstance().TraceRay(ray, report);
               if (report.mGotAHit)
               {
                  particleToCheck.FlagToDelete();
                  dtPhysics::CollisionContact contactReport;
                  dtPhysics::PalVecToVectorType(contactReport.mNormal, report.mClosestHit.m_vHitNormal);
                  dtPhysics::PalVecToVectorType(contactReport.mPosition, report.mClosestHit.m_vHitPosition);
                  contactReport.mDistance = report.mClosestHit.m_fDistance;

                  dtPhysics::PhysicsActComp* physActComp = dynamic_cast<dtPhysics::PhysicsActComp*>(report.mClosestHitsObject->GetUserData());

                  if (report.mClosestHitsObject != NULL && physActComp != NULL)
                  {
                        dtGame::GameActor* ga = NULL;
                        physActComp->GetOwner(ga);

                        mWeapon->ReceiveContactReport(contactReport, &ga->GetGameActorProxy());
                  }
                  else
                  {
                     mWeapon->ReceiveContactReport(contactReport, NULL);
                  }

                  return true;
               }
            }

            physicsObject->GetTransform(xform);
            osg::Vec3 pos;
            xform.GetTranslation(pos);
            particleToCheck.SetLastPosition(pos);
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActor::Fire()
      {
         AddParticle();
      }

      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActor::AddParticle()
      {
         if (!GetGameActorProxy().IsInGM())
         {
            LOG_ERROR("Firing when the actor is being deleted.");
         }

         bool isTracer = GetSystemToUseTracers() && mCurrentTracerRoundNumber >= mFrequencyOfTracers;

         //we obtain the rendering support component so that the particle effect can add a dynamic light effect
         SimCore::Components::RenderingSupportComponent* renderComp = NULL;

         if (isTracer)
         {
            GetGameActorProxy().GetGameManager()->GetComponentByName(
                     SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                     renderComp);
         }


         dtCore::UniqueId id;
         dtCore::RefPtr<MunitionsPhysicsParticle> particle = new MunitionsPhysicsParticle(renderComp, id.ToString(), mParticleLengthOfStay);

         dtCore::Transform ourTransform;
         GetTransform(ourTransform);

         osg::Vec3 xyz;
         ourTransform.GetTranslation(xyz);

         particle->SetLastPosition(xyz);

         osg::Vec4 ourTranslation;
         ourTranslation[0] = xyz[0];
         ourTranslation[1] = xyz[1];
         ourTranslation[2] = xyz[2];
         osg::Matrix ourRotationMatrix;
         ourTransform.GetRotation(ourRotationMatrix);

         osg::Vec4 positionRandMax(mStartingPositionRandMax, 0.0);
         osg::Vec4 positionRandMin(mStartingPositionRandMin, 0.0);

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

         particle->mObj = new dtCore::Transformable(id.ToString().c_str());

         bool orientDrawable = false;

         // Determine if this system uses tracers.
         if( GetSystemToUseTracers() )
         {
            ++mCurrentTracerRoundNumber;
            if(isTracer)
            {
               mCurrentTracerRoundNumber = 0;
               osg::MatrixTransform* node = particle->mObj->GetMatrixNode();

               // Avoid adding another tracer geometry if this is a recycled particle.
               // NOTE: 1 child is for the model matrix node, used in preserving scale
               // but optimizing matrix transformations.
               if( NULL != node && node->getNumChildren() == 0 )
               {
                  dtCore::RefPtr<SimCore::Actors::VolumetricLine> line
                     = new SimCore::Actors::VolumetricLine( 10.0f, 0.5f, "VolumetricLines", "TracerGroup" );
                  particle->mObj->AddChild( line.get() );

                  orientDrawable = true;
                  //line->SetMatrix(ourRotationMatrix);
               }
            }
            else
            {
               if (GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::TWO_D)
               {
               }
               else if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::THREE_D)
               {
                  LoadParticleResource(*particle, mPathOfFileToLoad[0]);
                  orientDrawable = true;
               }
            }
         }
         else
         {
            if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::TWO_D)
            {
            }
            else if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::THREE_D)
            {
               LoadParticleResource(*particle, mPathOfFileToLoad[0]);
               orientDrawable = true;
            }
         }

         dtCore::Transform xform;
         xform.Set(ourRotationMatrix);
         xform.SetTranslation(osg::Vec3(ourTranslation.x(), ourTranslation.y(), ourTranslation.z()));

         dtPhysics::PhysicsObject* newActor = NULL;
         dtCore::RefPtr<dtPhysics::PhysicsObject> newObject = new dtPhysics::PhysicsObject(id.ToString());
         newObject->SetCollisionGroup(collisionGroupToSendIn);
         newObject->SetMechanicsType(dtPhysics::MechanicsType::DYNAMIC);
         newObject->SetMass(mPhysicsActComp->GetMass());
         newObject->SetPrimitiveType(mPhysicsActComp->GetDefaultPrimitiveType());
         newObject->SetExtents(mPhysicsActComp->GetDimensions());
         newObject->SetTransform(xform);
         newObject->SetNotifyCollisions(true);
         newObject->CreateFromProperties(particle->mObj->GetOSGNode());
         newActor = newObject.get();

         if( orientDrawable )
         {
            particle->mObj->SetTransform(xform);
         }
         //////////////////////////////////////////////////////////////////////////
         // Set up emitter values on the particle...

         osg::Vec3 linearVelocities;
         linearVelocities[0] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[0], mStartingLinearVelocityScaleMin[0]);
         linearVelocities[1] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[1], mStartingLinearVelocityScaleMin[1]);
         linearVelocities[2] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[2], mStartingLinearVelocityScaleMin[2]);

         linearVelocities = ourRotationMatrix.preMult(linearVelocities);

         linearVelocities[0] += mParentsWorldRelativeVelocityVector[0];
         linearVelocities[1] += mParentsWorldRelativeVelocityVector[1];
         linearVelocities[2] += mParentsWorldRelativeVelocityVector[2];

         osg::Vec3 vRandVec(linearVelocities);

         newActor->SetLinearVelocity(vRandVec);

         // DEBUG: std::cout << vRandVec << std::endl;

         vRandVec.set(  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[0], mStartingAngularVelocityScaleMin[0]),
                  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[1], mStartingAngularVelocityScaleMin[1]),
                  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[2], mStartingAngularVelocityScaleMin[2]));

         newActor->SetAngularVelocity(vRandVec);


         AddChild(particle->mObj.get());
         //GetGameActorProxy().GetGameManager()->GetScene().AddDrawable(particle->mObj.get());
         mPhysicsActComp->AddPhysicsObject(*newActor);

         //newActor->userData = mPhysicsActComp.get();
         particle->SetPhysicsObject(newActor);

         ++mAmountOfParticlesThatHaveSpawnedTotal;
         // add to our list for updating and such....
         mOurParticleList.push_back(particle.get());
      }


      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActor::PostPhysicsUpdate()
      {
         bool isATracer = false;
         std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
         for(;iter!= mOurParticleList.end(); ++iter)
         {
            PhysicsParticle& particle = **iter;
            if(!particle.ShouldBeRemoved())
            {
               isATracer = false;
               MunitionsPhysicsParticle* munitionsParticle = dynamic_cast<MunitionsPhysicsParticle*>(&particle);
               if (munitionsParticle != NULL)
               {
                  if(munitionsParticle->IsATracer())
                  {
                     isATracer = true;
                  }
               }

               dtPhysics::PhysicsObject* physicsObject = particle.GetPhysicsObject();
               if (physicsObject->IsActive())
               {
                  dtCore::Transform physicsXform;
                  physicsObject->GetTransform(physicsXform);

                  if (isATracer)
                  {
                     dtCore::Transform objXform;
                     osg::Vec3 pos;
                     physicsXform.GetTranslation(pos);
                     // make the tracer point along the direction of travel.
                     objXform.Set(pos, physicsObject->GetLinearVelocity() + pos, osg::Vec3(0.0f, 0.0f, 1.0f));
                     particle.mObj->SetTransform(objXform);
                  }
                  else
                  {
                     particle.mObj->SetTransform(physicsXform);
                  }
               }
            }
         }

      }

      ////////////////////////////////////////////////////////////////////
      // Actor Proxy Below here
      ////////////////////////////////////////////////////////////////////
      MunitionParticlesActorProxy::MunitionParticlesActorProxy()
      {
         SetClassName("MunitionParticlesActor");
      }

      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActorProxy::BuildPropertyMap()
      {
         const std::string GROUP = "MunitionParticlesActor";

         PhysicsParticleSystemActorProxy::BuildPropertyMap();
         MunitionParticlesActor* actor = NULL;
         GetActor(actor);

         AddProperty(new dtDAL::IntActorProperty("FrequencyOfTracers", "FrequencyOfTracers",
                  dtDAL::IntActorProperty::SetFuncType(actor, &MunitionParticlesActor::SetFrequencyOfTracers),
                  dtDAL::IntActorProperty::GetFuncType(actor, &MunitionParticlesActor::GetFrequencyOfTracers),
                  "", GROUP));

         AddProperty(new dtDAL::BooleanActorProperty("UseTracers", "UseTracers",
                  dtDAL::BooleanActorProperty::SetFuncType(actor, &MunitionParticlesActor::SetSystemToUseTracers),
                  dtDAL::BooleanActorProperty::GetFuncType(actor, &MunitionParticlesActor::GetSystemToUseTracers),
                  "", GROUP));
      }

      ////////////////////////////////////////////////////////////////////
      MunitionParticlesActorProxy::~MunitionParticlesActorProxy(){}

      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActorProxy::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();

         dtPhysics::PhysicsActComp* physAC = NULL;
         GetComponent(physAC);
         if (physAC != NULL)
         {
            physAC->SetDefaultCollisionGroup(SimCore::CollisionGroup::GROUP_BULLET);
         }
      }

      ////////////////////////////////////////////////////////////////////
      void MunitionParticlesActorProxy::CreateActor()
      {
         SetActor(*new MunitionParticlesActor(*this));
      }
   }
}
