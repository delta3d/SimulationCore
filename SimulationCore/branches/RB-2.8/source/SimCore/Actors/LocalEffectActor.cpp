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

#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/actorproxyicon.h>
#include <dtDAL/exceptionenum.h>

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

      dtDAL::ActorProxyIcon* LocalEffectActorProxy::GetBillBoardIcon()
      {
         if(!mBillBoardIcon.valid())
         {
            mBillBoardIcon =
               new dtDAL::ActorProxyIcon(dtDAL::ActorProxyIcon::IMAGE_BILLBOARD_PARTICLESYSTEM);
         }

         return mBillBoardIcon.get();
      }

      void LocalEffectActorProxy::BuildPropertyMap()
      {
         LocalEffectActor *sa = NULL;
         GetActor(sa);

         dtGame::GameActorProxy::BuildPropertyMap();

         AddProperty(new dtDAL::FloatActorProperty("Bounding Sphere Radius", "Bounding Sphere Radius", 
            dtDAL::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetBoundingSphereRadius),
            dtDAL::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetBoundingSphereRadius),
            "Sets the bounding sphere radius", "Smoke"));

         AddProperty(new dtDAL::FloatActorProperty("Smoke Plume Length", "Smoke Plume Length", 
            dtDAL::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetSmokePlumeLength),
            dtDAL::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetSmokePlumeLength),
            "Length of the smoke plume"));

         AddProperty(new dtDAL::FloatActorProperty("Horizontal Velocity", "Horizontal Velocity", 
            dtDAL::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetHorizontalVelocity),
            dtDAL::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetHorizontalVelocity),
            "Horizontal velocity of this smoke"));

         AddProperty(new dtDAL::FloatActorProperty("Vertical Velocity", "Vertical Velocity", 
            dtDAL::FloatActorProperty::SetFuncType(sa, &LocalEffectActor::SetVerticalVelocity),
            dtDAL::FloatActorProperty::GetFuncType(sa, &LocalEffectActor::GetVerticalVelocity),
            "Vertical velocity of this smoke"));

         AddProperty(new dtDAL::BooleanActorProperty("Enable", "Enable", 
            dtDAL::BooleanActorProperty::SetFuncType(sa, &LocalEffectActor::SetEnabled),
            dtDAL::BooleanActorProperty::GetFuncType(sa, &LocalEffectActor::GetEnabled),
            "Toggles the state of the particle system", "Smoke"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, 
            "Smoke File", "Smoke File", dtDAL::ResourceActorProperty::SetFuncType(this, &LocalEffectActorProxy::LoadFile),
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
      LocalEffectActor::LocalEffectActor(dtGame::GameActorProxy &proxy) : 
         IGActor(proxy), 
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
