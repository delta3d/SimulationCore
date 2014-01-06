/* -*-c++-*-
* Simulation Core
* Copyright 2010, Alion Science and Technology
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

#ifndef TRAIL_EFFECT_ACT_COMP_H_
#define TRAIL_EFFECT_ACT_COMP_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtDAL/resourcedescriptor.h>
#include <dtUtil/getsetmacros.h>
#include <dtGame/actorcomponentbase.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class ParticleSystem;
}

namespace dtGame
{
   class GameActor;
   class TickMessage;
}



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT TrailEffectActComp : public dtGame::ActorComponent
      {
         public:
            typedef dtGame::ActorComponent BaseClass;

            static const ActorComponent::ACType TYPE;

            // Property Names
            static const dtUtil::RefString PROPERTY_TRAIL_PARTICLES;
            static const dtUtil::RefString PROPERTY_TRAIL_ATTACHED;
            static const dtUtil::RefString PROPERTY_TRAIL_ATTACH_NODE_NAME;
            static const dtUtil::RefString PROPERTY_TRAIL_CLAMP_INTERVAL;
            static const dtUtil::RefString PROPERTY_TRAIL_ENABLE_DISTANCE;

            // Default Values
            static const float DEFAULT_TRAIL_CLAMP_INTERVAL;
            static const float DEFAULT_TRAIL_ENABLE_DISTANCE;

            TrailEffectActComp();

            ////////////////////////////////////////////////////////////////////
            // PROPERTY DECLARATIONS - getter, setter and member variable.
            ////////////////////////////////////////////////////////////////////
            /**
             * Time in seconds between ground clamping attempts.
             */
            DT_DECLARE_ACCESSOR(float, TrailClampInterval);
            
            /**
             * Max distance above the surface in which the trail effect remains enabled
             */
            DT_DECLARE_ACCESSOR(float, TrailEnableDistance);

            /**
             * Initialization flag for determining how the loaded particle effect
             * is attached to the parent actor.
             */
            DT_DECLARE_ACCESSOR(bool, TrailAttached);

            /**
             * Node name on the actor to which the effect may be attached.
             */
            DT_DECLARE_ACCESSOR(std::string, TrailAttachNodeName);

            /**
             * The particle system file that represents the effect.
             */
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, TrailParticlesFile);

            ////////////////////////////////////////////////////////////////////
            // SPECIAL METHODS
            ////////////////////////////////////////////////////////////////////

            /**
             * Method to determine if this actor component should be ticked.
             */
            bool IsTickable() const;

            /**
             * Set the effect enabled.
             */
            void SetEnabled(bool enable);
            bool IsEnabled() const;

            /**
             * Primary method for updating the effect.
             */
            void Update(float timeDelta);

            /**
             * Remote tick handler for owner actors in remote-mode.
             */
            virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

            ////////////////////////////////////////////////////////////////////
            // OVERRIDE METHODS - ActorComponent
            ////////////////////////////////////////////////////////////////////
            
            /**
             * Local tick handler for owner actors in local-mode.
             */
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

            /** 
             * Called when this ActorComponent is removed from the parent actor.
             * @param actor The GameActor this ActorComponent has just been removed from.
             */
            virtual void OnRemovedFromActor(dtGame::GameActor& actor);

            /**
             * Override method to handle additional setup that can only be handled
             * prior to entering the world, such as message handler registration.
             */
            virtual void OnEnteredWorld();

            /**
             * Override method to handle tear-down when the parent actor is removed
             * from the world. Any tick handlers will be removed at this point.
             */
            virtual void OnRemovedFromWorld();

            /**
             * Handles the setup and registration of its properties.
             */
            virtual void BuildPropertyMap();
         
         protected:
            TrailEffectActComp(const ActorComponent::ACType& actType); // for derived classes

            virtual ~TrailEffectActComp();

            /**
             * Load particles from the currently set file descriptor.
             */
            bool LoadParticles();

            /**
             * Method to attach the particle effect to the currently set
             * attach node name.
             * @return Success of attaching the particles.
             */
            bool AttachParticles();
            
            /**
             * Method to detach the particle effect.
             * @return Number of parents the particles were detached from.
             */
            int DetachParticles();

            /**
             * Method for registering the remote tick handler.
             */
            void RegisterForRemoteTicks();

            /**
             * Method for unregistering the remote tick handler.
             */
            void UnregisterForRemoteTicks();

            /**
             * Method for registering the local/remote tick handler
             * relevant to the owner actors local/remote mode.
             */
            void RegisterTickHandlers();
            
            /**
             * Method for unregistering the local/remote tick handler
             * relevant to the owner actors local/remote mode.
             */
            void UnregisterTickHandlers();

            /**
             * Method for setting the absolute position of the particle effect.
             */
            void SetParticlePosition(const osg::Vec3& pos);

         private:
            void SetDefaults();

            bool mOwnerIsPlatform;
            float mClampTimer;
            osg::Vec3 mClampPoint;
            std::shared_ptr<dtCore::ParticleSystem> mParticles;
      };

   }
}

#endif
