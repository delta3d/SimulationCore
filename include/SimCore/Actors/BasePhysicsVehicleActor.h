/* -*-c++-*-
* Simulation Core
* Copyright 2008, Alion Science and Technology
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
* @author Curtiss Murphy
*/
#ifndef _BASE_PHYSICS_VEHICLE_
#define _BASE_PHYSICS_VEHICLE_

#include <SimCore/Export.h>

//#include <NxAgeiaFourWheelVehiclePhysicsHelper.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/VehicleInterface.h>
#include <SimCore/PhysicsTypes.h>
#ifdef AGEIA_PHYSICS
#include <NxAgeiaPhysicsHelper.h>
#else
#include <dtPhysics/physicshelper.h>
#endif

namespace dtAudio
{
   class Sound;
}

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////////
      /* This class provides the basic physics behavior for vehicles. It makes some assumptions
       * about the types of things a vehicle would need, but also allows you to override just about
       * everything. This class can be used to wildy divergent vehicles (from a formal 4 wheel
       * jeep to a hover craft to a 2 wheel bicycle).
       * This class is abstract. Makes no sense to have a base instantiation...
       */
      class SIMCORE_EXPORT BasePhysicsVehicleActor : public Platform,
#ifdef AGEIA_PHYSICS
      public dtAgeiaPhysX::NxAgeiaPhysicsInterface,
#endif
                                                       public VehicleInterface
      {
         public:
            typedef Platform BaseClass;

            /// Constructor
            BasePhysicsVehicleActor (PlatformActorProxy &proxy);

         protected:
            /// Destructor
            virtual ~BasePhysicsVehicleActor();

         // INHERITED PUBLIC
         public:
            /**
            * This method is an invokable called when an object is local and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

            /// Called once we decided to publish - does a quick calc on acceleration. 
            //virtual void SetLastKnownValuesBeforePublish(const osg::Vec3& pos, const osg::Vec3& rot);

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void OnTickRemote(const dtGame::TickMessage &tickMessage);

            /**
             * Called when the actor has been added to the game manager.
             * You can respond to OnEnteredWorld on either the proxy or actor or both.
             */
            virtual void OnEnteredWorld();

            /**
             * Overridden to unregister the helper when removed from the GM.
             */
            virtual void OnRemovedFromWorld();

#ifdef AGEIA_PHYSICS
            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            /// Does most of what is needed. Shouldn't need to override this behavior.
            virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport,
               dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit) {}

            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const dtPhysics::PhysicsObject& ourSelf,
               const dtPhysics::PhysicsObject& whatWeHit){}
#else
            /// dtPhysics post physics callback.
            virtual void PrePhysicsUpdate();
            virtual void PostPhysicsUpdate();

