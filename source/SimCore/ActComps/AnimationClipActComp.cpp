/* -*-c++-*-
 * SimulationCore
 * Copyright 2013, David Guthrie
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
 * Chris Rodgers
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/ActComps/AnimationClipActComp.h>
#include <dtAnim/animclippath.h>
#include <dtCore/propertycontaineractorproperty.h>
#include <dtCore/propertymacros.h>
#include <dtCore/arrayactorpropertycomplex.h>
#include <dtCore/enumactorproperty.h>
#include <dtGame/gameactor.h>
#include <osg/AnimationPath>
#include <osg/NodeVisitor>


using namespace dtAnim;



namespace SimCore
{
    namespace ActComps
    {
        ////////////////////////////////////////////////////////////////////////////////
        // CONSTANTS
        ////////////////////////////////////////////////////////////////////////////////
        IMPLEMENT_ENUM(PlayModeEnum)
        PlayModeEnum PlayModeEnum::ONCE("ONCE");
        PlayModeEnum PlayModeEnum::LOOP("LOOP");
        PlayModeEnum PlayModeEnum::SWING("SWING");



        ////////////////////////////////////////////////////////////////////////////////
        // ANIMATION PROPERTY CONTAINER
        ////////////////////////////////////////////////////////////////////////////////
        AnimationPropertyContainer::AnimationPropertyContainer()
            : BaseClass()
            , mBeginTime(0.0f)
            , mEndTime(0.0f)
            , mTimeScale(1.0f)
            , mPlayMode(&PlayModeEnum::ONCE)
        {
            const dtUtil::RefString GROUPNAME("AnimationPropertyContainer");
            typedef dtCore::PropertyRegHelper<AnimationPropertyContainer&, AnimationPropertyContainer> PropertyRegType;
            PropertyRegType propertyRegHelper ( *this, this, GROUPNAME );

            DT_REGISTER_PROPERTY(
                Name,
                "Name to refer to a segment of a long animation.",
                PropertyRegType, propertyRegHelper );

            DT_REGISTER_PROPERTY(
                BeginTime,
                "Time in within a long animation that marks the beginning of an animation segment",
                PropertyRegType, propertyRegHelper );
            
            DT_REGISTER_PROPERTY(
                EndTime,
                "Time in within a long animation that marks the ending of an animation segment",
                PropertyRegType, propertyRegHelper );
            
            DT_REGISTER_PROPERTY(
                TimeScale,
                "Time scale to speed up or slow down playback. Defaults to 1.",
                PropertyRegType, propertyRegHelper );
            
            AddProperty(new dtCore::EnumActorProperty<PlayModeEnum>(
                "Play Mode",
                "Play Mode",
                dtCore::EnumActorProperty<PlayModeEnum>::SetFuncType(this,&AnimationPropertyContainer::SetPlayMode),
                dtCore::EnumActorProperty<PlayModeEnum>::GetFuncType(this,&AnimationPropertyContainer::GetPlayMode),
                "Sets the tether mode for this tripod actor.", GROUPNAME));
        }

        AnimationPropertyContainer::~AnimationPropertyContainer()
        {
        }

        void AnimationPropertyContainer::SetName(const std::string& name)
        {
            mName = name;
        }

        const std::string& AnimationPropertyContainer::GetName() const
        {
            return mName;
        }

        void AnimationPropertyContainer::SetBeginTime(float beginTime)
        {
            mBeginTime = beginTime;
        }

        float AnimationPropertyContainer::GetBeginTime() const
        {
            return mBeginTime;
        }

        void AnimationPropertyContainer::SetEndTime(float endTime)
        {
            mEndTime = endTime;
        }

        float AnimationPropertyContainer::GetEndTime() const
        {
            return mEndTime;
        }

        void AnimationPropertyContainer::SetTimeScale(float timeScale)
        {
            mTimeScale = timeScale;
        }

        float AnimationPropertyContainer::GetTimeScale() const
        {
            return mTimeScale;
        }

        void AnimationPropertyContainer::SetPlayMode(PlayModeEnum& mode)
        {
            mPlayMode = &mode;
        }

        PlayModeEnum& AnimationPropertyContainer::GetPlayMode() const
        {
            return *mPlayMode;
        }



        ////////////////////////////////////////////////////////////////////////////////
        // ANIMATION CLIP ACTOR COMPONENT
        ////////////////////////////////////////////////////////////////////////////////
        const dtGame::ActorComponent::ACType AnimationClipActComp::TYPE("AnimationClipActComp::");
        const dtUtil::RefString AnimationClipActComp::PROPERTY_ANIMATION_PROPERTY_ARRAY("Animation Property Array");
        const dtUtil::RefString AnimationClipActComp::PROPERTY_PAUSED;

        AnimationClipActComp::AnimationClipActComp()
            : BaseClass(AnimationClipActComp::TYPE)
            , mIsValid(false)
            , mPaused(false)
            , mCurrentAnimation(-1)
        {
        }

        AnimationClipActComp::~AnimationClipActComp()
        {
        }

        osg::Node* AnimationClipActComp::GetOwnerNode()
        {
            dtGame::GameActor* actor = dynamic_cast<dtGame::GameActor*>(GetOwner());
            if (actor == NULL)
            {
                return NULL;
            }
            
            return actor->GetOSGNode();
        }

        const osg::Node* AnimationClipActComp::GetOwnerNode() const
        {
            const dtGame::GameActor* actor = dynamic_cast<const dtGame::GameActor*>(GetOwner());
            if (actor == NULL)
            {
                return NULL;
            }
            
            return actor->GetOSGNode();
        }

        bool AnimationClipActComp::IsValid() const
        {
            return mIsValid;
        }

        int AnimationClipActComp::GetAnimationPropertyContainerIndex(const std::string& name) const
        {
            int result = -1;

            AnimPropVector::const_iterator iter = mAnimProps.begin();
            AnimPropVector::const_iterator iterEnd = mAnimProps.end();
            for (int i = 0; iter != iterEnd; ++iter, ++i)
            {
                if (iter->get()->GetName() == name)
                {
                    result = i;
                    break;
                }
            }

            return result;
        }

        AnimationPropertyContainer* AnimationClipActComp::GetAnimationPropertyContainerByName(const std::string& name)
        {
            int index = GetAnimationPropertyContainerIndex(name);
            return GetAnimationPropertyContainer(index);
        }
        
        AnimationPropertyContainer* AnimationClipActComp::GetAnimationPropertyContainer(int index)
        {
            if (index >= 0 && size_t(index) < mAnimProps.size())
            {
                return mAnimProps[index];
            }

            return NULL;
        }

        void AnimationClipActComp::SetAnimationPropertyContainer(int index, AnimationPropertyContainer* animProps)
        {
            if (animProps != NULL && mAnimProps.size() > size_t(index) && index >= 0)
            {
                mAnimProps[index] = new AnimationPropertyContainer(*animProps);
            }
        }

        void AnimationClipActComp::InsertNewAnimationPropertyContainer(int index)
        {
            if (index >= 0 && size_t(index) <= mAnimProps.size())
            {
                mAnimProps.insert(mAnimProps.begin() + index, new AnimationPropertyContainer());
            }
        }

        void AnimationClipActComp::RemoveAnimationPropertyContainer(int index)
        {
            if (index >= 0 && size_t(index) < mAnimProps.size())
            {
                mAnimProps.erase(mAnimProps.begin() + index);
            }
        }

        size_t AnimationClipActComp::GetNumAnimationPropertyContainers() const
        {
            return mAnimProps.size();
        }
        
        void AnimationClipActComp::SetPaused(bool paused)
        {
            if (mPaused != paused)
            {
                InternalPauseAnimation(paused);
            }
        }
        
        bool AnimationClipActComp::IsPaused() const
        {
            return mPaused;
        }

        int AnimationClipActComp::GetCurrentAnimationPropertyIndex() const
        {
            return mCurrentAnimation;
        }

        int AnimationClipActComp::PlayAnimation(const std::string& name, bool reset)
        {
            int index = GetAnimationPropertyContainerIndex(name);

            if (index >= 0)
            {
                InternalPlayAnimation(index, reset);
            }

            return index;
        }

        bool AnimationClipActComp::PlayAnimation(int animIndex, bool reset)
        {
            //AnimationPropertyContainer* apc = GetAnimationPropertyContainer(animIndex);

            if (animIndex >= 0 && size_t(animIndex) < mAnimProps.size())
            {
                InternalPlayAnimation(animIndex, reset);
            }

            return !mPaused;
        }

        void AnimationClipActComp::ResetAnimation()
        {
            if( ! mIsValid)
            {
                return;
            }

            osg::Node* node = GetOwnerNode();
            if (node != NULL)
            {
                AnimCallbackVisitor visitor;
                node->accept(visitor);

                visitor.ResetCallbacks();
            }
        }

        void AnimationClipActComp::OnEnteredWorld()
        {
            BaseClass::OnEnteredWorld();

            ProcessModel();
        }

        void AnimationClipActComp::BuildPropertyMap()
        {
            BaseClass::BuildPropertyMap();

            const dtUtil::RefString ANIM_CLIP_PROPERTY_GROUP("Animation Clip Properties");
            typedef dtCore::PropertyRegHelper<AnimationClipActComp&, AnimationClipActComp> PropRegType;
            PropRegType propRegHelper(*this, this, ANIM_CLIP_PROPERTY_GROUP);

            typedef dtCore::ArrayActorPropertyComplex<dtCore::RefPtr<AnimationPropertyContainer> > AnimPropArrayPropType;
            dtCore::RefPtr<AnimPropArrayPropType> arrayProp = new AnimPropArrayPropType(
                 PROPERTY_ANIMATION_PROPERTY_ARRAY,
                 PROPERTY_ANIMATION_PROPERTY_ARRAY,
                 AnimPropArrayPropType::SetFuncType(this, &AnimationClipActComp::SetAnimationPropertyContainer),
                 AnimPropArrayPropType::GetFuncType(this, &AnimationClipActComp::GetAnimationPropertyContainer),
                 AnimPropArrayPropType::GetSizeFuncType(this, &AnimationClipActComp::GetNumAnimationPropertyContainers),
                 AnimPropArrayPropType::InsertFuncType(this, &AnimationClipActComp::InsertNewAnimationPropertyContainer),
                 AnimPropArrayPropType::RemoveFuncType(this, &AnimationClipActComp::RemoveAnimationPropertyContainer),
                 "Array of animation clip parameters. Each element defines an individual animation clip to play from a shared larger animation.",
                 ANIM_CLIP_PROPERTY_GROUP);
            
            dtCore::RefPtr<dtCore::BasePropertyContainerActorProperty> propContainerProp =
                new dtCore::SimplePropertyContainerActorProperty<AnimationPropertyContainer>(
                    "AnimationClipParameters",
                    "AnimationClipParameters",
                    dtCore::SimplePropertyContainerActorProperty<AnimationPropertyContainer>::SetFuncType(arrayProp.get(), &AnimPropArrayPropType::SetCurrentValue),
                    dtCore::SimplePropertyContainerActorProperty<AnimationPropertyContainer>::GetFuncType(arrayProp.get(), &AnimPropArrayPropType::GetCurrentValue),
                    "Defines a segment from a larger animation to play.",
                    ANIM_CLIP_PROPERTY_GROUP);

            arrayProp->SetArrayProperty(*propContainerProp);
            AddProperty(arrayProp);

            AddProperty(new dtCore::BooleanActorProperty(
                PROPERTY_PAUSED,
                PROPERTY_PAUSED,
                dtCore::BooleanActorProperty::SetFuncType(this, &AnimationClipActComp::SetPaused),
                dtCore::BooleanActorProperty::GetFuncType(this, &AnimationClipActComp::IsPaused),
                "Set all animation callbacks paused.",
                ANIM_CLIP_PROPERTY_GROUP));
        }

        void AnimationClipActComp::ProcessModel()
        {
            osg::Node* node = GetOwnerNode();
            if (node == NULL)
            {
                LOG_ERROR("No access to a root node.");
            }
            else
            {
                if (mIsValid)
                {
                    InternalPauseAnimation(true);
                }

                AnimCallbackVisitor visitor;
                node->accept(visitor);
                
                // Convert the animation paths to path types
                // that can play segments of a full animation.
                // This could take a good amount of time depending
                // the number of objects and length of animation
                // to be copied between animation paths.
                typedef AnimCallbackVisitor::AnimCallbackVector AnimCallbackVector;
                AnimCallbackVector::iterator iter = visitor.GetAnimCallbacks().begin();
                AnimCallbackVector::iterator iterEnd = visitor.GetAnimCallbacks().end();
                for (; iter != iterEnd; ++iter)
                {
                    dtCore::RefPtr<AnimClipPath> newPath
                        = new AnimClipPath(*(iter->get()->getAnimationPath()));
                    iter->get()->setAnimationPath(newPath);
                }

                mIsValid = ! visitor.GetAnimCallbacks().empty();

                // Ensure proper states.
                InternalPauseAnimation(mPaused);
            }
        }

        void AnimationClipActComp::InternalPlayAnimation(int animIndex, bool reset)
        {
            if( ! mIsValid)
            {
                return;
            }

            osg::Node* node = GetOwnerNode();
            AnimCallbackVisitor visitor;
            if (node == NULL)
            {
                LOG_ERROR("Could not access actor root node.");
                return;
            }
            
            node->accept(visitor);

            // Apply another set of parameters if another index has been specified.
            if (mCurrentAnimation != animIndex)
            {
                AnimationPropertyContainer* animProps = GetAnimationPropertyContainer(animIndex);

                if (animProps == NULL)
                {
                    std::ostringstream oss;
                    oss << "Could not access animation parameters at index "
                        << animIndex << "." << std::endl;
                    LOG_ERROR(oss.str());

                    mCurrentAnimation = -1;
                    return;
                }
                
                ApplyAnimationClipParameters(*animProps, visitor, true, true);

                mCurrentAnimation = animIndex;
            }
            else
            {
                if (reset)
                {
                    visitor.ResetCallbacks();
                }

                visitor.SetPaused(false);
            }

            mPaused = false;
        }

        void AnimationClipActComp::InternalPauseAnimation(bool pause)
        {
            if( ! mIsValid)
            {
                return;
            }

            dtGame::GameActor* actor = dynamic_cast<dtGame::GameActor*>(GetOwner());
            if (actor == NULL)
            {
                return;
            }

            osg::Node* node = actor->GetOSGNode();

            if (node != NULL)
            {
                AnimCallbackVisitor visitor;
                node->accept(visitor);

                visitor.SetPaused(pause);
                mPaused = pause;
            }
        }

        void AnimationClipActComp::ApplyAnimationClipParameters(
            AnimationPropertyContainer& animParams,
            osg::NodeVisitor& visitorBaseType,
            bool play, bool reset)
        {
            AnimCallbackVisitor* visitor = static_cast<AnimCallbackVisitor*>(&visitorBaseType);

            AnimClipPath* curPath = NULL;
            osg::AnimationPath::LoopMode curLoopMode = osg::AnimationPath::NO_LOOPING;

            typedef AnimCallbackVisitor::AnimCallbackVector AnimCallbackVector;
            AnimCallbackVector::iterator iter = visitor->GetAnimCallbacks().begin();
            AnimCallbackVector::iterator iterEnd = visitor->GetAnimCallbacks().end();
            for (; iter != iterEnd; ++iter)
            {
                if(animParams.GetPlayMode() == PlayModeEnum::LOOP)
                {
                    curLoopMode = osg::AnimationPath::LOOP;
                }
                else if(animParams.GetPlayMode() == PlayModeEnum::SWING)
                {
                    curLoopMode = osg::AnimationPath::SWING;
                }
                else
                {
                    curLoopMode = osg::AnimationPath::NO_LOOPING;
                }
                
                curPath = dynamic_cast<AnimClipPath*>(iter->get()->getAnimationPath());
                if(curPath != NULL)
                {
                    curPath->setBeginTime(animParams.GetBeginTime());
                    curPath->setEndTime(animParams.GetEndTime());
                    curPath->setLoopMode(curLoopMode);
                }

                iter->get()->setTimeMultiplier(animParams.GetTimeScale());
            }

            if (reset)
            {
                visitor->ResetCallbacks();
            }

            visitor->SetPaused(!play);
        }

    }
}
