/* -*-c++-*-
 * SimulationCore
 * Copyright 2011, Alion Science and Technology
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
 * 
 * David Guthrie
 */
#include <prefix/SimCorePrefix.h>
#include <SimCore/ActComps/WheelActComp.h>
#include <SimCore/Actors/IGActor.h>
#include <dtGame/basemessages.h>
#include <osgSim/DOFTransform>
#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <dtCore/transform.h>

#include <dtGame/deadreckoninghelper.h>
#include <dtGame/invokable.h>
#include <dtGame/messagetype.h>
#include <dtUtil/mathdefines.h>

namespace SimCore
{

   namespace ActComps
   {

      /////////////////////////////////////////////////////
      Axle::Axle()
      : mIsSteerable(false)
      , mMaxSteerAngle(25.0f)
      {}

      DT_IMPLEMENT_ACCESSOR(Axle, osg::Vec2, WheelWidthAndRadius)
      DT_IMPLEMENT_ACCESSOR(Axle, bool, IsSteerable)
      DT_IMPLEMENT_ACCESSOR(Axle, float, MaxSteerAngle)

      /////////////////////////////////////////////////////
      Axle::~Axle() {}

      class DOFAxle : public Axle
      {
      public:
         DOFAxle(osgSim::DOFTransform* left, osgSim::DOFTransform* right)
         {
            if (left != NULL)
            {
               mWheelDofs[0] = left;
               mWheelDofs[1] = right;
            }
            else
            {
               mWheelDofs[0] = right;
               mWheelDofs[1] = NULL;
            }

            if (mWheelDofs[0] == NULL)
            {
               throw dtUtil::Exception("Can't create an axle with no wheels", __FILE__, __LINE__);
            }

            SetWheelWidthAndRadius(ComputeWheelWidthAndRadius());
         }

         virtual unsigned GetNumWheels() const
         {
            unsigned result = 0;
            for (unsigned i = 0; i < 2; ++i)
            {
               if (mWheelDofs[i] != NULL)
               {
                  ++result;
               }
            }
            return result;
         }

         osg::Vec2 ComputeWheelWidthAndRadius()
         {
            osg::Vec2 result;

            osg::ComputeBoundsVisitor bb;
            mWheelDofs[0]->accept(bb);
            result[0] = (bb.getBoundingBox().xMax() - bb.getBoundingBox().xMin()) / 2.0f;
            result[1] = (bb.getBoundingBox().zMax() - bb.getBoundingBox().zMin()) / 2.0f;
            return result;
         }

         virtual void UpdateAxleRotation(float dt, float steerAngle, float speedmps)
         {
            float twoPi = (2.0 * osg::PI);

            for (unsigned i = 0; i < GetNumWheels(); ++i)
            {
               float adjustedSteerAngle = osg::DegreesToRadians(steerAngle);
               float distance = speedmps * dt;
               // Arc length divided by radius is the radians of rotation.
               float rotationDT = distance / GetWheelWidthAndRadius()[1];

               osgSim::DOFTransform& dof = *mWheelDofs[i];
               osg::Vec3 hpr = dof.getCurrentHPR();
               // pitch of the hpr is the rotation of the wheel and rolling backward is the positive pitch
               // so we subtract.
               hpr[1] -= rotationDT;
               hpr[1] -= std::floor(hpr[1] / twoPi) * twoPi;

               // If it's not steerable, don't touch the heading ever.
               if (GetIsSteerable())
               {
                  float oldSteeringAngle = hpr[0];
                  hpr[0] = 0;
                  float angleDiff = adjustedSteerAngle - oldSteeringAngle;
                  float angleDiffAbs = dtUtil::Abs(angleDiff);
                  float maxSteerRadians = osg::DegreesToRadians(GetMaxSteerAngle());

                  // Max speed of steering change is the equivalent of the max steer angle per second.
                  if (angleDiffAbs / dt > maxSteerRadians)
                  {
                     float angleDiffSign = angleDiff / angleDiffAbs;
                     angleDiff = angleDiffSign * maxSteerRadians * dt;
                     adjustedSteerAngle = oldSteeringAngle + angleDiff;
                  }

                  float steerAngleAbs = dtUtil::Abs(adjustedSteerAngle);
                  float sign = adjustedSteerAngle / steerAngleAbs;
                  adjustedSteerAngle = sign * dtUtil::Min(steerAngleAbs, maxSteerRadians);
                  adjustedSteerAngle = dtUtil::IsFinite(adjustedSteerAngle) ? adjustedSteerAngle : oldSteeringAngle;
                  hpr[0] = adjustedSteerAngle;
               }
               dof.setCurrentHPR(hpr);
            }
         }

         virtual void UpdateWheelPosition(unsigned whichWheel, float jounceRebound)
         {
            if (whichWheel < GetNumWheels())
            {
               osgSim::DOFTransform& dof = *mWheelDofs[whichWheel];
               osg::Vec3 translate = dof.getCurrentTranslate();
               translate.z() = jounceRebound;
               dof.setCurrentTranslate(translate);
            }
         }

