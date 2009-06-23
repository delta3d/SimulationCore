/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>

#include <Actors/DRGhostActor.h>

//#include <dtDAL/enginepropertytypes.h>
//#include <dtABC/application.h>
//#include <dtUtil/matrixutil.h>
//#include <dtUtil/mathdefines.h>
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

namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   DRGhostActor::DRGhostActor(DRGhostActorProxy &proxy)
      : dtActors::GameMeshActor(proxy)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   DRGhostActor::~DRGhostActor(void)
   {
      if (mSlavedEntity.valid())
      {
         // See Baseentity::OnEnteredWorld for more info on why we do this
         mSlavedEntity->GetDeadReckoningHelper().SetMaxRotationSmoothingTime(0.0f);
      }
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void DRGhostActor::SetSlavedEntity(SimCore::Actors::BaseEntity *newEntity)
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
            mSlavedEntity->GetDeadReckoningHelper().SetMaxRotationSmoothingTime(1.0f);
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

      UpdateOurPosition();

      static float countDownToDebug = 1.0f;
      countDownToDebug -= tickMessage.GetDeltaSimTime();
      if (countDownToDebug < 0.0)
      {
         countDownToDebug = 1.0f;
         if (mSlavedEntity.valid())
         {
            std::cout << "GHOST - Vel[" << mSlavedEntity->GetDeadReckoningHelper().GetVelocityVector() << 
               "]." << std::endl;
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
      const std::string GROUP = "DRGhost Props";
      BaseClass::BuildPropertyMap();
      DRGhostActor& actor = *static_cast<DRGhostActor*>(GetActor());

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

} // namespace
