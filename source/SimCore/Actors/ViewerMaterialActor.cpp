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
* @author Allen Danklefsen
*/

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/functor.h>
#include <SimCore/Components/ViewerMaterialComponent.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////
      ViewerMaterialActorProxy::ViewerMaterialActorProxy()
      {
         SetClassName("ViewerMaterialActor");

      }

      //////////////////////////////////////////////////////////
      ViewerMaterialActorProxy::~ViewerMaterialActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void ViewerMaterialActorProxy::BuildPropertyMap()
      {
         const std::string GROUP       = "Not Categorized Material Types";
         const std::string PHYSGROUP   = "Physics Material Types";
         const std::string SHADGROUP   = "Shader Material Types";
         const std::string PARTGROUP   = "Particle Systems Material Types";
         const std::string SFXGROUP    = "Sound Effects Material Types";
         const std::string MUSCGROUP   = "Music Material Types";
         const std::string NETGROUP    = "Networked Material Types";
         const std::string VISLGROUP   = "Visual Material Types";
         const std::string OTHRGROUP   = "Other Types or Multiple Material Types";

         ViewerMaterialActor *actor = dynamic_cast<ViewerMaterialActor*>(GetDrawable());

         AddProperty(new dtCore::ColorRgbaActorProperty("Base Color Value", "Base Color Value",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetBaseColorvalue),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetBaseColorvalue),
            "", VISLGROUP));

         AddProperty(new dtCore::ColorRgbaActorProperty("Highlight Color Value", "Highlight Color Value",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetHighlighteColorvalue),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetHighlighteColorvalue),
            "", VISLGROUP));

         AddProperty(new dtCore::FloatActorProperty("DynamicFriction", "DynamicFriction",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetDynamicFriction),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetDynamicFriction),
            "Ageia Material Setting - coefficient of dynamic friction -- should be in [0, +inf]. If set to greater than staticFriction, the effective value of staticFriction will be increased to match. if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)", PHYSGROUP));

         AddProperty(new dtCore::FloatActorProperty("StaticFriction", "StaticFriction",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetStaticFriction),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetStaticFriction),
            "Ageia Material Setting - coefficient of static friction -- should be in [0, +inf] if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)", PHYSGROUP));

         AddProperty(new dtCore::FloatActorProperty("Restitution", "Restitution",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetRestitution),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetRestitution),
            "Ageia Material Setting - coefficient of restitution -- 0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce. Note that values close to or above 1 may cause stability problems and/or increasing energy. Range: [0,1]", PHYSGROUP));


         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND,
            "SmallHitSoundEffect", "SmallHitSoundEffect",  dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetSmallHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "MediumHitSoundEffect", "MediumHitSoundEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetMediumHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "LargeHitSoundEffect", "LargeHitSoundEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetLargeHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "AmbientSoundEffect", "AmbientSoundEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetAmbientSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "AmbientMusicEffect", "AmbientMusicEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetAmbientMusicEffect),
            "", MUSCGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TEXTURE, "DecalSmallHit", "DecalSmallHit",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetDecalSmallHit),
            "", VISLGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TEXTURE, "DecalMediumHit", "DecalMediumHit",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetDecalMediumHit),
            "", VISLGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TEXTURE, "DecalLargeHit", "DecalLargeHit",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetDecalLargeHit),
            "", VISLGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "DustTrailEffect", "DustTrailEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetDustTrailEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "SmallHitEffect", "SmallHitEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetSmallHitEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "MediumHitEffect", "MediumHitEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetMediumHitEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "LargeHitEffect", "LargeHitEffect",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetLargeHitEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::StringActorProperty("PhysicsParticleLookupStringOne", "PhysicsParticleLookupStringOne",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringOne),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringOne),
            "", VISLGROUP));

         AddProperty(new dtCore::StringActorProperty("PhysicsParticleLookupStringTwo", "PhysicsParticleLookupStringTwo",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringTwo),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringTwo),
            "", VISLGROUP));

         AddProperty(new dtCore::StringActorProperty("PhysicsParticleLookupStringThr", "PhysicsParticleLookupStringThr",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringThr),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringThr),
            "", VISLGROUP));

         AddProperty(new dtCore::StringActorProperty("PhysicsParticleLookupStringFour", "PhysicsParticleLookupStringFour",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringFour),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringFour),
            "", VISLGROUP));

         AddProperty(new dtCore::StringActorProperty("PhysicsParticleLookupStringFive", "PhysicsParticleLookupStringFive",
            dtCore::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringFive),
            dtCore::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringFive),
            "", VISLGROUP));

      }

      // Creates the actor
      void ViewerMaterialActorProxy::CreateDrawable()
      {
         SetDrawable(*new ViewerMaterialActor(*this));
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////
      ViewerMaterialActor::ViewerMaterialActor(dtGame::GameActorProxy &proxy) : dtGame::GameActor(proxy)
         , m_PHYS_DynamicFriction(0.5f)
         , m_PHYS_StaticFriction(0.5f)
         , m_PHYS_Restitution(0.2f)
      {
         SetPhysicsParticleLookupStringOne("");
         SetPhysicsParticleLookupStringTwo("");
         SetPhysicsParticleLookupStringThr("");
         SetPhysicsParticleLookupStringFour("");
         SetPhysicsParticleLookupStringFive("");
      }

      void ViewerMaterialActor::OnEnteredWorld()
      {
         SimCore::Components::ViewerMaterialComponent* materialComponent = dynamic_cast<SimCore::Components::ViewerMaterialComponent*>(GetGameActorProxy().GetGameManager()->GetComponentByName("ViewerMaterialComponent"));
         if(materialComponent == NULL)
         {
            LOG_ERROR("materialComponent Is not initialized, make sure a new one was made before loading a map in");
         }
         else
            materialComponent->RegisterAMaterialWithComponent(this);
      }
   }
}
