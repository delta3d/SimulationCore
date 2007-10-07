/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2007, Alion Science and Technology.
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

namespace dtUtil
{
   class Log;
}

namespace dtAI
{
   class WorldState;
}

namespace osg
{
   class Geode;
}

//namespace dtAnim
//{
//   class AnimationHelper;
//}

namespace SimCore
{
   namespace Actors
   {
      class HumanOperator;
      class WalkRunBlend;
      
      class SIMCORE_EXPORT BasicStanceEnum : public dtUtil::Enumeration
      {
            DECLARE_ENUM(BasicStanceEnum);
         public:
            static BasicStanceEnum IDLE;
            static BasicStanceEnum STANDING;
            static BasicStanceEnum KNEELING;
            static BasicStanceEnum PRONE;
         private:
            BasicStanceEnum(const std::string& name);
      };

      class SIMCORE_EXPORT AnimationOperators
      {
         public:
            typedef std::map<std::string, dtAI::Operator* > NameOperMap;
            
            static const std::string ANIM_STAND_READY;
            static const std::string ANIM_STAND_DEPLOYED;
            static const std::string ANIM_WALK_READY;
            static const std::string ANIM_WALK_DEPLOYED;
            static const std::string ANIM_LOW_WALK_READY;
            static const std::string ANIM_LOW_WALK_DEPLOYED;
            static const std::string ANIM_CRAWL_READY;
            static const std::string ANIM_CRAWL_DEPLOYED;
            static const std::string ANIM_KNEEL_READY;
            static const std::string ANIM_KNEEL_DEPLOYED;
            static const std::string ANIM_STAND_TO_KNEEL;
            static const std::string ANIM_KNEEL_TO_STAND;
            static const std::string ANIM_PRONE_READY;
            static const std::string ANIM_PRONE_DEPLOYED;
            static const std::string ANIM_PRONE_TO_KNEEL;
            static const std::string ANIM_KNEEL_TO_PRONE;
            static const std::string ANIM_SHOT_STANDING;
            static const std::string ANIM_SHOT_KNEELING;
            static const std::string ANIM_SHOT_PRONE;
            static const std::string ANIM_DEAD_STANDING;
            static const std::string ANIM_DEAD_KNEELING;
            static const std::string ANIM_DEAD_PRONE;
            static const std::string OPER_DEPLOYED_TO_READY;
            static const std::string OPER_READY_TO_DEPLOYED;
            
            AnimationOperators(dtAI::PlannerHelper& plannerHelper, dtAnim::AnimationHelper& animHelper);
            ~AnimationOperators();

         private:
            void CreateOperators();
            HumanOperator* AddOperator(const std::string& name);
            
            dtAI::PlannerHelper& mPlannerHelper;
            dtCore::RefPtr<dtAnim::AnimationHelper> mAnimHelper;
            NameOperMap mOperators;
      };

      class SIMCORE_EXPORT HumanActorProxy : public BaseEntityActorProxy
      {
         public:
            typedef BaseEntityActorProxy BaseClass;

            static const std::string PROPERTY_SKELETAL_MESH;
            static const std::string PROPERTY_STANCE;
            static const std::string PROPERTY_PRIMARY_WEAPON_STATE;
            static const std::string PROPERTY_MIN_RUN_VELOCITY;
            static const std::string PROPERTY_FULL_RUN_VELOCITY;
            
            class SIMCORE_EXPORT StanceEnum : public dtUtil::Enumeration
            {
                  DECLARE_ENUM(StanceEnum);
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
                  
                  BasicStanceEnum& GetAssocBasicStanceEnum();
               private:
                  StanceEnum(const std::string& name, BasicStanceEnum& assocBasicStance);
                  BasicStanceEnum* mAssocBasicStance;
            };

            class SIMCORE_EXPORT WeaponStateEnum : public dtUtil::Enumeration
            {
                  DECLARE_ENUM(WeaponStateEnum);
               public:
                  static WeaponStateEnum NO_WEAPON;
                  static WeaponStateEnum STOWED;
                  static WeaponStateEnum DEPLOYED;
                  static WeaponStateEnum FIRING_POSITION;
               private:
                  WeaponStateEnum(const std::string& name);
            };

            HumanActorProxy();
            /**
             * Build the properties common to all human objects
             */
            /*virtual*/ void BuildPropertyMap();

            /// Build the invokables
            /*virtual*/ void BuildInvokables();

