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
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/Human.h>

#include <dtAnim/cal3ddatabase.h>
#include <dtAnim/animnodebuilder.h>
#include <dtAnim/animationcomponent.h>
#include <dtAnim/animationsequence.h>
#include <dtAnim/animationchannel.h>
#include <dtAnim/animationwrapper.h>
#include <dtAnim/sequencemixer.h>
#include <dtAnim/cal3dmodelwrapper.h>

#include <dtAI/worldstate.h>
#include <dtAI/basenpcutils.h>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoninghelper.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/datatype.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/log.h>
#include <dtUtil/macros.h>

#include <osg/Geode>
#include <osg/MatrixTransform>

#ifdef DELTA_WIN32
   #pragma warning(disable : 4355)
#endif


namespace SimCore
{

   namespace Actors
   {

      //A class to simulate a walk run blend
      class WalkRunBlend: public dtAnim::AnimationSequence
      {
      public:

         class WRController: public dtAnim::AnimationSequence::AnimationController
         {
            public:
               typedef dtAnim::AnimationSequence::AnimationController BaseClass;

               WRController(WalkRunBlend& pWR, Human* h)
                  : BaseClass(pWR)
                  , mSpeed(0.0f)
                  , mWalkStart(0.000001f)//in m/s
                  , mWalkFadeIn(0.15f)//in m/s
                  , mWalkStop(1.5f)//in m/s
                  , mWalkFadeOut(0.15f)//in m/s
                  , mRunStart(1.35f)//in m/s
                  , mRunFadeIn(0.15f) //in m/s
                  , mRunStop(std::numeric_limits<float>::max()) //we dont want to stop running
                  , mRunFadeOut(std::numeric_limits<float>::max()) //we dont want to stop running
                  , mParentHuman(h)
               {
               }

               WRController(const WRController& pWR)
                  : BaseClass(pWR)
                  , mSpeed(pWR.mSpeed)
                  , mWalkStart(pWR.mWalkStart)
                  , mWalkFadeIn(pWR.mWalkFadeIn)
                  , mWalkStop(pWR.mWalkStop)
                  , mWalkFadeOut(pWR.mWalkFadeOut)
                  , mRunStart(pWR.mRunStart)
                  , mRunFadeIn(pWR.mRunFadeIn) 
                  , mRunStop(pWR.mRunStop) 
                  , mRunFadeOut(pWR.mRunFadeOut)
                  , mParentHuman(pWR.mParentHuman)
               {

               }

               void SetAnimations(dtAnim::Animatable* walk, dtAnim::Animatable* run)
               {
                  mRun = run;
                  mWalk = walk;
               }

               //this sets the basic necessary blend values, the others get expected values
               void SetRunWalkBasic(float walkStop, float walkFadeOut, float runStart, float runFadeIn)
               {
                  mWalkStart = 0.000001f;
                  mWalkFadeIn = 0.25f;
                  mWalkStop= walkStop;
                  mWalkFadeOut = walkFadeOut;
                  mRunStart = runStart;
                  mRunFadeIn = runFadeIn;
                  mRunStop = std::numeric_limits<float>::max();
                  mRunFadeOut = std::numeric_limits<float>::max();
               }

               //customize the walk
               void SetWalk(float start, float fadeIn, float stop, float fadeOut)
               {
                  mWalkStart = start;
                  mWalkFadeIn = fadeIn;
                  mWalkStop = stop;
                  mWalkFadeOut = fadeOut;
               }

               //customize the run
               void SetRun(float start, float fadeIn, float stop, float fadeOut)
               {
                  mRunStart = start;
                  mRunFadeIn = fadeIn;
                  mRunStop = stop;
                  mRunFadeOut = fadeOut;               
               }

               void SetCurrentSpeed(float pSpeed)
               {
                  mSpeed = pSpeed;
               }

               /*virtual*/ void Update(float dt)
               {
                  //update our velocity vector
                  if(mParentHuman.valid())
                  {
                     mSpeed = mParentHuman->GetVelocityVector().length();
                     //std::cout << "Human Speed: " << mSpeed << std::endl;
                  }
                  else
                  {
                     LOG_ERROR("Controller has no valid parent pointer");
                  }

                  float walkWeight = ComputeWeight(mWalk.get(), mWalkStart, mWalkFadeIn, mWalkStop, mWalkFadeOut);
                  float runWeight = ComputeWeight(mRun.get(), mRunStart, mRunFadeIn, mRunStop, mRunFadeOut);

                  mWalk->SetCurrentWeight(walkWeight);
                  mRun->SetCurrentWeight(runWeight);

                  if(walkWeight > 0.0f)
                  {
                     mWalk->Update(dt);
                     //std::cout << "Walking Weight: " << walkWeight << std::endl;
                  }

                  if(runWeight > 0.0f)
                  {
                     mRun->Update(dt);
                     //std::cout << "Running Weight: " << runWeight << std::endl;
                  }
               }

