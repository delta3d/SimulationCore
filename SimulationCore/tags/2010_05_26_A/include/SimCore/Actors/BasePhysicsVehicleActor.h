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
#include <dtPhysics/physicshelper.h>

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

            /// dtPhysics post physics callback.
            virtual void PrePhysicsUpdate();
            virtual void PostPhysicsUpdate();


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

            void SetPhysicsHelper(dtPhysics::PhysicsHelper* newHelper);
            dtPhysics::PhysicsHelper* GetPhysicsHelper();

            void SetHasDriver(bool hasDriver);
            bool GetHasDriver() const;

            void SetPerformAboveGroundSafetyCheck(bool enable);
            bool GetPerformAboveGroundSafetyCheck() const;


         protected:
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
