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



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // INNER CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::MultiSurfaceRuntimeData::MultiSurfaceRuntimeData(
         const dtGame::GroundClampingData& data )
         : BaseClass()
         , mClampingData(data)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::MultiSurfaceRuntimeData::~MultiSurfaceRuntimeData()
      {
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
         , mDefaultDomain(&SimCore::Actors::BaseEntityActorProxy::DomainEnum::GROUND)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MultiSurfaceClamper::~MultiSurfaceClamper()
      {
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
         const dtCore::Transform& xform,
         osg::Vec3 inOutPoints[3] )
      {
         using namespace SimCore::Actors;
         BaseEntityActorProxy::DomainEnum& domain = GetDomain( proxy );

         if( mSurfaceWater.valid() && IsWaterOnlyDomain( domain ) )
         {
            bool isSub = domain == BaseEntityActorProxy::DomainEnum::SUBMARINE;

            osg::Vec3 normal;

            osg::Vec3* curVec = &inOutPoints[0];
            GetWaterSurfaceHit( curVec->z(), *curVec, normal, ! isSub, isSub );

            curVec = &inOutPoints[1];
            GetWaterSurfaceHit( curVec->z(), *curVec, normal, ! isSub, isSub );

            curVec = &inOutPoints[2];
            GetWaterSurfaceHit( curVec->z(), *curVec, normal, ! isSub, isSub );
         }
         else
         {
            BaseClass::GetSurfacePoints( proxy, xform, inOutPoints );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool MultiSurfaceClamper::GetClosestHit(
         const dtDAL::TransformableActorProxy& proxy,
         dtCore::BatchIsector::SingleISector& single, float pointZ,
         osg::Vec3& inOutHit, osg::Vec3& outNormal )
      {
         using namespace SimCore::Actors;

         bool success = false;

         // Determine if the actor can be affected by water.
         BaseEntityActorProxy::DomainEnum& domain = GetDomain( proxy );
         bool isAffectedByWater = IsWaterDomain( domain );

         // Ground clamp if actor is not exclusive to water.
         if( ! IsWaterOnlyDomain( domain ) )
         {
            success = BaseClass::GetClosestHit( proxy, single, pointZ, inOutHit, outNormal );

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
         float pointZ, osg::Vec3& inOutHit, osg::Vec3& outNormal )
      {
         using namespace SimCore::Actors;

         bool success = false;

         // This method is being called because GetClosestHit has returned FALSE,
         // most likely because the involved Isector did not detect a hit.

         // Determine if the actor can be affected by water.
         BaseEntityActorProxy::DomainEnum& domain = GetDomain( proxy );
         if( IsWaterDomain( domain ) )
         {
            bool isSub = domain == BaseEntityActorProxy::DomainEnum::SUBMARINE;
            success = GetWaterSurfaceHit( pointZ, inOutHit, outNormal, 
               ! isSub, // Only force clamp surface vessels.
               isSub  );
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
               || surfaceHeight > objectHeight && ! clampUnderneath // Surface Clamp
               || surfaceHeight < objectHeight && clampUnderneath ) // Sub-surface Clamp
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

         // TODO: Update runtime data with final inertia.
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
         const dtDAL::TransformableActorProxy& proxy ) const
      {
         using namespace SimCore::Actors;
         const BaseEntity* entity = dynamic_cast<const BaseEntity*>(proxy.GetActor());

         BaseEntityActorProxy::DomainEnum* domain = mDefaultDomain;

         if( entity != NULL )
         {
             domain = &entity->GetDomain();
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