               dtAnim::Animatable* GetWalk()
               {
                  return mWalk.get();
               }

               dtAnim::Animatable* GetRun()
               {
                  return mRun.get();
               }

               dtCore::RefPtr<WRController> CloneDerived() const
               {
                  return new WRController(*this);
               }
               
            protected:
               ~WRController()
               {

               }

               float ComputeWeight(dtAnim::Animatable* pAnim, float startSpeed, float fadeIn, float stopSpeed, float fadeOut)
               {
                  //we will have the default imply mSpeed is between startSpeed and stopSpeed
                  //which basically just saves us another if check
                  float weight = 1.0f;

                  //if we are out of bounds 
                  if(mSpeed < startSpeed || mSpeed > stopSpeed)
                  {
                     weight = 0.0f;
                  }
                  else if(mSpeed < startSpeed + fadeIn) //else if we are fading in
                  {
                     weight = (mSpeed - startSpeed) / fadeIn;
                  }
                  else if(mSpeed > (stopSpeed - fadeOut)) //else we are fading out
                  {
                     weight = (stopSpeed - mSpeed) / fadeOut;      
                  }

                  dtUtil::Clamp(weight, 0.0f, 1.0f);
                  return weight;
               }

               float mSpeed;
               float mWalkStart, mWalkFadeIn, mWalkStop, mWalkFadeOut;
               float mRunStart, mRunFadeIn, mRunStop, mRunFadeOut;


               dtCore::ObserverPtr<Human> mParentHuman;
               dtCore::RefPtr<dtAnim::Animatable> mWalk;
               dtCore::RefPtr<dtAnim::Animatable> mRun;
         };


            WalkRunBlend(Human* h)
            {
               mWRController = new WRController(*this, h);
               SetController(mWRController.get());
            }

            WalkRunBlend(WRController& controller)
               : mWRController(&controller)

            {
               SetController(mWRController.get());
            }

            void SetAnimations(dtAnim::Animatable* walk, dtAnim::Animatable* run)
            {
               AddAnimation(walk);
               AddAnimation(run);
               mWRController->SetAnimations(walk, run);
            }

            void SetCurrentSpeed(float speed)
            {
               mWRController->SetCurrentSpeed(speed);
            }


            WRController& GetWalkRunController()
            {
               return *mWRController;
            }

            dtCore::RefPtr<dtAnim::Animatable> Clone(dtAnim::Cal3DModelWrapper* modelWrapper) const
            {
               WalkRunBlend* wrb = new WalkRunBlend(*mWRController->CloneDerived());

               if(mWRController->GetWalk() && mWRController->GetRun())
               {
                  wrb->SetAnimations(mWRController->GetWalk()->Clone(modelWrapper).get(), mWRController->GetRun()->Clone(modelWrapper).get());
               }
               return wrb;
            }

      protected:
         ~WalkRunBlend()
         {
            mWRController = 0;
         }

         dtCore::RefPtr<WRController> mWRController;
      };





      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////    HumanActorProxy      ///////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(HumanActorProxy::StanceEnum);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::NOT_APPLICABLE("NOT_APPLICABLE",
            BasicStanceEnum::STANDING);
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

      const dtUtil::RefString HumanActorProxy::PROPERTY_SKELETAL_MESH("Skeletal Mesh");
      const dtUtil::RefString HumanActorProxy::PROPERTY_WEAPON_MESH("Primary Weapon Mesh Name");
      const dtUtil::RefString HumanActorProxy::PROPERTY_STANCE("Stance");
      const dtUtil::RefString HumanActorProxy::PROPERTY_PRIMARY_WEAPON_STATE("Primary Weapon State");
      const dtUtil::RefString HumanActorProxy::PROPERTY_MIN_RUN_VELOCITY("Minimum Run Velocity");
      const dtUtil::RefString HumanActorProxy::PROPERTY_FULL_RUN_VELOCITY("Full Run Velocity");
      
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

