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
#ifndef _WEAPON_FLASH_ACTOR_H_
#define _WEAPON_FLASH_ACTOR_H_

#include <SimCore/Actors/IGActor.h>
#include <dtGame/gameactor.h>
#include <SimCore/Actors/VolumetricLine.h>

namespace dtCore
{
   class ParticleSystem;
}

namespace SimCore
{
   namespace Actors
   {
      class VolumetricLine;

      //////////////////////////////////////////////////////////////////////////
      // Weapon Flash ( Non-Actor Drawable )
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT WeaponFlash : public VolumetricLine
      {
         public:

            // Constructor
            WeaponFlash( float lineLength, float lineThickness,
               const std::string& shaderName, const std::string& shaderGroup );

            void SetVisible( bool visible );
            bool IsVisible() const { return mVisible; }

         protected:

            // Destructor
            virtual ~WeaponFlash() {}

         private:

            bool mVisible;

      };



      //////////////////////////////////////////////////////////////////////////
      // Weapon Flash Actor
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT WeaponFlashActor : public dtGame::GameActor
      {
         public:

            // Constructor
            WeaponFlashActor( dtGame::GameActorProxy &proxy );

            void SetParticleEffect( const std::string& filePath ) { mParticleFile = filePath; }

            void SetFlashTime( float lifeTime ) { mFlashTime = lifeTime; }
            float GetFlashTime() const { return mFlashTime; }

            void SetLength( float lineLength );
            float GetLength() const { return mLength; }

            void SetThickness( float lineThickness );
            float GetThickness() const { return mThickness; }

            void SetShaderName( const std::string& shaderName );
            std::string GetShaderName() const { return mShaderName; }

            void SetShaderGroup( const std::string& shaderGroup );
            std::string GetShaderGroup() const { return mShaderGroup; }

            void SetVisible( bool visible );
            bool IsVisible() const { return mVisible; }

            virtual void OnEnteredWorld();

            virtual void OnTickLocal( const dtGame::TickMessage& tickMessage );

            void SetShader( const std::string& shaderName, const std::string& shaderGroup );

         protected:

            // Destructor
            virtual ~WeaponFlashActor();

            void UpdateDrawable();

            bool LoadParticles();

         private:

            bool mVisible;
            float mCurTime; // current age time of the flash; increases
            float mFlashTime; // life time of flash in seconds
            float mLength;
            float mThickness;
            std::string mShaderName;
            std::string mShaderGroup;
            std::string mParticleFile;
            dtCore::RefPtr<WeaponFlash> mFlash;
            dtCore::RefPtr<dtCore::ParticleSystem> mParticles;

      };



      //////////////////////////////////////////////////////////////////////////
      // Weapon Flash Actor Proxy
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT WeaponFlashActorProxy : public dtGame::GameActorProxy
      {
         public:

            static const std::string CLASS_NAME;

            // Constructor
            WeaponFlashActorProxy();

            // Creates the actor
            void CreateActor() { SetActor(*new WeaponFlashActor(*this)); }

            // Adds the properties associated with this actor
            void BuildPropertyMap();

         protected:

            // Destructor
            virtual ~WeaponFlashActorProxy();

         private:

      };
   }
}

#endif