#endif

            /**
             * Handle forces received from the environment, such as detonations and impacts
             */
            virtual void ApplyForce(const osg::Vec3& force,
               const osg::Vec3& location = osg::Vec3(0.0f, 0.0f, 0.0f), bool isImpulse = false);

            /// Overridden so that it will flag the actor as being transformed when you set the position.
            virtual void SetTransform(const dtCore::Transform& xform, dtCore::Transformable::CoordSysEnum cs = dtCore::Transformable::ABS_CS);

            /// Utility Methods
            virtual float GetMPH();

            /// Tells the physics helper to reset teh vehicle to starting. Override if you need to do other stuff also.
            virtual void ResetVehicle();

            /// Turns it up and moves up
            virtual void RepositionVehicle(float deltaTime);

            //virtual bool ShouldForceUpdate( const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            void SetPhysicsHelper(dtPhysics::PhysicsHelper* newHelper);
            dtPhysics::PhysicsHelper* GetPhysicsHelper();

            //?? virtual void SetVehicleInsideModel(const std::string &value)  {VEHICLE_INSIDE_MODEL = value;}

            void SetHasDriver(bool hasDriver);
            bool GetHasDriver() const;

            void SetPerformAboveGroundSafetyCheck(bool enable);
            bool GetPerformAboveGroundSafetyCheck() const;

            //void SetSecsSinceLastUpdateSent(float secsSinceLastUpdateSent);
            //float GetSecsSinceLastUpdateSent() const;

            /**
             * Sets the max number of times per second an update may be sent if the dead reckoning tolerances
             * this it should be.
             */
            //void SetMaxUpdateSendRate(float maxUpdateSendRate);

            /**
             * @return the max number of times per second an update may be sent if the dead reckoning tolerances
             * this it should be.
             */
            //float GetMaxUpdateSendRate() const;

            //void SetVelocityMagnitudeUpdateThreshold(float);
            //float GetVelocityMagnitudeUpdateThreshold() const;
            //void SetVelocityDotProductUpdateThreshold(float);
            //float GetVelocityDotProductUpdateThreshold() const;

            ////void SetUseVelocityInDRUpdateDecision(bool);
            ////bool GetUseVelocityInDRUpdateDecision() const;

            /**
             * Computes and assigns the current velocity using a moving average.
             * @see SetVelocityAverageFrameCount
             */
            //virtual void ComputeCurrentVelocity(float deltaTime);

            /// Accum Acceleration is computed each frame inside ComputeCurrentVel. Override that if you want to set this.
            //void SetAccumulatedAcceleration(const osg::Vec3 &newValue) { mAccumulatedAcceleration = newValue; }
            //osg::Vec3 GetAccumulatedAcceleration() const { return mAccumulatedAcceleration; }

            /**
             * The current velocity is computed using a moving average of the
             * change in position over time.  The frame count passed in is used to
             * to decide about how many frames the velocity will be average across.
             */
            //void SetVelocityAverageFrameCount(int frames);

            /**
             * @see SetVelocityAverageFrameCount
             */
            //int GetVelocityAverageFrameCount() const;

         protected:
            /**
            * Set our DR values - velocity, acceleration, and angular velocity
            * Note - we MUST do this BEFORE we calculate new physics forces on our vehicle
            * The physics component runs at the start of the frame, so we set the values
            * on baseentity in case it needs to do a publish this frame. (see ShouldForceUpdate())
            * This is called from OnTickLocal() BEFORE the other update methods.
            * By default - handles most of the settings you need to keep your Dead Reckoning in sync.
            */
            //virtual void UpdateDeadReckoningValues(float deltaTime);

            /// Angles/ steering moving etc done here. From TickLocal - called second -
            /// after UpdateDeadReckoningValues(). This does nothing by default.
            virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

            /// Called update the dofs for your vehicle. Wheels or whatever. From TickLocal -
            /// called third - after UpdateVehicleTorquesAndAngles. The default does nothing.
            virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

            /// Do your vehicle, driving specific sounds. From TickLocal -
            /// called fourth - after UpdateRotationDOFS. Does nothing by default.
            virtual void UpdateSoundEffects(float deltaTime);

            /// Check if the actor is above ground.
            void KeepAboveGround();

            /**
            * Get the point on the PhysX terrain at the specified location.
            * @param location Location in world space where a ray should be used
            *        to find a PhysX terrain point.
            * @param outPoint Point on the terrain where a ray has detected terrain.
            * @return TRUE if terrain was detected.
            */
            bool GetTerrainPoint( const osg::Vec3& location, osg::Vec3& outPoint );

            /**
            * Checks for PhysX terrain and will turn on gravity if it is found.
            * @return TRUE if terrain is found.
            */
            bool IsTerrainPresent();

            /**
             * Sets the distance above the terrain to use when the physics terrain is found, and the vehicle is dropped
             */
            void SetTerrainPresentDropHeight(float zDistance);

            /**
             * @return the distance above the terrain to use when the physics terrain is found, and the vehicle is dropped
             */
            float GetTerrainHeightDropHeight() const;

            /// utility function for the UpdatedeadReckoning function
            float GetPercentageChangeDifference(float startValue, float newValue);

            bool GetPushTransformToPhysics() const;
            void SetPushTransformToPhysics(bool flag);
         // Private vars
         private:

            dtCore::RefPtr<dtPhysics::PhysicsHelper> mPhysicsHelper;


            ///////////////////////////////////////////////////
            // properties
            std::string VEHICLE_INSIDE_MODEL;      /// for interior views
            ///////////////////////////////////////////////////

            //osg::Vec3 mLastPos;
            //osg::Vec3 mAccumulatedLinearVelocity;
            //osg::Vec3 mAccumulatedAcceleration; 
            //osg::Vec3 mAccelerationCalculatedForLastPublish; // Used in SetLastKnownValuesBeforePublish()
            //int mVelocityAverageFrameCount;

            ///////////////////////////////////////////////////
            // sending out dead reckoning
            float mSecsSinceLastUpdateSent;
            float mMaxUpdateSendRate;
            float mVelocityMagThreshold;
            float mVelocityDotThreshold;
            float mInstantaneousVelocityWeight;

            float mTerrainPresentDropHeight;

            ///////////////////////////////////////////////////
            // is there currently a driver inside?
            bool mHasDriver : 1;

            ///////////////////////////////////////////////////
            // Was terrain currently found? Used for startup checks.
            bool mHasFoundTerrain : 1;

            /// Should the physics coll. det. fail, this will keep the vehicle above ground
            /// at the cost of some runtime performance.
            bool mPerformAboveGroundSafetyCheck : 1;

            /// When this is true, the position and rotation will be pushed to the physics engine on pre physics.
            bool mPushTransformToPhysics : 1;

            ////bool mUseVelocityInDRUpdateDecision: 1;

      };

      ////////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BasePhysicsVehicleActorProxy : public PlatformActorProxy
      {
         public:
            BasePhysicsVehicleActorProxy();
            virtual void BuildPropertyMap();

            virtual dtCore::RefPtr<dtDAL::ActorProperty> GetDeprecatedProperty(const std::string& name);

         protected:
            virtual ~BasePhysicsVehicleActorProxy();
            virtual void OnEnteredWorld();
      };

   }
}

#endif
