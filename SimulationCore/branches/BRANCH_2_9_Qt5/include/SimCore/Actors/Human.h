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
 * @author David Guthrie
 */
#ifndef HUMAN_H_
#define HUMAN_H_

#include <string>
#include <map>

#include <SimCore/Export.h>
#include <SimCore/Actors/BaseEntity.h>

#include <dtAI/statevariable.h>
#include <dtAI/operator.h>
#include <dtAI/planner.h>
#include <dtAI/plannerhelper.h>

#include <dtUtil/enumeration.h>

#include <dtCore/refptr.h>
#include <dtAnim/animationhelper.h>
#include <dtCore/resourcedescriptor.h>

namespace dtUtil
{
   class Log;
}

namespace dtAnim
{
   class HumanOperator;
   class AnimationOperators;
   class WeaponStateEnum;
   class BasicStanceEnum;
}

namespace osg
{
   class Node;
}

namespace SimCore
{
   namespace Actors
   {

      class SIMCORE_EXPORT HumanActorProxy: public BaseEntityActorProxy
      {
      public:
         typedef BaseEntityActorProxy BaseClass;

         static const dtUtil::RefString PROPERTY_SKELETAL_MESH;
         static const dtUtil::RefString PROPERTY_WEAPON_MESH;
         static const dtUtil::RefString PROPERTY_STANCE;
         static const dtUtil::RefString PROPERTY_PRIMARY_WEAPON_STATE;
         static const dtUtil::RefString PROPERTY_MIN_RUN_VELOCITY;
         static const dtUtil::RefString PROPERTY_FULL_RUN_VELOCITY;
         static const dtUtil::RefString PROPERTY_WALK_ANIMATION_SPEED;

         class SIMCORE_EXPORT StanceEnum: public dtUtil::Enumeration
         {
         DECLARE_ENUM(StanceEnum)
            ;
         public:
            static StanceEnum NOT_APPLICABLE;
            static StanceEnum UPRIGHT_STANDING;
            static StanceEnum UPRIGHT_WALKING;
            static StanceEnum UPRIGHT_RUNNING;
            static StanceEnum KNEELING;
            static StanceEnum PRONE;
            static StanceEnum CRAWLING;
            static StanceEnum SWIMMING;
            static StanceEnum PARACHUTING;
            static StanceEnum JUMPING;
            static StanceEnum SITTING;
            static StanceEnum SQUATTING;
            static StanceEnum CROUCHING;
            static StanceEnum WADING;

            dtAnim::BasicStanceEnum& GetAssocBasicStanceEnum();
         private:
            StanceEnum(const std::string& name, dtAnim::BasicStanceEnum& assocBasicStance);
            dtAnim::BasicStanceEnum* mAssocBasicStance;
         };


         HumanActorProxy();
         /**
          * Build the properties common to all human objects
          */
         /*virtual*/
         void BuildPropertyMap();

         /// Build the invokables
         /*virtual*/
         void BuildInvokables();

         /*virtual*/
         void BuildActorComponents();

         /*virtual*/
         void CreateDrawable();
      protected:
         virtual ~HumanActorProxy();
      };

      class SIMCORE_EXPORT Human: public BaseEntity
      {
      public:
         typedef BaseEntity BaseClass;

         ///setting this config option will force a weapon to be shown if one exists
         static const dtUtil::RefString CONFIG_ALWAYS_SHOW_WEAPON;

         Human(dtGame::GameActorProxy& parent);

         /// Changes the human's stance
         void SetStance(HumanActorProxy::StanceEnum& stance);
         /// @return the human's stance
         HumanActorProxy::StanceEnum& GetStance() const;

         /// Changes the human's first weapon state
         void SetPrimaryWeaponState(dtAnim::WeaponStateEnum& weaponState);
         /// @return the human's first weapon state
         dtAnim::WeaponStateEnum& GetPrimaryWeaponState() const;

         /// Overridden to update the state on the planner.
         /*override*/ void SetDamageState(BaseEntityActorProxy::DamageStateEnum& damageState);

         /// @return the numeric id of the weapon mesh within the skeletal mesh.
         const std::string& GetWeaponMeshName() const
         {
            return mWeaponMeshName;
         }
         /// Sets the numeric id of the weapon mesh within the skeletal mesh.
         void SetWeaponMeshName(const std::string& meshName)
         {
            mWeaponMeshName = meshName;
         }

         /*virtual*/
         void OnEnteredWorld();
         /*virtual*/
         void OnTickRemote(const dtGame::TickMessage& tickRemote);
         /*virtual*/
         void OnTickLocal(const dtGame::TickMessage& tickLocal);

         /// @return true if this Human should be visible based on the options given.
         bool ShouldBeVisible(const SimCore::VisibilityOptions& options);

         void ExecuteAction(const dtUtil::RefString& animatableName, HumanActorProxy::StanceEnum& stance = HumanActorProxy::StanceEnum::UPRIGHT_STANDING);

         /// these have to be public so that they can be connected by the HumanActorProxy
         virtual void SkeletalMeshLoadCallback(dtAnim::AnimationHelper*);
         virtual void SkeletalMeshUnloadCallback(dtAnim::AnimationHelper*);

         /// This is called by the walk/run animation to get the value for the walk speed.  Override to change the way it works.
         virtual float CalculateWalkingSpeed() const;

         /// This is the inherent speed of the walk animation.
         DT_DECLARE_ACCESSOR(float, WalkAnimationSpeed);

      protected:
         virtual ~Human();

         void UpdateWeapon();
         bool GetContainsWeaponName(const std::vector<std::string>& vec, const std::string& meshName) const;

         void SetupWalkRunBlend(dtAnim::AnimationHelper* helper, const dtUtil::RefString& OpName,
               const std::vector<dtUtil::RefString>& nameWalkOptions, const std::string& newWalkAnimName,
               const std::vector<dtUtil::RefString>& nameRunOptions, const std::string& newRunAnimName,
               const std::vector<dtUtil::RefString>& nameStandOptions, const std::string& newStandAnimName, float walkSpeed,
               float runSpeed);
      private:
         /// Apply the effects of the operator, and get the animatable, if any, associated with it.
         const dtAnim::Animatable* ApplyOperatorAndGetAnimatable(const dtAI::Operator& op);

         std::string mIdleAnimatableName;
         std::string mRunWalkAnimatableName;
         std::string mWeaponMeshName;

         dtCore::RefPtr<osg::Node> mModelNode;

         HumanActorProxy::StanceEnum* mStance;
         dtAnim::WeaponStateEnum* mPrimaryWeaponStateEnum;

         dtUtil::Log& mLogger;

         bool mModelLoadedAndWaiting;
      };

   }
}

#endif /*HUMAN_H_*/
