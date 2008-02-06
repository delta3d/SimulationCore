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

#include <SimCore/Actors/NxAgeiaMunitionsPSysActor.h>

#include <NxAgeiaWorldComponent.h>

#include <SimCore/Actors/VolumetricLine.h>
#include <SimCore/Actors/NxAgeiaTerraPageLandActor.h>
#include <SimCore/Actors/TerrainActorProxy.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/WeaponActor.h>

#include <dtDAL/enginepropertytypes.h>

#include <dtGame/basemessages.h>

#include <dtCore/batchisector.h>
#include <dtCore/shadermanager.h>
#include <dtCore/uniqueid.h>
#include <dtCore/scene.h>
#include <dtCore/isector.h>
#include <dtCore/object.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/BlendFunc>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/PrimitiveSet>


//////////////////////////////////////////////////////////////////////////////////////////////////
//MunitionsPhysicsParticle
//////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////
// This class is used to prevent a collision with our selves.
//////////////////////////////////////////////////////////////////////////////////////////////////////
   class MunitionRaycastReport : public NxUserRaycastReport
   {
      public:
         /////////////////////////////////////////////////////////////////////////////////////////////
         MunitionRaycastReport(/*NxActor *actor, */dtCore::DeltaDrawable* ownerActor) : NxUserRaycastReport()
            //, mOurActor(actor)
            , mGotAHit(false)
            , mOwnerActor(ownerActor)
            , mClosestHitsHelper(NULL)
         {
         }

         /////////////////////////////////////////////////////////////////////////////////////////////
         virtual ~MunitionRaycastReport(){}

         /////////////////////////////////////////////////////////////////////////////////////////////
         virtual bool onHit(const NxRaycastHit& hit)
         {
            dtAgeiaPhysX::NxAgeiaPhysicsHelper* physicsHelper = 
               (dtAgeiaPhysX::NxAgeiaPhysicsHelper*)(hit.shape->getActor().userData);

            dtCore::DeltaDrawable *hitTarget = NULL;

            if(physicsHelper != NULL)
            {
               // null checked up above in the return
               hitTarget = physicsHelper->GetPhysicsGameActorProxy()->GetActor();
            }

            // We don't want to hit ourselves.  So, if we don't have a 'self' owner, then we take 
            // whatever hit we get.  Otherwise, we check the owner drawables
            if (mOwnerActor == NULL || hitTarget != mOwnerActor 
               // So we dont want to return false if collision is off, this onHit is called for
               // every hit along the line, and returning false tells it to stop the raycast
               // report, its amazing how rereading the sdk can help so much :(
               &&  hit.shape->getActor().readActorFlag(NX_AF_DISABLE_COLLISION) == false)
            {
               if (!mGotAHit || mClosestHit.distance > hit.distance)
               {
                  mClosestHitsHelper = physicsHelper;
                  mGotAHit = true;
                  mClosestHit = hit;
               }
            }

            return true;
         }	

      public:
         bool mGotAHit;
         NxRaycastHit mClosestHit;
         dtCore::DeltaDrawable *mOwnerActor;
         dtAgeiaPhysX::NxAgeiaPhysicsHelper* mClosestHitsHelper;
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
         mDynamicLight = new dtCore::Transformable();
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
NxAgeiaMunitionsPSysActor::NxAgeiaMunitionsPSysActor(dtGame::GameActorProxy &proxy) : NxAgeiaParticleSystemActor(proxy)
, mUseTracers(false)
, mCurrentTracerRoundNumber(0)
, mFrequencyOfTracers(10)
{
   mISector = new dtCore::Isector();
}

////////////////////////////////////////////////////////////////////
NxAgeiaMunitionsPSysActor::~NxAgeiaMunitionsPSysActor()
{
   ResetParticleSystem();
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::TickRemote(const dtGame::Message &tickMessage){}
////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::TickLocal(const dtGame::Message &tickMessage)
{
   float ElapsedTime = (float)static_cast<const dtGame::TickMessage&>(tickMessage).GetDeltaSimTime();

   std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
   for(;iter!= mOurParticleList.end();)
   {
      NxActor* ourActor = (*iter)->GetPhysicsActor();
      MunitionsPhysicsParticle* munitionsParticle = dynamic_cast<MunitionsPhysicsParticle*>((*iter).get());

      if( ourActor == NULL )
         continue;
      
      // CURT HACK FOR IPT 2 - DISABLED FORCE ON MUNITIONS
      /*if(false && mApplyForces)
      {
         ourActor->addForce(
            NxVec3(  GetRandBetweenTwoFloats(mForceVectorMax[0], mForceVectorMin[0]), 
                     GetRandBetweenTwoFloats(mForceVectorMax[1], mForceVectorMin[1]),
                     GetRandBetweenTwoFloats(mForceVectorMax[2], mForceVectorMin[2])));
      }*/

      (*iter)->UpdateTime(ElapsedTime);

      if((*iter)->ShouldBeRemoved())
      {
         //mPhysicsHelper->ReleasePhysXObject((*iter)->GetName().c_str());
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

      ResolveISectorCollision(*munitionsParticle);
   }
}

////////////////////////////////////////////////////////////////////
bool NxAgeiaMunitionsPSysActor::ResolveISectorCollision(MunitionsPhysicsParticle& particleToCheck)
{
   NxActor* ourActor = particleToCheck.GetPhysicsActor();
   if(ourActor != NULL && mWeapon.valid())
   {
      osg::Vec3 lastposition = particleToCheck.GetLastPosition();

      osg::Vec3 currentPosition = osg::Vec3( ourActor->getGlobalPosition()[0], 
                                             ourActor->getGlobalPosition()[1],
                                             ourActor->getGlobalPosition()[2]);

      if(currentPosition != lastposition)
      {
         
         NxRay ourRay;
         ourRay.orig = NxVec3(lastposition[0],lastposition[1],lastposition[2]);
         ourRay.dir = NxVec3(currentPosition[0] - lastposition[0], currentPosition[1] - lastposition[1],currentPosition[2] - lastposition[2]);
         float length = ourRay.dir.normalize();
         NxRaycastHit   mOurHit;

         // Drop a ray through the world to see what we hit. Make sure we don't hit ourselves.  And,
         // Make sure we DO hit hte terrain appropriatelys
         MunitionRaycastReport myReport((mWeapon.valid() ? mWeapon->GetOwner() : NULL));
         //NxShape* shape = ourActor->getScene().raycastClosestShape(ourRay, NX_ALL_SHAPES,  mOurHit, (1 << 0));
         NxU32 numHits = ourActor->getScene().raycastAllShapes(ourRay, myReport, NX_ALL_SHAPES, 
            (1 << 0) | (1 << 23) | (1 << 26) | (1 << 30) | (1 << 31) );
         if(numHits > 0 && myReport.mGotAHit)
         {
            if (myReport.mClosestHit.distance <= length)
            //if(mOurHit.distance < length)
            {
               particleToCheck.FlagToDelete();
               dtAgeiaPhysX::ContactReport report;
               report.nxVec3crContactNormal = myReport.mClosestHit.worldNormal; //;mOurHit.worldNormal;
               NxVec3 contactPoint(myReport.mClosestHit.worldImpact); //mOurHit.worldImpact);
               report.lsContactPoints.push_back(contactPoint);
               //dtAgeiaPhysX::NxAgeiaPhysicsHelper* physicsHelper = 
               //   (dtAgeiaPhysX::NxAgeiaPhysicsHelper*)(mOurHit.shape->getActor().userData);
               if(myReport.mClosestHitsHelper != NULL) //physicsHelper != NULL)
                  mWeapon->ReceiveContactReport( report, myReport.mClosestHitsHelper->GetPhysicsGameActorProxy());//physicsHelper->GetPhysicsGameActorProxy() );
               else
                  mWeapon->ReceiveContactReport( report, NULL);
               return true;
            }
         }
         // We didn't hit anything, but AGEIA doesn't hit the terrain yet.
         // So, do our own ISector against the terrain. This is SOOOO haxor! OMGWTFBBQ!
         // NOTE - If we didn't get a valid hit above and return, then we check the terrain
         //else 
         //{
            //std::vector<dtDAL::ActorProxy*> toFill;
            //GetGameActorProxy().GetGameManager()->FindActorsByName("Terrain", toFill);
            //dtDAL::ActorProxy* terrainNode = NULL;
            //if(!toFill.empty())
            //   terrainNode = (dynamic_cast<dtDAL::ActorProxy*>(&*toFill[0]));

            //osg::Vec3 hp;
            //dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
            //iSector->SetScene( &GetGameActorProxy().GetGameManager()->GetScene() );
            //iSector->SetQueryRoot(terrainNode->GetActor());
            //dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
            //SingleISector.SetSectorAsLineSegment(lastposition, currentPosition);
            //if( iSector->Update(osg::Vec3(0,0,0), true) )
            //{
            //   if( SingleISector.GetNumberOfHits() > 0 ) 
            //   {
            //      SingleISector.GetHitPoint(hp);
            //      particleToCheck.FlagToDelete();
            //      dtAgeiaPhysX::ContactReport report;
            //      osg::Vec3 hitNormal;
            //      SingleISector.GetHitPointNormal(hitNormal);
            //      report.nxVec3crContactNormal =NxVec3(hitNormal[0],hitNormal[1],hitNormal[2]);
            //      
            //      NxVec3 contactPoint(hp[0], hp[1], hp[2]);
            //      report.lsContactPoints.push_back(contactPoint);
            //      mWeapon->ReceiveContactReport( report, NULL); //terrainNode);
            //      return true;
            //   }
            //}
         //}         
      }

      particleToCheck.SetLastPosition( osg::Vec3(  ourActor->getGlobalPosition()[0], 
                                                   ourActor->getGlobalPosition()[1],
                                                   ourActor->getGlobalPosition()[2])) ;
   }
   return false;
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, const NxActor& whatWeHit)
{
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::RemoveParticle(PhysicsParticle& whichOne)
{
   NxAgeiaParticleSystemActor::RemoveParticle(whichOne);
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::Fire()
{
   AddParticle();
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::AddParticle()
{
   bool isTracer = GetSystemToUseTracers() && mCurrentTracerRoundNumber >= mFrequencyOfTracers;

   //we obtain the rendering support component so that the particle effect can add a dynamic light effect
   SimCore::Components::RenderingSupportComponent* renderComp = NULL;
   
   if( isTracer ) 
   {
      renderComp = dynamic_cast<SimCore::Components::RenderingSupportComponent*>
         (GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME));
   }


   dtCore::UniqueId _id;
   dtCore::RefPtr<MunitionsPhysicsParticle> _particle = new MunitionsPhysicsParticle(renderComp, _id.ToString(), mParticleLengthOfStay);

   dtCore::Transform ourTransform;
   GetTransform(ourTransform);
   
   _particle->SetLastPosition(ourTransform.GetTranslation());


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

   _particle->mObj = new dtCore::Object(_id.ToString().c_str());

   bool orientDrawable = false;

   // Determine if this system uses tracers.
   if( GetSystemToUseTracers() )
   {
      ++mCurrentTracerRoundNumber;
      if(isTracer)
      {
         mCurrentTracerRoundNumber = 0;
         osg::MatrixTransform* node = (_particle->mObj->GetMatrixNode());

         // Avoid adding another tracer geometry if this is a recycled particle.
         if( NULL != node && node->getNumChildren() == 0 )
         {
            dtCore::RefPtr<SimCore::Actors::VolumetricLine> line 
               = new SimCore::Actors::VolumetricLine( 20.0f, 0.5f, "VolumetricLines", "TracerGroup" );
            _particle->mObj->AddChild( line.get() );

            node = (line->GetMatrixNode());
            if( node != NULL )
            {
               node->setMatrix(ourRotationMatrix);
            }
         }
      }
      else
      {
         if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::TWOD)
         {
         }
         else if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::THREED)
         {
            LoadParticleResource(*_particle, mPathOfFileToLoad[0]);
            orientDrawable = true;
         }
      }
   }
   else
   {
      if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::TWOD)
      {
      }
      else if(GetTwoDOrThreeDTypeEnum() == TwoDOrThreeDTypeEnum::THREED)
      {
         LoadParticleResource(*_particle, mPathOfFileToLoad[0]);
         orientDrawable = true;
      }
   }

   if( orientDrawable )
   {
      osg::Group* g = _particle->mObj->GetOSGNode()->asGroup();
      osg::MatrixTransform* node = dynamic_cast<osg::MatrixTransform*>(g->getChild(0));
      if( node != NULL )
      {
         node->setMatrix(ourRotationMatrix);
      }
   }

   NxActor* newActor = NULL;
   //////////////////////////////////////////////////////////////////////////
   // Set up the physics values for the object
   if(mPhysicsHelper->GetPhysicsModelTypeEnum() == dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper::PhysicsModelTypeEnum::CUBE)
   {
      newActor = mPhysicsHelper->SetCollisionBox(NxVec3(ourTranslation[0], ourTranslation[1], ourTranslation[2]),
         dimensions, 
         mPhysicsHelper->GetDensity(), mPhysicsHelper->GetAgeiaMass(), collisionGroupToSendIn, mPhysicsHelper->GetSceneName(), _id.ToString().c_str(), true);
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
				 mPhysicsHelper->GetDensity(),mPhysicsHelper->GetAgeiaMass(), 
				 mPhysicsHelper->GetLoadAsCached(), mPathOfFileToLoad[0], 
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
                 mPhysicsHelper->GetLoadAsCached(), mPathOfFileToLoad[0], 
				 mPhysicsHelper->GetSceneName(), _id.ToString().c_str(), collisionGroupToSendIn);

      SetTransform(initialTransform);
   }
   
   //////////////////////////////////////////////////////////////////////////
   // Set up emitter values on the particle...
   
   osg::Vec4 linearVelocities;
   linearVelocities[0] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[0], mStartingLinearVelocityScaleMin[0]);
   linearVelocities[1] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[1], mStartingLinearVelocityScaleMin[1]);
   linearVelocities[2] = GetRandBetweenTwoFloats(mStartingLinearVelocityScaleMax[2], mStartingLinearVelocityScaleMin[2]);
   
   linearVelocities = ourRotationMatrix.preMult(linearVelocities);

   linearVelocities[0] += mParentsWorldRelativeVelocityVector[0];
   linearVelocities[1] += mParentsWorldRelativeVelocityVector[1];
   linearVelocities[2] += mParentsWorldRelativeVelocityVector[2];

   NxVec3 vRandVec(linearVelocities[0], linearVelocities[1], linearVelocities[2]);
   newActor->setLinearVelocity(vRandVec);

   vRandVec.set(  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[0], mStartingAngularVelocityScaleMin[0]), 
                  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[1], mStartingAngularVelocityScaleMin[1]), 
                  GetRandBetweenTwoFloats(mStartingAngularVelocityScaleMax[2], mStartingAngularVelocityScaleMin[2]));
   newActor->setAngularVelocity(vRandVec);
   if(!mGravityEnabled) newActor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
   
   GetGameActorProxy().GetGameManager()->GetScene().AddDrawable(_particle->mObj.get());

   ++mAmountOfParticlesThatHaveSpawnedTotal;

   mPhysicsHelper->SetAgeiaUserData(mPhysicsHelper.get(), _id.ToString().c_str());
   //newActor->userData = mPhysicsHelper.get();
   _particle->SetPhysicsActor(newActor);

   // add to our list for updating and such....
   mOurParticleList.push_back(_particle.get());
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::OnEnteredWorld()
{
   NxAgeiaParticleSystemActor::OnEnteredWorld();
   mISector = new dtCore::Isector();
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit)
{
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActor::AgeiaPostPhysicsUpdate()
{
   bool isATracer = false;
   std::list<dtCore::RefPtr<PhysicsParticle> >::iterator iter = mOurParticleList.begin();
   for(;iter!= mOurParticleList.end(); ++iter)
   {
      if((*iter)->ShouldBeRemoved() == false)
      {
         isATracer = false;
         MunitionsPhysicsParticle* munitionsParticle = dynamic_cast<MunitionsPhysicsParticle*>((*iter).get());
         if(munitionsParticle != NULL)
         {
            if(munitionsParticle->IsATracer())
            {
               isATracer = true;
            }
         }

         NxActor* physXActor = (*iter)->GetPhysicsActor();//mPhysicsHelper->GetPhysXObject((*iter)->GetName().c_str());
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

            if(isATracer)
            {
               osg::Matrix receiveMatrix = (*iter)->mObj->GetMatrixNode()->getMatrix();
               receiveMatrix.ptr()[12] = physXActor->getGlobalPosition()[0];
               receiveMatrix.ptr()[13] = physXActor->getGlobalPosition()[1];
               receiveMatrix.ptr()[14] = physXActor->getGlobalPosition()[2];
               (*iter)->mObj->GetMatrixNode()->setMatrix(receiveMatrix);
            }
            else
            {
               (*iter)->mObj->GetMatrixNode()->setMatrix(osg::Matrix(glmat));
            }
         }
      }
   }
}

