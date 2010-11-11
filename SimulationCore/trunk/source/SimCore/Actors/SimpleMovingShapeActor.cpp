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

namespace SimCore
{

   namespace Actors
   {

      //////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::SimpleShapeDRHelper::SimpleShapeDRHelper()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      bool SimpleMovingShapeActorProxy::SimpleShapeDRHelper::DoDR(dtGame::GameActor& gameActor, dtCore::Transform& xform, dtUtil::Log* pLogger, dtGame::BaseGroundClamper::GroundClampRangeType*& gcType)
      {
         BaseClass::DoDR(gameActor, xform, pLogger, gcType);
            
         bool useAcceleration = GetDeadReckoningAlgorithm() == dtGame::DeadReckoningAlgorithm::VELOCITY_AND_ACCELERATION;

         osg::Vec3 pos;
         mDRScale.DeadReckonThePosition(pos, pLogger, gameActor, 
            useAcceleration, 1.0f / 60.0f/*mCurTimeDelta*/, GetUseCubicSplineTransBlend());
         
         SimpleMovingShapeActorProxy& s = static_cast<SimpleMovingShapeActorProxy&>(gameActor.GetGameActorProxy());
         s.SetCurrentDimensions(pos);
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
      const dtUtil::RefString SimpleMovingShapeActorProxy::PROPERTY_LAST_KNOWN_DIMENSIONS("Last Known Dimension");
      const dtUtil::RefString SimpleMovingShapeActorProxy::PROPERTY_DIMENSIONS_VELOCITY_VECTOR("Dimension Velocity Vector");

      ////////////////////////////////////////////////////////////////////////////
      SimpleMovingShapeActorProxy::SimpleMovingShapeActorProxy()
      : mOwner("")
      , mIndex(0)
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
         dtGame::GameActor* ga = NULL;
         GetActor(ga);

         // DEAD RECKONING - ACT COMPONENT
         if (!ga->HasComponent(dtGame::DeadReckoningHelper::TYPE)) // not added by a subclass
         {
            // TODO, use a subclassed dr actor comp that makes the shape dr too.
            mDRHelper = new SimpleShapeDRHelper();
            mDRHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::VELOCITY_ONLY);
            mDRHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::NONE);
            ga->AddComponent(*mDRHelper);
         }

         BaseClass::BuildActorComponents();
      }

      ////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(SimpleMovingShapeActorProxy, dtCore::UniqueId, Owner);
      DT_IMPLEMENT_ACCESSOR(SimpleMovingShapeActorProxy, int, Index);

      ////////////////////////////////////////////////////////////////////////////
      void SimpleMovingShapeActorProxy::OnEnteredWorld()
      {
         ////add a shape volume for the beam
         SimCore::Components::VolumeRenderingComponent* vrc = NULL;
         GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 

         if(vrc != NULL)
         {
            mShapeVolume = new SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord();
            mShapeVolume->mPosition.set(0.0f, 0.0f, 0.0f);
            mShapeVolume->mColor.set(1.0f, 1.0f, 1.0f, 1.0f);
            mShapeVolume->mShapeType = SimCore::Components::VolumeRenderingComponent::ELLIPSOID;
            mShapeVolume->mRadius.set(1.0f, 1.0f, 1.0f);
            mShapeVolume->mNumParticles = 35;
            mShapeVolume->mParticleRadius = 5.0f;
            mShapeVolume->mVelocity = 0.15f;
            mShapeVolume->mDensity = 0.08f;
            mShapeVolume->mTarget = &GetGameActor();
            mShapeVolume->mAutoDeleteOnTargetNull = true;
            mShapeVolume->mRenderMode = SimCore::Components::VolumeRenderingComponent::PARTICLE_VOLUME;

            vrc->CreateShapeVolume(mShapeVolume);
         }

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

         if(mShapeVolume.valid())
         {
            mShapeVolume->mRadius = dim;
            mShapeVolume->mDirtyParams = true;

            SimCore::Components::VolumeRenderingComponent* vrc = NULL;
            GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 

            if(vrc != NULL)
            {
               vrc->ComputeParticleRadius(*mShapeVolume);
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      const osg::Vec3& SimpleMovingShapeActorProxy::GetCurrentDimensions() const
      {
         return mDimensions;
      }
   }

}
