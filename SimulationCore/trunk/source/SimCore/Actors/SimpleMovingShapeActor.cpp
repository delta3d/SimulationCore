/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
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
 *
 * David Guthrie
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/SimpleMovingShapeActor.h>
#include <dtGame/gameactor.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtDAL/propertymacros.h>
#include <dtUtil/mathdefines.h>
#include <iostream>
namespace SimCore
{

   namespace Actors
   {

      //////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::SimpleShapeDRHelper::SimpleShapeDRHelper()
         : mCurTimeDelta(0.0f)
      {

      }

      //////////////////////////////////////////////////////////////////////////
      bool SimpleMovingShapeActorProxy::SimpleShapeDRHelper::DoDR(dtGame::GameActor& gameActor, dtCore::Transform& xform, dtUtil::Log* pLogger, dtGame::BaseGroundClamper::GroundClampRangeType*& gcType)
      {
         BaseClass::DoDR(gameActor, xform, pLogger, gcType);
            
         bool useAcceleration = GetDeadReckoningAlgorithm() == dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION;

         if (IsUpdated())
         {
            //CalculateSmoothingTimes(xform);
          
            // If doing Cubic splines, we have to pre-compute some values
            if (GetUseCubicSplineTransBlend() && GetDeadReckoningAlgorithm() == dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION)
            {  // Use Accel
               mDRScale.RecomputeTransSplineValues(mDRScale.mAcceleration);
            }
            else if (GetUseCubicSplineTransBlend()) // No accel
            {
               osg::Vec3 zeroAccel;
               mDRScale.RecomputeTransSplineValues(zeroAccel);
            }
         }

         osg::Vec3 pos;
         mDRScale.DeadReckonThePosition(pos, pLogger, gameActor, 
            useAcceleration, mDRScale.mLastUpdatedTime, GetUseCubicSplineTransBlend());
         
         if (GetEffectiveUpdateMode(gameActor.IsRemote())
                  == DeadReckoningHelper::UpdateMode::CALCULATE_AND_MOVE_ACTOR)
         {
            //std::cout << "DIMS: " << pos << std::endl;
            SimpleMovingShapeActorProxy& s = static_cast<SimpleMovingShapeActorProxy&>(gameActor.GetGameActorProxy());
            s.SetCurrentDimensions(pos);
         }
         return true;
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::SimpleShapeDRHelper::CalculateSmoothingTimes( const dtCore::Transform& xform )
      {
         BaseClass::CalculateSmoothingTimes(xform);

         // TRANSLATION
         if (GetUseFixedSmoothingTime())
         {
            mDRScale.mEndSmoothingTime = GetFixedSmoothingTime();
         }
         else 
         {
            mDRScale.mEndSmoothingTime = GetMaxTranslationSmoothingTime();
            // Use our avg update time if it's smaller than our max
            if (GetMaxTranslationSmoothingTime() > mDRScale.mAvgTimeBetweenUpdates)
            {
               mDRScale.mEndSmoothingTime = mDRScale.mAvgTimeBetweenUpdates;
            }

            osg::Vec3 pos;
            xform.GetTranslation(pos);

            //Order of magnitude check - if the entity could not possibly get to the new position
            // in max smoothing time based on the magnitude of it's velocity, then smooth quicker (ie 1 second).
            if (mDRScale.mLastVelocity.length2() * (mDRScale.mEndSmoothingTime*mDRScale.mEndSmoothingTime) 
               < (mDRScale.mLastValue - pos).length2() )
            {
               mDRScale.mEndSmoothingTime = std::min(1.0f, mDRScale.mEndSmoothingTime);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::SimpleShapeDRHelper::IncrementTimeSinceUpdate( float simTimeDelta, float curSimulationTime )
      {
         BaseClass::IncrementTimeSinceUpdate(simTimeDelta, curSimulationTime);

         if(mDRScale.mUpdated)
         {
            mDRScale.SetLastUpdatedTime(curSimulationTime - simTimeDelta);
            mDRScale.mElapsedTimeSinceUpdate = 0.0f;
            mCurTimeDelta = 0.0f;
            mDRScale.mUpdated = false;
         }

         float transElapsedTime = mDRScale.mElapsedTimeSinceUpdate + simTimeDelta;
         if (transElapsedTime < 0.0) transElapsedTime = 0.0f;
         
         mCurTimeDelta = dtUtil::Max(transElapsedTime - mDRScale.mElapsedTimeSinceUpdate, 0.0f);
         mDRScale.mElapsedTimeSinceUpdate = transElapsedTime;
      }

      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString SimpleMovingShapeActorProxy::PROPERTY_LAST_KNOWN_DIMENSIONS("Last Known Dimension");
      const dtUtil::RefString SimpleMovingShapeActorProxy::PROPERTY_DIMENSIONS_VELOCITY_VECTOR("Dimension Velocity Vector");

      ////////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::SimpleMovingShapeActorProxy()
      : mOwner("")
      , mIndex(0)
      , mIsCreated(false)
      {
         SetClassName("SimCore::Actors::SimpleMovingShapeActorProxy");
      }

      ////////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::~SimpleMovingShapeActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::BuildActorComponents()
      {
         // DEAD RECKONING - ACT COMPONENT
         if (!HasComponent(dtGame::DeadReckoningHelper::TYPE)) // not added by a subclass
         {
            // TODO, use a subclassed dr actor comp that makes the shape dr too.
            mDRHelper = new SimpleShapeDRHelper();
            mDRHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
            mDRHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::NONE);
            AddComponent(*mDRHelper);
         }

         BaseClass::BuildActorComponents();
      }

      ////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(SimpleMovingShapeActorProxy, dtCore::UniqueId, Owner);
      DT_IMPLEMENT_ACCESSOR(SimpleMovingShapeActorProxy, int, Index);

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::OnEnteredWorld()
      {
         mShapeVolume = new SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord();
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::CreateActor()
      {
         SetActor(*new dtGame::GameActor(*this));
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUPNAME("SimpleMovingShape");

         typedef dtDAL::PropertyRegHelper<SimpleMovingShapeActorProxy&, SimpleMovingShapeActorProxy> PropRegHelperType;
         PropRegHelperType propRegHelper(*this, this, GROUPNAME);

         DT_REGISTER_PROPERTY_WITH_NAME(LastKnownDimension, PROPERTY_LAST_KNOWN_DIMENSIONS, 
            "Sets the last know radius of this shapes volume", PropRegHelperType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME(LastKnownDimensionVelocity, PROPERTY_DIMENSIONS_VELOCITY_VECTOR, 
            "Sets the last known dimension velocity vector of this shapes volume", PropRegHelperType, propRegHelper);

         DT_REGISTER_PROPERTY(Index,
                  "The index key of this moving shape, so if an actor is keeping track of many shapes, it can identify them by their index.",
                  PropRegHelperType, propRegHelper);

         DT_REGISTER_ACTOR_ID_PROPERTY("", Owner, "Owning actor",
                  "The actor that owns this actor.",
                  PropRegHelperType, propRegHelper);



      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::SetLastKnownDimension(const osg::Vec3& r)
      {
         mDRHelper->mDRScale.SetLastKnownTranslation(r);
      }

      ////////////////////////////////////////////////////////////////////////////
      const osg::Vec3& SimpleMovingShapeActorProxy::GetLastKnownDimension() const
      {
         return mDRHelper->mDRScale.mLastValue;
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::SetLastKnownDimensionVelocity( const osg::Vec3& r )
      {
         mDRHelper->mDRScale.SetLastKnownVelocity(r);
      }

      ////////////////////////////////////////////////////////////////////////////
      const osg::Vec3& SimpleMovingShapeActorProxy::GetLastKnownDimensionVelocity() const
      {
         return mDRHelper->mDRScale.mLastVelocity;
      }

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::SetCurrentDimensions( const osg::Vec3& dim )
      {
         mDimensions = dim;
         unsigned numParticles = ComputeNumParticles(mDimensions);

         if(mShapeVolume.valid())
         {
            SimCore::Components::VolumeRenderingComponent* vrc = NULL;
            GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 

            //std::cout << "Setting Current Dimensions: " << dim << std::endl;
            if(vrc != NULL)
            {

               if(!mIsCreated)
               {
                  mIsCreated = true;

                  mShapeVolume->mPosition.set(0.0f, 0.0f, 0.0f);
                  mShapeVolume->mColor.set(1.0f, 1.0f, 1.0f, 0.5f);
                  mShapeVolume->mShapeType = SimCore::Components::VolumeRenderingComponent::ELLIPSOID;
                  mShapeVolume->mRadius = mDimensions;
                  mShapeVolume->mNumParticles = numParticles;
                  mShapeVolume->mParticleRadius = 2.0f;
                  mShapeVolume->mVelocity = 0.05f;
                  //if there are fewer particles they should be more dense
                  mShapeVolume->mDensity = 0.005f + ((150 - mShapeVolume->mNumParticles) * 0.001f);
                  mShapeVolume->mTarget = GetGameActor().GetOSGNode();
                  mShapeVolume->mAutoDeleteOnTargetNull = true;
                  mShapeVolume->mRenderMode = SimCore::Components::VolumeRenderingComponent::PARTICLE_VOLUME;

                  vrc->CreateShapeVolume(mShapeVolume);
                  vrc->ComputeParticleRadius(*mShapeVolume);

                  std::cout << std::endl << "Creating new plume:" << std::endl;
                  std::cout << "radius: " << mShapeVolume->mRadius << std::endl;
                  std::cout << "num particles: " << mShapeVolume->mNumParticles << std::endl;
                  std::cout << "particle radius: " << mShapeVolume->mParticleRadius << std::endl;
                  std::cout << "density: " << mShapeVolume->mDensity << std::endl << std::endl;
               }
               else if(numParticles >= (1.5f * mShapeVolume->mNumParticles))
               {
                  //remove and re-create
                  vrc->RemoveShapeVolume(mShapeVolume.get());

                  mShapeVolume->mRadius = mDimensions;
                  mShapeVolume->mNumParticles = numParticles;
                  mShapeVolume->mDensity = 0.005f + ((150 - mShapeVolume->mNumParticles) * 0.001f);
                  mShapeVolume->mTarget = GetGameActor().GetOSGNode();
                  mShapeVolume->mAutoDeleteOnTargetNull = true;

                  vrc->CreateShapeVolume(mShapeVolume);
                  vrc->ComputeParticleRadius(*mShapeVolume);


                  std::cout << std::endl << "Modifying plume:" << std::endl;
                  std::cout << "radius: " << mShapeVolume->mRadius << std::endl;
                  std::cout << "num particles: " << mShapeVolume->mNumParticles << std::endl;
                  std::cout << "particle radius: " << mShapeVolume->mParticleRadius << std::endl;
                  std::cout << "density: " << mShapeVolume->mDensity << std::endl << std::endl;

               }
               else
               {
                  mShapeVolume->mRadius = mDimensions;
                  mShapeVolume->mDirtyParams = true;
                  vrc->ComputeParticleRadius(*mShapeVolume);
               }
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      const osg::Vec3& SimpleMovingShapeActorProxy::GetCurrentDimensions() const
      {
         return mDimensions;
      }

      ////////////////////////////////////////////////////////////////////////////
      unsigned SimpleMovingShapeActorProxy::ComputeNumParticles( const osg::Vec3& dims )
      {
         float totalSize = dims[0] * dims[1] * dims[2];
         unsigned numParticles = unsigned(totalSize / 25.0f);
         if(numParticles > 150) numParticles = 150;
         else if (numParticles < 15) numParticles = 15;
         return numParticles;
      }
   }

}