            /*virtual*/ void CreateActor();
         protected:
            virtual ~HumanActorProxy();
      };

      class SIMCORE_EXPORT Human : public BaseEntity
      {
         public:
            typedef BaseEntity BaseClass;

            static const std::string STATE_BASIC_STANCE;
            static const std::string STATE_WEAPON;
            static const std::string STATE_DEAD;
            static const std::string STATE_MOVING;
            static const std::string STATE_TRANSITION;
            static const std::string STATE_SHOT;
            
            Human(dtGame::GameActorProxy& proxy);
            
            /// Changes the file name used for the skeletal mesh
            void SetSkeletalMeshFile(const std::string& fileName);
            /// @return the file name used for the skeletal mesh
            const std::string& GetSkeletalMeshFile();
            
            /// Changes the human's stance
            void SetStance(HumanActorProxy::StanceEnum& stance);
            /// @return the human's stance 
            HumanActorProxy::StanceEnum& GetStance() const;
            
            /// Changes the human's first weapon state
            void SetPrimaryWeaponState(HumanActorProxy::WeaponStateEnum& weaponState);
            /// @return the human's first weapon state 
            HumanActorProxy::WeaponStateEnum& GetPrimaryWeaponState() const;
            
            /// When the state is updated, this is called internally to update the plan.
            bool GenerateNewAnimationSequence();
            /// Actually runs the planner update.
            void UpdatePlanAndAnimations();
            /// Checks the desired state to see if a new plan need to be generated, and if so generates it.
            void CheckAndUpdateAnimationState();

            /// This exists for the sake of the unit tests.
            const dtAI::Planner::OperatorList& GetCurrentPlan();
            
            /*virtual*/ void OnEnteredWorld();
            /*virtual*/ void TickRemote(const dtGame::Message& tickRemote);
            /*virtual*/ void TickLocal(const dtGame::Message& tickLocal);

         protected:
            virtual ~Human();
            /*virtual*/ void HandleModelDrawToggle(bool draw);

            void SetupPlannerHelper();
            float GetRemainingCost(const dtAI::WorldState* pWS) const;
            bool IsDesiredState(const dtAI::WorldState* pWS) const;
            
         private:
            std::string mSkeletalMeshFileName;
            std::string mIdleAnimatableName;
            std::string mRunWalkAnimatableName;

            dtCore::RefPtr<osg::Geode> mModelGeode;
            dtCore::RefPtr<dtAnim::AnimationHelper> mAnimationHelper;

            dtAI::PlannerHelper mPlannerHelper;
            dtAI::Planner mPlanner;
            dtAI::Planner::OperatorList mCurrentPlan;
            
            AnimationOperators mAnimOperators;
            
            HumanActorProxy::StanceEnum* mStance;
            HumanActorProxy::WeaponStateEnum* mPrimaryWeaponStateEnum;
            
            float mMinRunVelocity;
            float mFullRunVelocity;
            
            dtUtil::Log& mLogger;
      };

      class SIMCORE_EXPORT BasicStanceState: public dtAI::IStateVariable
      {
         public:
            typedef BasicStanceEnum EnumValueType;
            
            BasicStanceState();
            ~BasicStanceState();

            dtAI::IStateVariable* Copy() const;

            BasicStanceEnum& GetValue() const { return GetStance(); }
            void SetValue(BasicStanceEnum& pStance){ SetStance(pStance); }
            
            BasicStanceEnum& GetStance() const;
            void SetStance(BasicStanceEnum& newStance);

         private:

            BasicStanceEnum* mStance;
      };

      class SIMCORE_EXPORT WeaponState: public dtAI::IStateVariable
      {
         public:
            typedef HumanActorProxy::WeaponStateEnum EnumValueType;
            
            WeaponState();
            ~WeaponState();

            dtAI::IStateVariable* Copy() const;

            HumanActorProxy::WeaponStateEnum& GetValue() const { return GetWeaponStateEnum(); }
            void SetValue(HumanActorProxy::WeaponStateEnum& pWeaponState){ SetWeaponStateEnum(pWeaponState); }
                        
            
            HumanActorProxy::WeaponStateEnum& GetWeaponStateEnum() const;
            void SetWeaponStateEnum(HumanActorProxy::WeaponStateEnum& newWeaponStateEnum);

         private:
            HumanActorProxy::WeaponStateEnum* mWeaponStateEnum;
      };
   }
}

#endif /*HUMAN_H_*/
