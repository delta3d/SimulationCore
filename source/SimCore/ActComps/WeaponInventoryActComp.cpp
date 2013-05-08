/* -*-c++-*-
 * SimulationCore
 * Copyright 2012, David Guthrie
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
 * David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#include <SimCore/ActComps/WeaponInventoryActComp.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/WeaponFlashActor.h>
#include <dtUtil/nodecollector.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/propertymacros.h>
#include <dtCore/project.h>

#include <dtCore/arrayactorpropertycomplex.h>
#include <dtCore/propertycontaineractorproperty.h>

#include <osgSim/DOFTransform>
#include <SimCore/Actors/IGActor.h>

#include <dtGame/cascadingdeleteactorcomponent.h>

#include <cmath>

#include <algorithm>

namespace SimCore
{

   namespace ActComps
   {

      //////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////
      WeaponInventoryActComp::WeaponDescription::WeaponDescription()
      : mWeaponPrototypeName("")
      , mShooterPrototypeName("")
      , mWeaponSwapRootNode("dof_gun_01")
      , mWeaponHotSpotNode("hotspot_01")
      {
         typedef dtDAL::PropertyRegHelper<WeaponInventoryActComp::WeaponDescription&, WeaponInventoryActComp::WeaponDescription> PropRegType;
         PropRegType propRegHelper(*this, this, "WeaponDescription");

         DT_REGISTER_PROPERTY(
            WeaponSwapRootNode,
            "The name of the group node to look for to use as the root of the weapon.  Set this to the hotspot name if your weapon actor has no mesh.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
            WeaponPrototypeName,
            "The name of the weapon prototype actor to create.  If you want the weapon to have a mesh to swap, assign it to this actor prototype.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
            ShooterPrototypeName,
            "The name of the shooter prototype actor to create",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY(
            WeaponHotSpotNode,
            "The name of the node to look for to use as the fire position of the weapon.  If the weapon actor has a mesh, it will look there, otherwise on this actor.",
            PropRegType, propRegHelper);


         DT_REGISTER_RESOURCE_PROPERTY(dtDAL::DataType::PARTICLE_SYSTEM,
                  FiringParticleSystem,
            "Firing Particle System",
            "The particle system to use for firing",
            PropRegType, propRegHelper);
      }

      WeaponInventoryActComp::WeaponDescription::~WeaponDescription()
      {
      }

      DT_IMPLEMENT_ACCESSOR(WeaponInventoryActComp::WeaponDescription, std::string, WeaponPrototypeName);
      DT_IMPLEMENT_ACCESSOR(WeaponInventoryActComp::WeaponDescription, std::string, ShooterPrototypeName);
      DT_IMPLEMENT_ACCESSOR(WeaponInventoryActComp::WeaponDescription, std::string, WeaponSwapRootNode);
      DT_IMPLEMENT_ACCESSOR(WeaponInventoryActComp::WeaponDescription, std::string, WeaponHotSpotNode);
      DT_IMPLEMENT_ACCESSOR(WeaponInventoryActComp::WeaponDescription, dtCore::ResourceDescriptor, FiringParticleSystem);

      //////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////

      SimCore::Actors::MunitionParticlesActorProxy* WeaponInventoryActComp::WeaponData::GetShooter()
      {
         SimCore::Actors::WeaponActor* weaponDraw = NULL;
         mWeapon->GetDrawable(weaponDraw);
         return weaponDraw->GetShooter();
      }

      //////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////

      const dtGame::ActorComponent::ACType WeaponInventoryActComp::TYPE("WeaponInventoryActComp");

      //////////////////////////////////////////////////////////////////////////
      WeaponInventoryActComp::WeaponInventoryActComp()
      : dtGame::ActorComponent(TYPE)
      , mCurrentWeapon()
      , mWeapons()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponInventoryActComp::~WeaponInventoryActComp()
      {
         mWeapons.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUPNAME("Weapon Inventory");
         typedef dtDAL::PropertyRegHelper<WeaponInventoryActComp&, WeaponInventoryActComp> PropRegType;
         PropRegType propRegHelper(*this, this, GROUPNAME);

         typedef dtCore::ArrayActorPropertyComplex<dtCore::RefPtr<WeaponInventoryActComp::WeaponDescription> > WeaponDescPropType;
         dtCore::RefPtr<WeaponDescPropType> arrayProp =
                  new WeaponDescPropType
                     ("Weapons", "Weapons",
                      WeaponDescPropType::SetFuncType(this, &WeaponInventoryActComp::SetWeaponDescription),
                      WeaponDescPropType::GetFuncType(this, &WeaponInventoryActComp::GetWeaponDescription),
                      WeaponDescPropType::GetSizeFuncType(this, &WeaponInventoryActComp::GetNumWeaponDescriptions),
                      WeaponDescPropType::InsertFuncType(this, &WeaponInventoryActComp::InsertWeaponDescription),
                      WeaponDescPropType::RemoveFuncType(this, &WeaponInventoryActComp::RemoveWeaponDescription),
                      "Tests the array with a property container by having a container that refers back to itself recursively.",
                      GROUPNAME
                     );

         dtCore::RefPtr<dtCore::BasePropertyContainerActorProperty> propContainerProp =
                  new dtCore::SimplePropertyContainerActorProperty<WeaponInventoryActComp::WeaponDescription>("NestedPropContainer",
                           "Nested Property Container",
                           dtCore::SimplePropertyContainerActorProperty<WeaponInventoryActComp::WeaponDescription>::SetFuncType(arrayProp.get(), &WeaponDescPropType::SetCurrentValue),
                           dtCore::SimplePropertyContainerActorProperty<WeaponInventoryActComp::WeaponDescription>::GetFuncType(arrayProp.get(), &WeaponDescPropType::GetCurrentValue),
                           "", GROUPNAME);

         arrayProp->SetArrayProperty(*propContainerProp);
         AddProperty(arrayProp);

      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         CreateWeaponsFromDescriptions();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::CreateWeapon(dtCore::RefPtr<WeaponDescription>& wd)
      {
         CreateAndAddWeapon(*wd, false);
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::CreateWeaponsFromDescriptions()
      {
         ForEachWeaponDescription(dtUtil::MakeFunctor(&WeaponInventoryActComp::CreateWeapon, this));
         SelectNextWeapon();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::OnRemovedFromWorld()
      {
         BaseClass::OnRemovedFromWorld();
         mWeapons.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponInventoryActComp::WeaponData* WeaponInventoryActComp::CreateAndAddWeapon(WeaponDescription& wd, bool makeCurrent)
      {
         SimCore::Actors::IGActor* owner;
         GetOwner(owner);
         if (owner == NULL)
         {
            return NULL;
         }

         dtGame::GameManager* gm = owner->GetGameActorProxy().GetGameManager();


         dtCore::RefPtr<SimCore::Actors::WeaponActorProxy> weaponActor;
         gm->CreateActorFromPrototype(wd.GetWeaponPrototypeName(), weaponActor);

         if (!weaponActor.valid())
         {
            LOG_ERROR("No weapon actor prototype named \"" + wd.GetWeaponPrototypeName() + "\" could be found.");
            return false;
         }

         SimCore::Actors::WeaponActor* outWeapon = NULL;
         weaponActor->GetActor(outWeapon);

         // Place the weapon into the world
         gm->AddActor(*weaponActor, false, false);
         outWeapon->SetOwner(&owner->GetGameActorProxy());
         outWeapon->Emancipate();

         float shotVelocity = 0.0f;

         if (!owner->IsRemote())
         {

            dtCore::RefPtr<SimCore::Actors::MunitionParticlesActorProxy> shooterActor;
            gm->CreateActorFromPrototype(wd.GetShooterPrototypeName(), shooterActor);

            if (!shooterActor.valid())
            {
               LOG_ERROR("No shooter actor prototype named \"" + wd.GetShooterPrototypeName() + "\" could be found.");
               return false;
            }

            SimCore::Actors::MunitionParticlesActor* shooter = NULL;
            shooterActor->GetActor(shooter);

            // Place the shooter into the world
            gm->AddActor(*shooterActor, false, false);
            shooter->Emancipate();

            // Delete the weapons when the actor is deleted.
            dtGame::CascadingDeleteActorComponent::Connect(owner->GetGameActorProxy(), *weaponActor);

            dtGame::CascadingDeleteActorComponent::Connect(*weaponActor, *shooterActor);

            outWeapon->SetShooter(shooterActor.get());

            // Set other properties of the particle system
            shooter->SetWeapon(*outWeapon);

            // HACK: temporary play values
            shooter->SetFrequencyOfTracers(outWeapon->GetTracerFrequency());

            shotVelocity = (shooter->GetLinearVelocityStartMax().length() + shooter->GetLinearVelocityStartMin().length()) / 2.0;

            // Attach the shooter to the weapon's flash point
            // the interesting part about this is that if the weapon does not have a mesh
            // it will just add  to the root and the weapon will be at the hotspot no trouble.
            outWeapon->AddChild(shooter, wd.GetWeaponHotSpotNode());

            if (!wd.GetFiringParticleSystem().IsEmpty())
            {
               std::string particleEffectFile = dtCore::Project::GetInstance().GetResourcePath(wd.GetFiringParticleSystem());
               // Create the flash effect for the weapon
               SimCore::Actors::WeaponFlashActor* flashDrawable = NULL;
               dtCore::RefPtr<SimCore::Actors::WeaponFlashActorProxy> flashActor;
               gm->CreateActor(*SimCore::Actors::EntityActorRegistry::WEAPON_FLASH_ACTOR_TYPE, flashActor);
               flashActor->GetDrawable(flashDrawable);
               flashDrawable->SetParticleEffect(particleEffectFile);
               outWeapon->SetFlashActorProxy(flashActor);
               outWeapon->AddChild(flashDrawable, wd.GetWeaponHotSpotNode());
               gm->AddActor(*flashActor, false, false);
               dtGame::CascadingDeleteActorComponent::Connect(*weaponActor, *flashActor);
            }
         }

         mWeapons.push_back(new WeaponData);
         dtCore::RefPtr<WeaponData>& weaponData = mWeapons.back();
         weaponData->mShotVelocity = shotVelocity;
         weaponData->mDescription = &wd;
         weaponData->mWeapon = weaponActor;

         if (makeCurrent)
         {
            SelectWeapon(weaponData);
         }

         return weaponData.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::AimWeapon(const osg::Vec3& target, bool aimShooter)
      {
         if (mCurrentWeapon != NULL)
         {
            dtCore::Transformable* weaponToAim = NULL;

            if (aimShooter)
            {
               mCurrentWeapon->GetShooter()->GetDrawable(weaponToAim);
            }
            else
            {
               mCurrentWeapon->mWeapon->GetDrawable(weaponToAim);
            }

            if (weaponToAim != NULL)
            {
               dtCore::Transform xform;
               weaponToAim->GetTransform(xform);
               
               double g = 9.81f;
               double v = mCurrentWeapon->mShotVelocity;
               double v2 = v * v;
               osg::Vec3 pos = xform.GetTranslation();
               osg::Vec2 vec2(target.x() - pos.x(), target.y() - pos.y()); 
               double distxy = vec2.length(); 
               double z = target.z() - pos.z();
               double s =  (v2 * v2) - (g * ((g * distxy * distxy) + (2 * z * v * v)));
               osg::Vec3 targetNew = target;
               if (s > 0.0)
               {
                  double sqrtS =  std::sqrt(s);
                  double shotHeight = ((v2) - sqrtS) / g;
                  double shotHeight2 = ((v2) + sqrtS) / g;


                  targetNew.z() = pos.z() + (shotHeight < shotHeight2 ? shotHeight : shotHeight2);
               }
               xform.Set(xform.GetTranslation(), targetNew, osg::Vec3(0.0, 0.0, 1.0));
               weaponToAim->SetTransform(xform);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::ClearWeaponAiming(const osg::Vec3& target, bool useShooter)
      {
         if (mCurrentWeapon != NULL)
         {
            dtCore::Transformable* weaponToAim = NULL;
            
            if (useShooter)
            {
               mCurrentWeapon->GetShooter()->GetDrawable(weaponToAim);
            }
            else
            {
               mCurrentWeapon->mWeapon->GetDrawable(weaponToAim);
            }

            if (weaponToAim != NULL)
            {
               // Identity Transform
               dtCore::Transform xform;
               weaponToAim->SetTransform(xform);
            }
         }
      }


      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::SelectNextWeapon()
      {
         if (mCurrentWeapon == NULL && !mWeapons.empty())
         {
            SelectWeapon(mWeapons[0]);
         }

         if(mWeapons.size() > 1)
         {
            int index = FindWeaponIndex(mCurrentWeapon);
            if(index >= 0)
            {
               ++index;
               if(unsigned(index) >= mWeapons.size())
               {
                  index = 0;
               }
            }
            SelectWeapon(mWeapons[index]);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::SelectPreviousWeapon()
      {
         if (mCurrentWeapon == NULL && !mWeapons.empty())
         {
            SelectWeapon(mWeapons[0]);
         }

         if(mWeapons.size() > 1)
         {
            int index = FindWeaponIndex(mCurrentWeapon);
            if(index >= 0)
            {
               if(index >= 1)
               {
                  --index;
               }
               else 
               {
                  index = mWeapons.size() - 1;
               }
               SelectWeapon(mWeapons[index]);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::SelectWeapon( const std::string& weaponName )
      {
         SelectWeapon(FindWeapon(weaponName));
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::SelectWeapon(WeaponData* wd)
      {
         SimCore::Actors::IGActor* owner = NULL;
         GetOwner(owner);
         if (owner == NULL)
         {
            return;
         }

         if (mCurrentWeapon.valid() && mCurrentWeapon->mWeapon.valid())
         {
            StopFiring();
            owner->RemoveChild(mCurrentWeapon->mWeapon->GetDrawable());
            mCurrentWeapon = NULL;
         }

         if (wd != NULL && wd->mWeapon != NULL && wd->mDescription != NULL)
         {
            owner->AddChild(wd->mWeapon->GetDrawable(), wd->mDescription->GetWeaponSwapRootNode());
            mCurrentWeapon = wd;
         }
      }


      //////////////////////////////////////////////////////////////////////////
      bool WeaponInventoryActComp::HasWeapon( const std::string& weaponName ) const
      {
         return FindWeapon(weaponName) != NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponInventoryActComp::WeaponData* WeaponInventoryActComp::GetCurrentWeapon() const
      {
         return mCurrentWeapon.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& WeaponInventoryActComp::GetCurrentWeaponName() const
      {
         if (mCurrentWeapon.valid())
         {
            return mCurrentWeapon->mDescription->GetWeaponPrototypeName();
         }
         
         static std::string emptyString;
         return emptyString;
      }

      struct CompareWeaponByName
      {
         CompareWeaponByName(const std::string& str): mName(str) {}

         bool operator()(const dtCore::RefPtr<WeaponInventoryActComp::WeaponData>& wp)
         {
            return mName == wp->mDescription->GetWeaponPrototypeName();
         }

         const std::string& mName;
      };

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::RemoveWeapon( const std::string& weaponName )
      {

         if (mCurrentWeapon.valid() && mCurrentWeapon->mDescription->GetWeaponPrototypeName() == weaponName)
         {
            SimCore::Actors::IGActor* owner = NULL;
            GetOwner(owner);
            if (owner == NULL)
            {
               return;
            }

            owner->RemoveChild(mCurrentWeapon->mWeapon->GetDrawable());
         }

         CompareWeaponByName compareWeaponByName(weaponName);
         mWeapons.erase(std::remove_if(mWeapons.begin(), mWeapons.end(), compareWeaponByName), mWeapons.end());
      }

      //////////////////////////////////////////////////////////////////////////
      WeaponInventoryActComp::WeaponData* WeaponInventoryActComp::FindWeapon( const std::string& weaponName )
      {
         CompareWeaponByName compareWeaponByName(weaponName);
         WeaponVector::iterator i = std::find_if(mWeapons.begin(), mWeapons.end(), compareWeaponByName);
         if (i != mWeapons.end())
         {
            return i->get();
         }

         return NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const WeaponInventoryActComp::WeaponData* WeaponInventoryActComp::FindWeapon( const std::string& weaponName ) const
      {
         CompareWeaponByName compareWeaponByName(weaponName);
         WeaponVector::const_iterator i = std::find_if(mWeapons.begin(), mWeapons.end(), compareWeaponByName);
         if (i != mWeapons.end())
         {
            return i->get();
         }

         return NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      int WeaponInventoryActComp::FindWeaponIndex(const std::string& weaponName) const
      {
         for(unsigned i = 0; i < mWeapons.size(); ++i)
         {
            if(mWeapons[i]->mDescription->GetWeaponPrototypeName() == weaponName)
            {
               return i;
            }
         }

         return -1;
      }

      //////////////////////////////////////////////////////////////////////////
      int WeaponInventoryActComp::FindWeaponIndex( const WeaponData* wp ) const
      {
         for(unsigned i = 0; i < mWeapons.size(); ++i)
         {
            if(mWeapons[i] == wp)
            {
               return i;
            }
         }

         return -1;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned WeaponInventoryActComp::GetNumWeapons() const
      {
         return mWeapons.size();
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::StartFiring()
      {
         if (mCurrentWeapon.valid() && mCurrentWeapon->mWeapon.valid())
         {
            SimCore::Actors::WeaponActor* weaponDrawable;
            mCurrentWeapon->mWeapon->GetDrawable(weaponDrawable);
            weaponDrawable->SetTriggerHeld(true);
            //if (weaponDrawable->GetFireRate() <= 0.0)
            //{
               weaponDrawable->Fire();
            //}
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void WeaponInventoryActComp::StopFiring()
      {
         if (mCurrentWeapon.valid() && mCurrentWeapon->mWeapon.valid())
         {
            SimCore::Actors::WeaponActor* weaponDrawable;
            mCurrentWeapon->mWeapon->GetDrawable(weaponDrawable);
            weaponDrawable->SetTriggerHeld(false);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool WeaponInventoryActComp::IsFiring() const
      {
         bool result = false;
         if (mCurrentWeapon.valid() && mCurrentWeapon->mWeapon.valid())
         {
            SimCore::Actors::WeaponActor* weaponDrawable;
            mCurrentWeapon->mWeapon->GetDrawable(weaponDrawable);
            result = weaponDrawable->IsTriggerHeld();
         }
         return result;
      }

      DT_IMPLEMENT_ARRAY_ACCESSOR(WeaponInventoryActComp, dtCore::RefPtr<WeaponInventoryActComp::WeaponDescription>, WeaponDescription, WeaponDescriptions, new WeaponInventoryActComp::WeaponDescription);

   }

}
