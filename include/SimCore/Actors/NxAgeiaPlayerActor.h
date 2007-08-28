/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#ifndef _NX_AGEIA_PLAYER_ACTOR_H_
#define _NX_AGEIA_PLAYER_ACTOR_H_

#include <SimCore/Actors/Human.h>

#ifdef AGEIA_PHYSICS
#include <dtAgeiaPhysX/NxAgeiaCharacterHelper.h>
#endif

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
      class SIMCORE_EXPORT  NxAgeiaPlayerActor : public Human, public dtAgeiaPhysX::NxAgeiaPhysicsInterface, 
                                                                             public dtAgeiaPhysX::NxAgeiaCharacterInterface
#else
      class SIMCORE_EXPORT  NxAgeiaPlayerActor : public Human
#endif
      {
         public:

            /// Constructor
            NxAgeiaPlayerActor(dtGame::GameActorProxy &proxy);

            /// Destructor
            virtual ~NxAgeiaPlayerActor();
      
            /**
            * This method is an invokable called when an object is local and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickLocal(const dtGame::Message &tickMessage);

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickRemote(const dtGame::Message &tickMessage);

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

#ifdef AGEIA_PHYSICS

            //////////////////////////////////////////////////////////////////////////////
            //Ageia Callbacks 
            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit);

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, const NxActor& whatWeHit){}

            // Shape report, used for character interactions for objects in the world.
            virtual NxControllerAction AgeiaCharacterShapeReport(const NxControllerShapeHit& shapeHit);

            // controller hit, used for character interactions with other characterz...
            virtual NxControllerAction AgeiaCharacterControllerReport(const NxControllersHit& controllerHit);
            //////////////////////////////////////////////////////////////////////////////

            /// our helper
            osg::ref_ptr<dtAgeiaPhysX::NxAgeiaCharacterHelper> mPhysicsHelper;
#endif

            void SetMovementTransform(const osg::Vec3& movement);

         private:
            bool        mAcceptInput;     // for ai vs human. 
            osg::Vec3   mMoveRateConstant;// for multiplying for movement amount.
            osg::Vec3   mPreviousTransform;
            osg::Vec3   mSentOverTransform;
            float mTimeForSendingDeadReckoningInfoOut;
            float mTimesASecondYouCanSendOutAnUpdate;
      }; 

      // proxy
      class SIMCORE_EXPORT NxAgeiaPlayerActorProxy : public HumanActorProxy
      {
         public:

            /// Constructor
            NxAgeiaPlayerActorProxy();

            /// Destructor
            virtual ~NxAgeiaPlayerActorProxy();

            /// Builds the properties associated with this player
            virtual void BuildPropertyMap();

            /// Builds the invokables associated with this player
            virtual void BuildInvokables();

            /// Instantiates the actor this proxy encapsulated
            virtual void CreateActor() 
            { 
               NxAgeiaPlayerActor* p = new NxAgeiaPlayerActor(*this);
               SetActor(*p); 

               if(!IsRemote())
               {
                  p->InitDeadReckoningHelper();
               }
            }

            //////////////////////////////////////
            virtual void OnEnteredWorld()
            {
               HumanActorProxy::OnEnteredWorld();

               if (IsRemote())
                  RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
               else
                  RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
            }
      };
   }
}
#endif
