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
 * @author Chris Rodgers
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/particlesystem.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <SimCore/Components/RenderingSupportComponent.h>


namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // Weapon Flash Code
      //////////////////////////////////////////////////////////////////////////
      WeaponFlash::WeaponFlash( float lineLength, float lineThickness,
         const std::string& shaderName, const std::string& ShaderGroup )
         : VolumetricLine(lineLength,lineThickness,shaderName,ShaderGroup),
         mVisible(true)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlash::SetVisible( bool visible )
      {
         mVisible = visible;
         GetOSGNode()->setNodeMask( visible ? 0xFFFFFFFF : 0 );
      }



      //////////////////////////////////////////////////////////////////////////
      // Weapon Flash Actor Code
      //////////////////////////////////////////////////////////////////////////
      WeaponFlashActor::WeaponFlashActor(dtGame::GameActorProxy &proxy)
         : dtGame::GameActor(proxy),
         mVisible(true),
         mCurTime(0.0f),
         mFlashTime(0.5f),
         mLength(1.0f),
         mThickness(0.3f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponFlashActor::~WeaponFlashActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponFlashActor::LoadParticles()
      {
         if( mParticleFile.empty() ) { return false; }

         dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();

         if( gm == NULL ) { return false; }

         mParticles = new dtCore::ParticleSystem("Flash");
         mParticles->SetParentRelative( true );
         mParticles->LoadFile( mParticleFile );
		 
		 GetOSGNode()->setNodeMask(SimCore::Components::RenderingSupportComponent::DISABLE_SHADOW_NODE_MASK);
         AddChild( mParticles.get() );

         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::OnEnteredWorld()
      {
         dtGame::GameActor::OnEnteredWorld();

         if( ! mParticles.valid() )
         {
            LoadParticles();
         }

         GetGameActorProxy().RegisterForMessages(
            dtGame::MessageType::TICK_LOCAL,
            dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE );
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::UpdateDrawable()
      {
         if( mFlash.valid() )
         {
            mFlash->SetLengthAndThickness( mLength, mThickness );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::SetLength( float lineLength )
      {
         mLength = lineLength;
         UpdateDrawable();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::SetThickness( float lineThickness )
      {
         mThickness = lineThickness;
         UpdateDrawable();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::SetShader( const std::string& shaderName, const std::string& shaderGroup )
      {
         WeaponFlash* oldFlash = mFlash.get();

         std::string flashName( "WeaponFlash" );
         mFlash = new WeaponFlash( mLength, mThickness, shaderName, shaderGroup );
         mFlash->SetName( flashName );

         if( oldFlash != NULL ) { RemoveChild( oldFlash ); }

         AddChild( mFlash.get() );
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::SetShaderName( const std::string& shaderName )
      {
         mShaderName = shaderName;

         if( ! mShaderName.empty() && ! mShaderGroup.empty() )
         {
            SetShader( mShaderName, mShaderGroup );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::SetShaderGroup( const std::string& shaderGroup )
      {
         mShaderGroup = shaderGroup;

         if( ! mShaderName.empty() && ! mShaderGroup.empty() )
         {
            SetShader( mShaderName, mShaderGroup );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::SetVisible( bool visible )
      {
         mVisible = visible;
         GetOSGNode()->setNodeMask( visible ? 0xFFFFFFFF : 0 );

         if( mVisible ) { mCurTime = 0.0f; }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         float timeDelta = tickMessage.GetDeltaSimTime();

         if( mFlashTime > 0.0 && mCurTime < mFlashTime )
         {
            mCurTime += timeDelta;
            if( mCurTime >= mFlashTime )
            {
               SetVisible( false );
            }
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // Weapon Flash Actor Proxy Code
      //////////////////////////////////////////////////////////////////////////
      const std::string WeaponFlashActorProxy::CLASS_NAME("SimCore::Actors::WeaponFlashActor");

      //////////////////////////////////////////////////////////////////////////
      WeaponFlashActorProxy::WeaponFlashActorProxy()
      {
         SetClassName( WeaponFlashActorProxy::CLASS_NAME );
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponFlashActorProxy::~WeaponFlashActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponFlashActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         WeaponFlashActor* actor = NULL;
         GetActor(actor);

         AddProperty(new dtDAL::BooleanActorProperty("Visible", "Visible",
            dtDAL::BooleanActorProperty::SetFuncType( actor, &WeaponFlashActor::SetVisible),
            dtDAL::BooleanActorProperty::GetFuncType( actor, &WeaponFlashActor::IsVisible),
            "Set the visibility to true to execute the flash"));

         AddProperty(new dtDAL::FloatActorProperty("Flash Time", "Flash Time",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponFlashActor::SetFlashTime),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponFlashActor::GetFlashTime),
            "The life time the flash in seconds. Negative values will make the flash hold its visibility."));

         AddProperty(new dtDAL::FloatActorProperty("Length", "Length",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponFlashActor::SetLength),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponFlashActor::GetLength),
            "The length of the flash effect in meters."));

         AddProperty(new dtDAL::FloatActorProperty("Thickness", "Thickness",
            dtDAL::FloatActorProperty::SetFuncType( actor, &WeaponFlashActor::SetThickness),
            dtDAL::FloatActorProperty::GetFuncType( actor, &WeaponFlashActor::GetThickness),
            "The thickness of the flash effect in meters."));

         AddProperty(new dtDAL::StringActorProperty("Shader Name","Shader Name",
            dtDAL::StringActorProperty::SetFuncType( actor, &WeaponFlashActor::SetShaderName),
            dtDAL::StringActorProperty::GetFuncType( actor, &WeaponFlashActor::GetShaderName),
            "The name of the volumetric line shader to be used."));

         AddProperty(new dtDAL::StringActorProperty("Shader Group","Shader Group",
            dtDAL::StringActorProperty::SetFuncType( actor, &WeaponFlashActor::SetShaderGroup),
            dtDAL::StringActorProperty::GetFuncType( actor, &WeaponFlashActor::GetShaderGroup),
            "The group name of the volumetric line shader to be used."));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Particle Effect", "Particle Effect",
            dtDAL::ResourceActorProperty::SetFuncType( actor, &WeaponFlashActor::SetParticleEffect),
            "The particle system that will represent the flash effect."));
      }

   }
}
