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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/Human.h>

#include <dtAnim/cal3ddatabase.h>
#include <dtAnim/animnodebuilder.h>
#include <dtAnim/animationhelper.h>
#include <dtAnim/animationcomponent.h>
#include <dtAnim/animationsequence.h>
#include <dtAnim/animationchannel.h>
#include <dtAnim/animationwrapper.h>
#include <dtAnim/sequencemixer.h>

#include <dtAI/worldstate.h>
#include <dtAI/basenpcutils.h>

#include <dtGame/gamemanager.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/datatype.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/log.h>

#include <osg/Geode>
#include <osg/MatrixTransform>

namespace SimCore
{

   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////    HumanActorProxy      ///////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(HumanActorProxy::StanceEnum);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::NOT_APPLICABLE("NOT_APPLICABLE",
            BasicStanceEnum::IDLE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::UPRIGHT_STANDING("UPRIGHT_STANDING",
            BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::UPRIGHT_WALKING("UPRIGHT_WALKING",
            BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::UPRIGHT_RUNNING("UPRIGHT_RUNNING",
            BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::KNEELING("KNEELING",
            BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::PRONE("PRONE",
            BasicStanceEnum::PRONE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::CRAWLING("CRAWLING",
            BasicStanceEnum::PRONE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::SWIMMING("SWIMMING",
            BasicStanceEnum::PRONE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::PARACHUTING("PARACHUTING",
            BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::JUMPING("JUMPING",
            BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::SITTING("SITTING",
            BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::SQUATTING("SQUATTING",
            BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::CROUCHING("CROUCHING",
            BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::WADING("WADING",
            BasicStanceEnum::STANDING);

      HumanActorProxy::StanceEnum::StanceEnum(const std::string& name, BasicStanceEnum& assocBasicStance) 
         : dtUtil::Enumeration(name)
      {
         AddInstance(this);
         mAssocBasicStance = &assocBasicStance;
      }

      BasicStanceEnum& HumanActorProxy::StanceEnum::GetAssocBasicStanceEnum()
      {
         return *mAssocBasicStance;
      }

      IMPLEMENT_ENUM(HumanActorProxy::WeaponStateEnum);
      HumanActorProxy::WeaponStateEnum HumanActorProxy::WeaponStateEnum::NO_WEAPON("NO_WEAPON");
      HumanActorProxy::WeaponStateEnum HumanActorProxy::WeaponStateEnum::STOWED("STOWED");
      HumanActorProxy::WeaponStateEnum HumanActorProxy::WeaponStateEnum::DEPLOYED("DEPLOYED");
      HumanActorProxy::WeaponStateEnum HumanActorProxy::WeaponStateEnum::FIRING_POSITION("FIRING_POSITION");

      HumanActorProxy::WeaponStateEnum::WeaponStateEnum(const std::string& name) : dtUtil::Enumeration(name)
      {
         AddInstance(this);
      }

      const std::string HumanActorProxy::PROPERTY_SKELETAL_MESH("Skeletal Mesh");
      const std::string HumanActorProxy::PROPERTY_STANCE("Stance");
      const std::string HumanActorProxy::PROPERTY_PRIMARY_WEAPON_STATE("Primary Weapon State");
      const std::string HumanActorProxy::PROPERTY_MIN_RUN_VELOCITY("Minimum Run Velocity");
      const std::string HumanActorProxy::PROPERTY_FULL_RUN_VELOCITY("Full Run Velocity");
      
      HumanActorProxy::HumanActorProxy()
      {
         SetClassName("SimCore::Actors::Human");
      }
      
      ////////////////////////////////////////////////////////////////////////////
      HumanActorProxy::~HumanActorProxy()
      {
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void HumanActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();
         
         Human& human = static_cast<Human&>(GetGameActor());
         
         static const std::string HUMAN_GROUP("Human");
         
         static const std::string PROPERTY_SKELETAL_MESH_DESC
            ("The skeletal mesh file that defines the human's look and animation set.");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SKELETAL_MESH,
               PROPERTY_SKELETAL_MESH, PROPERTY_SKELETAL_MESH, 
               dtDAL::MakeFunctor(human, &Human::SetSkeletalMeshFile),
               PROPERTY_SKELETAL_MESH_DESC, HUMAN_GROUP));

         static const std::string PROPERTY_STANCE_DESC
            ("The stance of the human.");
         AddProperty(new dtDAL::EnumActorProperty<HumanActorProxy::StanceEnum>(
               PROPERTY_STANCE, PROPERTY_STANCE, 
               dtDAL::MakeFunctor(human, &Human::SetStance),
               dtDAL::MakeFunctorRet(human, &Human::GetStance),
               PROPERTY_STANCE_DESC, HUMAN_GROUP));

         static const std::string PROPERTY_PRIMARY_WEAPON_STATE_DESC
            ("The state/availability of the primary weapon.");
         AddProperty(new dtDAL::EnumActorProperty<HumanActorProxy::WeaponStateEnum>(
               PROPERTY_PRIMARY_WEAPON_STATE, PROPERTY_PRIMARY_WEAPON_STATE, 
               dtDAL::MakeFunctor(human, &Human::SetPrimaryWeaponState),
               dtDAL::MakeFunctorRet(human, &Human::GetPrimaryWeaponState),
               PROPERTY_PRIMARY_WEAPON_STATE_DESC, HUMAN_GROUP));

         

//         static const std::string PROPERTY_MIN_RUN_VELOCITY_DESC
//            ("The Minimum velocity at which the human will begin running");
//
//         AddProperty(new dtDAL::FloatActorProperty(*this,
//               PROPERTY_MIN_RUN_VELOCITY, PROPERTY_MIN_RUN_VELOCITY, 
//               dtDAL::MakeFunctor(human, &Human::SetSkeletalMeshFile),
//               PROPERTY_MIN_RUN_VELOCITY_DESC, HUMAN_GROUP));
//
//         static const std::string PROPERTY_FULL_RUN_VELOCITY_DESC
//            ("The velocity at which the human will be fully running");
//
//         AddProperty(new dtDAL::FloatActorProperty(*this,
//               PROPERTY_FULL_RUN_VELOCITY, PROPERTY_FULL_RUN_VELOCITY, 
//               dtDAL::MakeFunctor(human, &Human::SetSkeletalMeshFile),
//               PROPERTY_FULL_RUN_VELOCITY_DESC, HUMAN_GROUP));
//
         
         
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void HumanActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();
      }

      ////////////////////////////////////////////////////////////////////////////
      void HumanActorProxy::CreateActor()
      {
         Human* human = new Human(*this);
         SetActor(*human);

         //we made a virtual function to create our dead reckoning helper so the helper
         //could be changed by a subclass of entity.... bga
         human->InitDeadReckoningHelper();
      }

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////    Human                ///////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      const std::string Human::STATE_BASIC_STANCE("BasicStanceState");
      const std::string Human::STATE_WEAPON("WeaponState");
      const std::string Human::STATE_DEAD("DeadState");
      const std::string Human::STATE_MOVING("MovingState");
      const std::string Human::STATE_TRANSITION("TranstionState");
      const std::string Human::STATE_SHOT("ShotState");

      ////////////////////////////////////////////////////////////////////////////
      Human::Human(dtGame::GameActorProxy& proxy) : BaseEntity(proxy),
         mAnimationHelper(new dtAnim::AnimationHelper), 
         mPlannerHelper(
               dtAI::PlannerHelper::RemainingCostFunctor(this, &Human::GetRemainingCost),
               dtAI::PlannerHelper::DesiredStateFunctor(this, &Human::IsDesiredState)
               ),
         mAnimOperators(mPlannerHelper, *mAnimationHelper),
         mStance(&HumanActorProxy::StanceEnum::UPRIGHT_STANDING),
         mPrimaryWeaponStateEnum(&HumanActorProxy::WeaponStateEnum::NO_WEAPON),
         mLogger(dtUtil::Log::GetInstance("Human.cpp"))
      {
         SetDrawingModel(true);
      }
      
      ////////////////////////////////////////////////////////////////////////////
      Human::~Human()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::SetupPlannerHelper()
      {
         dtAI::WorldState initialState;
         
         BasicStanceState* stanceState = new BasicStanceState();
         stanceState->SetStance(mStance->GetAssocBasicStanceEnum());
         
         initialState.AddState(STATE_BASIC_STANCE, stanceState);

         WeaponState* weaponState = new WeaponState();
         weaponState->SetWeaponStateEnum(*mPrimaryWeaponStateEnum);
         
         initialState.AddState(STATE_WEAPON,       weaponState);
         initialState.AddState(STATE_DEAD,         new dtAI::StateVariable(
               GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED));
         initialState.AddState(STATE_MOVING,       new dtAI::StateVariable(
               !dtUtil::Equivalent(GetVelocityVector().length2(), 0.0f)));
         //Setting transition to true will make the planner generate the correct initial animation.
         initialState.AddState(STATE_TRANSITION,   new dtAI::StateVariable(true));
         initialState.AddState(STATE_SHOT,         new dtAI::StateVariable(false));
         
         mPlannerHelper.SetCurrentState(initialState);
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void Human::SetSkeletalMeshFile(const std::string& fileName)
      {
         mSkeletalMeshFileName = fileName;

         osg::MatrixTransform& transform = GetScaleMatrixTransform();
         transform.removeChildren(0, transform.getNumChildren());
         mModelGeode = NULL;

         if (!fileName.empty() && mAnimationHelper->LoadModel(fileName))
         {
            mModelGeode = dtAnim::Cal3DDatabase::GetInstance().GetNodeBuilder().CreateGeode(mAnimationHelper->GetModelWrapper());
            HandleModelDrawToggle(IsDrawingModel());
         }
      }
      
      ////////////////////////////////////////////////////////////////////////////
      const std::string& Human::GetSkeletalMeshFile()
      {
         return mSkeletalMeshFileName;
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::HandleModelDrawToggle(bool draw)
      {
         if (!draw)
         {
            osg::MatrixTransform& transform = GetScaleMatrixTransform();
            transform.removeChildren(0, transform.getNumChildren());
         }
         else
         {
            if (mModelGeode.valid())
               GetScaleMatrixTransform().addChild(mModelGeode.get());
         }
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void Human::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         SetupPlannerHelper();
         UpdatePlanAndAnimations();

         dtAnim::AnimationComponent* animComponent = NULL;

         GetGameActorProxy().GetGameManager()->
            GetComponentByName(dtAnim::AnimationComponent::DEFAULT_NAME, animComponent);

         if (animComponent != NULL)
         {
            animComponent->RegisterActor(GetGameActorProxy(), *mAnimationHelper);
         }

         if (IsRemote())
         {
            RegisterWithDeadReckoningComponent();
            //Need tick remote to check for plan changes.
            GetGameActorProxy().RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::GenerateNewAnimationSequence()
      {
         mCurrentPlan.clear();
         mPlanner.Reset(&mPlannerHelper);
         dtAI::Planner::PlannerResult result = mPlanner.GeneratePlan();
         if (result == dtAI::Planner::PLAN_FOUND)
         {
            mCurrentPlan = mPlanner.GetConfig().mResult;
         }
      }
      
      ////////////////////////////////////////////////////////////////////////////
      const dtAI::Planner::OperatorList& Human::GetCurrentPlan()
      {
         return mCurrentPlan;
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::SetStance(HumanActorProxy::StanceEnum& stance)
      {
         mStance = &stance;
      }

      ////////////////////////////////////////////////////////////////////////////
      HumanActorProxy::StanceEnum& Human::GetStance() const
      {
         return *mStance;
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::SetPrimaryWeaponState(HumanActorProxy::WeaponStateEnum& weaponState)
      {
         mPrimaryWeaponStateEnum = &weaponState;
      }

      ////////////////////////////////////////////////////////////////////////////
      HumanActorProxy::WeaponStateEnum& Human::GetPrimaryWeaponState() const
      {
         return *mPrimaryWeaponStateEnum;
      }

      ////////////////////////////////////////////////////////////////////////////
      float Human::GetRemainingCost(const dtAI::WorldState* pWS) const
      {
         return 1.0f;
      }

      ////////////////////////////////////////////////////////////////////////////
      bool Human::IsDesiredState(const dtAI::WorldState* pWS) const
      {
         //If we are in a transition, we are not in the desired state.
         const dtAI::StateVariable* transState;
         pWS->GetState(STATE_TRANSITION, transState);
         if (transState->Get())
            return false;

         const BasicStanceState* stanceState;
         pWS->GetState(STATE_BASIC_STANCE, stanceState);
         
         if (stanceState->GetStance() != mStance->GetAssocBasicStanceEnum())
            return false;
            
         const WeaponState* weaponState;
         pWS->GetState(STATE_WEAPON, weaponState);
         
         if (weaponState->GetWeaponStateEnum() != *mPrimaryWeaponStateEnum)
            return false;
         
         const dtAI::StateVariable* movingState;
         pWS->GetState(STATE_MOVING, movingState);

         //This requires that plans be made in one frame.
         //Moving is the same as the velocity > 0.
         if (movingState->Get() != !dtUtil::Equivalent(GetVelocityVector().length2(), 0.0f))
            return false;

         const dtAI::StateVariable* deadState;
         pWS->GetState(STATE_DEAD, deadState);

         //dead is the same as the damage state being equal to destroyed.
         if (deadState->Get() != (GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED))
            return false;

         // we don't care about shot.
         //const dtAI::StateVariable* shotState;
         //pWS->GetState(STATE_SHOT, shotState);

         return true;
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::TickRemote(const dtGame::Message& tickRemote)
      {
         //just in case
         BaseClass::TickRemote(tickRemote);
         if (!IsDesiredState(mPlannerHelper.GetCurrentState()))
         {
            if (mLogger.IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               mLogger.LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                     "Planner is not in the desired state.  Generating animations.");
            }
            
            UpdatePlanAndAnimations();
            
         }
      }

      void Human::UpdatePlanAndAnimations()
      {
         GenerateNewAnimationSequence();
         dtAI::Planner::OperatorList::iterator i, iend;

         dtAnim::SequenceMixer& seqMixer = mAnimationHelper->GetSequenceMixer();

         dtCore::RefPtr<dtAnim::AnimationSequence> generatedSequence = new dtAnim::AnimationSequence();
         generatedSequence->SetName("transition");
         i = mCurrentPlan.begin();
         iend = mCurrentPlan.end();
         
         if (mLogger.IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            mLogger.LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "Current animation plan has \"%d\" steps.", mCurrentPlan.size());
         }

         const float blendTime = 0.5f;
         float accumulatedStartTime = 0.0f;
         for (; i != iend; ++i)
         {
            const dtAI::Operator* op = *i;
            op->Apply(mPlannerHelper.GetCurrentState());
            
            const dtAnim::Animatable* animatable = seqMixer.GetRegisteredAnimation(op->GetName());
            if (animatable != NULL)
            {
               dtCore::RefPtr<dtAnim::Animatable> newAnim = animatable->Clone(mAnimationHelper->GetModelWrapper());
               newAnim->SetStartDelay(std::max(0.0f, accumulatedStartTime - blendTime));
               
               newAnim->SetFadeIn(blendTime);
               newAnim->SetFadeOut(blendTime);
               
               dtAnim::AnimationChannel* animChannel = dynamic_cast<dtAnim::AnimationChannel*>(newAnim.get());
               if (animChannel != NULL)
               {
                  animChannel->SetMaxDuration(animChannel->GetAnimation()->GetDuration());
                  animChannel->SetAction(true);
               }
               
               generatedSequence->AddAnimation(animChannel);
            }
         }
         
         seqMixer.ClearActiveAnimations(blendTime);
         seqMixer.PlayAnimation(generatedSequence.get());
      }
      
      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(BasicStanceEnum);
      BasicStanceEnum BasicStanceEnum::IDLE("IDLE");
      BasicStanceEnum BasicStanceEnum::STANDING("STANDING");
      BasicStanceEnum BasicStanceEnum::KNEELING("KNEELING");
      BasicStanceEnum BasicStanceEnum::PRONE("PRONE");

      BasicStanceEnum::BasicStanceEnum(const std::string& name) : dtUtil::Enumeration(name)
      {
         AddInstance(this);
      }

      BasicStanceState::BasicStanceState():
         mStance(&BasicStanceEnum::IDLE)
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      BasicStanceState::~BasicStanceState()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      dtAI::IStateVariable* BasicStanceState::Copy() const
      {
         BasicStanceState* stanceState = new BasicStanceState;
         stanceState->mStance = mStance;
         return stanceState;
      }

      ////////////////////////////////////////////////////////////////////////////
      BasicStanceEnum& BasicStanceState::GetStance() const
      {
         return *mStance;
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void BasicStanceState::SetStance(BasicStanceEnum& newStance)
      {
         mStance = &newStance;
      }

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      WeaponState::WeaponState():
         mWeaponStateEnum(&HumanActorProxy::WeaponStateEnum::STOWED)
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      WeaponState::~WeaponState()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      dtAI::IStateVariable* WeaponState::Copy() const
      {
         WeaponState* weaponState = new WeaponState;
         weaponState->mWeaponStateEnum = mWeaponStateEnum;
         return weaponState;
      }

      ////////////////////////////////////////////////////////////////////////////
      HumanActorProxy::WeaponStateEnum& WeaponState::GetWeaponStateEnum() const
      {
         return *mWeaponStateEnum;
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void WeaponState::SetWeaponStateEnum(HumanActorProxy::WeaponStateEnum& newWeaponStateEnum)
      {
         mWeaponStateEnum = &newWeaponStateEnum;
      }
 

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      template <typename StateVarType>
      class EnumerationConditional: public dtAI::IConditional
      {
         public:
            typedef typename StateVarType::EnumValueType StateVarEnumType;
            
            EnumerationConditional(const std::string& pName, StateVarEnumType& pData): mName(pName), mData(pData) {}
            ~EnumerationConditional() {}

            /*virtual*/ const std::string& GetName() const
            {
               return mName;
            }

            /*virtual*/ bool Evaluate(const dtAI::WorldState* pWS)
            {
               const StateVarType* pStateVar;
               pWS->GetState(mName, pStateVar);
               if(pStateVar != NULL)
               {
                  return pStateVar->GetValue() == mData;
               }
               return false;
            }

         private:
            std::string mName;
            StateVarEnumType& mData;
      };
      
      template <typename StateVarType>
      class EnumeratedEffect: public dtAI::IEffect
      {
      public:
         typedef typename StateVarType::EnumValueType StateVarEnumType;

         EnumeratedEffect(const std::string& pName, StateVarEnumType& pData): mName(pName), mData(pData){}
          
         const std::string& GetName() const
         {
            return mName;
         }

         bool Apply(const dtAI::Operator*, dtAI::WorldState* pWSIn) const
         {
            StateVarType* pStateVar;
            pWSIn->GetState(mName, pStateVar);
            if(pStateVar != NULL)
            {
               pStateVar->SetValue(mData);
            }
            return true;
         }
         
      protected:
         ~EnumeratedEffect(){}
         
      private:
         std::string mName;
         StateVarEnumType& mData;
      };

      class HumanOperator: public dtAI::Operator
      {
         public:
            typedef dtAI::IEffect EffectType;
            typedef std::vector<dtCore::RefPtr<EffectType> > EffectList;

            typedef EnumerationConditional<BasicStanceState> BasicStanceEnumConditional;
            typedef EnumerationConditional<WeaponState> WeaponStateEnumConditional;
                  
            typedef EnumeratedEffect<BasicStanceState> BasicStanceEnumEffect;
            typedef EnumeratedEffect<WeaponState> WeaponStateEnumEffect;
            
         public:
            HumanOperator(const std::string& pName)
            : Operator(pName, Operator::ApplyOperatorFunctor(this, &HumanOperator::Apply))
            , mCost(1.0f)
            {}
            
            void SetCost(float pcost){mCost = pcost;}
            
            void AddEffect(EffectType* pEffect){mEffects.push_back(pEffect);}

            bool Apply(const dtAI::Operator* oper, dtAI::WorldState* pWSIn) const
            {
               EffectList::const_iterator iter = mEffects.begin();
               EffectList::const_iterator endOfList = mEffects.end();
               while(iter != endOfList)
               {
                  (*iter)->Apply(oper, pWSIn);
                  ++iter;
               }

               pWSIn->AddCost(mCost);
               return true;
            }

         private:
            
            float mCost;
            EffectList mEffects;
            
      };

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      const std::string AnimationOperators::ANIM_STAND_READY("Stand Ready");
      const std::string AnimationOperators::ANIM_STAND_DEPLOYED("Stand Deployed");
      const std::string AnimationOperators::ANIM_WALK_READY("Walk Ready");
      const std::string AnimationOperators::ANIM_WALK_DEPLOYED("Walk Deployed");
      const std::string AnimationOperators::ANIM_KNEEL_READY("Kneel Ready");
      const std::string AnimationOperators::ANIM_KNEEL_DEPLOYED("Kneel Ready");
      const std::string AnimationOperators::ANIM_STAND_TO_KNEEL("Stand To Kneel");
      const std::string AnimationOperators::ANIM_KNEEL_TO_STAND("Kneel To Stand");
      const std::string AnimationOperators::OPER_DEPLOYED_TO_READY("Deployed To Ready");
      const std::string AnimationOperators::OPER_READY_TO_DEPLOYED("Ready To Deployed");

      AnimationOperators::AnimationOperators(dtAI::PlannerHelper& plannerHelper, 
            dtAnim::AnimationHelper& animHelper):
         mPlannerHelper(plannerHelper), mAnimHelper(&animHelper)
      {
         CreateOperators();
      }
      
      ////////////////////////////////////////////////////////////////////////////
      AnimationOperators::~AnimationOperators()
      {
      }
      

      ////////////////////////////////////////////////////////////////////////////
      HumanOperator* AnimationOperators::AddOperator(const std::string& name)
      {
         HumanOperator* op = new HumanOperator(name);
         mOperators.insert(std::make_pair(op->GetName(), op));
         mPlannerHelper.AddOperator(op);
         return op;
      }

      ////////////////////////////////////////////////////////////////////////////
      void AnimationOperators::CreateOperators()
      {
         dtCore::RefPtr<HumanOperator::BasicStanceEnumConditional> standing 
            = new HumanOperator::BasicStanceEnumConditional(Human::STATE_BASIC_STANCE, BasicStanceEnum::STANDING); 

         dtCore::RefPtr<HumanOperator::BasicStanceEnumConditional>  kneeling
            = new HumanOperator::BasicStanceEnumConditional(Human::STATE_BASIC_STANCE, BasicStanceEnum::KNEELING); 
         
         dtCore::RefPtr<HumanOperator::BasicStanceEnumConditional> prone
            = new HumanOperator::BasicStanceEnumConditional(Human::STATE_BASIC_STANCE, BasicStanceEnum::PRONE); 
         
         dtCore::RefPtr<HumanOperator::BasicStanceEnumConditional> idle
            = new HumanOperator::BasicStanceEnumConditional(Human::STATE_BASIC_STANCE, BasicStanceEnum::IDLE); 

         
         dtCore::RefPtr<HumanOperator::WeaponStateEnumConditional> deployed
            = new HumanOperator::WeaponStateEnumConditional(Human::STATE_WEAPON, HumanActorProxy::WeaponStateEnum::DEPLOYED);

         dtCore::RefPtr<HumanOperator::WeaponStateEnumConditional> ready
            = new HumanOperator::WeaponStateEnumConditional(Human::STATE_WEAPON, HumanActorProxy::WeaponStateEnum::FIRING_POSITION);


         dtCore::RefPtr<dtAI::Precondition> isMoving 
            = new dtAI::Precondition(Human::STATE_MOVING, true);

         dtCore::RefPtr<dtAI::Precondition> notMoving
            = new dtAI::Precondition(Human::STATE_MOVING, false);


         dtCore::RefPtr<dtAI::Precondition> isDead
            = new dtAI::Precondition(Human::STATE_DEAD, true);

         dtCore::RefPtr<dtAI::Precondition> isShot
            = new dtAI::Precondition(Human::STATE_SHOT, true);


         dtCore::RefPtr<dtAI::Precondition> isTransition
            = new dtAI::Precondition(Human::STATE_TRANSITION, true);
         
         dtCore::RefPtr<dtAI::Precondition> notTransition
            = new dtAI::Precondition(Human::STATE_TRANSITION, false);

         dtCore::RefPtr<HumanOperator::BasicStanceEnumEffect> standingEff
            = new HumanOperator::BasicStanceEnumEffect(Human::STATE_BASIC_STANCE, BasicStanceEnum::STANDING);

         dtCore::RefPtr<HumanOperator::BasicStanceEnumEffect> kneelingEff
            = new HumanOperator::BasicStanceEnumEffect(Human::STATE_BASIC_STANCE, BasicStanceEnum::KNEELING);

         dtCore::RefPtr<HumanOperator::BasicStanceEnumEffect> proneEff
            = new HumanOperator::BasicStanceEnumEffect(Human::STATE_BASIC_STANCE, BasicStanceEnum::PRONE);


         dtCore::RefPtr<HumanOperator::WeaponStateEnumEffect> readyEff
            = new HumanOperator::WeaponStateEnumEffect(Human::STATE_WEAPON, HumanActorProxy::WeaponStateEnum::FIRING_POSITION);

         dtCore::RefPtr<HumanOperator::WeaponStateEnumEffect> deployedEff
            = new HumanOperator::WeaponStateEnumEffect(Human::STATE_WEAPON, HumanActorProxy::WeaponStateEnum::DEPLOYED);


         dtCore::RefPtr<dtAI::Effect> 
            movingEff = new dtAI::Effect(Human::STATE_MOVING, true);
         
         dtCore::RefPtr<dtAI::Effect> 
            notMovingEff = new dtAI::Effect(Human::STATE_MOVING, false);

         dtCore::RefPtr<dtAI::Effect> 
            deadEff = new dtAI::Effect(Human::STATE_DEAD, true);
         
         dtCore::RefPtr<dtAI::Effect> 
            shotEff = new dtAI::Effect(Human::STATE_SHOT, true);
         
         dtCore::RefPtr<dtAI::Effect> 
            transitionEff = new dtAI::Effect(Human::STATE_TRANSITION, true);

         dtCore::RefPtr<dtAI::Effect> 
            notTransitionEff = new dtAI::Effect(Human::STATE_TRANSITION, false);
         
         HumanOperator* newOp;
         newOp = AddOperator(ANIM_STAND_READY);
         newOp->AddPreCondition(standing.get());
         newOp->AddPreCondition(ready.get());
         
         newOp->AddEffect(standingEff.get());
         newOp->AddEffect(notMovingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_STAND_DEPLOYED);
         newOp->AddPreCondition(standing.get());
         newOp->AddPreCondition(deployed.get());

         newOp->AddEffect(standingEff.get());
         newOp->AddEffect(notMovingEff.get());
         newOp->AddEffect(notTransitionEff.get());
         
         newOp = AddOperator(ANIM_WALK_READY);
         newOp->AddPreCondition(standing.get());
         newOp->AddPreCondition(ready.get());

         newOp->AddEffect(standingEff.get());
         newOp->AddEffect(movingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_WALK_DEPLOYED);
         newOp->AddPreCondition(standing.get());
         newOp->AddPreCondition(deployed.get());

         newOp->AddEffect(standingEff.get());
         newOp->AddEffect(movingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_KNEEL_READY);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(ready.get());

         newOp->AddEffect(kneelingEff.get());
         newOp->AddEffect(notMovingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_KNEEL_DEPLOYED);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(deployed.get());

         newOp->AddEffect(kneelingEff.get());
         newOp->AddEffect(notMovingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_STAND_TO_KNEEL);
         newOp->AddPreCondition(standing.get());
         newOp->AddPreCondition(deployed.get());
         newOp->AddPreCondition(notMoving.get());

         newOp->AddEffect(kneelingEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(ANIM_KNEEL_TO_STAND);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(deployed.get());
         newOp->AddPreCondition(notMoving.get());

         newOp->AddEffect(standingEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(OPER_READY_TO_DEPLOYED);
         newOp->AddPreCondition(ready.get());
         newOp->AddEffect(deployedEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(OPER_DEPLOYED_TO_READY);
         newOp->AddPreCondition(deployed.get());
         newOp->AddEffect(readyEff.get());
         newOp->AddEffect(transitionEff.get());
      }
   }
}