         virtual void SetWheelBaseTransform(unsigned whichWheel, const osg::Matrix& xform, bool worldRelative)
         {
            if (whichWheel < GetNumWheels())
            {
               osg::Node* nodeToMove = mWheelDofs[whichWheel]->getParent(0);
               if (nodeToMove == NULL)
               {
                  LOG_ERROR("Unable to move the wheel base transform because the node doesn't have one.")
                  return;
               }

               osg::Matrix worldMat;
               if (worldRelative && nodeToMove->getParent(0) != NULL)
               {
                  dtCore::Transformable::GetAbsoluteMatrix(nodeToMove->getParent(0), worldMat);
               }
               osg::Matrix relMat = xform * osg::Matrix::inverse(worldMat);

               dtCore::Transform removeScale;
               removeScale.Set(relMat);
               removeScale.Rescale(osg::Vec3(1.0f, 1.0f, 1.0f));
               removeScale.Get(relMat);

               osgSim::DOFTransform* dof = dynamic_cast<osgSim::DOFTransform*>(nodeToMove);
               if (dof != NULL)
               {
                  dtCore::Transform tmp;
                  tmp.Set(relMat);
                  osg::Vec3 hpr;
                  tmp.GetRotation(hpr);

                  dof->setHPRMultOrder(osgSim::DOFTransform::HPR);
                  dof->setCurrentHPR(hpr);
                  dof->setCurrentTranslate(xform.getTrans());
               }
               else
               {
                  osg::MatrixTransform* matTrans = dynamic_cast<osg::MatrixTransform*>(nodeToMove);
                  if (matTrans != NULL)
                  {
                     matTrans->setMatrix(relMat);
                  }
               }

            }
         }

         virtual void GetWheelBaseTransform(unsigned whichWheel, osg::Matrix& xform, bool worldRelative)
         {
            if (whichWheel < GetNumWheels())
            {
               osg::Node* parentNode = mWheelDofs[whichWheel]->getParent(0);
               if (parentNode != NULL)
               {
                  if (worldRelative)
                  {
                     dtCore::Transformable::GetAbsoluteMatrix(parentNode, xform);
                  }
                  else
                  {
                     osg::MatrixTransform* matTrans = dynamic_cast<osg::MatrixTransform*>(parentNode);
                     xform = matTrans->getMatrix();
                  }
               }
               else
               {
                  LOG_ERROR("Unable to move the wheel base transform because the node doesn't have one.")
               }
            }
         }

      private:
         virtual ~DOFAxle() {}

         dtCore::RefPtr<osgSim::DOFTransform> mWheelDofs[2];
      };

      IMPLEMENT_ENUM(WheelActComp::AutoUpdateModeEnum);
      WheelActComp::AutoUpdateModeEnum WheelActComp::AutoUpdateModeEnum::OFF("OFF");
      WheelActComp::AutoUpdateModeEnum WheelActComp::AutoUpdateModeEnum::REMOTE_ONLY("REMOTE_ONLY");
      WheelActComp::AutoUpdateModeEnum WheelActComp::AutoUpdateModeEnum::LOCAL_AND_REMOTE("LOCAL_AND_REMOTE");
      WheelActComp::AutoUpdateModeEnum::AutoUpdateModeEnum(const std::string& name)
      : dtUtil::Enumeration(name)
      {
      }

      const dtGame::ActorComponent::ACType WheelActComp::TYPE(new dtCore::ActorType("WheelActComp","ActorComponents",
            "Wheel finding and managing actor component.", dtGame::ActorComponent::BaseActorComponentType));

      /////////////////////////////////////////////////////
      WheelActComp::WheelActComp()
      : dtGame::ActorComponent(WheelActComp::TYPE)
      , mAutoConfigure(true)
      , mAutoUpdateMode(&AutoUpdateModeEnum::REMOTE_ONLY)
      , mWheelNodePrefix("dof_wheel_")
      , mSteeringAngle(0.0f)
      , mMaxSteeringAngle(25.0f)
      , mLastFrameHeading(0.0f)
      {
      }

      /////////////////////////////////////////////////////
      WheelActComp::~WheelActComp()
      {
      }

      DT_IMPLEMENT_ACCESSOR(WheelActComp, bool, AutoConfigure)

      DT_IMPLEMENT_ACCESSOR(WheelActComp, dtUtil::EnumerationPointer<WheelActComp::AutoUpdateModeEnum>, AutoUpdateMode)

      DT_IMPLEMENT_ACCESSOR(WheelActComp, dtUtil::RefString, WheelNodePrefix)

      DT_IMPLEMENT_ACCESSOR(WheelActComp, float, SteeringAngle)

      DT_IMPLEMENT_ACCESSOR(WheelActComp, float, MaxSteeringAngle)

      /////////////////////////////////////////////////////
      void WheelActComp::OnAddedToActor(dtGame::GameActor& actor)
      {
      }

      /////////////////////////////////////////////////////
      void WheelActComp::OnRemovedFromActor(dtGame::GameActor& actor)
      {

      }

