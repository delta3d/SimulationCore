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

#ifndef ANIMATIONCLIPACTCOMP_H_
#define ANIMATIONCLIPACTCOMP_H_

#include <SimCore/Export.h>
#include <dtGame/actorcomponent.h>
#include <dtCore/propertycontainer.h>

namespace osg
{
    class Node;
    class NodeVisitor;
}

namespace SimCore
{
    namespace ActComps
    {
        class SIMCORE_EXPORT PlayModeEnum : public dtUtil::Enumeration
        {
            DECLARE_ENUM(PlayModeEnum)

            public:

            static PlayModeEnum ONCE;
            static PlayModeEnum LOOP;
            static PlayModeEnum SWING;

            private:

            PlayModeEnum( const std::string &name ) : dtUtil::Enumeration(name)
            {
                AddInstance(this);
            }
        };



        class SIMCORE_EXPORT AnimationPropertyContainer : public dtCore::PropertyContainer
        {
        public:
            typedef dtCore::PropertyContainer BaseClass;

            AnimationPropertyContainer();
            virtual ~AnimationPropertyContainer();

            void SetName(const std::string& name);
            const std::string& GetName() const;

            void SetBeginTime(float beginTime);
            float GetBeginTime() const;

            void SetEndTime(float endTime);
            float GetEndTime() const;

            void SetTimeScale(float timeScale);
            float GetTimeScale() const;

            void SetPlayMode(PlayModeEnum& mode);
            PlayModeEnum& GetPlayMode() const;

        private:
            std::string mName;
            float mBeginTime;
            float mEndTime;
            float mTimeScale;
            PlayModeEnum* mPlayMode;
        };



        /*
         * Class that simplifies control over rigid keyframed animation.
         */
        class SIMCORE_EXPORT AnimationClipActComp : public dtGame::ActorComponent
        {
        public:
            typedef dtGame::ActorComponent BaseClass;

            static const dtGame::ActorComponent::ACType TYPE;
            static const dtUtil::RefString PROPERTY_ANIMATION_PROPERTY_ARRAY;
            static const dtUtil::RefString PROPERTY_PAUSED;

            AnimationClipActComp();

            osg::Node* GetOwnerNode();
            const osg::Node* GetOwnerNode() const;

            bool IsValid() const;

            int GetAnimationPropertyContainerIndex(const std::string& name) const;

            AnimationPropertyContainer* GetAnimationPropertyContainerByName(const std::string& name);
            
            AnimationPropertyContainer* GetAnimationPropertyContainer(int index);
            void SetAnimationPropertyContainer(int index, AnimationPropertyContainer* animProps);

            void InsertNewAnimationPropertyContainer(int index);

            void RemoveAnimationPropertyContainer(int index);

            size_t GetNumAnimationPropertyContainers() const;
            
            void SetPaused(bool paused);
            bool IsPaused() const;

            int GetCurrentAnimationPropertyIndex() const;

            int PlayAnimation(const std::string& name, bool reset = false);

            bool PlayAnimation(int animIndex, bool reset = false);

            void ResetAnimation();

            virtual void OnEnteredWorld();

            virtual void BuildPropertyMap();

            virtual void ProcessModel();

        private:
            virtual ~AnimationClipActComp();

            void InternalPlayAnimation(int animIndex, bool reset = false);
            void InternalPauseAnimation(bool pause);

            void ApplyAnimationClipParameters(
                AnimationPropertyContainer& animParams,
                osg::NodeVisitor& visitor,
                bool play,
                bool reset);

            bool mIsValid;
            bool mPaused;
            int mCurrentAnimation;

            typedef std::vector<dtCore::RefPtr<AnimationPropertyContainer> > AnimPropVector;
            AnimPropVector mAnimProps;
        };
    }
}

#endif