         static const dtUtil::RefString HUMAN_GROUP("Human");

         RemoveProperty(BaseEntityActorProxy::PROPERTY_FLAMES_PRESENT);
         RemoveProperty(BaseEntityActorProxy::PROPERTY_SMOKE_PLUME_PRESENT);

         static const dtUtil::RefString PROPERTY_SKELETAL_MESH_DESC
            ("The skeletal mesh file that defines the human's look and animation set.");
         AddProperty(new dtDAL::ResourceActorProperty(*this, dtDAL::DataType::SKELETAL_MESH,
               PROPERTY_SKELETAL_MESH, PROPERTY_SKELETAL_MESH, 
               dtDAL::MakeFunctor(human, &Human::SetSkeletalMeshFile),
               PROPERTY_SKELETAL_MESH_DESC, HUMAN_GROUP));

         static const dtUtil::RefString PROPERTY_STANCE_DESC
            ("The stance of the human.");
         AddProperty(new dtDAL::EnumActorProperty<HumanActorProxy::StanceEnum>(
               PROPERTY_STANCE, PROPERTY_STANCE, 
               dtDAL::MakeFunctor(human, &Human::SetStance),
               dtDAL::MakeFunctorRet(human, &Human::GetStance),
               PROPERTY_STANCE_DESC, HUMAN_GROUP));

         static const dtUtil::RefString PROPERTY_PRIMARY_WEAPON_STATE_DESC
            ("The state/availability of the primary weapon.");
         AddProperty(new dtDAL::EnumActorProperty<HumanActorProxy::WeaponStateEnum>(
               PROPERTY_PRIMARY_WEAPON_STATE, PROPERTY_PRIMARY_WEAPON_STATE, 
               dtDAL::MakeFunctor(human, &Human::SetPrimaryWeaponState),
               dtDAL::MakeFunctorRet(human, &Human::GetPrimaryWeaponState),
               PROPERTY_PRIMARY_WEAPON_STATE_DESC, HUMAN_GROUP));

         static const dtUtil::RefString PROPERTY_WEAPON_MESH_DESC
            ("The name of the mesh in the skeletal mesh that refers to the weapon");
         AddProperty(new dtDAL::StringActorProperty(
               PROPERTY_WEAPON_MESH, PROPERTY_WEAPON_MESH, 
               dtDAL::MakeFunctor(human, &Human::SetWeaponMeshName),
               dtDAL::MakeFunctorRet(human, &Human::GetWeaponMeshName),
               PROPERTY_WEAPON_MESH_DESC, HUMAN_GROUP));

//         static const dtUtil::RefString PROPERTY_MIN_RUN_VELOCITY_DESC
//            ("The Minimum velocity at which the human will begin running");
//
//         AddProperty(new dtDAL::FloatActorProperty(*this,
//               PROPERTY_MIN_RUN_VELOCITY, PROPERTY_MIN_RUN_VELOCITY, 
//               dtDAL::MakeFunctor(human, &Human::SetSkeletalMeshFile),
//               PROPERTY_MIN_RUN_VELOCITY_DESC, HUMAN_GROUP));
//
//         static const dtUtil::RefString PROPERTY_FULL_RUN_VELOCITY_DESC
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
      Human::Human(dtGame::GameActorProxy& proxy) 
         : BaseEntity(proxy)
         , mWeaponMeshName("PrimaryWeapon")
         , mAnimationHelper(new dtAnim::AnimationHelper)
         , mPlannerHelper(
               dtAI::PlannerHelper::RemainingCostFunctor(this, &Human::GetRemainingCost),
               dtAI::PlannerHelper::DesiredStateFunctor(this, &Human::IsDesiredState)
               )
         , mAnimOperators(mPlannerHelper, *mAnimationHelper)
         , mStance(&HumanActorProxy::StanceEnum::UPRIGHT_STANDING)
         , mPrimaryWeaponStateEnum(&HumanActorProxy::WeaponStateEnum::DEPLOYED)
         , mLogger(dtUtil::Log::GetInstance("Human.cpp"))
         , mMaxTimePerIteration(0.5)
      {
         SetDrawingModel(true);
         SetName("Human");
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
         stanceState->SetStance(BasicStanceEnum::STANDING);
         
         initialState.AddState(STATE_BASIC_STANCE, stanceState);

         WeaponState* weaponState = new WeaponState();
         weaponState->SetWeaponStateEnum(HumanActorProxy::WeaponStateEnum::DEPLOYED);
         initialState.AddState(STATE_WEAPON,       weaponState);

         initialState.AddState(STATE_DEAD,         new dtAI::StateVariable(false));

         initialState.AddState(STATE_MOVING,       new dtAI::StateVariable(false));
         //Setting transition to true will make the planner generate the correct initial animation.
         initialState.AddState(STATE_TRANSITION,   new dtAI::StateVariable(true));
         initialState.AddState(STATE_SHOT,         new dtAI::StateVariable(false));

         mPlannerHelper.SetCurrentState(initialState);
      }
      