      /////////////////////////////////////////////////////
      void WheelActComp::OnEnteredWorld()
      {
         if (GetAutoConfigure() && GetNumAxles() == 0)
         {
            FindAxles();
         }

         dtGame::GameActorProxy* actor = NULL;
         GetOwner(actor);

         if (GetAutoUpdateMode() == AutoUpdateModeEnum::LOCAL_AND_REMOTE
                  || (actor->IsRemote() && GetAutoUpdateMode() == AutoUpdateModeEnum::REMOTE_ONLY))
         {
            std::string tickInvokable = "Tick Remote " + GetType()->GetFullName();
            if (actor->GetInvokable(tickInvokable) == NULL)
            {
               actor->AddInvokable(*new dtGame::Invokable(tickInvokable,
                  dtUtil::MakeFunctor(&WheelActComp::Update, this)));
            }
            actor->RegisterForMessages(dtGame::MessageType::TICK_REMOTE, tickInvokable);
         }
      }

      /////////////////////////////////////////////////////
      void WheelActComp::OnRemovedFromWorld()
      {
         ClearAllAxles();
      }

      /////////////////////////////////////////////////////
      void WheelActComp::BuildPropertyMap()
      {

      }

      /////////////////////////////////////////////////////
      void WheelActComp::FindAxles(dtUtil::NodeCollector* nodeCollector)
      {
         dtCore::RefPtr<dtUtil::NodeCollector> nodeCollectorPtr = nodeCollector;
         if (!nodeCollectorPtr.valid())
         {
            dtGame::GameActorProxy* actor;
            GetOwner(actor);
            SimCore::Actors::IGActor* igDrawable = NULL;
            if (actor != NULL)
               actor->GetDrawable(igDrawable);
            if (igDrawable != NULL)
            {
               nodeCollectorPtr = igDrawable->GetNodeCollector();
            }
            else if (actor != NULL)
            {
               dtCore::DeltaDrawable* dd = NULL;
               actor->GetDrawable(dd);
               if (dd != NULL)
               {
                  nodeCollectorPtr = new dtUtil::NodeCollector(dd->GetOSGNode(), dtUtil::NodeCollector::AllNodeTypes);
               }
            }
         }

         if (nodeCollectorPtr.valid())
         {
            std::string indexString;
            // Reserve the string...
            std::string lookupString(mWheelNodePrefix + "      ");

            static const std::string leftString("lt_");
            static const std::string rightString("rt_");

            unsigned axle = 1;
            bool found = true;
            while (found)
            {
               dtUtil::MakeIndexString(axle, indexString, 2);
               lookupString = mWheelNodePrefix + leftString + indexString;
               osgSim::DOFTransform* wheelLeft = nodeCollectorPtr->GetDOFTransform(lookupString);
               lookupString = mWheelNodePrefix + rightString + indexString;
               osgSim::DOFTransform* wheelRight = nodeCollectorPtr->GetDOFTransform(lookupString);

               found = wheelLeft != NULL || wheelRight != NULL;
               if (found)
               {
                  dtCore::RefPtr<Axle> newAxle = new DOFAxle(wheelLeft, wheelRight);
                  // default only the first axle to steer.
                  newAxle->SetIsSteerable(axle == 1);
                  mAxles.push_back(newAxle);
               }
               ++axle;
            }
            if (mAxles.empty())
            {
               LOGN_INFO("WheelActComp.cpp", "Couldn't find any wheels.");
               return;
            }
         }
      }

      DT_IMPLEMENT_ARRAY_ACCESSOR(WheelActComp, dtCore::RefPtr<Axle>, Axle, Axles, NULL)

      /////////////////////////////////////////////////////
      void WheelActComp::Update(const dtGame::TickMessage& msg)
      {
         dtGame::GameActorProxy* actor = NULL;
         GetOwner(actor);

         if (actor == NULL)
         {
            LOG_ERROR("The owner actor is not a transformable!  Can't rotate the wheels.");
            return;
         }

         dtGame::DeadReckoningHelper* drHelper = NULL;
         actor->GetComponent(drHelper);

         osg::Vec3 vvec;
         float curHeading = 0.0f;
         if (drHelper != NULL)
         {
            vvec = drHelper->GetCurrentInstantVelocity();
            curHeading = drHelper->GetCurrentDeadReckonedRotation()[0];
         }

         dtCore::Transform xform;
         dtCore::Transformable* xformable;
         actor->GetDrawable(xformable);
         xformable->GetTransform(xform);
         osg::Vec3 fwd;
         osg::Vec3 right;
         xform.GetRow(1, fwd);
         xform.GetRow(0, right);

         osg::Vec3 vvecNormal = vvec;
         float length = vvecNormal.normalize();

         float steerAngle = 0.0f;
         float mps = 0.0f;

         if (length > FLT_EPSILON)
         {
            float dot = fwd * vvecNormal;

            mps = dot * length;

            steerAngle = (curHeading - mLastFrameHeading) / msg.GetDeltaSimTime();
            mLastFrameHeading = curHeading;
         }

         for (unsigned i = 0; i < GetNumAxles(); ++i)
         {
            Axle* axle = GetAxle(i);
            axle->UpdateAxleRotation(msg.GetDeltaSimTime(), steerAngle, mps);
         }
      }

   }

}
