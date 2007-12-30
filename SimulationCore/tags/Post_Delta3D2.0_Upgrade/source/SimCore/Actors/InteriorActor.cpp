/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/InteriorActor.h>

#include <osgSim/DOFTransform>
#include <dtCore/nodecollector.h>
namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////
      // Proxy code
      //////////////////////////////////////////////////////////
      InteriorActorProxy::InteriorActorProxy()
      {
         SetClassName("SimCore::Actors::InteriorActor");
      }

      //////////////////////////////////////////////////////////
      InteriorActorProxy::~InteriorActorProxy()
      {

      }

      //////////////////////////////////////////////////////////
      void InteriorActorProxy::CreateActor()
      {
         InteriorActor* pActor = new InteriorActor(*this);
         SetActor(*pActor); 
         pActor->InitDeadReckoningHelper();
      }
      

      //////////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////////
      InteriorActor::InteriorActor(dtGame::GameActorProxy &proxy)
         : Platform(proxy),
         mVisible(true)
         , mLoadedDamageFiles(false)
      {
      }

      //////////////////////////////////////////////////////////
      InteriorActor::~InteriorActor()
      {
      }

      //////////////////////////////////////////////////////////
      void InteriorActor::SetVisible( bool visible )
      {
         mVisible = visible;
         SetDrawingModel(mVisible);
      }

      //////////////////////////////////////////////////////////
      bool InteriorActor::IsVisible() const
      {
         return mVisible;
      }

      //////////////////////////////////////////////////////////
      osgSim::DOFTransform* InteriorActor::GetSteeringWheelDOF(const std::string& dofName)
      {
         return GetNodeCollector()->GetDOFTransform(dofName);
      }
   }
}

