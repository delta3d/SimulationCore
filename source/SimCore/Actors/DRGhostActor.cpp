/* -*-c++-*-
 * Driver Demo - HoverVehicleActor (.cpp & .h) - Using 'The MIT License'
 * Copyright (C) 2008, Alion Science and Technology Corporation
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
 * Curtiss Murphy
 */
#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Actors/DRGhostActor.h>

#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/basemessages.h>
#include <osg/Switch>
#include <osgSim/DOFTransform>


// For alpha
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/Uniform>
#include <osg/MatrixTransform>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/Platform.h>

#include <iostream>

namespace SimCore
{
   namespace Actors
   {
      ///////////////////////////////////////////////////////////////////////////////////
      DRGhostActor::DRGhostActor(DRGhostActorProxy& proxy)
      : dtActors::GameMeshActor(proxy)
      {
      }

      ///////////////////////////////////////////////////////////////////////////////////
      DRGhostActor::~DRGhostActor(void)
      {
         if (mSlavedEntity.valid())
         {
            // See Baseentity::OnEnteredWorld for more info on why we do this
            mSlavedEntity->GetDeadReckoningHelper().SetMaxTranslationSmoothingTime(0.0f);
            mSlavedEntity->GetDeadReckoningHelper().SetMaxRotationSmoothingTime(0.0f);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::SetSlavedEntity(SimCore::Actors::BaseEntity* newEntity)
      {
         mSlavedEntity = newEntity;
      }


      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActor::OnEnteredWorld()
      {

         if(!IsRemote())
         {
            // Get our mesh from the entity we are modeling
            if (mSlavedEntity.valid())
            {
               SimCore::Actors::Platform *platform = dynamic_cast<SimCore::Actors::Platform*>(mSlavedEntity.get());
               if (platform != NULL)
               {
                  SetMesh(platform->GetNonDamagedNodeFileName());
               }
               mSlavedEntity->GetDeadReckoningHelper().SetMaxRotationSmoothingTime(dtGame::DeadReckoningHelper::DEFAULT_MAX_SMOOTHING_TIME_ROT);
               mSlavedEntity->GetDeadReckoningHelper().SetMaxTranslationSmoothingTime(dtGame::DeadReckoningHelper::DEFAULT_MAX_SMOOTHING_TIME_POS);
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
         }

         BaseClass::OnEnteredWorld();
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
      {
         BaseClass::OnTickLocal( tickMessage );

         // Move to TickRemote().
         UpdateOurPosition();

         static float countDownToDebug = 1.0f;
         countDownToDebug -= tickMessage.GetDeltaSimTime();
         if (countDownToDebug < 0.0)
         {
            countDownToDebug = 1.0f;
            if (mSlavedEntity.valid())
            {
               //std::cout << "GHOST - Vel[" << mSlavedEntity->GetDeadReckoningHelper().GetLastKnownVelocity() <<
               //   "]." << std::endl;
            }
            else
            {
               std::cout << "GHOST - NO SLAVE!" << std::endl;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////
      void DRGhostActor::UpdateOurPosition()
      {
         if (mSlavedEntity.valid())
         {
            dtCore::Transform ourTransform;
            GetTransform(ourTransform);

            ourTransform.SetTranslation(mSlavedEntity->
                     GetDeadReckoningHelper().GetCurrentDeadReckonedTranslation());
            ourTransform.SetRotation(mSlavedEntity->
                     GetDeadReckoningHelper().GetCurrentDeadReckonedRotation());

            SetTransform(ourTransform);
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
      void DRGhostActorProxy::CreateActor()
      {
         SetActor(*new DRGhostActor(*this));
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void DRGhostActorProxy::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         if (!IsRemote())
         {
            RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
         }
      }
   }
} // namespace
