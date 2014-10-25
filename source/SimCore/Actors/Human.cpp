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
 * @author Bradley Anderegg
 */
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/VisibilityOptions.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/ActComps/WeaponInventoryActComp.h>

#include <dtAnim/animnodebuilder.h>
#include <dtAnim/animationcomponent.h>
#include <dtAnim/animationsequence.h>
#include <dtAnim/animationchannel.h>
#include <dtAnim/basemodelwrapper.h>
#include <dtAnim/sequencemixer.h>
#include <dtAnim/walkrunblend.h>
#include <dtAnim/animationtransitionplanner.h>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/messagetype.h>
#include <dtGame/drpublishingactcomp.h>

#include <dtCore/functor.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/datatype.h>
#include <dtCore/project.h>
#include <dtCore/actortype.h>

#include <dtUtil/log.h>
#include <dtUtil/macros.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/configproperties.h>

DT_DISABLE_WARNING_ALL_START
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/io_utils>
#include <sstream>
DT_DISABLE_WARNING_END

#ifdef DELTA_WIN32
   #pragma warning(disable : 4355)
#endif


namespace SimCore
{

   namespace Actors
   {

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////    HumanActorProxy      ///////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(HumanActorProxy::StanceEnum);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::NOT_APPLICABLE("NOT_APPLICABLE",
            dtAnim::BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::UPRIGHT_STANDING("UPRIGHT_STANDING",
            dtAnim::BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::UPRIGHT_WALKING("UPRIGHT_WALKING",
            dtAnim::BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::UPRIGHT_RUNNING("UPRIGHT_RUNNING",
            dtAnim::BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::KNEELING("KNEELING",
            dtAnim::BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::PRONE("PRONE",
            dtAnim::BasicStanceEnum::PRONE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::CRAWLING("CRAWLING",
            dtAnim::BasicStanceEnum::PRONE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::SWIMMING("SWIMMING",
            dtAnim::BasicStanceEnum::PRONE);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::PARACHUTING("PARACHUTING",
            dtAnim::BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::JUMPING("JUMPING",
            dtAnim::BasicStanceEnum::STANDING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::SITTING("SITTING",
            dtAnim::BasicStanceEnum::SITTING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::SQUATTING("SQUATTING",
            dtAnim::BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::CROUCHING("CROUCHING",
            dtAnim::BasicStanceEnum::KNEELING);
      HumanActorProxy::StanceEnum HumanActorProxy::StanceEnum::WADING("WADING",
            dtAnim::BasicStanceEnum::STANDING);

      HumanActorProxy::StanceEnum::StanceEnum(const std::string& name, dtAnim::BasicStanceEnum& assocBasicStance)
         : dtUtil::Enumeration(name)
      {
         AddInstance(this);
         mAssocBasicStance = &assocBasicStance;
      }

      dtAnim::BasicStanceEnum& HumanActorProxy::StanceEnum::GetAssocBasicStanceEnum()
      {
         return *mAssocBasicStance;
      }

      const dtUtil::RefString HumanActorProxy::PROPERTY_WEAPON_MESH("Primary Weapon Mesh Name");
      const dtUtil::RefString HumanActorProxy::PROPERTY_STANCE("Current Stance");
      const dtUtil::RefString HumanActorProxy::PROPERTY_PRIMARY_WEAPON_STATE("Primary Weapon State");
      const dtUtil::RefString HumanActorProxy::PROPERTY_MIN_RUN_VELOCITY("Minimum Run Velocity");
      const dtUtil::RefString HumanActorProxy::PROPERTY_FULL_RUN_VELOCITY("Full Run Velocity");
      const dtUtil::RefString HumanActorProxy::PROPERTY_WALK_ANIMATION_SPEED("Walk Animation Speed");

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

         Human* human;
         GetDrawable(human);

         static const dtUtil::RefString HUMAN_GROUP("Human");

         RemoveProperty(BaseEntityActorProxy::PROPERTY_FLAMES_PRESENT);
         RemoveProperty(BaseEntityActorProxy::PROPERTY_SMOKE_PLUME_PRESENT);

         static const dtUtil::RefString PROPERTY_STANCE_DESC
            ("The stance of the human.");
         AddProperty(new dtCore::EnumActorProperty<HumanActorProxy::StanceEnum>(
               PROPERTY_STANCE, PROPERTY_STANCE,
               dtCore::EnumActorProperty<HumanActorProxy::StanceEnum>::SetFuncType(human, &Human::SetStance),
               dtCore::EnumActorProperty<HumanActorProxy::StanceEnum>::GetFuncType(human, &Human::GetStance),
               PROPERTY_STANCE_DESC, HUMAN_GROUP));

         static const dtUtil::RefString PROPERTY_PRIMARY_WEAPON_STATE_DESC
            ("The state/availability of the primary weapon.");
         AddProperty(new dtCore::EnumActorProperty<dtAnim::WeaponStateEnum>(
               PROPERTY_PRIMARY_WEAPON_STATE, PROPERTY_PRIMARY_WEAPON_STATE,
               dtCore::EnumActorProperty<dtAnim::WeaponStateEnum>::SetFuncType(human, &Human::SetPrimaryWeaponState),
               dtCore::EnumActorProperty<dtAnim::WeaponStateEnum>::GetFuncType(human, &Human::GetPrimaryWeaponState),
               PROPERTY_PRIMARY_WEAPON_STATE_DESC, HUMAN_GROUP));

         static const dtUtil::RefString PROPERTY_WEAPON_MESH_DESC
            ("The name of the mesh in the skeletal mesh that refers to the weapon");
         AddProperty(new dtCore::StringActorProperty(
               PROPERTY_WEAPON_MESH, PROPERTY_WEAPON_MESH,
               dtCore::StringActorProperty::SetFuncType(human, &Human::SetWeaponMeshName),
               dtCore::StringActorProperty::GetFuncType(human, &Human::GetWeaponMeshName),
               PROPERTY_WEAPON_MESH_DESC, HUMAN_GROUP));

         static const dtUtil::RefString PROPERTY_WALK_SPEED_DESC
            ("The inherent speed of the Walk animation.");
         AddProperty(new dtCore::FloatActorProperty(
               PROPERTY_WALK_ANIMATION_SPEED, PROPERTY_WALK_ANIMATION_SPEED,
               dtCore::FloatActorProperty::SetFuncType(human, &Human::SetWalkAnimationSpeed),
               dtCore::FloatActorProperty::GetFuncType(human, &Human::GetWalkAnimationSpeed),
               PROPERTY_WALK_SPEED_DESC, HUMAN_GROUP));

//         static const dtUtil::RefString PROPERTY_MIN_RUN_VELOCITY_DESC
//            ("The Minimum velocity at which the human will begin running");
//
//         static const dtUtil::RefString PROPERTY_FULL_RUN_VELOCITY_DESC
//            ("The velocity at which the human will be fully running");
//
//         AddProperty(new dtCore::FloatActorProperty(*this,
//               PROPERTY_FULL_RUN_VELOCITY, PROPERTY_FULL_RUN_VELOCITY,
//               dtCore::MakeFunctor(human, &Human::SetSkeletalMeshFile),
//               PROPERTY_FULL_RUN_VELOCITY_DESC, HUMAN_GROUP));
//
      }

      ////////////////////////////////////////////////////////////////////////////
      void HumanActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();
      }

      ////////////////////////////////////////////////////////////////////////////
      void HumanActorProxy::BuildActorComponents()
      {
         const dtCore::ActorType& at = GetActorType();

         BaseClass::BuildActorComponents();

         dtAnim::AnimationHelper* animAC = NULL;
         if (!HasComponent(dtAnim::AnimationHelper::TYPE))
         {
            animAC = new dtAnim::AnimationHelper();
            AddComponent(*animAC);
         }
         else
         {
            animAC = GetComponent<dtAnim::AnimationHelper>();
         }

         if (animAC != NULL)
         {
            animAC->SetLoadModelAsynchronously(true);
            animAC->SetEnableAttachingNodeToDrawable(false);
            Human* human;
            GetDrawable(human);
            animAC->ModelLoadedSignal.connect_slot(human, &Human::SkeletalMeshLoadCallback);
            animAC->ModelUnloadedSignal.connect_slot(human, &Human::SkeletalMeshUnloadCallback);
         }

         AddComponent(*new dtAnim::AnimationTransitionPlanner);

         if (at.InstanceOf(*EntityActorRegistry::WARFIGHTER_ACTOR_TYPE))
         {
            if (!HasComponent(SimCore::ActComps::WeaponInventoryActComp::TYPE))
            {
               AddComponent(*new SimCore::ActComps::WeaponInventoryActComp());
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void HumanActorProxy::CreateDrawable()
      {
         Human* human = new Human(*this);
         SetDrawable(*human);
      }

      ////////////////////////////////////////////////////////////////////////////
      ////////////////////////    Human                ///////////////////////////
      ////////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString Human::CONFIG_ALWAYS_SHOW_WEAPON("SimCore.Human.AlwaysShowWeapon");

      ////////////////////////////////////////////////////////////////////////////
      Human::Human(dtGame::GameActorProxy& owner)
         : BaseEntity(owner)
         , mWalkAnimationSpeed(1.00f)
         , mWeaponMeshName("PrimaryWeapon")
         , mStance(&HumanActorProxy::StanceEnum::UPRIGHT_STANDING)
         , mPrimaryWeaponStateEnum(&dtAnim::WeaponStateEnum::DEPLOYED)
         , mLogger(dtUtil::Log::GetInstance("Human.cpp"))
         , mModelLoadedAndWaiting(false)
      {
         SetName("Human");
      }

      ////////////////////////////////////////////////////////////////////////////
      Human::~Human()
      {
      }

      /////////////////////////////////////////////////////////////////
      void Human::SkeletalMeshUnloadCallback(dtAnim::AnimationHelper*)
      {
         SetNodeCollector(NULL);
         osg::MatrixTransform& transform = GetScaleMatrixTransform();
         transform.removeChild(mModelNode);
         mModelNode = NULL;
      }

      /////////////////////////////////////////////////////////////////
      void SearchAndRegisterAnimationOptions(dtAnim::SequenceMixer& seqMixer, const dtUtil::RefString& name, const std::vector<dtUtil::RefString>& options, dtAnim::BaseModelWrapper& wrapper)
      {
         dtCore::RefPtr<const dtAnim::Animatable> anim = seqMixer.GetRegisteredAnimation(name);
         if (anim != NULL) return;

         for (unsigned i = 0; anim == NULL && i < options.size(); ++i)
         {
            anim = seqMixer.GetRegisteredAnimation(options[i]);
         }

         if (anim != NULL)
         {
            LOG_ALWAYS("Registering Animation named \"" + anim->GetName() + "\" in place of missing \"" + name + "\"");
            dtCore::RefPtr<dtAnim::Animatable> animClone = anim->Clone(&wrapper).get();
            animClone->SetName(name);

            seqMixer.RegisterAnimation(animClone);
         }
      }

      /////////////////////////////////////////////////////////////////
      void Human::SetupWalkRunBlend(dtAnim::AnimationHelper* helper, const dtUtil::RefString& OpName,
            const std::vector<dtUtil::RefString>& nameWalkOptions, const std::string& newWalkAnimName,
            const std::vector<dtUtil::RefString>& nameRunOptions, const std::string& newRunAnimName,
            const std::vector<dtUtil::RefString>& nameStandOptions, const std::string& newStandAnimName,
            float walkSpeed, float runSpeed)
      {
         dtCore::RefPtr<dtAnim::WalkRunBlend> newWRBlend;
         if (IsRemote())
         {
            newWRBlend = new dtAnim::WalkRunBlend(*GetComponent<dtGame::DeadReckoningHelper>());
         }
         else
         {
            newWRBlend = new dtAnim::WalkRunBlend(*GetComponent<dtGame::DRPublishingActComp>());
         }
         newWRBlend->SetName(OpName);

         dtAnim::SequenceMixer& seqMixer = helper->GetSequenceMixer();
         dtAnim::BaseModelWrapper* wrapper = helper->GetModelWrapper();

         SearchAndRegisterAnimationOptions(seqMixer, newWalkAnimName, nameWalkOptions, *wrapper);
         SearchAndRegisterAnimationOptions(seqMixer, newRunAnimName, nameRunOptions, *wrapper);
         SearchAndRegisterAnimationOptions(seqMixer, newStandAnimName, nameStandOptions, *wrapper);

         const dtAnim::Animatable* stand = seqMixer.GetRegisteredAnimation(newStandAnimName);
         const dtAnim::Animatable* walk = seqMixer.GetRegisteredAnimation(newWalkAnimName);
         const dtAnim::Animatable* run = seqMixer.GetRegisteredAnimation(newRunAnimName);

         if(stand != NULL && walk != NULL && run != NULL)
         {
            newWRBlend->SetAnimations(stand->Clone(wrapper).get(), walk->Clone(wrapper).get(), run->Clone(wrapper).get());
            newWRBlend->Setup(walkSpeed, runSpeed);
            seqMixer.RegisterAnimation(newWRBlend);
         }
         else
         {
            if (stand != NULL && walk != NULL)
            {
               newWRBlend->SetAnimations(stand->Clone(wrapper).get(), walk->Clone(wrapper).get(), NULL);
               newWRBlend->Setup(walkSpeed, runSpeed);
               seqMixer.RegisterAnimation(newWRBlend);
            }
            else if (walk != NULL)
            {
               // Can't do much right now with just a walk.
               dtCore::RefPtr<dtAnim::Animatable> walkClone = walk->Clone(wrapper).get();
               walkClone->SetName(OpName);
               seqMixer.RegisterAnimation(walkClone);
               LOGN_WARNING("Human.cpp", "Cannot load/find a motionless animation for: " + OpName);
            }
            else if (stand != NULL)
            {
               // Can't do much right now with just a stand.
               dtCore::RefPtr<dtAnim::Animatable> standClone = stand->Clone(wrapper).get();
               standClone->SetName(OpName);
               seqMixer.RegisterAnimation(standClone);
               LOGN_WARNING("Human.cpp", "Cannot load/find a walking animation for: " + OpName);
            }
            else
            {
               LOGN_WARNING("Human.cpp", "Cannot load any walk or run animations for: " + OpName);
            }
         }
      }

      /////////////////////////////////////////////////////////////////
      void Human::SkeletalMeshLoadCallback(dtAnim::AnimationHelper* helper)
      {
         mModelLoadedAndWaiting = false;

         osg::MatrixTransform& transform = GetScaleMatrixTransform();
         mModelNode = helper->GetNode();
         transform.addChild(mModelNode.get());
         mModelNode->setName(helper->GetSkeletalMesh().GetResourceIdentifier());

         std::vector<dtUtil::RefString> optionsWalk;
         std::vector<dtUtil::RefString> optionsRun;
         std::vector<dtUtil::RefString> optionsStand;

         dtUtil::RefString animationNamesW[4] = { "WalkReady", "Walk", "Walk Deployed", "" };
         dtUtil::RefString animationNamesR[4] = { "RunReady", "Run", "Run Deployed", "" };
         dtUtil::RefString animationNamesIdle[8] = { "Stand Deployed", "StandDeployed", "Stand", "Idle", "Idle_2", "StandReady", "Stand Ready", "" };

         optionsWalk.insert(optionsWalk.end(), &animationNamesW[0], &animationNamesW[3]);
         optionsRun.insert(optionsRun.end(), &animationNamesR[0], &animationNamesR[3]);
         optionsStand.insert(optionsStand.end(), &animationNamesIdle[0], &animationNamesIdle[7]);


         SetupWalkRunBlend(helper, dtAnim::AnimationOperators::ANIM_WALK_READY, optionsWalk, "Walk Ready",
               optionsRun, "Run Ready",
               optionsStand, "Stand Ready",
               GetWalkAnimationSpeed(), GetWalkAnimationSpeed() * 2.0f);

         std::reverse(optionsWalk.begin(), optionsWalk.end());
         std::reverse(optionsRun.begin(), optionsRun.end());
         std::reverse(optionsStand.begin(), optionsStand.end());
         SetupWalkRunBlend(helper, dtAnim::AnimationOperators::ANIM_WALK_DEPLOYED, optionsWalk, "Walk Deployed",
               optionsRun, "Run Deployed",
               optionsStand, "Stand Deployed",
               GetWalkAnimationSpeed(), GetWalkAnimationSpeed() * 2.0f);


         dtUtil::RefString animationNamesCrawl[6] = { "Crawl Ready", "CrawlReady", "Crawl", "CrawlDeployed", "Crawl Deployed", "" };
         dtUtil::RefString animationNamesProne[6] = { "Prone Deployed", "ProneDeployed", "Prone", "ProneReady","Prone Ready", "" };

         optionsStand.clear();
         optionsWalk.clear();
         optionsRun.clear();
         optionsWalk.insert(optionsWalk.end(), &animationNamesCrawl[0], &animationNamesCrawl[5]);
         optionsStand.insert(optionsStand.end(), &animationNamesProne[0], &animationNamesProne[5]);

         SetupWalkRunBlend(helper, dtAnim::AnimationOperators::ANIM_CRAWL_READY, optionsWalk, "Crawl Ready",
               optionsRun, "",
               optionsStand, "Prone Ready",
               GetWalkAnimationSpeed(), GetWalkAnimationSpeed() * 2.0f);

         std::reverse(optionsWalk.begin(), optionsWalk.end());
         std::reverse(optionsRun.begin(), optionsRun.end());
         std::reverse(optionsStand.begin(), optionsStand.end());
         SetupWalkRunBlend(helper, dtAnim::AnimationOperators::ANIM_CRAWL_DEPLOYED, optionsWalk, "Crawl Deployed",
               optionsRun, "",
               optionsStand, "Prone Deployed",
               GetWalkAnimationSpeed(), GetWalkAnimationSpeed() * 2.0f);

         dtUtil::RefString animationNamesLowWalk[6] = { "Low Walk Ready", "LowWalkReady", "Low Walk", "LowWalkDeployed", "Low Walk Deployed", "" };
         dtUtil::RefString animationNamesCrouch[6] = { "Crouch Deployed", "CrouchDeployed", "Crouch", "CrouchReady","Crouch Ready", "" };

         optionsStand.clear();
         optionsWalk.clear();
         optionsRun.clear();
         optionsWalk.insert(optionsWalk.end(), &animationNamesLowWalk[0], &animationNamesLowWalk[5]);
         optionsStand.insert(optionsStand.end(), &animationNamesCrouch[0], &animationNamesCrouch[5]);

         SetupWalkRunBlend(helper, dtAnim::AnimationOperators::ANIM_LOW_WALK_READY, optionsWalk, "Low Walk Ready",
               optionsRun, "",
               optionsStand, "Kneel Ready",
               GetWalkAnimationSpeed(), GetWalkAnimationSpeed() * 2.0f);

         std::reverse(optionsWalk.begin(), optionsWalk.end());
         std::reverse(optionsRun.begin(), optionsRun.end());
         std::reverse(optionsStand.begin(), optionsStand.end());
         SetupWalkRunBlend(helper, dtAnim::AnimationOperators::ANIM_LOW_WALK_DEPLOYED, optionsWalk, "Low Walk Deployed",
               optionsRun, "",
               optionsStand, "Kneel Deployed",
               GetWalkAnimationSpeed(), GetWalkAnimationSpeed() * 2.0f);

         dtAnim::AttachmentController* atcl = helper->GetAttachmentController();
         if (atcl == NULL)
         {
            LOG_ERROR("Human \"" + GetName() + "\" does not have an attachement controller for character model.");
         }
         else
         {
            for (unsigned i = 0; i < atcl->GetNumAttachments(); ++i)
            {
               AddChild(atcl->GetAttachment(i)->first, GetScaleMatrixTransform().getName());
            }
         }

         //initialize helper
         UpdateWeapon();

         SimCore::ActComps::WeaponInventoryActComp* weaponsAC = GetComponent<SimCore::ActComps::WeaponInventoryActComp>();
         if (weaponsAC != NULL)
         {
            SimCore::ActComps::WeaponInventoryActComp::WeaponData* wd = weaponsAC->GetCurrentWeapon();
            if (wd != NULL)
            {
                weaponsAC->SelectWeapon(NULL);
            }
         }

         SetNodeCollector(NULL);
         LoadNodeCollector();
                
         if (weaponsAC != NULL)
         {
             weaponsAC->SelectNextWeapon();
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         //No burning or smoking people.
         //SetFlamesPresentFile("");
         //SetSmokePlumesFile("");

         dtGame::DeadReckoningHelper* drHelper = NULL;
         GetComponent(drHelper);

         //RegisterWithDeadReckoningComponent(); // Now handled in base class
         drHelper->SetUseModelDimensions(false);
         drHelper->SetAdjustRotationToGround(false);

         if (IsRemote())
         {
            //Need tick remote to check for plan changes.
            GetGameActorProxy().RegisterForMessages(dtGame::MessageType::TICK_REMOTE,
                     dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
         }
         else
         {
            drHelper->SetUpdateMode(dtGame::DeadReckoningHelper::UpdateMode::CALCULATE_ONLY);
         }

      }


      ////////////////////////////////////////////////////////////////////////////
      void Human::ExecuteAction(const dtUtil::RefString& animatableName, HumanActorProxy::StanceEnum& stance)
      {
         GetComponent<dtAnim::AnimationTransitionPlanner>()->ExecuteAction(animatableName, stance.GetAssocBasicStanceEnum());
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::SetStance(HumanActorProxy::StanceEnum& stance)
      {
         mStance = &stance;
         GetComponent<dtAnim::AnimationTransitionPlanner>()->SetStance(stance.GetAssocBasicStanceEnum());
      }

      ////////////////////////////////////////////////////////////////////////////
      HumanActorProxy::StanceEnum& Human::GetStance() const
      {
         return *mStance;
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::SetPrimaryWeaponState(dtAnim::WeaponStateEnum& weaponState)
      {
         mPrimaryWeaponStateEnum = &weaponState;
         GetComponent<dtAnim::AnimationTransitionPlanner>()->SetWeaponState(weaponState);
         UpdateWeapon();
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::SetDamageState(BaseEntityActorProxy::DamageStateEnum& damageState)
      {
         BaseClass::SetDamageState(damageState);
         GetComponent<dtAnim::AnimationTransitionPlanner>()->SetIsDead(damageState == BaseEntityActorProxy::DamageStateEnum::DESTROYED);
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::UpdateWeapon()
      {
         /// No weapon mesh was set.
         if (GetWeaponMeshName().empty())
         {
            return;
         }

         dtAnim::BaseModelWrapper* wrapper = GetComponent<dtAnim::AnimationHelper>()->GetModelWrapper();

         //Can't update if the wrapper is NULL.
         if (wrapper == NULL)
            return;

         //TODO is it worth holding onto this vector, to avoid having to re-parse it
         dtUtil::IsDelimeter del(',');
         std::vector<std::string> tokens;

         dtUtil::StringTokenizer<dtUtil::IsDelimeter>::tokenize(tokens, GetWeaponMeshName(), del);

         //get all data for the meshes and emit
         dtAnim::MeshInterface* mesh = NULL;
         dtAnim::MeshArray meshes;
         wrapper->GetMeshes(meshes);
         dtAnim::MeshArray::iterator curIter = meshes.begin();
         dtAnim::MeshArray::iterator endIter = meshes.end();
         for (; curIter != endIter; ++curIter)
         {
            mesh = curIter->get();
            const std::string& nameToSend = mesh->GetName();
            if (GetContainsWeaponName(tokens, nameToSend))
            {
               //in the editor environment there may be no game manager
               bool alwaysShowWeapon = false;
               if (!GetGameActorProxy().IsInSTAGE())
               {
                  dtUtil::ConfigProperties& configParams = GetGameActorProxy().GetGameManager()->GetConfiguration();
                  alwaysShowWeapon = dtUtil::ToType<bool>(configParams.GetConfigPropertyValue(CONFIG_ALWAYS_SHOW_WEAPON, "false"));
               }
               

               bool visibleWeapon = alwaysShowWeapon || (*mPrimaryWeaponStateEnum == dtAnim::WeaponStateEnum::FIRING_POSITION)
                  || (*mPrimaryWeaponStateEnum == dtAnim::WeaponStateEnum::DEPLOYED);

               mesh->SetVisible(visibleWeapon);

               break;
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      bool Human::GetContainsWeaponName(const std::vector<std::string>& tokens, const std::string& meshName) const
      {
         std::vector<std::string>::const_iterator iter = tokens.begin();
         std::vector<std::string>::const_iterator iterEnd = tokens.end();
         for(; iter != iterEnd; ++iter)
         { 
            if(meshName == *iter)
            {
               return true;
            }
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////
      float Human::CalculateWalkingSpeed() const
      {
         if (IsRemote())
         {
            return GetComponent<dtGame::DeadReckoningHelper>()->GetVelocity().length();
         }
         return GetComponent<dtGame::DRPublishingActComp>()->GetVelocity().length();
      }

      DT_IMPLEMENT_ACCESSOR(Human, float, WalkAnimationSpeed);

      ////////////////////////////////////////////////////////////////////////////
      dtAnim::WeaponStateEnum& Human::GetPrimaryWeaponState() const
      {
         return *mPrimaryWeaponStateEnum;
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::OnTickRemote(const dtGame::TickMessage& tickRemote)
      {
         //just in case
         BaseClass::OnTickRemote(tickRemote);
      }

      ////////////////////////////////////////////////////////////////////////////
      void Human::OnTickLocal(const dtGame::TickMessage& tickLocal)
      {
         //just in case
         BaseClass::OnTickLocal(tickLocal);
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool Human::ShouldBeVisible(const SimCore::VisibilityOptions& options)
      {
         const BasicVisibilityOptions& basicOptions = options.GetBasicOptions();
         bool baseVal = BaseClass::ShouldBeVisible(options);
         return baseVal && basicOptions.mDismountedInfantry;
      }


   }
}
