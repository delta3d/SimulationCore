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

#ifndef WEAPONCOMPONENT_H_
#define WEAPONCOMPONENT_H_

#include <dtGame/gmcomponent.h>
#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/WeaponActor.h>

#include <dtCore/refptr.h>
#include <vector>

namespace dtGame
{
   class ActorUpdateMessage;
}

namespace NetDemo
{

   class WeaponComponent: public dtGame::GMComponent
   {
   public:
      static const dtUtil::RefString DEFAULT_NAME;
      WeaponComponent(const dtUtil::RefString& name = DEFAULT_NAME);

      virtual void ProcessMessage(const dtGame::Message&);

      void InitializeWeapons();
      void DeleteWeapons();

      bool CreateWeapon(const std::string& weaponName,
            const std::string& shooterName, const std::string& flashEffectFile,
            SimCore::Actors::WeaponActor*& outWeapon);

      SimCore::Actors::WeaponActor* GetCurrentWeapon();
      void SetCurrentWeaponIndex(unsigned index);
      unsigned GetCurrentWeaponIndex() const;
      unsigned GetNumWeapons() const;

      void StartFiring();
      void StopFiring();
   protected:
      void SetCurrentWeapon(SimCore::Actors::WeaponActor* weapon);
      void HandleActorUpdateMessage(const dtGame::ActorUpdateMessage& updateMessage);
      void HandleActorDeleteMessage(const dtGame::Message&);

      virtual ~WeaponComponent();

      std::vector<dtCore::RefPtr<SimCore::Actors::WeaponActor> > mWeaponList;  // all weapons
      unsigned mWeaponIndex;
      dtCore::RefPtr<SimCore::Actors::WeaponActor> mCurrentWeapon;
      dtCore::RefPtr<SimCore::Actors::BaseEntityActorProxy> mWeaponOwner;
   };

}

#endif /* WEAPONCOMPONENT_H_ */