////////////////////////////////////////////////////////////////////
// Actor Proxy Below here
////////////////////////////////////////////////////////////////////
NxAgeiaMunitionsPSysActorProxy::NxAgeiaMunitionsPSysActorProxy()
{
   SetClassName("NxAgeiaMunitionsPSysActor");
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActorProxy::BuildPropertyMap()
{
   const std::string GROUP = "NxAgeiaMunitionsPSysActor";

   NxAgeiaParticleSystemActorProxy::BuildPropertyMap();
   NxAgeiaMunitionsPSysActor &actor = static_cast<NxAgeiaMunitionsPSysActor&>(GetGameActor());

   AddProperty(new dtDAL::IntActorProperty("FrequencyOfTracers", "FrequencyOfTracers",
      dtDAL::MakeFunctor(actor, &NxAgeiaMunitionsPSysActor::SetFrequencyOfTracers),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaMunitionsPSysActor::GetFrequencyOfTracers),
      "", GROUP));

   AddProperty(new dtDAL::BooleanActorProperty("UseTracers", "UseTracers",
      dtDAL::MakeFunctor(actor, &NxAgeiaMunitionsPSysActor::SetSystemToUseTracers),
      dtDAL::MakeFunctorRet(actor, &NxAgeiaMunitionsPSysActor::GetSystemToUseTracers),
      "", GROUP));
}

////////////////////////////////////////////////////////////////////
NxAgeiaMunitionsPSysActorProxy::~NxAgeiaMunitionsPSysActorProxy(){}
////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActorProxy::CreateActor()
{
   SetActor(*new NxAgeiaMunitionsPSysActor(*this));
}

////////////////////////////////////////////////////////////////////
void NxAgeiaMunitionsPSysActorProxy::OnEnteredWorld()
{
   NxAgeiaParticleSystemActorProxy::OnEnteredWorld();
}
#endif
