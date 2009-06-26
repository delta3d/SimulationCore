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

#ifndef NETDEMO_AIUTILITY_H
#define NETDEMO_AIUTILITY_H

#include <osg/Referenced>
#include <osg/Vec3>
#include <osg/Matrix>

#include <dtAI/steeringutility.h>
#include <dtAI/steeringbehavior.h>

//for the steering pipeline
#include <dtUtil/objectfactory.h>

//include needed just for getting to PathData internal to BezierController
//todo- move PathData out of BezierController
#include <dtABC/beziercontroller.h>

//used for the waypoint array typedef WaypointPath
//todo- move WaypointArray out of AIPluginInterface
#include <dtAI/aiplugininterface.h>

//use a property container to hold ai data
#include <dtDAL/propertycontainer.h>
#include <dtDAL/containeractorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/command.h>
#include <dtUtil/functor.h>

#include <dtUtil/log.h>

namespace NetDemo
{

   struct Kinematic
   {
      osg::Matrix mTransform;
      osg::Vec3 mLinearVelocity;
      osg::Vec3 mAngularVelocity;
   };

   struct SteeringOutput
   {
      //these are the control inputs
      //all are floats from 1 to -1
      //which represents percentage of maximum
      float mThrust, mLift, mYaw;

      osg::Vec3 mLinearVelocity;

      void Reset()
      {
         mThrust = 0.0f;
         mLift = 0.0f;
         mYaw = 0.0f;
         mLinearVelocity.set(0.0f, 0.0f, 0.0f);
      }
   };

   typedef std::vector<dtABC::BezierController::PathData>  BezierPath;
   typedef dtAI::AIPluginInterface::WaypointArray WaypointPath;
   typedef dtAI::SteeringBehavior<dtAI::KinematicGoal, Kinematic, SteeringOutput> SteeringBehaviorType;

   //class ErrorCondition: public dtUtil::Enumeration
   //{
   //   DECLARE_ENUM(ErrorCondition);
   //public:
   //   ErrorCondition(const std::string&);
   //};

   //this allows for a default behavior that doesn't do anything... idle?
   class DoNothing: public SteeringBehaviorType
   {
   public:
      typedef SteeringBehaviorType BaseClass;
      DoNothing(){}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:
   };

   class BombDive: public SteeringBehaviorType
   {
   public:
      typedef SteeringBehaviorType BaseClass;
      BombDive(float speed): mSpeed(speed){}

      /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   private:
      float mSpeed;
   };

   /**
    * Align is used to align our orientation with the current dtAI::KinematicGoal's orientation (rotation)
    */
   class Align: public SteeringBehaviorType
   {
   public:
     typedef SteeringBehaviorType BaseClass;
     Align(float lookAhead, float timeToTarget)
        : mLookAhead(lookAhead)
        , mTimeToTarget(timeToTarget)
     {}

     /*virtual*/ void Think(float dt, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, BaseClass::SteeringOutByRefParam result);

   protected:
      float Sgn(float x);
      osg::Vec3 GetTargetPosition(float dt, BaseClass::ConstKinematicGoalParam goal);
      float GetTargetForward(float dt, const osg::Vec3& targetPos, BaseClass::ConstKinematicGoalParam current_goal, BaseClass::ConstKinematicParam current_state, osg::Vec3& vec_in);

      float mLookAhead, mTimeToTarget;
   };

   /**
    * Follow path can be used to follow waypoints
    */
   class FollowPath: public Align
   {
   public:
      typedef Align BaseClass;

      FollowPath(float minSpeed, float maxSpeed, float lookAhead, float timeToTarget, float lookAheadRot, float timeToTargetRot)
         : BaseClass(lookAheadRot, timeToTargetRot)
         , mMinSpeed(minSpeed)
         , mMaxSpeed(maxSpeed)
         , mLookAhead(lookAhead)
         , mTimeToTarget(timeToTarget)
      {}

     /*virtual*/ void Think(float dt, Align::BaseClass::ConstKinematicGoalParam current_goal, BaseClass::BaseClass::ConstKinematicParam current_state, BaseClass::BaseClass::SteeringOutByRefParam result);

   private:

      float mMinSpeed, mMaxSpeed, mLookAhead, mTimeToTarget;
   };


   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class AIPipelineFunctorsBase
   {
   public:
      //these typedefs are used by the steering pipeline to resolve type info
      //on the 3 template parameters of a Controllable
      typedef State_ StateType;
      typedef GoalState_ GoalStateType;
      typedef PathType_ PathType;
      typedef Controls_ ControlType;
   };



   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class Controllable: public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
   public:
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;

