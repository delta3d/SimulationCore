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
 *
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Components/MultiSurfaceClamper.h>
#include <dtCore/batchisector.h>
#include <dtCore/transform.h>
#include <dtCore/transformable.h>
#include <dtDAL/actortype.h>
#include <dtDAL/transformableactorproxy.h>
#include <dtUtil/mathdefines.h>



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      SurfacePointData::SurfacePointData()
         : mSolid(true) 
         , mVelocity(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SurfacePointData::~SurfacePointData()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void SurfacePointData::SetPointSolid( bool pointIsSolid )
      {
         mSolid = pointIsSolid;
      }

      //////////////////////////////////////////////////////////////////////////
      bool SurfacePointData::IsPointSolid() const
      {
         return mSolid;
      }

      //////////////////////////////////////////////////////////////////////////
      void SurfacePointData::SetVelocity( float velocity )
      {
         mVelocity = velocity;
      }

      //////////////////////////////////////////////////////////////////////////
      float SurfacePointData::GetVelocity() const
      {
         return mVelocity;
      }

      //////////////////////////////////////////////////////////////////////////
      void SurfacePointData::SetLastClampPoint( const osg::Vec3& clampPoint )
      {
         mClampLastKnown = clampPoint;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& SurfacePointData::GetLastClampPoint() const
      {
         return mClampLastKnown;
      }



      //////////////////////////////////////////////////////////////////////////
      // INNER CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      const float MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_MAX_TIME_STEP = 1.0f/30.0f;
      const float MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_POINT_MASS = 1.0f;
      const float MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_POINT_RADIUS = 0.5f;
      const float MultiSurfaceClamper::MultiSurfaceRuntimeData::MINIMUM_MAX_TIME_STEP = 0.001f;
      const float MultiSurfaceClamper::MultiSurfaceRuntimeData::MINIMUM_POINT_MASS = 0.001f;
      const float MultiSurfaceClamper::MultiSurfaceRuntimeData::MINIMUM_POINT_RADIUS = 0.001f;

      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::MultiSurfaceRuntimeData::MultiSurfaceRuntimeData(
         const dtGame::GroundClampingData& data )
         : BaseClass()
         , mRadius(MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_POINT_RADIUS)
         , mMass(MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_POINT_MASS)
         , mMaxTimeStep(MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_MAX_TIME_STEP)
         , mClampingData(data)
      {
         SurfacePointData pointData;
         mPointData.push_back(pointData);
         mPointData.push_back(pointData);
         mPointData.push_back(pointData);
      }

      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::MultiSurfaceRuntimeData::~MultiSurfaceRuntimeData()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::MultiSurfaceRuntimeData::SetEntity( SimCore::Actors::BaseEntity* entity )
      {
         mEntity = entity;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseEntity*
         MultiSurfaceClamper::MultiSurfaceRuntimeData::GetEntity()
      {
         return mEntity.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::BaseEntity*
         MultiSurfaceClamper::MultiSurfaceRuntimeData::GetEntity() const
      {
         return mEntity.get();
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseEntityActorProxy::DomainEnum*
         MultiSurfaceClamper::MultiSurfaceRuntimeData::GetDomain() const
      {
         return mEntity.valid() ? &mEntity->GetDomain() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::MultiSurfaceRuntimeData::SetPointRadius( float radius )
      {
         mRadius = radius <= 0.0f
            ? MultiSurfaceClamper::MultiSurfaceRuntimeData::MINIMUM_POINT_RADIUS : radius;
      }

      //////////////////////////////////////////////////////////////////////////
      float MultiSurfaceClamper::MultiSurfaceRuntimeData::GetPointRadius() const
      {
         return mRadius;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::MultiSurfaceRuntimeData::SetPointMass( float mass )
      {
         mMass = mass <= 0.0f
            ? MultiSurfaceClamper::MultiSurfaceRuntimeData::MINIMUM_POINT_MASS : mass;
      }

      //////////////////////////////////////////////////////////////////////////
      float MultiSurfaceClamper::MultiSurfaceRuntimeData::GetPointMass() const
      {
         return mMass;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::MultiSurfaceRuntimeData::SetMaxTimeStep( float seconds )
      {
         mMaxTimeStep = seconds <= 0.0f
            ? MultiSurfaceClamper::MultiSurfaceRuntimeData::MINIMUM_MAX_TIME_STEP : seconds;
      }

      //////////////////////////////////////////////////////////////////////////
      float MultiSurfaceClamper::MultiSurfaceRuntimeData::GetMaxTimeStep() const
      {
         return mMaxTimeStep;
      }

      //////////////////////////////////////////////////////////////////////////
      std::vector<SurfacePointData>& MultiSurfaceClamper::MultiSurfaceRuntimeData::GetSurfacePointData()
      {
         return mPointData;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::vector<SurfacePointData>& MultiSurfaceClamper::MultiSurfaceRuntimeData::GetSurfacePointData() const
      {
         return mPointData;
      }

      //////////////////////////////////////////////////////////////////////////
      const dtGame::GroundClampingData&
         MultiSurfaceClamper::MultiSurfaceRuntimeData::GetClampingData() const
      {
         return mClampingData;
      }



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::MultiSurfaceClamper()
         : BaseClass()
         , mCurrentSimTime(0.0)
         , mDefaultDomain(&SimCore::Actors::BaseEntityActorProxy::DomainEnum::GROUND)
      {
         SetIntermittentGroundClampingTimeDelta(2.0f);
      }

      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::~MultiSurfaceClamper()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      double MultiSurfaceClamper::GetCurrentSimTime() const
      {
         return mCurrentSimTime;
      }

      /////////////////////////////////////////////////////////////////////////////
      dtGame::BaseGroundClamper::GroundClampingType& MultiSurfaceClamper::GetBestClampType(
         dtGame::BaseGroundClamper::GroundClampingType& suggestedClampType,
         const dtDAL::TransformableActorProxy& proxy,
         const dtGame::GroundClampingData& data,
         bool transformChanged, const osg::Vec3& velocity) const
      {
         dtGame::BaseGroundClamper::GroundClampingType* clampType = &suggestedClampType;

         // Make sure all surface vessels always 3-point clamp to moving water
         // even though they may not be moving on their own power.
         if( IsWaterDomain( GetDomain( data ) ) )
         {
            clampType = &dtGame::BaseGroundClamper::GroundClampingType::RANGED;
         }
         // NOTE: Animation component does not specify a velocity but does set
         // transformChanged flag to TRUE. Checking the flag will allow the animated
         // characters clamp as expected.
         else if( ! transformChanged && velocity.length2() == 0.0f )
         {
            clampType = &dtGame::BaseGroundClamper::GroundClampingType::INTERMITTENT_SAVE_OFFSET;
         }

         return *clampType;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::ClampToGround( GroundClampingType& type,
         double currentTime, dtCore::Transform& xform,
         dtDAL::TransformableActorProxy& proxy, dtGame::GroundClampingData& data,
         bool transformChanged, const osg::Vec3& velocity)
      {
         // Maintain the current simulation time across subsequent methods.
         mCurrentSimTime = currentTime;

         // Setup runtime data for clamping operations for the current proxy.
         MultiSurfaceRuntimeData& runtimeData = GetOrCreateRuntimeData( data );

         // Determine if the proxy is an Entity and if so, cast it and maintain
         // a reference to it for optimization reasons.
         if( runtimeData.GetEntity() == NULL )
         {
            SimCore::Actors::BaseEntityActorProxy* entityProxy
               = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>(&proxy);
            if( entityProxy != NULL )
            {
               SimCore::Actors::BaseEntity* entity = NULL;
               entityProxy->GetActor( entity );
               runtimeData.SetEntity( entity );
            }
         }

         // Continue with regular clamping operations.
         BaseClass::ClampToGround(type, currentTime, xform, proxy, data,
            transformChanged, velocity);
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::SetDefaultDomainClamping(
         SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain )
      {
         mDefaultDomain = &domain;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseEntityActorProxy::DomainEnum&
         MultiSurfaceClamper::GetDefaultDomainClamping() const
      {
         return *mDefaultDomain;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::GetSurfacePoints(
         const dtDAL::TransformableActorProxy& proxy,
         dtGame::GroundClampingData& data,
         const dtCore::Transform& xform,
         osg::Vec3 inOutPoints[3] )
      {
         using namespace SimCore::Actors;
         BaseEntityActorProxy::DomainEnum& domain = GetDomain( data );

         if( mSurfaceWater.valid() && IsWaterOnlyDomain( domain ) )
         {
            bool isSub = domain == BaseEntityActorProxy::DomainEnum::SUBMARINE;

            osg::Vec3 normal; // This will just satisfy the method call for now.
            osg::Vec3 pos;
            xform.GetTranslation( pos );

            GetWaterSurfaceHit( pos.z(), inOutPoints[0], normal, ! isSub, isSub );
            GetWaterSurfaceHit( pos.z(), inOutPoints[1], normal, ! isSub, isSub );
            GetWaterSurfaceHit( pos.z(), inOutPoints[2], normal, ! isSub, isSub );
         }
         else
         {
            BaseClass::GetSurfacePoints( proxy, data, xform, inOutPoints );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::GetClosestHit(
         const dtDAL::TransformableActorProxy& proxy,
         dtGame::GroundClampingData& data,
         dtCore::BatchIsector::SingleISector& single, float pointZ,
         osg::Vec3& inOutHit, osg::Vec3& outNormal )
      {
         using namespace SimCore::Actors;

         bool success = false;

         // Determine if the actor can be affected by water.
         BaseEntityActorProxy::DomainEnum& domain = GetDomain( data );
         bool isAffectedByWater = IsWaterDomain( domain );

         // Ground clamp if actor is not exclusive to water.
         if( ! isAffectedByWater || ! IsWaterOnlyDomain( domain ) )
         {
            success = BaseClass::GetClosestHit( proxy, data, single, pointZ, inOutHit, outNormal );

            // If a ground hit was successful...
            if( success )
            {
               // Obtain its current height so it can be checked with water if needed.
               pointZ = inOutHit.z();
            }
         }

         // Only clamp to water if the actor can be affected by water.
         // NOTE: A valid ground point may have been detected prior to this.
         if( isAffectedByWater )
         {
            bool isSub = domain == BaseEntityActorProxy::DomainEnum::SUBMARINE;
            success = GetWaterSurfaceHit( pointZ, inOutHit, outNormal,
               ! success && ! isSub, // Only force clamp surface vessels.
               isSub );
         }

         return success;
      }
      
      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::GetMissingHit( const dtDAL::TransformableActorProxy& proxy,
         dtGame::GroundClampingData& data, float pointZ, osg::Vec3& inOutHit, osg::Vec3& outNormal )
      {
         using namespace SimCore::Actors;

         bool success = false;

         // This method is being called because GetClosestHit has returned FALSE,
         // most likely because the involved Isector did not detect a hit.

         // Determine if the actor can be affected by water.
         BaseEntityActorProxy::DomainEnum& domain = GetDomain( data );
         if( IsWaterDomain( domain ) )
         {
            bool isSub = domain == BaseEntityActorProxy::DomainEnum::SUBMARINE;
            success = GetWaterSurfaceHit( pointZ, inOutHit, outNormal, 
               ! isSub, // Only force clamp surface vessels.
               isSub );
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::GetWaterSurfaceHit( float objectHeight,
         osg::Vec3& inOutHit, osg::Vec3& outNormal, bool forceClamp, bool clampUnderneath )
      {
         bool success = false;

         if( mSurfaceWater.valid() )
         {
            // There is a water surface and it should return a procedurally valid point.
            success = true;

            // Set default values.
            float surfaceHeight = mSurfaceWater->GetWaterHeight();
            outNormal.set( 0.0f, 0.0f, 1.0f );

            // Set the X & Y of the detection point.
            /*dtCore::Transform xform;
            const dtGame::GameActor* actor = NULL;
            proxy.GetActor( actor );
            actor->GetTransform( xform, dtCore::Transformable::REL_CS );
            xform.GetTranslation( outHit );*/ // This does not seem to have a valid position at this point in the process.

            // NOTE: outHit SHOULD have the X & Y location of the object already.

            // Get the water height and normal.
            mSurfaceWater->GetHeightAndNormalAtPoint( inOutHit, surfaceHeight, outNormal );

            // Clamp to water only if it is above the Z height.
            if( forceClamp
               || ( surfaceHeight > objectHeight && ! clampUnderneath ) // Surface Clamp
               || ( surfaceHeight < objectHeight && clampUnderneath ) ) // Sub-surface Clamp
            {
               // Set the final hit point at the detected height.
               inOutHit.z() = surfaceHeight;
            }
            else
            {
               inOutHit.z() = objectHeight;
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::FinalizeSurfacePoints( dtDAL::TransformableActorProxy& proxy,
         dtGame::GroundClampingData& data, osg::Vec3 inOutPoints[3] )
      {
         // Get the runtime data to be updated.
         MultiSurfaceRuntimeData* runtimeData = dynamic_cast<MultiSurfaceRuntimeData*>(data.GetUserData());

         // NOTE: Runtime data should exist by this point.
         if( runtimeData == NULL )
         {
            if( GetLogger().IsLevelEnabled(dtUtil::Log::LOG_WARNING) )
            {
               // Create warning log message.
               std::ostringstream oss;
               oss << proxy.GetActorType().GetName() << " (" << proxy.GetId().ToString()
                  << ") does not have associated Multi Surface Runtime Data. Cannot finalize surface points."
                  << std::endl;

               // Log the warning.
               GetLogger().LogMessage( dtUtil::Log::LOG_WARNING,
                  __FUNCTION__, __LINE__, oss.str() );
            }
            return;
         }

         // TODO: Modify points with inertia factored in.
         SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain = GetDomain( data );
         if( IsWaterDomain( domain ) )
         {
            UpdatePointBuoyancy_Simple( *runtimeData, inOutPoints );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::UpdatePointBuoyancy_Simple(
         MultiSurfaceRuntimeData& inOutData, osg::Vec3 inOutPoints[3] )
      {
         float timeStep = GetCurrentSimTime() - inOutData.GetLastClampedTime();

         // Get the surface points and their data.
         std::vector<SurfacePointData>& pointData = inOutData.GetSurfacePointData();
         inOutPoints[0].z() += (inOutPoints[0].z() - pointData[0].GetLastClampPoint().z()) * timeStep;
         inOutPoints[1].z() += (inOutPoints[1].z() - pointData[1].GetLastClampPoint().z()) * timeStep;
         inOutPoints[2].z() += (inOutPoints[2].z() - pointData[2].GetLastClampPoint().z()) * timeStep;

         pointData[0].SetLastClampPoint( inOutPoints[0] );
         pointData[1].SetLastClampPoint( inOutPoints[1] );
         pointData[2].SetLastClampPoint( inOutPoints[2] );
      }

      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::MultiSurfaceRuntimeData& MultiSurfaceClamper::GetOrCreateRuntimeData(
         dtGame::GroundClampingData& data )
      {
         MultiSurfaceRuntimeData* runtimeData = dynamic_cast<MultiSurfaceRuntimeData*>(data.GetUserData());
         if( runtimeData == NULL )
         {
            if( data.GetUserData() != NULL )
            {
               std::ostringstream oss;
               oss << "Ground Clamping Data user data is being replaced by a new MultiSurfaceRuntime Data.";
               GetLogger().LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__, oss.str() );
            }

            data.SetUserData( new MultiSurfaceRuntimeData(data) );
            runtimeData = static_cast<MultiSurfaceRuntimeData*>(data.GetUserData());
         }

         return *runtimeData;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseEntityActorProxy::DomainEnum& MultiSurfaceClamper::GetDomain(
         const dtGame::GroundClampingData& data ) const
      {
         // Cast the runtime data directly to th expected sub-class.
         // This clamper object should have already changed the user data
         // to Multi Surface Runtime Data prior to getting to this method.
         const MultiSurfaceRuntimeData* runtimeData = static_cast<const MultiSurfaceRuntimeData*>(data.GetUserData());
         SimCore::Actors::BaseEntityActorProxy::DomainEnum* domain = mDefaultDomain;

         if( runtimeData != NULL )
         {
            domain = runtimeData->GetDomain();
         }

         return *domain;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::IsGroundDomain(
         SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain ) const
      {
         return ! IsWaterOnlyDomain( domain );
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::IsWaterDomain(
         SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain ) const
      {
         return IsWaterOnlyDomain( domain )
            || domain == SimCore::Actors::BaseEntityActorProxy::DomainEnum::AMPHIBIOUS
            || domain == SimCore::Actors::BaseEntityActorProxy::DomainEnum::MULTI;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::IsWaterOnlyDomain(
         SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain ) const
      {
         return domain == SimCore::Actors::BaseEntityActorProxy::DomainEnum::SURFACE
            || domain == SimCore::Actors::BaseEntityActorProxy::DomainEnum::SUBMARINE;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::SetWaterSurface( SimCore::Actors::BaseWaterActor* water )
      {
         mSurfaceWater = water;
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Actors::BaseWaterActor* MultiSurfaceClamper::GetWaterSurface()
      {
         return mSurfaceWater.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::BaseWaterActor* MultiSurfaceClamper::GetWaterSurface() const
      {
         return mSurfaceWater.get();
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::HasValidSurface() const
      {
         return GetTerrainActor() != NULL || GetWaterSurface() != NULL;
      }
   }

}
