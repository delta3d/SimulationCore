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
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/functor.h>
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

         ViewerMaterialActor *actor = dynamic_cast<ViewerMaterialActor*>(GetActor());

         AddProperty(new dtDAL::ColorRgbaActorProperty("Base Color Value", "Base Color Value",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetBaseColorvalue),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetBaseColorvalue),
            "", VISLGROUP));

         AddProperty(new dtDAL::ColorRgbaActorProperty("Highlight Color Value", "Highlight Color Value",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetHighlighteColorvalue),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetHighlighteColorvalue),
            "", VISLGROUP));

         AddProperty(new dtDAL::FloatActorProperty("DynamicFriction", "DynamicFriction",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetDynamicFriction),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetDynamicFriction),
            "Ageia Material Setting - coefficient of dynamic friction -- should be in [0, +inf]. If set to greater than staticFriction, the effective value of staticFriction will be increased to match. if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("StaticFriction", "StaticFriction",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetStaticFriction),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetStaticFriction),
            "Ageia Material Setting - coefficient of static friction -- should be in [0, +inf] if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Restitution", "Restitution",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetRestitution),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetRestitution),
            "Ageia Material Setting - coefficient of restitution -- 0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce. Note that values close to or above 1 may cause stability problems and/or increasing energy. Range: [0,1]", PHYSGROUP));

         AddProperty(new dtDAL::StringActorProperty("SceneName", "SceneName",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetSceneName),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetSceneName),
            "What is the name of the scene this material resides in; only need for physics materials.", PHYSGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND,
            "SmallHitSoundEffect", "SmallHitSoundEffect",  dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetSmallHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND, "MediumHitSoundEffect", "MediumHitSoundEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetMediumHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND, "LargeHitSoundEffect", "LargeHitSoundEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetLargeHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND, "AmbientSoundEffect", "AmbientSoundEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetAmbientSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SOUND, "AmbientMusicEffect", "AmbientMusicEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetAmbientMusicEffect),
            "", MUSCGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TEXTURE, "DecalSmallHit", "DecalSmallHit",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetDecalSmallHit),
            "", VISLGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TEXTURE, "DecalMediumHit", "DecalMediumHit",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetDecalMediumHit),
            "", VISLGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::TEXTURE, "DecalLargeHit", "DecalLargeHit",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetDecalLargeHit),
            "", VISLGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, "DustTrailEffect", "DustTrailEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetDustTrailEffect),
            "", PARTGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, "SmallHitEffect", "SmallHitEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetSmallHitEffect),
            "", PARTGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, "MediumHitEffect", "MediumHitEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetMediumHitEffect),
            "", PARTGROUP));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM, "LargeHitEffect", "LargeHitEffect",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetLargeHitEffect),
            "", PARTGROUP));

         AddProperty(new dtDAL::StringActorProperty("PhysicsParticleLookupStringOne", "PhysicsParticleLookupStringOne",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringOne),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringOne),
            "", VISLGROUP));

         AddProperty(new dtDAL::StringActorProperty("PhysicsParticleLookupStringTwo", "PhysicsParticleLookupStringTwo",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringTwo),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringTwo),
            "", VISLGROUP));

         AddProperty(new dtDAL::StringActorProperty("PhysicsParticleLookupStringThr", "PhysicsParticleLookupStringThr",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringThr),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringThr),
            "", VISLGROUP));

         AddProperty(new dtDAL::StringActorProperty("PhysicsParticleLookupStringFour", "PhysicsParticleLookupStringFour",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringFour),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringFour),
            "", VISLGROUP));

         AddProperty(new dtDAL::StringActorProperty("PhysicsParticleLookupStringFive", "PhysicsParticleLookupStringFive",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetPhysicsParticleLookupStringFive),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPhysicsParticleLookupStringFive),
            "", VISLGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Shininess", "Shininess",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetShinyness),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetShinyness),
            "", SHADGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Moisture", "Moisture",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetWetness),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetWetness),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Reflectivity", "Reflectivity",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetReflectivity),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetReflectivity),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Softness", "Softness",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetSoftness),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetSoftness),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Stretchiness", "Stretchiness",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetStretchiness),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetStretchiness),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Flammibility", "Flammibility",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetFlammibility),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetFlammibility),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("WindResistance", "WindResistance",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetWindResistance),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetWindResistance),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Bumpiness", "Bumpiness",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetBumpiness),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetBumpiness),
            "", SHADGROUP));

         AddProperty(new dtDAL::FloatActorProperty("SelfIllumination", "SelfIllumination",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetSelfIlluminating),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetSelfIlluminating),
            "", VISLGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Inflatability", "Inflatability",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetInflatableness),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetInflatableness),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("RefractionAmount", "RefractionAmount",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetRefractionAmount),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetRefractionAmount),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Pressureization", "Pressureization",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetPressureization),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetPressureization),
            "", PHYSGROUP));

         AddProperty(new dtDAL::FloatActorProperty("Temperature", "Temperature",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetTemperature),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetTemperature),
            "", VISLGROUP));

         AddProperty(new dtDAL::FloatActorProperty("SoundDensityValue", "SoundDensityValue",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetSoundDensityValue),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetSoundDensityValue),
            "", SFXGROUP));

         AddProperty(new dtDAL::FloatActorProperty("ParticleSpoutAmount", "ParticleSpoutAmount",
            dtDAL::MakeFunctor(*actor, &ViewerMaterialActor::SetParticleSpoutAmount),
            dtDAL::MakeFunctorRet(*actor, &ViewerMaterialActor::GetParticleSpoutAmount),
            "", PARTGROUP));
      }

      // Creates the actor
      void ViewerMaterialActorProxy::CreateActor()
      {
         SetActor(*new ViewerMaterialActor(*this));
      }

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////
      ViewerMaterialActor::ViewerMaterialActor(dtGame::GameActorProxy &proxy) : dtGame::GameActor(proxy)
         , m_PHYS_DynamicFriction(0.5f)
         , m_PHYS_StaticFriction(0.5f)
         , m_PHYS_Restitution(0.2f)
         , m_OTHR_Wetness(0.0f)
         , m_OTHR_Softness(0.0f)
         , m_OTHR_Stretchiness(0.0f)
         , m_OTHR_Flammability(0.0f)
         , m_OTHR_Temperature(0.0f)
         , m_OTHR_WindResistance(0.0f)
         , m_OTHR_Inflatableness(0.0f)
         , m_OTHR_Pressurization(0.0f)
         , m_SHAD_Bumpiness(0.0f)
         , m_SFX_DensityForSound(0.0f)
         , m_VISL_Shininess(0.0f)
         , m_VISL_Reflectivity(0.0f)
         , m_VISL_Refraction(0.0f)
         , m_VISL_SelfIlluminating(0.0f)
         , m_PART_SpoutAmount(0.0f)
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
