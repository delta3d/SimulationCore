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
#ifndef _MULTIPLE_DETONATION_ACTOR_H_
#define _MULTIPLE_DETONATION_ACTOR_H_


#include <SimCore/Actors/DetonationActor.h>


namespace SimCore
{
   namespace Actors
   {

      class SIMCORE_EXPORT MultipleDetonationActor : public DetonationActor
      {
         public:
            typedef DetonationActor BaseClass;
            /// Constructor
            MultipleDetonationActor(dtGame::GameActorProxy& proxy);

            // Invoked when a actor is added to the Game Managerring
            virtual void OnEnteredWorld();

            DT_DECLARE_ACCESSOR_INLINE(dtDAL::ResourceDescriptor, MultipleImpactEffect);

            DT_DECLARE_ACCESSOR_INLINE(int, NumDetonations);
            DT_DECLARE_ACCESSOR_INLINE(float, DetonationRadius);

         protected:
            /// Destructor
            virtual ~MultipleDetonationActor();

         private:

            /*virtual*/ void PlaySound();
            /*virtual*/ void RenderDetonation();
            /*virtual*/ void RenderSmoke();
            /*virtual*/ void StopRenderingSmoke();

            void CreateDetonationParticles();
            void CreateSmokeParticles();
            void CreateDynamicLights();
            void CreateRandomOffsets();


            typedef std::vector<osg::Vec3> PositionArray;
            PositionArray mDetonationOffsets;

            typedef std::vector<dtCore::RefPtr<dtCore::ParticleSystem> > ParticleSystemArray;
            ParticleSystemArray mExplosionArray;
            ParticleSystemArray mSmokeArray;
            
      };

      class SIMCORE_EXPORT MultipleDetonationActorProxy : public DetonationActorProxy
      {
         public:
            typedef DetonationActorProxy BaseClass;

            static const std::string CLASS_NAME;

            /// Constructor
            MultipleDetonationActorProxy();

            /// Creates the actor
            void CreateActor() { SetActor(*new MultipleDetonationActor(*this)); }

            /// Builds the properties this actor has
            void BuildPropertyMap();

            /// Builds the invokables of this actor
            void BuildInvokables();

            /// Clear all timers from the GameManager
            void ClearTimers();

         protected:

            /// Destructor
            virtual ~MultipleDetonationActorProxy();
      };
   }
}
#endif
