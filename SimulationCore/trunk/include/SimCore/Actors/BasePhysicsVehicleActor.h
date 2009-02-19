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

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void OnTickRemote(const dtGame::TickMessage &tickMessage);

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

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
            void PrePhysicsUpdate();
            void PostPhysicsUpdate();

#endif

            /**
             * Handle forces received from the environment, such as detonations and impacts
             * Future - use the location as the source of the force impulse
             */
            virtual void ApplyForce(const osg::Vec3& force, const osg::Vec3& location);

            /**
             * Handle forces received from the environment, such as detonations and impacts
             */
            virtual void ApplyForce(const osg::Vec3& force);

            /// Overridden so that it will flag the actor as being transformed when you set the position.
            virtual void SetTransform(dtCore::Transform& xform, dtCore::Transformable::CoordSysEnum cs = dtCore::Transformable::ABS_CS);

            /// Utility Methods
            virtual float GetMPH();

            /// Tells the physics helper to reset teh vehicle to starting. Override if you need to do other stuff also.
            virtual void ResetVehicle();

            /// Turns it up and moves up
            virtual void RepositionVehicle(float deltaTime);

            virtual bool ShouldForceUpdate( const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            void SetPhysicsHelper(dtPhysics::PhysicsHelper* newHelper) {mPhysicsHelper = newHelper;}
            dtPhysics::PhysicsHelper* GetPhysicsHelper() {return mPhysicsHelper.get();}

            //?? virtual void SetVehicleInsideModel(const std::string &value)  {VEHICLE_INSIDE_MODEL = value;}

            void SetHasDriver( bool hasDriver )           { mHasDriver = hasDriver; }
            bool GetHasDriver() const       { return mHasDriver; }

            void SetPerformAboveGroundSafetyCheck( bool enable )  { mPerformAboveGroundSafetyCheck = enable; }
            bool GetPerformAboveGroundSafetyCheck() const  { return mPerformAboveGroundSafetyCheck;}

            virtual void SetTimeForSendingDeadReckoningInfoOut( float timeForSendingDeadReckoningInfoOut )           { mTimeForSendingDeadReckoningInfoOut = timeForSendingDeadReckoningInfoOut; }
            virtual float GetTimeForSendingDeadReckoningInfoOut() const       { return mTimeForSendingDeadReckoningInfoOut; }

            virtual void SetTimesASecondYouCanSendOutAnUpdate( float timesASecondYouCanSendOutAnUpdate )           { mTimesASecondYouCanSendOutAnUpdate = timesASecondYouCanSendOutAnUpdate; }
            virtual float GetTimesASecondYouCanSendOutAnUpdate() const       { return mTimesASecondYouCanSendOutAnUpdate; }

            void SetPublishLinearVelocity(bool publishLinearVelocity)   { mPublishLinearVelocity = publishLinearVelocity; }
            bool GetPublishLinearVelocity() const { return mPublishLinearVelocity; }

            void SetPublishAngularVelocity(bool publishAngularVelocity)   { mPublishAngularVelocity = publishAngularVelocity; }
            bool GetPublishAngularVelocity() const { return mPublishAngularVelocity; }


         protected:
            /// Angles/ steering moving etc done here. Of the updates, this is called first.
            /// This does nothing by default.
            virtual void UpdateVehicleTorquesAndAngles(float deltaTime);

            /// Called update the dofs for your vehicle. Wheels or whatever. Of the updates, this is called second
            /// By default, this does nothing.
            virtual void UpdateRotationDOFS(float deltaTime, bool insideVehicle);

            /// called from tick. Do your sounds. Of the updates, this is called third.
            /// Does nothing by default.
            virtual void UpdateSoundEffects(float deltaTime);

            /// called from tick, does local dead reckon & determines to send out updates. Of the updates, this is called last
            /// By default, does most of the work you need to do for dead reckoning and sending out actor updates based on position.
            virtual void UpdateDeadReckoning(float deltaTime);

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

            ///////////////////////////////////////////////////
            // sending out dead reckoning
            float mTimeForSendingDeadReckoningInfoOut;
            float mTimesASecondYouCanSendOutAnUpdate;

            ///////////////////////////////////////////////////
            // is there currently a driver inside?
            bool mHasDriver : 1;

            ///////////////////////////////////////////////////
            // Was terrain currently found? Used for startup checks.
            bool mHasFoundTerrain : 1;

            ///////////////////////////////////////////////////
            // Should this vehicle send a full actor update when asked?
            bool mNotifyFullUpdate : 1;
            bool mNotifyPartialUpdate : 1;

            /// Should the physics coll. det. fail, this will keep the vehicle above ground
            /// at the cost of some runtime performance.
            bool mPerformAboveGroundSafetyCheck : 1;

            /// When publishing pos/rot updates vs dead reckoning - do we send linear velocity or not?
            /// Allows tightly controlled entities that do motion model stuff to not mess up linear velocity
            /// If false, linear velocity will always be zero.
            bool mPublishLinearVelocity : 1;

            /// When publishing pos/rot updates vs dead reckoning - do we send angular velocity or not?
            /// Allows tightly controlled entities that do motion model stuff to not mess up angular velocity
            /// If false, angular velocity will always be zero.
            bool mPublishAngularVelocity : 1;

            /// When this is true, the position and rotation will be pushed to the physics engine on pre physics.
            bool mPushTransformToPhysics : 1;


      };

      class SIMCORE_EXPORT BasePhysicsVehicleActorProxy : public PlatformActorProxy
      {
         public:
            BasePhysicsVehicleActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~BasePhysicsVehicleActorProxy();
            // When you implement CreateActor(), be sure to call InitDeadReckoningHelper on your entityactor
            //virtual void CreateActor();
            virtual void OnEnteredWorld();
      };

   }
}

#endif
