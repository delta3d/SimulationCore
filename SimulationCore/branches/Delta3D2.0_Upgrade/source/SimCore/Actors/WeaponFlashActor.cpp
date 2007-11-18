/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/particlesystem.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/basemessages.h>

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
      void WeaponFlashActor::TickLocal( const dtGame::Message& tickMessage )
      {
         const dtGame::TickMessage& tickMsg = dynamic_cast<const dtGame::TickMessage&> (tickMessage);
         float timeDelta = tickMsg.GetDeltaSimTime();

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

         WeaponFlashActor& actor = static_cast<WeaponFlashActor&>(GetGameActor());

         AddProperty(new dtDAL::BooleanActorProperty("Visible", "Visible",
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetVisible),
            dtDAL::MakeFunctorRet( actor, &WeaponFlashActor::IsVisible),
            "Set the visibility to true to execute the flash"));

         AddProperty(new dtDAL::FloatActorProperty("Flash Time", "Flash Time",
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetFlashTime),
            dtDAL::MakeFunctorRet( actor, &WeaponFlashActor::GetFlashTime),
            "The life time the flash in seconds. Negative values will make the flash hold its visibility."));

         AddProperty(new dtDAL::FloatActorProperty("Length", "Length",
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetLength),
            dtDAL::MakeFunctorRet( actor, &WeaponFlashActor::GetLength),
            "The length of the flash effect in meters."));

         AddProperty(new dtDAL::FloatActorProperty("Thickness", "Thickness",
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetThickness),
            dtDAL::MakeFunctorRet( actor, &WeaponFlashActor::GetThickness),
            "The thickness of the flash effect in meters."));

         AddProperty(new dtDAL::StringActorProperty("Shader Name","Shader Name",
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetShaderName),
            dtDAL::MakeFunctorRet( actor, &WeaponFlashActor::GetShaderName),
            "The name of the volumetric line shader to be used."));

         AddProperty(new dtDAL::StringActorProperty("Shader Group","Shader Group",
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetShaderGroup),
            dtDAL::MakeFunctorRet( actor, &WeaponFlashActor::GetShaderGroup),
            "The group name of the volumetric line shader to be used."));

         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Particle Effect", "Particle Effect", 
            dtDAL::MakeFunctor( actor, &WeaponFlashActor::SetParticleEffect),
            "The particle system that will represent the flash effect."));
      }

   }
}