      //typedef Controllable<Kinematic, SteeringOutput, SteeringOutput> MineControllable;
      //typedef dtAI::SteeringBehavior<dtAI::KinematicGoal, Kinematic, SteeringOutput> SteeringBehaviorType;


      typename BaseClass::ControlType mCurrentControls;

      typename BaseClass::StateType mCurrentState;
      typename BaseClass::StateType mNextPredictedState;

      typename BaseClass::GoalStateType mGoalState;

      typename BaseClass::StateType mStateConstraints;
      typename BaseClass::ControlType mControlConstraints;

      typename BaseClass::PathType mPathToFollow;
      typename BaseClass::PathType mPredictedPath;
   };



   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class Targeter: public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
   public:
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;

      void SetEnable(bool b){mEnabled = b;};
      bool GetEnable(){return mEnabled;};

      //returns true to continue
      virtual bool GetGoal(const typename BaseClass::StateType& current_state, typename BaseClass::GoalStateType& result) const = 0;

   protected:
      bool mEnabled;
   };


   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class Decomposer: public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
   public:
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;

      virtual void Decompose(const typename BaseClass::StateType& current_state, typename BaseClass::GoalStateType& result) const = 0;
   };


   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class Constraint: public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
   public:
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;


      virtual bool WillViolate(const typename BaseClass::PathType& pathToFollow) = 0;
      virtual void Suggest(const typename BaseClass::PathType& pathToFollow, const typename BaseClass::StateType& current_state, typename BaseClass::GoalStateType& result) = 0;
   };


   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class Actuator: public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
   public:
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;

      virtual bool GetPath(const typename BaseClass::StateType& current_state, const typename BaseClass::GoalStateType& goal, typename BaseClass::PathType& result) = 0;
      virtual void Output(const typename BaseClass::PathType& pathToFollow, const typename BaseClass::StateType& current_state, typename BaseClass::ControlType& result) = 0;
   };

   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   class Updater: public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
   public:
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;
      typedef Controllable<State_, GoalState_, Controls_, PathType_> ControllableType;

      virtual void Update(float dt, const typename BaseClass::ControlType& steerData, ControllableType& state) = 0;
   };


   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   struct ObtainGoal : public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;

      ObtainGoal(const typename BaseClass::StateType& k): mState(k){}

      bool operator()(const Targeter<State_, GoalState_, Controls_, PathType_>& target)
      {
         return target.GetGoal(mState, mGoal);
      }

      const typename BaseClass::StateType& mState;
      typename BaseClass::GoalStateType mGoal;
   };

   template <class State_, class GoalState_, class Controls_, class PathType_ = std::vector<GoalState_> >
   struct DecomposeGoal : public AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_>
   {
      typedef AIPipelineFunctorsBase<State_, GoalState_, Controls_, PathType_> BaseClass;

      DecomposeGoal(const typename BaseClass::StateType& k, typename BaseClass::GoalStateType& g)
         : mState(k)
         , mGoal(g)
      {}

      void operator()(const Decomposer<State_, GoalState_, Controls_, PathType_>& target)
      {
         return target.Decompose(mState, mGoal);
      }

      const typename BaseClass::StateType& mState;
      typename BaseClass::GoalStateType& mGoal;
   };

   template<class IterType, class Functor>
   void ForEachIf(IterType from, IterType to, Functor& func)
   {
      bool result = true;
      for (; from != to && result; ++from) result = func(*from);
   }

   template<class IterType, class Functor>
   void ForEach(IterType from, IterType to, Functor func)
   {
      for (; from != to; ++from) func(*from);
   }


   template <class ControllableType_, class TargeterType_, class DecomposerType_, class ConstraintType_, class ActuatorType_, class UpdaterType_ >
   class SteeringPipeline : public AIPipelineFunctorsBase<typename ControllableType_::StateType,
                                                          typename ControllableType_::GoalStateType,
                                                          typename ControllableType_::ControlType,
                                                          typename ControllableType_::PathType>
   {
   public:
      typedef AIPipelineFunctorsBase<typename ControllableType_::StateType,
                                     typename ControllableType_::GoalStateType,
                                     typename ControllableType_::ControlType,
                                     typename ControllableType_::PathType> BaseClass;

      typedef ControllableType_ ControllableType;

      typedef typename ControllableType::StateType AIState;
      typedef typename ControllableType::GoalStateType AIGoal;
      typedef typename ControllableType::ControlType AIControlState;
      typedef typename ControllableType::PathType AIPath;

      typedef TargeterType_ TargeterType;
      typedef DecomposerType_ DecomposerType;
      typedef ConstraintType_ ConstraintType;
      typedef ActuatorType_ ActuatorType;
      typedef UpdaterType_ UpdaterType;

      typedef SteeringPipeline<ControllableType, TargeterType, DecomposerType, ConstraintType, ActuatorType, UpdaterType> value_type;


   public:
      SteeringPipeline()
         : mMaxUpdateAttempts(10)
      {
      }

      ~SteeringPipeline()
      {
      }

      bool Step(float dt, ControllableType& entityToStep)
      {
         ObtainGoal<AIState, AIGoal, AIControlState, AIPath> og(entityToStep.mCurrentState);
         ForEachIf(mTargeters.begin(), mTargeters.end(), og);

         AIGoal curr_goal = og.mGoal;

         DecomposeGoal<AIState, AIGoal, AIControlState, AIPath> dg(entityToStep.mCurrentState, curr_goal);
         ForEach(mDecomposers.begin(), mDecomposers.end(), dg);

         AIControlState controlsOut = entityToStep.mCurrentControls;
         bool hasOutput = FindGoal(curr_goal, entityToStep.mCurrentState, controlsOut, mMaxUpdateAttempts);

         //if no constraints are satisfied then we fall back to a default Behavior
         if(hasOutput)
         {
            //update our state based on our steering output
            mUpdater.Update(dt, controlsOut, entityToStep);
         }
         else
         {
            //mDefaultBehavior->Think(dt, curr_goal, mKinematicState, output);
            LOG_ERROR("Steering Pipeline unable to satisfy constraints.");
         }

         return hasOutput;
      }

   private:

      bool FindGoal(AIGoal& g, const AIState& state, AIControlState& output, int maxAttempts)
      {
         AIPath p;

         if(maxAttempts <= 0 || !mActuator.GetPath(state, g, p))
         {
            //we cannot complete our goal
            return false;
         }

         //todo- use ForEachIF() as above
         typename ConstraintArray::iterator iter = mConstraints.begin();
         typename ConstraintArray::iterator iterEnd = mConstraints.end();

         for(;iter != iterEnd; ++iter)
         {
            Constraint<AIState, AIGoal, AIControlState, AIPath>& cont = *iter;
            if(cont.WillViolate(p))
            {
               cont.Suggest(p, state, g);
               FindGoal(g, state, output, --maxAttempts);
            }
         }

         mActuator.Output(p, state, output);
         return true;
      }


      unsigned mMaxUpdateAttempts;
      AIState mStateTemplate;
      //SteeringBehavior* mDefaultBehavior;

      ActuatorType mActuator;
      UpdaterType mUpdater;

      typedef std::vector<TargeterType> TargetArray;
      TargetArray mTargeters;

      //An array of Decomposers able to
      typedef std::vector<DecomposerType> DecomposerArray;
      DecomposerArray mDecomposers;

      //An array of constraints not to violate
      typedef std::vector<ConstraintType> ConstraintArray;
      ConstraintArray mConstraints;

      //and a static factory
      //typedef dtUtil::ObjectFactory<std::string, ControllableType> AIEntityFactory;
      //static dtCore::RefPtr<AIEntityFactory> mEntityFactory;
   };





   ///////////////////////////////////////////////////////////////////////////
   //PROPERTY CONTAINER TEMPLATE STUFF
   //////////////////////////////////////////////////////////////////////////
   template <typename _Type>
   struct TypeToActorProperty
   {
   private:
      //if no specializations are used we will just do everything by copy
      template <class U, typename T = _Type>
      struct _TypeToActorProperty_
      {
         typedef U value_type;
         typedef value_type GetValueType;
         typedef value_type SetValueType;

         typedef dtUtil::Functor<GetValueType, TYPELIST_0()> GetFuncType;
         typedef dtUtil::Functor<void, TYPELIST_1(SetValueType)> SetFuncType;
      };

      //TODO- ActorActor, GameEvent, Resource, Enumeration, and ColorRGBA

      template <typename T>
      struct _TypeToActorProperty_<bool, T>
      {
         typedef dtDAL::BooleanActorProperty value_type;

         typedef bool GetValueType;
         typedef bool SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<int, T>
      {
         typedef dtDAL::IntActorProperty value_type;

         typedef int GetValueType;
         typedef int SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<long, T>
      {
         typedef dtDAL::LongActorProperty value_type;

         typedef long GetValueType;
         typedef long SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<float, T>
      {
         typedef dtDAL::FloatActorProperty value_type;

         typedef float GetValueType;
         typedef float SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<double, T>
      {
         typedef dtDAL::DoubleActorProperty value_type;

         typedef double GetValueType;
         typedef double SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<std::string, T>
      {
         typedef dtDAL::StringActorProperty value_type;

         typedef std::string GetValueType;
         typedef const std::string& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<osg::Vec2f, T>
      {
         typedef dtDAL::Vec3ActorProperty value_type;

         typedef const osg::Vec2f& GetValueType;
         typedef const osg::Vec2f& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<osg::Vec2d, T>
      {
         typedef dtDAL::Vec3dActorProperty value_type;

         typedef const osg::Vec2d& GetValueType;
         typedef const osg::Vec2d& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<osg::Vec3f, T>
      {
         typedef dtDAL::Vec3fActorProperty value_type;

         typedef const osg::Vec3f& GetValueType;
         typedef const osg::Vec3f& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<osg::Vec3d, T>
      {
         typedef dtDAL::Vec3dActorProperty value_type;

         typedef const osg::Vec3d& GetValueType;
         typedef const osg::Vec3d& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<osg::Vec4f, T>
      {
         typedef dtDAL::Vec4fActorProperty value_type;

         typedef const osg::Vec4f& GetValueType;
         typedef const osg::Vec4f& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };

      template <typename T>
      struct _TypeToActorProperty_<osg::Vec4d, T>
      {
         typedef dtDAL::Vec4dActorProperty value_type;

         typedef const osg::Vec4d& GetValueType;
         typedef const osg::Vec4d& SetValueType;

         typedef value_type::GetFuncType GetFuncType;
         typedef value_type::SetFuncType SetFuncType;
      };


   public:
      typedef typename _TypeToActorProperty_<_Type>::value_type value_type;

      typedef typename _TypeToActorProperty_<_Type>::GetValueType GetValueType;
      typedef typename _TypeToActorProperty_<_Type>::SetValueType SetValueType;

      typedef typename _TypeToActorProperty_<_Type>::SetFuncType SetFuncType;
      typedef typename _TypeToActorProperty_<_Type>::GetFuncType GetFuncType;
   };

   template <class ContainerType, class FuncObj>
   struct PropertyRegHelper
   {
      typedef PropertyRegHelper<ContainerType, FuncObj> Type;
      typedef FuncObj FunctorObjectType;
      typedef ContainerType ContainerObjectType;

      typedef dtDAL::Vec3ActorProperty Vec3;

      PropertyRegHelper(ContainerType con, FuncObj* objPtr, const std::string& str)
         : mPropCon(con)
         , mFuncObj(objPtr)
         , mGroup(str)
      {}

      template <class PropType, typename SetPtr, typename GetPtr>
      void RegisterProperty(PropType prop, SetPtr setter, GetPtr getter, const std::string& name, const std::string& desc)
      {

         mPropCon.AddProperty(new typename TypeToActorProperty<PropType>::value_type(name, name,
            typename TypeToActorProperty<PropType>::SetFuncType(mFuncObj, setter),
            typename TypeToActorProperty<PropType>::GetFuncType(mFuncObj, getter),
            desc, mGroup));
      }

      ContainerType mPropCon;
      FuncObj* mFuncObj;

      const std::string& mGroup;
   };

#define DECLARE_PROPERTY(PropertyType, PropertyName) \
   PropertyType  m ## PropertyName; \
public: \
   \
   void Set ## PropertyName(TypeToActorProperty<PropertyType>::SetValueType value)\
   {\
      m ## PropertyName = value; \
   };\
   \
   TypeToActorProperty<PropertyType>::GetValueType Get ## PropertyName() const\
   {\
      return m ## PropertyName;\
   };\
   \


#define CREATE_PROPERTY_GETTER_HELPER_MACRO(RegHelperType_, PropertyName)\
   &RegHelperType_::FunctorObjectType::Get ## PropertyName\

#define CREATE_PROPERTY_SETTER_HELPER_MACRO(RegHelperType_, PropertyName)\
   &RegHelperType_::FunctorObjectType::Set ## PropertyName\


#define REGISTER_PROPERTY(PropertyName, PropertyDesc, RegHelperType_, RegHelperInstance) \
   RegHelperInstance.RegisterProperty(m ## PropertyName, \
                                      CREATE_PROPERTY_SETTER_HELPER_MACRO(RegHelperType_, PropertyName), \
                                      CREATE_PROPERTY_GETTER_HELPER_MACRO(RegHelperType_, PropertyName), \
                                      #PropertyName, #PropertyDesc);\


}//namespace NetDemo

#endif //NETDEMO_AIUTILITY_H
