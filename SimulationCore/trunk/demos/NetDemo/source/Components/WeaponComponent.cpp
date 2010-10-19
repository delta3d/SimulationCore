/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @author David Guthrie
*/

#include <Components/WeaponComponent.h>
#include <ActorRegistry.h>
#include <Actors/PlayerStatusActor.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <SimCore/Actors/MunitionParticlesActor.h>

namespace NetDemo
{

   const dtUtil::RefString WeaponComponent::DEFAULT_NAME("Weapon Component");

   //////////////////////////////////////////////////////////////////////////
   WeaponComponent::WeaponComponent(const dtUtil::RefString& name)
   : dtGame::GMComponent(name)
   , mWeaponIndex(0U)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   WeaponComponent::~WeaponComponent()
   {
   }

   //////////////////////////////////////////////////////////////
   void WeaponComponent::ProcessMessage(const dtGame::Message& message)
   {
      const dtGame::MessageType& msgType = message.GetMessageType();
      if (msgType == dtGame::MessageType::INFO_ACTOR_UPDATED)
      {
         HandleActorUpdateMessage(static_cast<const dtGame::ActorUpdateMessage&>(message));
      }
      else if (msgType == dtGame::MessageType::INFO_ACTOR_DELETED)
      {
         HandleActorDeleteMessage(message);
      }
      else if (msgType == dtGame::MessageType::INFO_MAP_UNLOAD_BEGIN)
      {
         mCurrentWeapon = NULL;
         mWeaponList.clear();
         mWeaponOwner = NULL;
      }
   }

