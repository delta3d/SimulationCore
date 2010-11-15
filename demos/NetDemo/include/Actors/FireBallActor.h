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
* @author Bradley Anderegg 
*/
#ifndef DELTA_FIRE_BALL_ACTOR_H
#define DELTA_FIRE_BALL_ACTOR_H

#include <DemoExport.h>
#include <dtGame/gameactor.h>
#include <SimCore/Components/VolumeRenderingComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/BasePhysicsVehicleActor.h>

namespace NetDemo
{
   class NETDEMO_EXPORT FireBallActor : public SimCore::Actors::BasePhysicsVehicleActor
   {
      public:
         typedef SimCore::Actors::BasePhysicsVehicleActor BaseClass;

         /// Constructor
         FireBallActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy);

         virtual void BuildActorComponents();

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         //virtual void PostPhysicsUpdate();

         virtual void OnTickLocal( const dtGame::TickMessage& tickMessage );
         virtual void OnTickRemote( const dtGame::TickMessage& tickMessage );

         void SetVelocity(const osg::Vec3& vel);
         const osg::Vec3& GetVelocity() const;

         void SetPosition(const osg::Vec3& pos);
         const osg::Vec3& GetPosition() const;

         void DoExplosion(float);


      protected:

         /// Destructor
         virtual ~FireBallActor();


      private:

         osg::Vec3 mVelocity;
         dtCore::RefPtr<SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord> mShapeVolume;
         dtCore::RefPtr<SimCore::Components::RenderingSupportComponent::DynamicLight> mDynamicLight;

   };

   class NETDEMO_EXPORT FireBallActorProxy : public SimCore::Actors::BasePhysicsVehicleActorProxy
   {
      public:

         /// Constructor
         FireBallActorProxy();

         /// Adds the properties associated with this actor
         void BuildPropertyMap();

         /// Creates the actor
         void CreateActor() { SetActor(*new FireBallActor(*this)); }


      protected:

         /// Destructor
         virtual ~FireBallActorProxy();


   };
}

#endif
