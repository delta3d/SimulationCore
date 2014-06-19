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
* @author Bradley Anderegg
*/

#ifndef DELTA_BASEAIHELPER_H
#define DELTA_BASEAIHELPER_H

#include <DemoExport.h>
#include <AISteeringModel.h>
#include <AIPhysicsModel.h>
#include <AIUtility.h>

#include <dtAI/fsm.h>
#include <dtAI/npcstate.h>
#include <dtAI/controllable.h>


#include <dtCore/transform.h>

namespace NetDemo
{

   class AIEvent;
   class AIStateType;
   class EnemyDescriptionActor;

   class NETDEMO_EXPORT BaseAIHelper: public BaseAIControllable,
                                      public osg::Referenced
   {
   public:
      typedef BaseAIControllable BaseClass;
      
   public:
      BaseAIHelper();
      BaseAIHelper(const EnemyDescriptionActor* desc);

      void Init(const EnemyDescriptionActor* desc);
      virtual void OnInit(const EnemyDescriptionActor* desc);

      virtual void Spawn();
      virtual void Update(float dt);

      virtual void PreSync(const dtCore::Transform& trans);
      virtual void PostSync(dtCore::Transform& trans) const;

      virtual void GetTransform(dtCore::Transform& transIn) const;
      virtual void SetTransform(const dtCore::Transform& trans);

      const dtUtil::RefString& GetPrototypeName() const;
      void GetPrototypeName(const dtUtil::RefString& name);

      dtAI::FSM& GetStateMachine() { return mStateMachine; }
      const dtAI::FSM& GetStateMachine() const { return mStateMachine; }

      dtAI::FSM::FactoryType* GetStateFactory() { return mFactory.get(); }
      const dtAI::FSM::FactoryType* GetStateFactory() const { return mFactory.get(); }

      AISteeringModel* GetSteeringModel() { return mSteeringModel.get(); }
      const AISteeringModel* GetSteeringModel() const { return mSteeringModel.get(); }

      AIPhysicsModel* GetPhysicsModel() { return mPhysicsModel.get(); }
      const AIPhysicsModel* GetPhysicsModel() const { return mPhysicsModel.get(); }

      //derived from Controllable and allows the steering pipeline to operate on us
      /*virtual*/ bool FindPath(const BaseClass::AIState& fromState, const BaseClass::AIGoal& goal, BaseClass::AIPath& resultingPath) const;
      /*virtual*/ void UpdateState(float dt, const BaseClass::AIControlState& steerData);
      /*virtual*/ void OutputControl(const BaseClass::AIPath& pathToFollow, const BaseClass::AIState& current_state, BaseClass::AIControlState& result) const;

      /*virtual*/ void RegisterProperties(dtCore::PropertyContainer& pc, const std::string& group);

      /**
      * A function to add transitions to the finite state machine using the AIStateType
      * which avoids redundant calls to FSM::GetState.
      */
      void AddTransition(const AIEvent* eventToTriggerTransition, const AIStateType* fromState, const AIStateType* toState);


   protected:
      BaseAIHelper(const BaseAIHelper&);  //not implemented by design
      BaseAIHelper& operator=(const BaseAIHelper&);  //not implemented by design
      ~BaseAIHelper();

      virtual void RegisterStates();
      virtual void CreateStates();
      virtual void SetupTransitions();
      virtual void SetupFunctors();

      virtual void SelectState(float dt);

   //private:

      dtCore::RefPtr<dtAI::FSM::FactoryType> mFactory;
      dtAI::FSM mStateMachine;
      dtCore::RefPtr<AISteeringModel> mSteeringModel;
      dtCore::RefPtr<AIPhysicsModel> mPhysicsModel;
      
      BaseSteeringTargeter* mDefaultTargeter;

      dtUtil::RefString mPrototypeName;
   };

} //namespace NetDemo

#endif //DELTA_BASEAIHELPER_H
