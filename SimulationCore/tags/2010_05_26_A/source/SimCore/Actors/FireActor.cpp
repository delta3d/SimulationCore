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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/FireActor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/exceptionenum.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      FireActorProxy::FireActorProxy()
      {
         SetClassName("SimCore::Actors::FireActor");
      }

      FireActorProxy::~FireActorProxy()
      {

      }

      dtDAL::ActorProxyIcon* FireActorProxy::GetBillBoardIcon()
      {
         if(!mBillBoardIcon.valid())
         {
            mBillBoardIcon =
               new dtDAL::ActorProxyIcon(dtDAL::ActorProxyIcon::IMAGE_BILLBOARD_PARTICLESYSTEM);
         }

         return mBillBoardIcon.get();
      }

      void FireActorProxy::BuildPropertyMap()
      {
         FireActor* fa = NULL;
         GetActor(fa);

         LocalEffectActorProxy::BuildPropertyMap();

         AddProperty(new dtDAL::BooleanActorProperty("Enable Fire", "Enable Fire", 
            dtDAL::BooleanActorProperty::SetFuncType(fa, &FireActor::SetEnabled),
            dtDAL::BooleanActorProperty::GetFuncType(fa, &FireActor::GetEnabled),
            "Toggles the state of the particle system", "Fire"));
            
         AddProperty(new dtDAL::FloatActorProperty("Light Range", "Light Range", 
            dtDAL::FloatActorProperty::SetFuncType(fa, &FireActor::SetLightRange),
            dtDAL::FloatActorProperty::GetFuncType(fa, &FireActor::GetLightRange),
            "Light range of the illumination of this fire"));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, 
            "Fire File", "Fire File", dtDAL::ResourceActorProperty::SetFuncType(this, &FireActorProxy::LoadFile),
            "Loads the file of this particle system", "Fire"));
      }

      /// Loads the file the particle system will use
      void FireActorProxy::LoadFile(const std::string &fileName) 
      {
         FireActor *fa = static_cast<FireActor*>(GetActor());
      
         fa->LoadFireFile(fileName); 
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      FireActor::FireActor(dtGame::GameActorProxy &proxy) : 
         LocalEffectActor(proxy), 
         mLightRange(0)
      {
       
      }

      FireActor::~FireActor()
      {

      }
      
      void FireActor::LoadFireFile(const std::string& fileName) 
      { 
         if(!mParticleSystem->LoadFile(fileName))
         {
            LOG_ERROR("Failed to load the particle system file: " + fileName);
         }
      }
   }
}
