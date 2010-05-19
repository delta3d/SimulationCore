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

#ifndef _STEALTH_ACTOR_H_
#define _STEALTH_ACTOR_H_

#include <SimCore/Export.h>
#include <SimCore/Actors/Platform.h>
#include <osg/Vec3>

namespace dtGame
{
   class DeadReckoningAlgorithm;
}
namespace SimCore
{
   class AttachToActorMessage;

   namespace Actors
   {
      class SIMCORE_EXPORT StealthActor : public Platform
      {
         public:

            /// Constructor
            StealthActor(dtGame::GameActorProxy& proxy);

            /**
             * Sets the on the Stealth Actor
             * @param aatp The new state of 3rd person actor attaching
             */
            void SetAttachAsThirdPerson(bool aatp) { mAttachAsThirdPerson = aatp; }

            /**
             * Gets if we are attaching as a third person
             * @return mAttachAsThirdPerson
             */
            bool GetAttachAsThirdPerson() const { return mAttachAsThirdPerson; }

            void SetAttachOffset( const osg::Vec3& offset ) { mAttachOffset = offset; }
            const osg::Vec3& GetAttachOffset() const { return mAttachOffset; }

            /**
             * Invokable used to make this entity attach to another.
             */
            virtual void AttachToActor(const dtGame::Message& attachMessage);
            void Detach(const dtGame::Message& msg);
            void UpdateFromParent(const dtGame::Message& msg);

            /**
             * Invokable used to make this entity warp to a position.  It will also detach.
             */
            void WarpToPosition(const dtGame::Message& warpToPosMessage);

            // Returns true if we think we are attached to something.
            bool IsAttachedToActor();

            /// Direct callable version of attaching/detaching without an attach message
            void AttachOrDetachActor(dtGame::GameActorProxy* ga, 
               const dtCore::UniqueId& id, const std::string& attachPointNode = std::string(""),
               const osg::Vec3 &attachRotationHPR = osg::Vec3());

         protected:

            /// Destructor
            virtual ~StealthActor();

         private:

            /**
             * Helper method to actually do attaching and detaching
             * @param ataMsg A pointer to the attach message.  NULL, or an empty attach id mean to detach.
             */
            void AttachOrDetachActor(const AttachToActorMessage* ataMsg = NULL);
            void DoDetach();
            virtual void DoAttach(dtGame::GameActorProxy& ga, const std::string& attachPointNode,
               const osg::Vec3& attachRotationHPR);
            void DoAttach(const AttachToActorMessage& ataMsg, dtGame::GameActorProxy& ga);

            /// This should be set to false in the PlayerActor subclass of this class
            bool mAttachAsThirdPerson;

            dtGame::DeadReckoningAlgorithm *mOldDRA;

            osg::Vec3 mAttachOffset;
      };

      class SIMCORE_EXPORT StealthActorProxy : public PlatformActorProxy
      {
         public:

            /// Constructor
            StealthActorProxy();

            /// Builds the properties for this proxy
            void BuildPropertyMap();

            /// Builds the invokables for this proxy
            void BuildInvokables();

            /// Creates the actor for this proxy
            void CreateActor();

            virtual void OnEnteredWorld();

         protected:

            /// Destructor
            virtual ~StealthActorProxy();
      };
   }
}

#endif