   ////////////////////////////////////////////////////////////////////
   void WeaponComponent::HandleActorUpdateMessage(const dtGame::ActorUpdateMessage& updateMessage)
   {
      // PLAYER STATUS - if it's ours, then update our attached vehicle
      if (updateMessage.GetActorType() == NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE &&
         updateMessage.GetSource() == GetGameManager()->GetMachineInfo())
      {
         // Find the actor in the GM
         PlayerStatusActorProxy* playerProxy;
         GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId(), playerProxy);

         if (playerProxy == NULL) // Could be deleted or not fully created from partial
         {
            return;
         }

         PlayerStatusActor& playerStatus = playerProxy->GetActorAsPlayerStatus();

         // If we don't have a vehicle yet, or our current vehicle is different, then attach to it.
         if (!mWeaponOwner.valid() || mWeaponOwner->GetId() != playerStatus.GetAttachedVehicleID())
         {
            SimCore::Actors::BaseEntityActorProxy* owner = NULL;
            GetGameManager()->FindActorById(playerStatus.GetAttachedVehicleID(), owner);
            if (owner != NULL)
            {
               mWeaponOwner = owner;
            }
            else
            {
               mWeaponOwner = NULL;
            }

            if (mWeaponOwner.valid())
            {
               InitializeWeapons();
               SetCurrentWeaponIndex(0);
            }
         }

      }
   }

   //////////////////////////////////////////////////////////////////////////
   void WeaponComponent::HandleActorDeleteMessage(const dtGame::Message& msg)
   {
      if (mWeaponOwner.valid() && msg.GetAboutActorId() == mWeaponOwner->GetId())
      {
         DeleteWeapons();
         mWeaponOwner = NULL;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void WeaponComponent::StartFiring()
   {
      if (mCurrentWeapon.valid())
      {
         mCurrentWeapon->SetTriggerHeld(true);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void WeaponComponent::StopFiring()
   {
      if (mCurrentWeapon.valid())
      {
         mCurrentWeapon->SetTriggerHeld(false);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void WeaponComponent::DeleteWeapons()
   {
      for (unsigned i = 0; i < mWeaponList.size(); ++i)
      {
         GetGameManager()->DeleteActor(mWeaponList[i]->GetGameActorProxy());
      }
      mWeaponList.clear();
      mCurrentWeapon = NULL;
   }

   //////////////////////////////////////////////////////////////////////////
   void WeaponComponent::InitializeWeapons()
   {
      DeleteWeapons();

      SimCore::Actors::WeaponActor* curWeapon = NULL;

      // weapons -- for array iterator.
      // WEAPON 1
      CreateWeapon( "Weapon_MachineGun",
         "Particle_System_Weapon_GunWithTracer",
         "weapon_gun_flash.osg", curWeapon );
      if (curWeapon != NULL) { mWeaponList.push_back(curWeapon); }
      curWeapon = NULL;

      // WEAPON 2
      CreateWeapon( "Weapon_Grenade",
         "Particle_System_Weapon_Grenade",
         "weapon_gun_flash.osg", curWeapon );
      if (curWeapon != NULL) { mWeaponList.push_back(curWeapon); }
      curWeapon = NULL;

      mWeaponIndex = 0;
   }

   //////////////////////////////////////////////////////////////////////////
   bool WeaponComponent::CreateWeapon( const std::string& weaponName,
         const std::string& shooterName, const std::string& flashEffectFile,
            SimCore::Actors::WeaponActor*& outWeapon )
   {
      dtGame::GameManager* gm = GetGameManager();

      dtCore::RefPtr<SimCore::Actors::WeaponActorProxy> weaponProxy;
      gm->CreateActorFromPrototype(weaponName, weaponProxy);

      if (!weaponProxy.valid())
      {
         LOG_ERROR("No weapon actor prototype named \"" + weaponName + "\" could be found.");
         return false;
      }

      weaponProxy->GetActor(outWeapon);

      dtCore::RefPtr<SimCore::Actors::MunitionParticlesActorProxy> shooterProxy;
      gm->CreateActorFromPrototype(shooterName, shooterProxy);

      if (!shooterProxy.valid())
      {
         LOG_ERROR("No shooter actor prototype named \"" + shooterName + "\" could be found.");
         return false;
      }

      SimCore::Actors::MunitionParticlesActor* shooter = NULL;
      shooterProxy->GetActor(shooter);

      // Place the weapon into the world
      gm->AddActor(*weaponProxy, false, false);
      outWeapon->Emancipate();
      outWeapon->SetOwner(mWeaponOwner.get());
      // Place the shooter into the world
      gm->AddActor(*shooterProxy, false, false);
      shooter->Emancipate();

      outWeapon->SetShooter(shooterProxy.get());

      // Set other properties of the particle system
      shooter->SetWeapon(*outWeapon);

      // HACK: temporary play values
      shooter->SetFrequencyOfTracers(outWeapon->GetTracerFrequency());

      // Attach the shooter to the weapon's flash point
      outWeapon->AddChild(shooter, "dof_hotspot_01");

      // Create the flash effect for the weapon
      SimCore::Actors::WeaponFlashActor* flash = NULL;
      dtCore::RefPtr<SimCore::Actors::WeaponFlashActorProxy> flashProxy;
      gm->CreateActor(*SimCore::Actors::EntityActorRegistry::WEAPON_FLASH_ACTOR_TYPE, flashProxy);
      flashProxy->GetActor(flash);
      std::ostringstream flashFilePath;
      flashFilePath << "Particles/" << flashEffectFile;
      flash->SetParticleEffect(flashFilePath.str());
      outWeapon->SetFlashActor(flash);
      outWeapon->AddChild(flash, "dof_hotspot_01");
      gm->AddActor(*flashProxy, false, false);

      return true;
   }

   //////////////////////////////////////////////////////////////////////////////////
   SimCore::Actors::WeaponActor* WeaponComponent::GetCurrentWeapon()
   {
      return mCurrentWeapon.get();
   }

   ////////////////////////////////////////////////////////////////////////////////
   void WeaponComponent::SetCurrentWeapon(SimCore::Actors::WeaponActor* weapon)
   {
      StopFiring();
      // Clean up old weapon if there is already one set.

      if (mCurrentWeapon.valid())
      {
         // Turn off the weapon if its trigger is currently held.
         mCurrentWeapon->SetTriggerHeld( false );
      }

      // Make the change
      mCurrentWeapon = weapon;

      if( mCurrentWeapon.valid() )
      {
         // Set the owner
         if (mWeaponOwner.valid())
         {
            mCurrentWeapon->SetOwner(mWeaponOwner.get());
            SimCore::Actors::IGActor* owner = NULL;
            mWeaponOwner->GetActor(owner);
            //TODO: get rid if icky quoted string
            owner->AddChild(mCurrentWeapon.get(), "dof_gun_01");
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void WeaponComponent::SetCurrentWeaponIndex(unsigned index)
   {
      if (mWeaponIndex >= mWeaponList.size())
      {
         mWeaponIndex = mWeaponList.size();
         SetCurrentWeapon(NULL);
         return;
      }

      mWeaponIndex = index;
      SetCurrentWeapon(mWeaponList[mWeaponIndex].get());
   }

   ////////////////////////////////////////////////////////////////////////////////
   unsigned WeaponComponent::GetCurrentWeaponIndex() const
   {
      return mWeaponIndex;
   }

   ////////////////////////////////////////////////////////////////////////////////
   unsigned WeaponComponent::GetNumWeapons() const
   {
      return mWeaponList.size();
   }
}
