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

#ifndef MULTI_SURFACE_CLAMPER_H
#define MULTI_SURFACE_CLAMPER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/MatrixTransform>
#include <dtGame/defaultgroundclamper.h>
#include <SimCore/Actors/BaseWaterActor.h>
#include <SimCore/Actors/BaseEntity.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class BatchIsector::SingleISector;
   class Transform;
}

namespace dtCore
{
   class TransformableActorProxy;
}



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT SurfacePointData
      {
         public:
            SurfacePointData();
            virtual ~SurfacePointData();

            void SetPointSolid( bool pointIsSolid );
            bool IsPointSolid() const;

            void SetVelocity( float velocity );
            float GetVelocity() const;

            void SetLastClampPoint( const osg::Vec3& clampPoint );
            const osg::Vec3& GetLastClampPoint() const;

            osg::MatrixTransform* GetDrawable();
            const osg::MatrixTransform* GetDrawable() const;

            bool AddDrawableToScene( osg::Group& scene );
            bool RemoveDrawableFromScene( osg::Group& scene );

         private:
            bool mSolid;
            float mVelocity;
            osg::Vec3 mClampLastKnown;

            dtCore::RefPtr<osg::MatrixTransform> mDrawable;
      };
      typedef std::vector<SurfacePointData> SurfacePointDataArray;



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MultiSurfaceClamper : public dtGame::DefaultGroundClamper
      {
         public:

            typedef dtGame::DefaultGroundClamper BaseClass;



            ////////////////////////////////////////////////////////////////////
            // INNER CLASS CODE
            ////////////////////////////////////////////////////////////////////
            class SIMCORE_EXPORT MultiSurfaceRuntimeData : public dtGame::DefaultGroundClamper::RuntimeData
            {
               public:
                  typedef dtGame::DefaultGroundClamper::RuntimeData BaseClass;

                  static const float DEFAULT_MAX_TIME_STEP;
                  static const float DEFAULT_POINT_MASS;
                  static const float DEFAULT_POINT_RADIUS;
                  static const float MINIMUM_MAX_TIME_STEP;
                  static const float MINIMUM_POINT_MASS;
                  static const float MINIMUM_POINT_RADIUS;

                  /**
                   * Constructor
                   * @param data Back-reference to the parent Ground Clamping Data.
                   */
                  MultiSurfaceRuntimeData( const dtGame::GroundClampingData& data );

                  /**
                   * Set/get the entity associated with this data.
                   */
                  void SetEntity( SimCore::Actors::BaseEntity* entity );
                  SimCore::Actors::BaseEntity* GetEntity();
                  const SimCore::Actors::BaseEntity* GetEntity() const;

                  /**
                   * Get the associated object's clamping domain.
                   * @return NULL if no entity was assigned.
                   */
                  SimCore::Actors::BaseEntityActorProxy::DomainEnum* GetDomain() const;

                  /**
                   * Set/get the metric radius for clamp points, used for liquid buoyancy.
                   */
                  void SetPointRadius( float radius );
                  float GetPointRadius() const;

                  /**
                   * Set/get the average mass for clamp points, used for liquid buoyancy.
                   */
                  void SetPointMass( float mass );
                  float GetPointMass() const;

                  /**
                   * Set/get the maximum time step to apply to buoyancy calculations.
                   */
                  void SetMaxTimeStep( float seconds );
                  float GetMaxTimeStep() const;

                  /**
                   * Get data pertaining to clamp points, used to track changes and modify
                   * final clamping points (for point oscillation) on a surface, usually that of liquid.
                   */
                  SurfacePointDataArray& GetSurfacePointData();
                  const SurfacePointDataArray& GetSurfacePointData() const;

                  // HACK:
                  /**
                   * @return Reference back to the parent Clamping Data, which is
                   * otherwise difficult to obtain efficiently per clamped object.
                   */
                  const dtGame::GroundClampingData& GetClampingData() const;

                  void SetSceneNode( osg::Group* sceneNode );
                  osg::Group* GetSceneNode();
                  const osg::Group* GetSceneNode() const;

                  void SetDebugMode( bool debugMode );
                  bool GetDebugMode() const;

               protected:
                  virtual ~MultiSurfaceRuntimeData();

               private:

                  bool mDebugMode;
                  float mRadius;
                  float mMass;
                  float mMaxTimeStep;

                  // Data about the last clamping points.
                  SurfacePointDataArray mPointData;

                  // HACK:
                  // Back-reference to the parent Clamping Data.
                  const dtGame::GroundClampingData& mClampingData;

                  // Entity associated with this data.
                  dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mEntity;
                  dtCore::ObserverPtr<osg::Group> mScene;
            };

            MultiSurfaceClamper();

            /**
             * Get the simulation time from the last call to ClampToGround.
             */
            double GetCurrentSimTime() const;

            /**
             * Override this method to make special clamp type determinations on the type of object.
             * This method allows a sub-class clamper to change the clamp type for optimization
             * based on factors such as velocity, transformation change and special-case objects.
             * @param suggestedClampType Clamp type sent into ClampToGround for the specified object.
             * @param proxy Actor to be clamped and is passed in case collision geometry is needed.
             * @param data Ground Clamping Data containing clamping options and runtime data.
             * @param tranformChanged Flag to make a better determination for clamping.
             * @param velocity Instantaneous velocity of the object for the current frame.
             * @return INTERMITTENT if velocity is zero and the transform has not changed; otherwise
             *         suggestedCalmpType is returned.
             */
            virtual dtGame::BaseGroundClamper::GroundClampRangeType& GetBestClampType(
               dtGame::BaseGroundClamper::GroundClampRangeType& suggestedClampType,
               const dtCore::TransformableActorProxy& proxy,
               const dtGame::GroundClampingData& data,
               bool transformChanged, const osg::Vec3& velocity) const;

            /**
             * Clamps an actor to the ground.  It will pick, based on the type and eye point 
             * which algorithm to Use.
             * @param type Ground clamping type to perform.
             * @param currentTime Current simulation time. Used for intermittent ground clamping.
             * @param xform Current absolute transform of the actor.
             * @param proxy Actor to be clamped and is passed in case collision geometry is needed.
             * @param data Ground Clamping Data containing clamping options.
             * @param transformChanged Flag to help the clamper to determine if it should perform a clamp or not.
             * @param velocity The transformable's instantaneous velocity for the current frame.
             */
            virtual void ClampToGround( GroundClampRangeType& type, double currentTime, dtCore::Transform& xform,
               dtCore::TransformableActorProxy& proxy, dtGame::GroundClampingData& data,
               bool transformChanged = false, const osg::Vec3& velocity = osg::Vec3());

            /**
             * Set the default domain to use if an object does not have a domain
             * property associated with it.
             */
            void SetDefaultDomainClamping( SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain );
            SimCore::Actors::BaseEntityActorProxy::DomainEnum& GetDefaultDomainClamping() const;

            /**
             * Override method to extend surface point detection for 3-point clamping
             * Calculate the surface points from the specified location and detection points.
             * NOTE: This override will subsequently call GetWaterSurfaceHit if the object's
             * domain includes water.
             * @param proxy Actor that owns the specified transform.
             * @param data Ground Clamping Data associated with the proxy.
             * @param xform Transform that has the location and rotation in world space.
             * @param inOutPoints IN: detection points relative to the transform.
             *                    OUT: resulting surface points.
             */
            virtual void GetSurfacePoints( const dtCore::TransformableActorProxy& proxy,
               dtGame::GroundClampingData& data, const dtCore::Transform& xform,
               osg::Vec3 inOutPoints[3] );

            /**
             * Override method to extend surface point detection for single point clamping.
             * Gets the ground clamping hit that is closest to the Z value.
             * NOTE: This override will subsequently call GetWaterSurfaceHit if the object's
             * domain includes water.
             * @param proxy Actor proxy that is to be clamped. This parameter may help
             *              sub-classes of Ground Clamper make better determinations.
             * @param data Ground Clamping Data associated with the proxy.
             * @param single Isector containing points to be checked.
             * @param pointZ Z value to compare against the Isector points.
             * @param inOutHit Point to capture the values of the point that is the closest match.
             *                 IN: Point containing the object's current X & Y position.
             *                 OUT: Final surface hit point.
             * @param outNormal Normal to capture values of the matched hit point's normal.
             * @return TRUE if a hit point was detected, otherwise FALSE if there are no points
             *         contained in the specified Isector.
             */
            virtual bool GetClosestHit( const dtCore::TransformableActorProxy& proxy,
               dtGame::GroundClampingData& data, 
               dtCore::BatchIsector::SingleISector& single, float pointZ,
               osg::Vec3& inOutHit, osg::Vec3& outNormal );

            /**
             * Get a hit from the water surface.
             * @param objectHeight Height in meters where the object is currently at.
             * @param inOutHit Point containing the object's current X & Y position
             *        but will be used to capture the final surface hit point.
             * @param outNormal The surface normal of the surface hit.
             * @param forceClamp Flag determining if the point should be force clamped
             *        to the surface and not deviate from it.
             * @param clampUnderneath Flag determining clamping should happen from
             *        underneath, such as the case with a submersible.
             * @return TRUE if a point was successfully calculated.
             */
            virtual bool GetWaterSurfaceHit( float objectHeight, osg::Vec3& inOutHit,
               osg::Vec3& outNormal, bool forceClamp = false, bool clampUnderneath = false );
            
            /**
             * Method to handle any surface point modification after the surface points have been
             * detected. Override this method to extend or change the point modification procedures.
             * This is handy for modifying points so that the terrain appears to be soft, like sand,
             * mud, or water.
             * @param proxy Actor proxy that is being clamped. This is passed in case a sub-class of
             *              ground clamper needs to make any determinations based on the actor.
             * @param data Ground clamping data that may have some data fields modified. This may
             *             be used to maintain inertial data per clamp point or rotation velocity.
             * @param inOutPoints IN: Surface points that MAY be modified.
             *                    OUT: Points modified to their final positions.
             */
            virtual void FinalizeSurfacePoints( dtCore::TransformableActorProxy& proxy,
               dtGame::GroundClampingData& data, osg::Vec3 inOutPoints[3] );

            /**
             * Get the domain of the generic transformable object.
             * @param data Data associated with the clamped proxy that has a reference to
             *        the proxy and thus its domain property.
             * @return Valid domain of the object; the set Default Domain will be returned
             *         if the object does not have an accessible domain-related property.
             */
            SimCore::Actors::BaseEntityActorProxy::DomainEnum& GetDomain(
               const dtGame::GroundClampingData& data ) const;

            /**
             * Determine if the specified domain includes ground; most domains should because of gravity.
             */
            bool IsGroundDomain( SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain ) const;

            /**
             * Determine if the specified domain includes water, such as for various amphibious vehicles.
             */
            bool IsWaterDomain( SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain ) const;

            /**
             * Determine if the specified domain is water only, such as for ships, boats, and submersibles.
             */
            bool IsWaterOnlyDomain( SimCore::Actors::BaseEntityActorProxy::DomainEnum& domain ) const;

            /**
             * Set the water surface to be detected when trying to clamp water craft.
             */
            void SetWaterSurface( SimCore::Actors::BaseWaterActor* water );
            SimCore::Actors::BaseWaterActor* GetWaterSurface();
            const SimCore::Actors::BaseWaterActor* GetWaterSurface() const;

            /**
             * Determine if the Ground Clamper or any sub-class has a valid surface to clamp to.
             * NOTE: This will allow the base implementation of Default Ground Clamper to determine
             * if it can continue with clamping without worry; such as in the case where no terrain
             * exists but a water surface does, like the ocean.
             */
            virtual bool HasValidSurface() const;

            /**
             * Convenience method for updating points needing to be clamped to a water surface.
             * This method updates the buoyancy effect for such points.
             */
            void UpdatePointBuoyancy_Simple( MultiSurfaceRuntimeData& inOutData, osg::Vec3 inOutPoints[3] );

         protected:
            virtual ~MultiSurfaceClamper();

            /**
             * Get or create a Runtime Data for the specified Ground Clamp Data.
             * Any existing data that is NOT a Runtime Data instance will be replaced
             * with a new Runtime Data and an error will be logged.
             * Override this method if a sub-class of Ground Clamper uses an extended
             * type of RuntimeData.
             * @data Ground Clamping Data to have a Runtime Data reference accessed;
             *       also created if one does not exist on the specified data.
             */
            virtual MultiSurfaceRuntimeData& GetOrCreateRuntimeData( dtGame::GroundClampingData& data );

         private:

            double mCurrentSimTime;
            SimCore::Actors::BaseEntityActorProxy::DomainEnum* mDefaultDomain;
            dtCore::ObserverPtr<SimCore::Actors::BaseWaterActor> mSurfaceWater;
      };
   }

}

#endif