      ////////////////////////////////////////////////////////////////////////////
      void Human::SetSkeletalMeshFile(const std::string& fileName)
      {
         if (mSkeletalMeshFileName != fileName)
         {
            mSkeletalMeshFileName = fileName;
   
            osg::MatrixTransform& transform = GetScaleMatrixTransform();
            transform.removeChildren(0, transform.getNumChildren());
            mModelNode = NULL;
   
            if (!fileName.empty() && mAnimationHelper->LoadModel(fileName))
            {
               mModelNode = dtAnim::Cal3DDatabase::GetInstance().GetNodeBuilder().CreateNode(mAnimationHelper->GetModelWrapper());

               HandleModelDrawToggle(IsDrawingModel());

               //setup speed blends
               WalkRunBlend* walkRunReady = new WalkRunBlend(this);
               WalkRunBlend* walkRunDeployed = new WalkRunBlend(this);
               walkRunReady->SetName(AnimationOperators::ANIM_WALK_READY);
               walkRunDeployed->SetName(AnimationOperators::ANIM_WALK_DEPLOYED);

               dtAnim::SequenceMixer& seqMixer = mAnimationHelper->GetSequenceMixer();

               const dtAnim::Animatable* walkReady = seqMixer.GetRegisteredAnimation("Walk Ready");
               const dtAnim::Animatable* runReady = seqMixer.GetRegisteredAnimation("Run Ready");
               if(walkReady && runReady)
               {
                  walkRunReady->SetAnimations(walkReady->Clone(mAnimationHelper->GetModelWrapper()).get(), runReady->Clone(mAnimationHelper->GetModelWrapper()).get());
               }
               else
               {
                  LOG_ERROR("Cannot load animations 'Walk Ready' and 'Run Ready' necessary for speed blend.")
               }

               const dtAnim::Animatable* walkDeployed = seqMixer.GetRegisteredAnimation("Walk Deployed");
               const dtAnim::Animatable* runDeployed = seqMixer.GetRegisteredAnimation("Run Deployed");
               if(walkDeployed && runDeployed)
               {
                  walkRunDeployed->SetAnimations(walkDeployed->Clone(mAnimationHelper->GetModelWrapper()).get(), runDeployed->Clone(mAnimationHelper->GetModelWrapper()).get());
               }
               else
               {
                  LOG_ERROR("Cannot load animations 'Walk Deployed' and 'Run Deployed' necessary for speed blend.")
               }

               mAnimationHelper->GetSequenceMixer().RegisterAnimation(walkRunReady);
               mAnimationHelper->GetSequenceMixer().RegisterAnimation(walkRunDeployed);

               //initialize helper
               SetupPlannerHelper();
               UpdatePlanAndAnimations();
               UpdateWeapon();
            }
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
            if (mModelNode.valid())
               GetScaleMatrixTransform().addChild(mModelNode.get());
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         
         //No burning or smoking people.
         //SetFlamesPresentFile("");
         //SetSmokePlumesFile("");

         SetupPlannerHelper();

         if (!GetSkeletalMeshFile().empty())
         {
            UpdatePlanAndAnimations();
         }
         
         dtAnim::AnimationComponent* animComponent = NULL;

         GetGameActorProxy().GetGameManager()->
            GetComponentByName(dtAnim::AnimationComponent::DEFAULT_NAME, animComponent);

         if (animComponent != NULL)
         {
            animComponent->RegisterActor(GetGameActorProxy(), *mAnimationHelper);
         }

         RegisterWithDeadReckoningComponent();
         GetDeadReckoningHelper().SetUseModelDimensions(false);
         GetDeadReckoningHelper().SetAdjustRotationToGround(false);

         if (IsRemote())
         {
            //Need tick remote to check for plan changes.
            GetGameActorProxy().RegisterForMessages(dtGame::MessageType::TICK_REMOTE,
                     dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }
         else
         {
            GetDeadReckoningHelper().SetUpdateMode(dtGame::DeadReckoningHelper::UpdateMode::CALCULATE_ONLY);
         }

      }

      ////////////////////////////////////////////////////////////////////////////
      bool Human::GenerateNewAnimationSequence()
      {
         mCurrentPlan.clear();
         mPlanner.Reset(&mPlannerHelper);
         
         mPlanner.GetConfig().mMaxTimePerIteration = mMaxTimePerIteration;
         
         dtAI::Planner::PlannerResult result = mPlanner.GeneratePlan();
         if (result == dtAI::Planner::PLAN_FOUND)
         {
            mCurrentPlan = mPlanner.GetConfig().mResult;
            //std::cout << " BOGUS TEST -- HUMAN.cpp - Plan took[" << mPlanner.GetConfig().mTotalElapsedTime << "]." << std::endl;
            return true;
         }
         else
         {
            std::ostringstream ss;
            ss << "Unable to generate a plan. Time[" << mPlanner.GetConfig().mTotalElapsedTime
               << "]\n\nGoing from:\n\n"
               << *mPlannerHelper.GetCurrentState()
               << "\n\n Going To:\n\n"
               << "Stance:  \"" << GetStance().GetName() 
               << "\" Primary Weapon: \"" << GetPrimaryWeaponState().GetName()
               << "\" Damage: \"" << GetDamageState().GetName() 
               << "\" Velocty: \"" << GetVelocityVector() << "\"";
            mLogger.LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__, ss.str());
         }
         return false;
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
         UpdateWeapon();
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::UpdateWeapon()
      {
         /// No weapon mesh was set.
         if (GetWeaponMeshName().empty())
            return;

         dtAnim::Cal3DModelWrapper* wrapper = mAnimationHelper->GetModelWrapper();

         //Can't update if the wrapper is NULL.
         if (wrapper == NULL)
            return;

         //get all data for the meshes and emit
         for (int meshID=0; meshID < wrapper->GetCoreMeshCount(); meshID++)
         {
            const std::string& nameToSend = wrapper->GetCoreMeshName(meshID);
            if (nameToSend == GetWeaponMeshName())
            {
               bool visibleWeapon = *mPrimaryWeaponStateEnum == HumanActorProxy::WeaponStateEnum::FIRING_POSITION
                  || *mPrimaryWeaponStateEnum == HumanActorProxy::WeaponStateEnum::DEPLOYED;

               if (visibleWeapon)
               {
                  wrapper->ShowMesh(meshID);
               }
               else
               {
                  wrapper->HideMesh(meshID);
               }
               break;
            }
         }
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

         const dtAI::StateVariable* deadState;
         pWS->GetState(STATE_DEAD, deadState);

         //dead is the same as the damage state being equal to destroyed.
         if (deadState->Get() != (GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED))
            return false;

         //If we are dead, ignore any other changes.  Just let's be dead, shall we :-)
         if (deadState->Get() && GetDamageState() == BaseEntityActorProxy::DamageStateEnum::DESTROYED)
            return true;

         const BasicStanceState* stanceState;
         pWS->GetState(STATE_BASIC_STANCE, stanceState);

         if (stanceState->GetStance() != mStance->GetAssocBasicStanceEnum())
            return false;

         const WeaponState* weaponState;
         pWS->GetState(STATE_WEAPON, weaponState);

         HumanActorProxy::WeaponStateEnum* effectiveWeaponState = &HumanActorProxy::WeaponStateEnum::FIRING_POSITION;
         if (*mPrimaryWeaponStateEnum != HumanActorProxy::WeaponStateEnum::FIRING_POSITION)
            effectiveWeaponState = &HumanActorProxy::WeaponStateEnum::DEPLOYED;

         if (weaponState->GetWeaponStateEnum() != *effectiveWeaponState)
            return false;

         const dtAI::StateVariable* movingState;
         pWS->GetState(STATE_MOVING, movingState);

         //This requires that plans be made in one frame.
         //Moving is the same as the velocity > 0.
         if (movingState->Get() != !dtUtil::Equivalent(GetVelocityVector().length2(), 0.0f))
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
         CheckAndUpdateAnimationState();
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::TickLocal(const dtGame::Message& tickLocal)
      {
         //just in case
         BaseClass::TickLocal(tickLocal);
         CheckAndUpdateAnimationState();
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::CheckAndUpdateAnimationState()
      {
         if (!IsDesiredState(mPlannerHelper.GetCurrentState()) && !GetSkeletalMeshFile().empty())
         {
            if (mLogger.IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               mLogger.LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                     "Planner is not in the desired state.  Generating animations.");
            }

            UpdatePlanAndAnimations();
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::UpdatePlanAndAnimations()
      {
         const float blendTime = 0.2f;

         const dtAI::StateVariable* deadState;
         mPlannerHelper.GetCurrentState()->GetState(STATE_DEAD, deadState);

         //if we WERE dead and now we are not, we have to reset our state.
         if (deadState->Get() && (GetDamageState() != BaseEntityActorProxy::DamageStateEnum::DESTROYED))
            SetupPlannerHelper();

         bool gottaSequence = GenerateNewAnimationSequence();
         if (!gottaSequence)
         {
            SetupPlannerHelper();
            gottaSequence = GenerateNewAnimationSequence();
         }
         
         if (gottaSequence)
         {
            dtAI::Planner::OperatorList::iterator i, iend;
            dtAnim::SequenceMixer& seqMixer = mAnimationHelper->GetSequenceMixer();
            dtCore::RefPtr<dtAnim::AnimationSequence> generatedSequence = new dtAnim::AnimationSequence();

            static int count = 0;
            std::ostringstream ss;
            //nasty hack to make sure the sequence names don't match.
            ss << "sequence " << count;
            ++count;
            generatedSequence->SetName(ss.str());

            if (mLogger.IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               mLogger.LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                     "Current animation plan has \"%d\" steps.", mCurrentPlan.size());
            }

            if (!mCurrentPlan.empty())
            {
               i = mCurrentPlan.begin();
               iend = mCurrentPlan.end();

               float accumulatedStartTime = 0.0f;

               dtCore::RefPtr<dtAnim::Animatable> newAnim;
               for (; i != iend; ++i)
               {
                  //if the last anim was NOT the last one, it has to end and be an action
                  if (newAnim.valid())
                  {
                     dtAnim::AnimationChannel* animChannel = dynamic_cast<dtAnim::AnimationChannel*>(newAnim.get());
                     if (animChannel != NULL)
                     {

                        float duration = animChannel->GetAnimation()->GetDuration();
                        accumulatedStartTime += (duration - blendTime);
                        animChannel->SetMaxDuration(duration);
                        animChannel->SetAction(true);
                     }
                  }

                  const dtAI::Operator* op = *i;

                  op->Apply(mPlannerHelper.GetCurrentState());

                  const dtAnim::Animatable* animatable = seqMixer.GetRegisteredAnimation(op->GetName());
                  if (animatable != NULL)
                  {
                     if (mLogger.IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
                     {
                        mLogger.LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                              "Adding animatable named \"%s\".", animatable->GetName().c_str());
                     }
                     newAnim = animatable->Clone(mAnimationHelper->GetModelWrapper());
                     newAnim->SetStartDelay(std::max(0.0f, accumulatedStartTime));
                     newAnim->SetFadeIn(blendTime);
                     newAnim->SetFadeOut(blendTime);

                     generatedSequence->AddAnimation(newAnim.get());
                  }
                  else
                  {
                     newAnim = NULL;
                  }
               }
               
               seqMixer.ClearActiveAnimations(blendTime);
               seqMixer.PlayAnimation(generatedSequence.get());
            }
         }
         else
         {
            //This is the error-out state.
            mAnimationHelper->ClearAll(blendTime);
            mAnimationHelper->PlayAnimation(AnimationOperators::ANIM_WALK_DEPLOYED);
         }
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
      const std::string AnimationOperators::ANIM_WALK_READY("Walk Run Ready");
      const std::string AnimationOperators::ANIM_WALK_DEPLOYED("Walk Run Deployed");
      
      const std::string AnimationOperators::ANIM_KNEEL_READY("Kneel Ready");
      const std::string AnimationOperators::ANIM_KNEEL_DEPLOYED("Kneel Deployed");

      const std::string AnimationOperators::ANIM_LOW_WALK_READY("Low Walk Ready");
      const std::string AnimationOperators::ANIM_LOW_WALK_DEPLOYED("Low Walk Deployed");

      const std::string AnimationOperators::ANIM_CRAWL_READY("Crawl Ready");
      const std::string AnimationOperators::ANIM_CRAWL_DEPLOYED("Crawl Deployed");

      const std::string AnimationOperators::ANIM_STAND_TO_KNEEL("Stand To Kneel");
      const std::string AnimationOperators::ANIM_KNEEL_TO_STAND("Kneel To Stand");

      const std::string AnimationOperators::ANIM_PRONE_READY("Prone Ready");
      const std::string AnimationOperators::ANIM_PRONE_DEPLOYED("Prone Deployed");

      const std::string AnimationOperators::ANIM_PRONE_TO_KNEEL("Prone To Kneel");
      const std::string AnimationOperators::ANIM_KNEEL_TO_PRONE("Kneel To Prone");

      const std::string AnimationOperators::ANIM_SHOT_STANDING("Shot Standing");
      const std::string AnimationOperators::ANIM_SHOT_KNEELING("Shot Kneeling");
      const std::string AnimationOperators::ANIM_SHOT_PRONE("Shot Prone");

      const std::string AnimationOperators::ANIM_DEAD_STANDING("Dead Standing");
      const std::string AnimationOperators::ANIM_DEAD_KNEELING("Dead Kneeling");
      const std::string AnimationOperators::ANIM_DEAD_PRONE("Dead Prone");

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

         newOp = AddOperator(ANIM_LOW_WALK_READY);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(ready.get());

         newOp->AddEffect(kneelingEff.get());
         newOp->AddEffect(movingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_LOW_WALK_DEPLOYED);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(deployed.get());

         newOp->AddEffect(kneelingEff.get());
         newOp->AddEffect(movingEff.get());
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

         newOp = AddOperator(ANIM_PRONE_READY);
         newOp->AddPreCondition(prone.get());
         newOp->AddPreCondition(ready.get());

         newOp->AddEffect(proneEff.get());
         newOp->AddEffect(notMovingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_PRONE_DEPLOYED);
         newOp->AddPreCondition(prone.get());
         newOp->AddPreCondition(deployed.get());

         newOp->AddEffect(proneEff.get());
         newOp->AddEffect(notMovingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_PRONE_TO_KNEEL);
         newOp->AddPreCondition(prone.get());
         newOp->AddPreCondition(deployed.get());
         newOp->AddPreCondition(notMoving.get());

         newOp->AddEffect(kneelingEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(ANIM_CRAWL_READY);
         newOp->AddPreCondition(prone.get());
         newOp->AddPreCondition(ready.get());

         newOp->AddEffect(proneEff.get());
         newOp->AddEffect(movingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_CRAWL_DEPLOYED);
         newOp->AddPreCondition(prone.get());
         newOp->AddPreCondition(deployed.get());

         newOp->AddEffect(proneEff.get());
         newOp->AddEffect(movingEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_KNEEL_TO_PRONE);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(deployed.get());
         newOp->AddPreCondition(notMoving.get());

         newOp->AddEffect(proneEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(OPER_READY_TO_DEPLOYED);
         newOp->AddPreCondition(ready.get());
         newOp->AddEffect(deployedEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(OPER_DEPLOYED_TO_READY);
         newOp->AddPreCondition(deployed.get());
         newOp->AddEffect(readyEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(ANIM_SHOT_STANDING);
         newOp->AddPreCondition(standing.get());
         newOp->AddEffect(shotEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(ANIM_SHOT_KNEELING);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddEffect(shotEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(ANIM_SHOT_PRONE);
         newOp->AddPreCondition(prone.get());
         newOp->AddEffect(shotEff.get());
         newOp->AddEffect(transitionEff.get());

         newOp = AddOperator(ANIM_DEAD_STANDING);
         newOp->AddPreCondition(standing.get());
         newOp->AddPreCondition(isShot.get());
         newOp->AddEffect(deadEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_DEAD_KNEELING);
         newOp->AddPreCondition(kneeling.get());
         newOp->AddPreCondition(isShot.get());
         newOp->AddEffect(deadEff.get());
         newOp->AddEffect(notTransitionEff.get());

         newOp = AddOperator(ANIM_DEAD_PRONE);
         newOp->AddPreCondition(prone.get());
         newOp->AddPreCondition(isShot.get());
         newOp->AddEffect(deadEff.get());
         newOp->AddEffect(notTransitionEff.get());
      }
   }
}
