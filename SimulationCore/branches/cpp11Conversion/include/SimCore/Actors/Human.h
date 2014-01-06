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

#include <dtUtil/refcountedbase.h>
#include <dtAnim/animationhelper.h>
#include <dtDAL/resourcedescriptor.h>

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
   class Node;
}

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

            // Used to help the planner determine distance.
            float GetCostValue() const;
         private:
            BasicStanceEnum(const std::string& name, float costValue);
            float mCostValue;
      };

      class SIMCORE_EXPORT AnimationOperators
      {
         public:
            typedef std::map<dtUtil::RefString, dtAI::Operator* > NameOperMap;

            static const dtUtil::RefString ANIM_STAND_READY;
            static const dtUtil::RefString ANIM_STAND_DEPLOYED;
            static const dtUtil::RefString ANIM_WALK_READY;
            static const dtUtil::RefString ANIM_WALK_DEPLOYED;
            static const dtUtil::RefString ANIM_LOW_WALK_READY;
            static const dtUtil::RefString ANIM_LOW_WALK_DEPLOYED;
            static const dtUtil::RefString ANIM_CRAWL_READY;
            static const dtUtil::RefString ANIM_CRAWL_DEPLOYED;
            static const dtUtil::RefString ANIM_KNEEL_READY;
            static const dtUtil::RefString ANIM_KNEEL_DEPLOYED;
            static const dtUtil::RefString ANIM_STAND_TO_KNEEL;
            static const dtUtil::RefString ANIM_KNEEL_TO_STAND;
            static const dtUtil::RefString ANIM_PRONE_READY;
            static const dtUtil::RefString ANIM_PRONE_DEPLOYED;
            static const dtUtil::RefString ANIM_PRONE_TO_KNEEL;
            static const dtUtil::RefString ANIM_KNEEL_TO_PRONE;
            static const dtUtil::RefString ANIM_SHOT_STANDING;
            static const dtUtil::RefString ANIM_SHOT_KNEELING;
            static const dtUtil::RefString ANIM_SHOT_PRONE;
            static const dtUtil::RefString ANIM_DEAD_STANDING;
            static const dtUtil::RefString ANIM_DEAD_KNEELING;
            static const dtUtil::RefString ANIM_DEAD_PRONE;
            static const dtUtil::RefString ANIM_STANDING_ACTION;
            static const dtUtil::RefString ANIM_KNEELING_ACTION;
            static const dtUtil::RefString ANIM_PRONE_ACTION;
            static const dtUtil::RefString OPER_DEPLOYED_TO_READY;
            static const dtUtil::RefString OPER_READY_TO_DEPLOYED;

            AnimationOperators(dtAI::PlannerHelper& plannerHelper);
            ~AnimationOperators();

         private:
            void CreateOperators();
            HumanOperator* AddOperator(const std::string& name);

            dtAI::PlannerHelper& mPlannerHelper;
            NameOperMap mOperators;
      };

      class SIMCORE_EXPORT HumanActorProxy : public BaseEntityActorProxy
      {
         public:
            typedef BaseEntityActorProxy BaseClass;

            static const dtUtil::RefString PROPERTY_SKELETAL_MESH;
            static const dtUtil::RefString PROPERTY_WEAPON_MESH;
            static const dtUtil::RefString PROPERTY_STANCE;
            static const dtUtil::RefString PROPERTY_PRIMARY_WEAPON_STATE;
            static const dtUtil::RefString PROPERTY_MIN_RUN_VELOCITY;
            static const dtUtil::RefString PROPERTY_FULL_RUN_VELOCITY;

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

            /*virtual*/ void BuildActorComponents();

            /*virtual*/ void CreateDrawable();
         protected:
            virtual ~HumanActorProxy();
      };

      class SIMCORE_EXPORT Human : public BaseEntity
      {
         public:
            typedef BaseEntity BaseClass;

            /// The basic stance of the character, such as standing or kneeling
            static const dtUtil::RefString STATE_BASIC_STANCE;
            /// The state of the weapon such as non-existent, deployed, etc.
            static const dtUtil::RefString STATE_WEAPON;
            /// A flag for if the human is dead.
            static const dtUtil::RefString STATE_DEAD;
            /// A flag that is true if the person is in motion.
            static const dtUtil::RefString STATE_MOVING;
            /// A flag marking that the human is in a transition,  this makes sure it never ends in a transition.
            static const dtUtil::RefString STATE_TRANSITION;
            /// The number of completed actions while standing.  This counter increments every time an action should be completed.
            static const dtUtil::RefString STATE_STANDING_ACTION_COUNT;
            /// The number of completed actions while kneeling.  This counter increments every time an action should be completed.
            static const dtUtil::RefString STATE_KNEELING_ACTION_COUNT;
            /// The number of completed actions while kneeling.  This counter increments every time an action should be completed.
            static const dtUtil::RefString STATE_PRONE_ACTION_COUNT;
            /// flag marking that the person has been shot.  Really this means the person is dying.
            static const dtUtil::RefString STATE_SHOT;

            ///setting this config option will force a weapon to be shown if one exists
            static const dtUtil::RefString CONFIG_ALWAYS_SHOW_WEAPON;

            Human(dtGame::GameActorProxy& proxy);

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

            /**
             * Sets the maximum amount of time per iteration the planner
             * can use to try and generate an animation sequence
             * @param time The new time to use
             */
            void SetMaxTimePerIteration(double time) { mMaxTimePerIteration = time; }

            /**
             * Returns the maximum amount of time to take generating
             * animation sequences
             * @return mMaxTimePerIteration
             */
            double GetMaxTimePerIteration() const { return mMaxTimePerIteration; }

            /// @return the numeric id of the weapon mesh within the skeletal mesh.
            const std::string& GetWeaponMeshName() const { return mWeaponMeshName; }
            /// Sets the numeric id of the weapon mesh within the skeletal mesh.
            void SetWeaponMeshName(const std::string& meshName) { mWeaponMeshName = meshName; }

            /*virtual*/ void OnEnteredWorld();
            /*virtual*/ void OnTickRemote(const dtGame::TickMessage& tickRemote);
            /*virtual*/ void OnTickLocal(const dtGame::TickMessage& tickLocal);

            /// @return true if this Human should be visible based on the options given.
            bool ShouldBeVisible(const SimCore::VisibilityOptions& options);

            unsigned GetExecutedActionCount(HumanActorProxy::StanceEnum& stance) const;

            void ExecuteAction(HumanActorProxy::StanceEnum& stance = HumanActorProxy::StanceEnum::UPRIGHT_STANDING,
                     const dtUtil::RefString& animatableName = AnimationOperators::ANIM_STANDING_ACTION);


            /// these have to be public so that they can be connected by the HumanActorProxy
            virtual void SkeletalMeshLoadCallback(dtAnim::AnimationHelper*);
            virtual void SkeletalMeshUnloadCallback(dtAnim::AnimationHelper*);

         protected:
            virtual ~Human();

            void SetupPlannerHelper();
            float GetRemainingCost(const dtAI::WorldState* pWS) const;
            bool IsDesiredState(const dtAI::WorldState* pWS) const;
            unsigned CheckActionState(const dtAI::WorldState* pWS, const std::string& stateName, unsigned desiredVal) const;
            void UpdateWeapon();
            bool GetContainsWeaponName(const std::vector<std::string>& vec, const std::string& meshName) const;


         private:
            /// Apply the effects of the operator, and get the animatable, if any, associated with it.
            const dtAnim::Animatable* ApplyOperatorAndGetAnimatable(const dtAI::Operator& op);

            std::string mIdleAnimatableName;
            std::string mRunWalkAnimatableName;
            std::string mWeaponMeshName;

            std::string mSequenceId;

            osg::ref_ptr<osg::Node> mModelNode;

            dtAI::PlannerHelper mPlannerHelper;
            dtAI::Planner mPlanner;
            dtAI::Planner::OperatorList mCurrentPlan;

            AnimationOperators mAnimOperators;

            HumanActorProxy::StanceEnum* mStance;
            HumanActorProxy::WeaponStateEnum* mPrimaryWeaponStateEnum;

            typedef std::map<BasicStanceEnum*, unsigned> ExecuteActionCountMap;
            ExecuteActionCountMap mExecutedActionCounts;

            float mMinRunVelocity;
            float mFullRunVelocity;

            dtUtil::Log& mLogger;

            double mMaxTimePerIteration;
            bool mModelLoadedAndWaiting;
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

            virtual const std::string ToString() const { return GetStance().GetName(); }

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

            virtual const std::string ToString() const { return GetWeaponStateEnum().GetName(); }

         private:
            HumanActorProxy::WeaponStateEnum* mWeaponStateEnum;
      };
   }
}

#endif /*HUMAN_H_*/
