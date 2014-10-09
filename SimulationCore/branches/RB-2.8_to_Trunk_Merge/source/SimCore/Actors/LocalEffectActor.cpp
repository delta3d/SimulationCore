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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/LocalEffectActor.h>

#include <dtCore/particlesystem.h>

#include <dtCore/enginepropertytypes.h>
#include <dtCore/actorproxyicon.h>
#include <dtCore/exceptionenum.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      LocalEffectActorProxy::LocalEffectActorProxy() 
      {
         SetClassName("SimCore::Actors::LocalEffectActor");
      }

      LocalEffectActorProxy::~LocalEffectActorProxy()
      {

      }

      dtCore::ActorProxyIcon* LocalEffectActorProxy::GetBillBoardIcon()
      {
         if(!mBillBoardIcon.valid())
         {
            mBillBoardIcon =
               new dtCore::ActorProxyIcon(dtCore::ActorProxyIcon::IMAGE_BILLBOARD_PARTICLESYSTEM);
         }

         return mBillBoardIcon.get();
      }

      void LocalEffectActorProxy::BuildPropertyMap()
      {
         LocalEffectActor *sa = NULL;
         GetDrawable(sa);

         dtGame::GameActorProxy::BuildPropertyMap();

         AddProperty(new dtCore::FloatActorProperty("Bounding Sphere Radius", "Bounding Sphere Radius", 
            dtCore::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetBoundingSphereRadius),
            dtCore::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetBoundingSphereRadius),
            "Sets the bounding sphere radius", "Smoke"));

         AddProperty(new dtCore::FloatActorProperty("Smoke Plume Length", "Smoke Plume Length", 
            dtCore::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetSmokePlumeLength),
            dtCore::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetSmokePlumeLength),
            "Length of the smoke plume"));

         AddProperty(new dtCore::FloatActorProperty("Horizontal Velocity", "Horizontal Velocity", 
            dtCore::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetHorizontalVelocity),
            dtCore::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetHorizontalVelocity),
            "Horizontal velocity of this smoke"));

         AddProperty(new dtCore::FloatActorProperty("Vertical Velocity", "Vertical Velocity", 
            dtCore::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetVerticalVelocity),
            dtCore::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetVerticalVelocity),
            "Vertical velocity of this smoke"));

         AddProperty(new dtCore::BooleanActorProperty("Enable", "Enable", 
            dtCore::BooleanActorProperty::SetFuncType(sa, &LocalEffectActor::SetEnabled),
            dtCore::BooleanActorProperty::GetFuncType(sa, &LocalEffectActor::GetEnabled),
            "Toggles the state of the particle system", "Smoke"));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, 
            "Smoke File", "Smoke File", dtCore::ResourceActorProperty::SetFuncType(this, &LocalEffectActorProxy::LoadFile),
            "Loads the smoke file for the particle system", "Smoke"));
      }

      void LocalEffectActorProxy::LoadFile(const std::string &fileName)
      {
         LocalEffectActor *da = static_cast<LocalEffectActor*>(GetDrawable());
         
         da->LoadSmokeFile(fileName);
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      LocalEffectActor::LocalEffectActor(dtGame::GameActorProxy& owner) :
         IGActor(owner),
         mParticleSystem(new dtCore::ParticleSystem),
         mBoundingSphereRadius(0),
         mSmokePlumeLength(0), 
         mHorizontalVelocity(0), 
         mVerticalVelocity(0)
      {
         mParticleSystem->SetEnabled(false);
         AddChild(mParticleSystem.get());
      }

      LocalEffectActor::~LocalEffectActor()
      {

      }

      void LocalEffectActor::LoadSmokeFile(const std::string &fileName) 
      {
         if(!mParticleSystem->LoadFile(fileName))
         {
            LOG_ERROR("Failed to load the particle system file: " + fileName);
         }
      }
   }
}
