/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/

#ifndef _NX_REMOTE_KINEMATIC_ACTOR_
#define _NX_REMOTE_KINEMATIC_ACTOR_

#include <SimCore/Export.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/VehicleInterface.h>


namespace SimCore
{
   namespace Actors
   {
#ifdef AGEIA_PHYSICS
   #include <NxAgeiaPrimitivePhysicsHelper.h>

      class SIMCORE_EXPORT NxAgeiaRemoteKinematicActor : public Platform, 
                                                      public dtAgeiaPhysX::NxAgeiaPhysicsInterface, 
                                                      public VehicleInterface
      #else
      class SIMCORE_EXPORT NxAgeiaRemoteKinematicActor : public Platform, 
                                                      public VehicleInterface
      #endif
      {
         public:
            /// constructor for NxAgeiaBaseActor
            NxAgeiaRemoteKinematicActor(PlatformActorProxy &proxy);
           
            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            /// change physics model
            virtual void SetDamageState(BaseEntityActorProxy::DamageStateEnum &damageState);

            /// your basic tick local
            virtual void TickLocal(const dtGame::Message& tickMessage);

            /// since it derives off vehicle interface
            virtual float GetMPH() {return 0.0f;}

      #ifdef AGEIA_PHYSICS

            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, 
               NxActor& ourSelf, NxActor& whatWeHit);

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, 
               const NxActor& whatWeHit){}

            // returns the physics helper for use
            dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper* GetPhysicsHelper() {return mPhysicsHelper.get();}
       
            // single method that can be called multiple places
            void LoadCollision();

      #endif
            ///////////////////////////////////////
            void SetNodeForGeometryToUse(osg::Node* nodeToUse)
            {
               mLoadGeomFromNode = true;
               mNodeForGeometry = nodeToUse;
            }

            /// default name
            static const std::string& DEFAULT_NAME;

            /// buildings default name
            static const std::string& BUILDING_DEFAULT_NAME;

         protected:
            /// destructor
            virtual ~NxAgeiaRemoteKinematicActor();

         private:
            bool                       mLoadGeomFromNode;
            dtCore::RefPtr<osg::Node>  mNodeForGeometry;
         #ifdef AGEIA_PHYSICS
            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mPhysicsHelper;
         #endif
      };

      ////////////////////////////////////////////////////////
      // PROXY
      ////////////////////////////////////////////////////////
      class SIMCORE_EXPORT NxAgeiaRemoteKinematicActorProxy : public PlatformActorProxy
      {
         public:
            NxAgeiaRemoteKinematicActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~NxAgeiaRemoteKinematicActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };
   }// namespace
}// namespace
#endif
