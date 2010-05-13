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
 * @author Eddie Johnson
 */
#ifndef _NX_AGEIA_PLAYER_ACTOR_H_
#define _NX_AGEIA_PLAYER_ACTOR_H_

#include <SimCore/Actors/Human.h>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaCharacterHelper.h>
#else
#include <dtPhysics/charactercontroller.h>
#endif

#include <SimCore/PhysicsTypes.h>

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   class MessageType;

   namespace Actors
   {
      // actor
#ifdef AGEIA_PHYSICS
      class SIMCORE_EXPORT  HumanWithPhysicsActor : public Human, public dtAgeiaPhysX::NxAgeiaPhysicsInterface,
                                                                             public dtAgeiaPhysX::NxAgeiaCharacterInterface
#else
      class SIMCORE_EXPORT  HumanWithPhysicsActor : public Human
#endif
      {
         public:

            /// Constructor
            HumanWithPhysicsActor(dtGame::GameActorProxy& proxy);

            /// Destructor
            virtual ~HumanWithPhysicsActor();

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
            virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

            virtual void SetPosition( const osg::Vec3& position );

            virtual void OffsetPosition( const osg::Vec3& offset );

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            virtual void SetLastKnownRotation(const osg::Vec3& vec);
            virtual void SetLastKnownTranslation(const osg::Vec3& vec);

            //virtual bool ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

#ifdef AGEIA_PHYSICS

            //////////////////////////////////////////////////////////////////////////////
            //Ageia Callbacks
            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit);

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const dtPhysics::PhysicsObject& ourSelf, const dtPhysics::PhysicsObject& whatWeHit){}

            // Shape report, used for character interactions for objects in the world.
            virtual NxControllerAction AgeiaCharacterShapeReport(const NxControllerShapeHit& shapeHit);

            // controller hit, used for character interactions with other characterz...
            virtual NxControllerAction AgeiaCharacterControllerReport(const NxControllersHit& controllerHit);
            //////////////////////////////////////////////////////////////////////////////

            dtAgeiaPhysX::NxAgeiaCharacterHelper* GetPhysicsHelper() {return mPhysicsHelper.get();}

         private:
            /// our helper
            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaCharacterHelper> mPhysicsHelper;
#else
            void PrePhysicsUpdate();
            // returns the physics helper for use
            dtPhysics::PhysicsHelper* GetPhysicsHelper();

         private:
            dtCore::RefPtr<dtPhysics::PhysicsHelper> mPhysicsHelper;
            dtCore::RefPtr<dtPhysics::CharacterController> mCharacterController;
#endif

         public:
            void SetMovementTransform(const osg::Vec3& movement);

         private:

            osg::Vec3   mMoveRateConstant;// for multiplying for movement amount.
            osg::Vec3   mPreviousTransform;
            osg::Vec3   mSentOverTransform;
            float mSecsSinceLastUpdateSent;
            float mMaxUpdateSendRate;
            bool        mAcceptInput;     // for ai vs human.
            bool        mNotifyChangePosition;
            bool        mNotifyChangeOrient;
            bool        mNotifyChangeVelocity;
      };

      // proxy
      class SIMCORE_EXPORT HumanWithPhysicsActorProxy : public HumanActorProxy
      {
         public:

            /// Constructor
            HumanWithPhysicsActorProxy();

            /// Destructor
            virtual ~HumanWithPhysicsActorProxy();

            /// Builds the properties associated with this player
            virtual void BuildPropertyMap();

            /// Builds the invokables associated with this player
            virtual void BuildInvokables();

            /// Instantiates the actor this proxy encapsulated
            virtual void CreateActor();
      };
   }
}
#endif
