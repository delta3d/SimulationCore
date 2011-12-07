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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/MultiSurfaceClamper.h>
#include <dtCore/batchisector.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtCore/transformable.h>
#include <dtDAL/actortype.h>
#include <dtDAL/transformableactorproxy.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/log.h>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Array>

#include <sstream>

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
         if( mDrawable.valid() )
         {
            osg::Matrix mtx( mDrawable->getMatrix() );
            mtx.setTrans( clampPoint );
            mDrawable->setMatrix( mtx );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& SurfacePointData::GetLastClampPoint() const
      {
         return mClampLastKnown;
      }

      //////////////////////////////////////////////////////////////////////////
      osg::MatrixTransform* SurfacePointData::GetDrawable()
      {
         return mDrawable.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::MatrixTransform* SurfacePointData::GetDrawable() const
      {
         return mDrawable.get();
      }

      //////////////////////////////////////////////////////////////////////////
      bool SurfacePointData::AddDrawableToScene( osg::Group& scene )
      {
         bool success = false;
         if( ! mDrawable.valid() )
         {
            // COLOR - used for other data: UVs , line thickness and length
            dtCore::RefPtr<osg::Vec4Array> color = new osg::Vec4Array( 6 );
            (*color)[0].set( 1.0f,  1.0f,  0.0f, 1.0f );
            (*color)[1].set( 1.0f,  0.0f,  0.0f, 1.0f );
            (*color)[2].set( 1.0f,  1.0f,  1.0f, 1.0f );
            (*color)[3].set( 1.0f,  1.0f,  1.0f, 1.0f );
            (*color)[4].set( 1.0f,  1.0f,  1.0f, 1.0f );
            (*color)[5].set( 1.0f,  1.0f,  1.0f, 1.0f );

            osg::Vec3 start( 0.0f, 0.0f, 0.0f );
            osg::Vec3 end( 0.0f, 0.0f, 1.0f );

            // VERTICES
            dtCore::RefPtr<osg::Vec3Array> verts = new osg::Vec3Array( 6 );
            (*verts)[0].set( start );
            (*verts)[1].set( end );
            (*verts)[2].set( start + osg::Vec3( 0.25f, 0.0f,  0.0f) );
            (*verts)[3].set( start + osg::Vec3(-0.25f, 0.0f,  0.0f) );
            (*verts)[4].set( start + osg::Vec3( 0.0f,  0.25f, 0.0f) );
            (*verts)[5].set( start + osg::Vec3( 0.0f, -0.25f, 0.0f) );

            dtCore::RefPtr<osg::StateSet> states = new osg::StateSet();
            states->setMode(GL_BLEND,osg::StateAttribute::ON);
            states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

            dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry;
            geom->setColorArray( color.get() ); // use colors for uvs and other parameters
            geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
            geom->setVertexArray( verts.get() );
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, verts->size()));

            dtCore::RefPtr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable( geom.get() );
            geode->setStateSet( states.get() );

            mDrawable = new osg::MatrixTransform;
            mDrawable->addChild( geode.get() );
            success = scene.addChild( mDrawable.get() );
         }
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool SurfacePointData::RemoveDrawableFromScene( osg::Group& scene )
      {
         bool success = false;
         if( mDrawable.valid() )
         {
            success = scene.removeChild( mDrawable.get() );
            mDrawable = NULL;
         }
         return success;
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
         , mDebugMode(false)
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
         SetSceneNode(NULL);
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
      SurfacePointDataArray&
         MultiSurfaceClamper::MultiSurfaceRuntimeData::GetSurfacePointData()
      {
         return mPointData;
      }

      //////////////////////////////////////////////////////////////////////////
      const SurfacePointDataArray&
         MultiSurfaceClamper::MultiSurfaceRuntimeData::GetSurfacePointData() const
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
      void MultiSurfaceClamper::MultiSurfaceRuntimeData::SetSceneNode( osg::Group* sceneNode )
      {
         if( mScene.valid() && sceneNode == NULL )
         {
            mPointData[0].RemoveDrawableFromScene( *mScene );
            mPointData[1].RemoveDrawableFromScene( *mScene );
            mPointData[2].RemoveDrawableFromScene( *mScene );
         }

         mScene = sceneNode;

         if( mScene.valid() )
         {
            // This will force the point drawables to be added to the current scene.
            SetDebugMode( GetDebugMode() );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Group* MultiSurfaceClamper::MultiSurfaceRuntimeData::GetSceneNode()
      {
         return mScene.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Group* MultiSurfaceClamper::MultiSurfaceRuntimeData::GetSceneNode() const
      {
         return mScene.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::MultiSurfaceRuntimeData::SetDebugMode( bool debugMode )
      {
         mDebugMode = debugMode;

         if( mScene.valid() )
         {
            if( debugMode )
            {
               mPointData[0].AddDrawableToScene( *mScene );
               mPointData[1].AddDrawableToScene( *mScene );
               mPointData[2].AddDrawableToScene( *mScene );
            }
            else
            {
               mPointData[0].RemoveDrawableFromScene( *mScene );
               mPointData[1].RemoveDrawableFromScene( *mScene );
               mPointData[2].RemoveDrawableFromScene( *mScene );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::MultiSurfaceRuntimeData::GetDebugMode() const
      {
         return mDebugMode;
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
      dtGame::BaseGroundClamper::GroundClampRangeType& MultiSurfaceClamper::GetBestClampType(
         dtGame::BaseGroundClamper::GroundClampRangeType& suggestedClampType,
         const dtDAL::TransformableActorProxy& proxy,
         const dtGame::GroundClampingData& data,
         bool transformChanged, const osg::Vec3& velocity) const
      {
         dtGame::BaseGroundClamper::GroundClampRangeType* clampType = &suggestedClampType;

         if (suggestedClampType != GroundClampRangeType::NONE)
         {
            // Make sure all surface vessels always 3-point clamp to moving water
            // even though they may not be moving on their own power.
            if (IsWaterDomain(GetDomain( data )))
            {
               clampType = &dtGame::BaseGroundClamper::GroundClampRangeType::RANGED;
            }
            // NOTE: Animation component does not specify a velocity but does set
            // transformChanged flag to TRUE. Checking the flag will allow the animated
            // characters clamp as expected.
            else if (!transformChanged && velocity.length2() == 0.0f)
            {
               clampType = &dtGame::BaseGroundClamper::GroundClampRangeType::INTERMITTENT_SAVE_OFFSET;
            }
         }

         return *clampType;
      }

      //////////////////////////////////////////////////////////////////////////
      void MultiSurfaceClamper::ClampToGround(GroundClampRangeType& type,
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

               // DEBUG: Use drawables to show the clamp points.
               //runtimeData.SetDebugMode( true );
               //runtimeData.SetSceneNode( entityProxy->GetGameManager()->GetScene().GetSceneNode() );
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
         BaseEntityActorProxy::DomainEnum& domain = GetDomain(data);

         // Do the clamping...
         if (mSurfaceWater.valid() && IsWaterOnlyDomain( domain ))
         {
            bool isSub = domain == BaseEntityActorProxy::DomainEnum::SUBMARINE;

            osg::Vec3 normal; // This will just satisfy the method call for now.
            osg::Vec3 pos;
            xform.GetTranslation( pos );

            GetWaterSurfaceHit(pos.z(), inOutPoints[0], normal, ! isSub, isSub);
            GetWaterSurfaceHit(pos.z(), inOutPoints[1], normal, ! isSub, isSub);
            GetWaterSurfaceHit(pos.z(), inOutPoints[2], normal, ! isSub, isSub);
         }
         else
         {
            BaseClass::GetSurfacePoints(proxy, data, xform, inOutPoints);
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
         // Get the simulation time delta.
         float simTimeDelta = GetCurrentSimTime() - inOutData.GetLastClampedTime();
         if(simTimeDelta >= MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_MAX_TIME_STEP)
         {
            simTimeDelta = MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_MAX_TIME_STEP;
         }

         // Calculate the size of the vessel.
         {
            osg::Vec3 rearCenter(inOutPoints[1] + inOutPoints[2]);
            rearCenter *= 0.5;
            osg::Vec3 lengthVec(inOutPoints[0] - rearCenter);
            osg::Vec3 widthVec(inOutPoints[1] - inOutPoints[2]);

            float sizeFactor = (widthVec.length() * lengthVec.length())/500.0f;
            dtUtil::Clamp(sizeFactor, 0.001f, /*MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_MAX_TIME_STEP, */1.0f);
            simTimeDelta /= sizeFactor;
            dtUtil::Clamp(simTimeDelta, 0.001f, /*MultiSurfaceClamper::MultiSurfaceRuntimeData::DEFAULT_MAX_TIME_STEP, */1.0f);
         }


         // Get the surface points and their data.
         SurfacePointDataArray& pointData = inOutData.GetSurfacePointData();
         inOutPoints[0].z() = pointData[0].GetLastClampPoint().z() + ((inOutPoints[0].z() - pointData[0].GetLastClampPoint().z()) * simTimeDelta);// * simTimeDelta;
         inOutPoints[1].z() = pointData[1].GetLastClampPoint().z() + ((inOutPoints[1].z() - pointData[1].GetLastClampPoint().z()) * simTimeDelta);// * simTimeDelta;
         inOutPoints[2].z() = pointData[2].GetLastClampPoint().z() + ((inOutPoints[2].z() - pointData[2].GetLastClampPoint().z()) * simTimeDelta);// * simTimeDelta;

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
