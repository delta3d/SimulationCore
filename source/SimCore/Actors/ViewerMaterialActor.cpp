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
#include <SimCore/Components/ViewerMaterialComponent.h>

#include <dtCore/propertymacros.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////
      ViewerMaterialActor::ViewerMaterialActor()
      : m_PHYS_DynamicFriction(0.5f)
      , m_PHYS_StaticFriction(0.5f)
      , m_PHYS_Restitution(0.2f)
      {
         SetClassName("ViewerMaterialActor");
         SetPhysicsParticleLookupStringOne("");
         SetPhysicsParticleLookupStringTwo("");
         SetPhysicsParticleLookupStringThree("");
         SetPhysicsParticleLookupStringFour("");
         SetPhysicsParticleLookupStringFive("");
      }

      //////////////////////////////////////////////////////////
      ViewerMaterialActor::~ViewerMaterialActor()
      {

      }

      //////////////////////////////////////////////////////////
      void ViewerMaterialActor::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUP       = "Not Categorized Material Types";
         static const dtUtil::RefString PHYSGROUP   = "Physics Material Types";
         static const dtUtil::RefString SHADGROUP   = "Shader Material Types";
         static const dtUtil::RefString PARTGROUP   = "Particle Systems Material Types";
         static const dtUtil::RefString SFXGROUP    = "Sound Effects Material Types";
         static const dtUtil::RefString MUSCGROUP   = "Music Material Types";
         static const dtUtil::RefString NETGROUP    = "Networked Material Types";
         static const dtUtil::RefString VISLGROUP   = "Visual Material Types";
         static const dtUtil::RefString OTHERGROUP   = "Other Types or Multiple Material Types";

         typedef dtCore::PropertyRegHelper<ViewerMaterialActor&, ViewerMaterialActor> RegHelperType;
         RegHelperType physReg(*this, this, PHYSGROUP);
         RegHelperType shadReg(*this, this, SHADGROUP);
         RegHelperType partReg(*this, this, PARTGROUP);
         RegHelperType sfxReg(*this, this, SFXGROUP);
         RegHelperType musReg(*this, this, MUSCGROUP);
         RegHelperType netReg(*this, this, NETGROUP);
         RegHelperType vislReg(*this, this, VISLGROUP);
         RegHelperType otherReg(*this, this, OTHERGROUP);



         AddProperty(new dtCore::ColorRgbaActorProperty("Base Color Value", "Base Color Value",
               dtCore::ColorRgbaActorProperty::SetFuncType(this, &ViewerMaterialActor::SetBaseColorvalue),
               dtCore::ColorRgbaActorProperty::GetFuncType(this, &ViewerMaterialActor::GetBaseColorvalue),
               "", VISLGROUP));

         AddProperty(new dtCore::ColorRgbaActorProperty("Highlight Color Value", "Highlight Color Value",
               dtCore::ColorRgbaActorProperty::SetFuncType(this, &ViewerMaterialActor::SetHighlighteColorvalue),
               dtCore::ColorRgbaActorProperty::GetFuncType(this, &ViewerMaterialActor::GetHighlighteColorvalue),
            "", VISLGROUP));

         DT_REGISTER_PROPERTY(DynamicFriction,
               "Coefficient of dynamic friction -- should be in [0, +inf].",
               RegHelperType, physReg);

         DT_REGISTER_PROPERTY(StaticFriction,
               "Coefficient of static friction -- should be in [0, +inf].",
               RegHelperType, physReg);

         DT_REGISTER_PROPERTY(Restitution,
               "Coefficient of restitution -- 0 to 1 where 1 means 100% of bouncing energy is conserved.",
               RegHelperType, physReg);


         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND,
            "SmallHitSoundEffect", "SmallHitSoundEffect",  dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetSmallHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "MediumHitSoundEffect", "MediumHitSoundEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetMediumHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "LargeHitSoundEffect", "LargeHitSoundEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetLargeHitSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "AmbientSoundEffect", "AmbientSoundEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetAmbientSoundEffect),
            "", SFXGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::SOUND, "AmbientMusicEffect", "AmbientMusicEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetAmbientMusicEffect),
            "", MUSCGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TEXTURE, "DecalSmallHit", "DecalSmallHit",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetDecalSmallHit),
            "", VISLGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TEXTURE, "DecalMediumHit", "DecalMediumHit",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetDecalMediumHit),
            "", VISLGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::TEXTURE, "DecalLargeHit", "DecalLargeHit",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetDecalLargeHit),
            "", VISLGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "DustTrailEffect", "DustTrailEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetDustTrailEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "SmallHitEffect", "SmallHitEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetSmallHitEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "MediumHitEffect", "MediumHitEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetMediumHitEffect),
            "", PARTGROUP));

         AddProperty(new dtCore::ResourceActorProperty(*this, dtCore::DataType::PARTICLE_SYSTEM, "LargeHitEffect", "LargeHitEffect",
            dtCore::ResourceActorProperty::SetFuncType(this, &ViewerMaterialActor::SetLargeHitEffect),
            "", PARTGROUP));

         DT_REGISTER_PROPERTY(PhysicsParticleLookupStringOne, "Random physics particle system prototype name", RegHelperType, vislReg);
         DT_REGISTER_PROPERTY(PhysicsParticleLookupStringTwo, "Random physics particle system prototype name", RegHelperType, vislReg);
         DT_REGISTER_PROPERTY(PhysicsParticleLookupStringThree, "Random physics particle system prototype name", RegHelperType, vislReg);
         DT_REGISTER_PROPERTY(PhysicsParticleLookupStringFour, "Random physics particle system prototype name", RegHelperType, vislReg);
         DT_REGISTER_PROPERTY(PhysicsParticleLookupStringFive, "Random physics particle system prototype name", RegHelperType, vislReg);

      }

      // Creates the actor
      void ViewerMaterialActor::CreateDrawable()
      {
         SetDrawable(*new dtGame::GameActor(*this));
      }

      void ViewerMaterialActor::OnEnteredWorld()
       {
          SimCore::Components::ViewerMaterialComponent* materialComponent = NULL;
          GetGameManager()->GetComponentByName("ViewerMaterialComponent", materialComponent);
          if(materialComponent == NULL)
          {
             LOG_ERROR("materialComponent Is not initialized, make sure a new one was made before loading a map in");
          }
          else
             materialComponent->RegisterAMaterialWithComponent(this);
       }


   }
}
