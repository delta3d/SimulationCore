/* -*-c++-*-
* Simulation Core
* Copyright 2009-2010, Alion Science and Technology
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
* Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>

#include <SimCore/Actors/DRGhostActor.h>

#include <dtCore/shadermanager.h>

#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/basegroundclamper.h>
#include <dtGame/basemessages.h>
#include <dtGame/environmentactor.h>
#include <dtGame/messagefactory.h>
#include <dtGame/messagetype.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>


// For alpha
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Uniform>
#include <osg/MatrixTransform>

///////////////////////
// For the particle system visitor. Delete once merged back to the trunk and SetEnable no longer has Freeze
#include <dtCore/particlesystem.h>
#include <osg/Group>
#include <osg/NodeVisitor>
// Delete the above when we merge back to the trunk and SetEnable is no longer has Freeze
///////////////////////

#include <osg/Array>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Switch>
#include <osg/StateSet>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/Platform.h>

#include <iostream>

namespace SimCore
{
   namespace Actors
   {

      ///  DUPLICATE FROM PARTICLESYSTEM.CPP. DELETE ONCE BACK ON THE TRUNK AND Freeze 
      ///  IS NOT PART OF SetEnable()
      /**
      * A visitor class that applies a set of particle system parameters.
      */
      class ParticleSystemParameterVisitor : public osg::NodeVisitor
      {
      public:
         ParticleSystemParameterVisitor(bool enabled)
            : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN),
            mEnabled(enabled)
         {}

         virtual void apply(osg::Node& node)
         {
            osg::Node* nodePtr = &node;
            if (osgParticle::Emitter* emitter =
               dynamic_cast<osgParticle::Emitter*>(nodePtr))
            {
               emitter->setEnabled(mEnabled);
            }
            traverse(node);
         }

      private:
         bool mEnabled;
      };


      ///////////////////////////////////////////////////////////////////////////////////
      DRGhostActor::DRGhostActor(DRGhostActorProxy& proxy)
      : dtActors::GameMeshActor(proxy)
      , mSlaveUpdatedParticleIsActive(false)
      , mPosUpdatedParticleCountdown(0)
      , mVelocityArrowColor(0.2f, 0.2f, 1.0f)
      , mAccelerationArrowColor(0.2f, 1.0f, 0.2f)
      , mArrowDrawScalar(1.0f)
      , mArrowMaxNumTrails(40)
      , mArrowCurrentIndex(0)
      , mArrowDrawOnNextFrame(false)
      {
         SetName("GhostActor");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      DRGhostActor::~DRGhostActor(void)
      {
         if (mSlavedEntity.valid())
         {
            // See Baseentity::OnEnteredWorld for more info on why we do this
            //mSlavedEntity->GetDeadReckoningHelper().SetMaxTranslationSmoothingTime
            //   (dtGame::DeadReckoningHelper::DEFAULT_MAX_SMOOTHING_TIME_POS);
            //mSlavedEntity->GetDeadReckoningHelper().SetMaxRotationSmoothingTime
            //   (dtGame::DeadReckoningHelper::DEFAULT_MAX_SMOOTHING_TIME_ROT);
         }

      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::CleanUp()
      {
         // Remove our velocity line node from the scene before we go.
         dtGame::IEnvGameActorProxy *envProxy = GetGameActorProxy().GetGameManager()->GetEnvironmentActor();
         if (mArrowGlobalParentNode.valid() && envProxy != NULL)
         {
            dtGame::IEnvGameActor *envActor;
            envProxy->GetDrawable(envActor);
            envActor->RemoveActor(*mArrowGlobalParentNode);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetSlavedEntity(SimCore::Actors::BaseEntity* newEntity)
      {
         mSlavedEntity = newEntity;
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetArrowMaxNumTrails(unsigned int newValue)
      {
         if (mArrowGlobalParentNode.valid())
         {
            LOG_ERROR("You cannot set the number of velocity trails AFTER the ghost has been added with GM.AddActor().");
         }
         else
         {
            mArrowMaxNumTrails = newValue;
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::OnEnteredWorld()
      {

         if(!IsRemote())
         {
            // Get our mesh from the entity we are modeling
            if (mSlavedEntity.valid())
            {
               PlatformActorProxy* platform = dynamic_cast<PlatformActorProxy*>(&mSlavedEntity->GetGameActorProxy());
               if (platform != NULL)
               {
                  dtCore::ResourceActorProperty* rap = NULL;
                  GetGameActorProxy().GetProperty("static mesh", rap);
                  if (rap != NULL)
                  {
                     rap->SetValue(platform->GetNonDamagedResource());
                  }
               }
               //mSlavedEntity->GetDeadReckoningHelper().SetMaxRotationSmoothingTime(dtGame::DeadReckoningHelper::DEFAULT_MAX_SMOOTHING_TIME_ROT);
               //mSlavedEntity->GetDeadReckoningHelper().SetMaxTranslationSmoothingTime(dtGame::DeadReckoningHelper::DEFAULT_MAX_SMOOTHING_TIME_POS);
            }
            SetShaderGroup("GhostVehicleShaderGroup");

            osg::Group* g = GetOSGNode()->asGroup();
            osg::StateSet* ss = g->getOrCreateStateSet();
            ss->setMode(GL_BLEND,osg::StateAttribute::ON);
            dtCore::RefPtr<osg::BlendFunc> trans = new osg::BlendFunc();
            trans->setFunction( osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
            ss->setAttributeAndModes(trans.get());
            ss->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

            UpdateOurPosition();

            // Add a particle system to the Ghost to see where it's been.
            mTrailParticles = new dtCore::ParticleSystem;
            mTrailParticles->LoadFile("Particles/SimpleSpotTrailRed.osg", true);
            mTrailParticles->SetEnabled(true);
            AddChild(mTrailParticles.get());


            // Add a particle system to the Ghost - that works ONLY right after our slave gets updated.
            mUpdateTrailParticles = new dtCore::ParticleSystem;
            mUpdateTrailParticles->LoadFile("Particles/SimpleSpotTrail.osg", true);
            AddChild(mUpdateTrailParticles.get());
            // Start our red system OFF. 
            //mUpdateTrailParticles->SetEnabled(false); // put back when ParticleSystemParameterVisitor is deleted
            ParticleSystemParameterVisitor pspv = ParticleSystemParameterVisitor(false);
            mUpdateTrailParticles->GetOSGNode()->accept(pspv);
            mSlaveUpdatedParticleIsActive = false;

            // Replace the ghost shader with a simple shader that uses no lighting (ie fully lit)
            dtCore::RefPtr<dtCore::ShaderProgram> shader = 
               dtCore::ShaderManager::GetInstance().FindShaderPrototype("GhostParticleShader", "GhostVehicleShaderGroup");
            if(!shader.valid()) 
            {
               LOG_ERROR("FAILED to load shader for Ghost Particles. Will not correctly visualize update particles.");
            }
            else
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*shader, *mTrailParticles->GetOSGNode());
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*shader, *mUpdateTrailParticles->GetOSGNode());
            }


            // Global Parent for Vel and Accel arrows
            // We put all of our arrows under a special node that is NOT a child 
            // of the slave OR the ghost. The parent is world relative and doesn't move.
            mArrowGlobalParentNode = new dtCore::Transformable("Arrow");
            dtGame::IEnvGameActorProxy *envProxy = GetGameActorProxy().GetGameManager()->GetEnvironmentActor();
            if (envProxy != NULL)
            {
               dtGame::IEnvGameActor *envActor;
               envProxy->GetDrawable(envActor);
               envActor->AddActor(*mArrowGlobalParentNode);

               // Make this a settable value.
               dtCore::Transform xform(0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f);
               mArrowGlobalParentNode->SetTransform(xform);
            }
            else 
            {
               LOG_ERROR("There is no environment actor - The DRGhost will not function correctly.");
            }

            SetupVelocityArrows();
            SetupAccelerationArrows();
         }

         BaseClass::OnEnteredWorld();
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetupVelocityArrows()
      {
         // Create a velocity pointer.
         mVelocityArrowGeode = new osg::Geode();
         mVelocityArrowGeom = new osg::Geometry();
         mVelocityArrowVerts = new osg::Vec3Array();

         SetupLineData(*mVelocityArrowGeode.get(), *mVelocityArrowGeom.get(), 
            *mVelocityArrowVerts.get(), mVelocityArrowColor);
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetupAccelerationArrows()
      {
         // Create a velocity pointer.
         mAccelerationArrowGeode = new osg::Geode();
         mAccelerationArrowGeom = new osg::Geometry();
         mAccelerationArrowVerts = new osg::Vec3Array();

         SetupLineData(*mAccelerationArrowGeode.get(), *mAccelerationArrowGeom.get(), 
            *mAccelerationArrowVerts.get(), mAccelerationArrowColor);
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetupLineData(osg::Geode& arrowGeode, osg::Geometry& arrowGeom, 
         osg::Vec3Array& arrowVerts, const osg::Vec3& arrowColor)
      {
         // 2 points to create a line for our velocity.
         arrowVerts.reserve(mArrowMaxNumTrails * 2);
         for(unsigned int i = 0; i < mArrowMaxNumTrails; ++i)
         {
            arrowVerts.push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
            arrowVerts.push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
         }

         arrowGeom.setUseDisplayList(false);
         arrowGeom.setVertexArray(&arrowVerts);
         mVelocityArrowGeode->addDrawable(&arrowGeom);
         osg::StateSet* ss(NULL);
         ss = arrowGeode.getOrCreateStateSet();
         ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

         arrowGeom.setColorBinding(osg::Geometry::BIND_OVERALL);
         // Note - for future - if the color changes on the actor, need to reset this.
         osg::Vec3Array* colors = new osg::Vec3Array;
         colors->push_back(arrowColor);
         arrowGeom.setColorArray(colors); 

         arrowGeom.setVertexArray(&arrowVerts);
         arrowGeom.addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2 * mArrowMaxNumTrails));

         mArrowGlobalParentNode->GetMatrixNode()->addChild(&arrowGeode);
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
      {
         BaseClass::OnTickLocal( tickMessage );

         // Move to TickRemote().
         UpdateOurPosition();

         // Update our Velocity line.
         if (mArrowDrawOnNextFrame && mSlavedEntity.valid())
         {
            mArrowDrawOnNextFrame = false;

            dtCore::Transform xform;
            GetTransform(xform);
            osg::Vec3 ghostPos = xform.GetTranslation();

            // Update one of our Velocity Lines
            osg::Vec3 velocity = mSlavedEntity->GetComponent<dtGame::DeadReckoningHelper>()->GetLastKnownVelocity();
            SetCurrentLine(*mVelocityArrowGeom.get(), ghostPos, velocity);

            // Update one of our Acceleration Lines
            osg::Vec3 acceleration = mSlavedEntity->GetComponent<dtGame::DeadReckoningHelper>()->GetLastKnownAcceleration();
            SetCurrentLine(*mAccelerationArrowGeom.get(), ghostPos, acceleration);

            mArrowCurrentIndex = (mArrowCurrentIndex + 1) % mArrowMaxNumTrails;
         }



         // If we previously activated our 'slave entity updated particle system', then turn it off
         mPosUpdatedParticleCountdown --;
         if (mPosUpdatedParticleCountdown < 0 && mSlaveUpdatedParticleIsActive)
         {
            //mUpdateTrailParticles->SetEnabled(false); // put back when ParticleSystemParameterVisitor is deleted
            mSlaveUpdatedParticleIsActive = false;
            ParticleSystemParameterVisitor pspv = ParticleSystemParameterVisitor(false);
            mUpdateTrailParticles->GetOSGNode()->accept(pspv);
         }

         /*
         static float countDownToDebug = 1.0f;
         countDownToDebug -= tickMessage.GetDeltaSimTime();
         if (countDownToDebug < 0.0)
         {
            countDownToDebug = 1.0f;
            if (mSlavedEntity.valid())
            {
               std::cout << "GHOST - Vel[" << mSlavedEntity->GetDeadReckoningHelper().GetLastKnownVelocity() << "]." << std::endl;
            }
            else
            {
               LOG_ERROR("Ghost DR Actor [" + GetName() + "] has NO SLAVE!");
            }
         }
         */
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetCurrentLine(osg::Geometry& arrowGeom, osg::Vec3& startPos, osg::Vec3& endPosDelta)
      {
         osg::Vec3Array* vertices = (osg::Vec3Array*)arrowGeom.getVertexArray();
         // Set our start point to be our current ghost position
         unsigned int curIndex = mArrowCurrentIndex * 2;
         (*vertices)[curIndex].x() = startPos.x();
         (*vertices)[curIndex].y() = startPos.y(); 
         (*vertices)[curIndex].z() = startPos.z(); 

         // Set our end point to be current ghost pos + velocity.
         (*vertices)[curIndex + 1].x() = startPos.x() + endPosDelta.x() * mArrowDrawScalar;
         (*vertices)[curIndex + 1].y() = startPos.y() + endPosDelta.y() * mArrowDrawScalar;
         (*vertices)[curIndex + 1].z() = startPos.z() + endPosDelta.z() * mArrowDrawScalar;

         // Make sure that we force a redraw and bounds update so we see our new verts.
         arrowGeom.dirtyDisplayList();
         arrowGeom.dirtyBound();

      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::ClearLinesAndParticles()
      {
         // Make all of our lines go away
         osg::Vec3 zeros(0.0f, 0.0f, 0.0f);
         for(mArrowCurrentIndex = 0; ((unsigned int)mArrowCurrentIndex) < mArrowMaxNumTrails; ++mArrowCurrentIndex)
         {
            SetCurrentLine(*mVelocityArrowGeom.get(), zeros, zeros);
            SetCurrentLine(*mAccelerationArrowGeom.get(), zeros, zeros);
         }
         mArrowCurrentIndex = 0;

         // Clear out our particles
         mUpdateTrailParticles->ResetTime();
         mTrailParticles->ResetTime();
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::UpdateOurPosition()
      {
         if (mSlavedEntity.valid())
         {
            dtGame::DeadReckoningHelper& drHelper(*mSlavedEntity->GetComponent<dtGame::DeadReckoningHelper>());
            dtCore::Transform ourTransform;
            GetTransform(ourTransform);

            ourTransform.SetTranslation(drHelper.GetCurrentDeadReckonedTranslation());
            ourTransform.SetRotation(drHelper.GetCurrentDeadReckonedRotation());

            SetTransform(ourTransform);


            // GROUND CLAMPING
            // The DR Component does not do ground clamping our slave, because it's a local actors. 
            // The DRGhost has to manually force the ground clamper to do it on the ghost
            if (drHelper.GetGroundClampType() != dtGame::GroundClampTypeEnum::NONE)
            {
               dtGame::DeadReckoningComponent* drComp = NULL;
               GetGameActorProxy().GetGameManager()->
                  GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drComp);

               //BaseGroundClamper::GroundClampingType* groundClampingType = &;
               osg::Vec3 velocity(drHelper.GetCurrentInstantVelocity());

               // Call the ground clamper for the current object. The ground clamper should 
               // be smart enough to know what to do with the supplied values.
               drComp->GetGroundClamper().ClampToGround(
                  dtGame::BaseGroundClamper::GroundClampRangeType::RANGED, 
                  GetGameActorProxy().GetGameManager()->GetSimulationTime(), ourTransform, 
                  GetGameActorProxy(),drHelper.GetGroundClampingData(), true, velocity);
               drComp->GetGroundClamper().FinishUp();
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::ProcessMessage(const dtGame::Message& message)
      {
         if (message.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED)
         {
            //const dtGame::ActorUpdateMessage &updateMessage =
            //   static_cast<const dtGame::ActorUpdateMessage&> (message);

            // When our slave entity got updated, we are going to turn on red particle shader for one frame.
            //mUpdateTrailParticles->SetEnabled(true); // put back when ParticleSystemParameterVisitor is deleted
            if (!mSlaveUpdatedParticleIsActive) // if not already active.
            {
               mSlaveUpdatedParticleIsActive = true;
               ParticleSystemParameterVisitor pspv = ParticleSystemParameterVisitor(true);
               mUpdateTrailParticles->GetOSGNode()->accept(pspv);
            }
            mPosUpdatedParticleCountdown = 2; // 2 to make sure particles draw, even if we have a whacky frame hiccup.

            mArrowDrawOnNextFrame = true;

            // Hack Test Debug prints just to check Vel and Accel
            //if (mSlavedEntity.valid())
            //{
            //   osg::Vec3 velocity = mSlavedEntity->GetDeadReckoningHelper().GetLastKnownVelocity();
            //   osg::Vec3 acceleration = mSlavedEntity->GetDeadReckoningHelper().GetLastKnownAcceleration();
            //   std::cout << "Ghost - Updated - Vel[" << velocity << 
            //      "], Accel[" << acceleration << "]." << std::endl;
            //}
         }
      }

      //////////////////////////////////////////////////////////////////////
      // PROXY
      //////////////////////////////////////////////////////////////////////
      DRGhostActorProxy::DRGhostActorProxy()
      {
         SetClassName("DRGhostActor");
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActorProxy::BuildPropertyMap()
      {
         //static const dtUtil::RefString GROUP("DRGhost Props");
         BaseClass::BuildPropertyMap();
         //DRGhostActor& actor = *static_cast<DRGhostActor*>(GetActor());

         // Add properties
      }

      ///////////////////////////////////////////////////////////////////////////////////
      DRGhostActorProxy::~DRGhostActorProxy(){}
      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActorProxy::CreateDrawable()
      {
         SetDrawable(*new DRGhostActor(*this));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActorProxy::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         if (!IsRemote())
         {
            //So it's not a frame behind, it needs to happen on tick remote.  This is because the DeadReckoningComponent
            //ticks on Tick-Remote..
            //RegisterForMessages(dtGame::MessageType::TICK_REMOTE, dtGame::GameActorProxy::TICK_REMOTE_INVOKABLE);
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);

            // Listen for actor updates on our slave entity.
            if (GetActorAsDRGhostActor().GetSlavedEntity() != NULL)
            {
               RegisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, 
                  GetActorAsDRGhostActor().GetSlavedEntity()->GetUniqueId(), PROCESS_MSG_INVOKABLE);
            }
            else 
            {
               LOG_ERROR("Ghost DR Actor [" + GetName() + "] has NO SLAVE - cannot register for updates!");
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActorProxy::OnRemovedFromWorld()
      {
         GetActorAsDRGhostActor().CleanUp();
      }
   }
} // namespace
