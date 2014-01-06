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
* @author David Guthrie
*/
#ifndef _LOCAL_EFFECT_ACTOR_H_
#define _LOCAL_EFFECT_ACTOR_H_

#include <SimCore/Actors/IGActor.h>
#include <dtCore/particlesystem.h>

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT LocalEffectActor : public IGActor
      {
         public:

            /// Constructor
            LocalEffectActor(dtGame::GameActorProxy &proxy);

            /**
             * Gets the radius of the bounding sphere of this dust actor
             * @return mBoundingSphereRadius
             */
            inline float GetBoundingSphereRadius() const { return mBoundingSphereRadius; }

            /**
             * Sets the radius of the bounding sphere of this dust actor
             * @param newRadius, the radius to set
             */
            inline void SetBoundingSphereRadius(float newRadius) { mBoundingSphereRadius = newRadius; }

            /**
             * Toggles the state of the particle system associated with this dust actor
             * @param enable True to enable, false to disable
             */
            inline void SetEnabled(bool enable) { mParticleSystem->SetEnabled(enable); }

            /**
             * Returns is the particle system is enabled
             * @return True if enabled, false if not
             */
            inline bool GetEnabled() { return mParticleSystem->IsEnabled(); }

            /**
             * Sets the smoke plume length on this actor
             * @param lat The new length
             */
            void SetSmokePlumeLength(float length) { mSmokePlumeLength = length; }

            /**
             * Gets the smoke plume length on this actor
             * @return mSmokePlumeLength
             */
            float GetSmokePlumeLength() const { return mSmokePlumeLength; }

            /**
             * Sets the horizontal velocity of this actor
             * @param vel The new velocity
             */
            void SetHorizontalVelocity(float vel) { mHorizontalVelocity = vel; }

            /**
             * Gets the horizontal velocity of this actor
             * @return mHorizontalVelocity
             */
            float GetHorizontalVelocity() const { return mHorizontalVelocity; }

            /**
             * Sets the vertical velocity of this actor
             * @param vel The new velocity
             */
            void SetVerticalVelocity(float vel) { mVerticalVelocity = vel; }

            /**
             * Gets the vertical velocity of this actor
             * @return mVerticalVelocity
             */
            float GetVerticalVelocity() const { return mVerticalVelocity; }

            /**
             * Loads in a particle system file
             * @param fileName The name of the file
             */
            void LoadSmokeFile(const std::string &fileName);

         protected:
            /// Destructor
            virtual ~LocalEffectActor();

            std::shared_ptr<dtCore::ParticleSystem> mParticleSystem;

         private:

            float mBoundingSphereRadius;
            float mSmokePlumeLength, mHorizontalVelocity, mVerticalVelocity;
      };

      class SIMCORE_EXPORT LocalEffectActorProxy : public dtGame::GameActorProxy
      {
         public:

            /// Constructor
            LocalEffectActorProxy();

            /// Adds the properties associated with this actor
            void BuildPropertyMap();

            /// Creates the actor
            void CreateDrawable() { SetDrawable(*new LocalEffectActor(*this)); }

            /**
             * @return the billboard used to represent particle systems.
             */
            virtual dtDAL::ActorProxyIconPtr GetBillBoardIcon();

            /// Loads a particle system file
            virtual void LoadFile(const std::string &fileName);

            /**
             * Gets the method by which a particle system is rendered.
             * @return dtDAL::BaseActorObject::RenderMode::DRAW_BILLBOARD_ICON.
             */
            virtual const dtDAL::BaseActorObject::RenderMode& GetRenderMode()
            {
                return dtDAL::BaseActorObject::RenderMode::DRAW_BILLBOARD_ICON;
            }
         protected:

            /// Destructor
            virtual ~LocalEffectActorProxy();

         private:
      };
   }
}

#endif
