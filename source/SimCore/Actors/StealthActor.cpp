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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <dtGame/basemessages.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtUtil/log.h>
#include <dtDAL/enginepropertytypes.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////
      // Actor Proxy code
      //////////////////////////////////////////////////////
      StealthActorProxy::StealthActorProxy()
      {
         SetClassName("SimCore::Actors::StealthActor");
      }

      //////////////////////////////////////////////////////
      StealthActorProxy::~StealthActorProxy()
      {
      }

      //////////////////////////////////////////////////////
      void StealthActorProxy::BuildInvokables()
      {
         PlatformActorProxy::BuildInvokables();

         StealthActor &sa = static_cast<StealthActor&>(GetGameActor());

         AddInvokable(*new dtGame::Invokable("AttachToActor",
            dtDAL::MakeFunctor(sa, &StealthActor::AttachToActor)));

         AddInvokable(*new dtGame::Invokable("Detach",
            dtDAL::MakeFunctor(sa, &StealthActor::Detach)));

         AddInvokable(*new dtGame::Invokable("UpdateFromParent",
            dtDAL::MakeFunctor(sa, &StealthActor::UpdateFromParent)));

         AddInvokable(*new dtGame::Invokable("WarpToPosition",
            dtDAL::MakeFunctor(sa, &StealthActor::WarpToPosition)));

      }

      //////////////////////////////////////////////////////
      void StealthActorProxy::BuildPropertyMap()
      {
	   	PlatformActorProxy::BuildPropertyMap();

         StealthActor &sa = static_cast<StealthActor&>(GetGameActor());

         AddProperty(new dtDAL::Vec3ActorProperty("Attach Offset", "Attach Offset",
             dtDAL::Vec3ActorProperty::SetFuncType(&sa, &StealthActor::SetAttachOffset),
             dtDAL::Vec3ActorProperty::GetFuncType(&sa, &StealthActor::GetAttachOffset),
             "Property for the attach offset."));
      }

      //////////////////////////////////////////////////////
       void StealthActorProxy::CreateActor()
       {
          StealthActor* pEntity = new StealthActor(*this);
          SetActor(*pEntity);
       }

       //////////////////////////////////////////////////////
      void StealthActorProxy::OnEnteredWorld()
      {
         if (!IsRemote())
         {
            RegisterForMessagesAboutSelf(SimCore::MessageType::ATTACH_TO_ACTOR, "AttachToActor");
            RegisterForMessagesAboutSelf(SimCore::MessageType::REQUEST_WARP_TO_POSITION, "WarpToPosition");
         }

         PlatformActorProxy::OnEnteredWorld();

         dtCore::RefPtr<dtGame::Message> playerMsg = GetGameManager()->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD);
         playerMsg->SetAboutActorId(GetId());
         GetGameManager()->SendMessage(*playerMsg);
      }

      //////////////////////////////////////////////////////
      // Actor code
      //////////////////////////////////////////////////////
      StealthActor::StealthActor(dtGame::GameActorProxy &proxy) :
         Platform(proxy),
         mAttachAsThirdPerson(true),
         mOldDRA(&GetDeadReckoningAlgorithm()),
         mAttachOffset(0.0f, 0.0f, 1.5f)
      {
         mLogger = &dtUtil::Log::GetInstance("StealthActor.cpp");
      }

      //////////////////////////////////////////////////////////////////////////////
      StealthActor::~StealthActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::DoDetach()
      {
         // Unattach from the old parent or scene.
         if (GetParent() != NULL)
         {
            BaseEntity* entityParent = dynamic_cast<BaseEntity*>(GetParent());
            if (entityParent != NULL)
            {
               if(!mAttachAsThirdPerson)
               {
                  entityParent->SetIsPlayerAttached(false);
                  entityParent->SetDrawingModel(true);
               }

               GetGameActorProxy().UnregisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_DELETED, entityParent->GetUniqueId(), "Detach");
               GetGameActorProxy().UnregisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, entityParent->GetUniqueId(), "UpdateFromParent");
            }

            SetDeadReckoningAlgorithm(*mOldDRA);
            GetParent()->RemoveChild(this);
         }
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::DoAttach(const AttachToActorMessage& ataMsg, dtGame::GameActorProxy& ga)
      {
         dtCore::UniqueId id = ataMsg.GetAttachToActor();

         GetGameActorProxy().RegisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_DELETED, id, "Detach");
         GetGameActorProxy().RegisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, id, "UpdateFromParent");

         dtCore::Transform xform;
         xform.MakeIdentity();
         xform.SetRotation(ataMsg.GetInitialAttachRotationHPR());

         BaseEntity* entity = dynamic_cast<BaseEntity*>(ga.GetActor());
         if (entity != NULL)
         {
            if(!mAttachAsThirdPerson)
            {
               entity->SetDrawingModel(false);
               entity->SetIsPlayerAttached(true);
            }
            else
            {
               entity->SetDrawingModel(true);
               entity->SetIsPlayerAttached(true);
            }
            SetDeadReckoningAlgorithm(entity->GetDeadReckoningAlgorithm());

         }

         bool foundNode = false;
         IGActor* igActor = dynamic_cast<IGActor*>(ga.GetActor());
         if (igActor != NULL)
         {
            //we don't support named parts at his level.
            if (GetSceneParent() != NULL)
            {
              GetSceneParent()->RemoveDrawable(this);
            }
            else if (GetParent() != NULL)
            {
              Emancipate();
            }

            foundNode = igActor->AddChild(this, ataMsg.GetAttachPointNodeName());
         }

         // Only set an offset if there is no attach node.
         if (ataMsg.GetAttachPointNodeName().empty() || !foundNode)
         {
            //Set the translation first to be the same as the parent.
            //Make an identity transform and move it up a bit.
            osg::Vec3 translation = mAttachOffset;
            xform.SetTranslation(translation);
         }

         SetTransform(xform, REL_CS);
      }
      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::AttachOrDetachActor(const AttachToActorMessage* ataMsg)
      {
         dtGame::GameActorProxy* ga = NULL;
         dtCore::UniqueId id("");

         if (ataMsg != NULL)
         {
            id = ataMsg->GetAttachToActor();
            ga = GetGameActorProxy().GetGameManager()->FindGameActorById(id);
         }

         dtCore::Transform originalTransform;
         GetTransform(originalTransform, dtCore::Transformable::ABS_CS);

         // If we have a new actor to attach to, or the caller explicitly passed in a NULL id
         // to mark a detach, then. detach from the old actor.
         if (ga != NULL || id.ToString().empty())
         {
            DoDetach();
         }

         if (ga != NULL)
         {
            // If we are for some reason attaching to the environment actor.
            if (GetGameActorProxy().GetGameManager()->GetEnvironmentActor() != NULL &&
                GetGameActorProxy().GetGameManager()->GetEnvironmentActor()->GetId() == id)
            {
               dtGame::IEnvGameActor& ea = static_cast<dtGame::IEnvGameActor&>(GetGameActorProxy().GetGameManager()->GetEnvironmentActor()->GetGameActor());
               ea.AddActor(*this);
               SetTransform(originalTransform, dtCore::Transformable::ABS_CS);
            }
            else
            {
               DoAttach(*ataMsg, *ga);
            }
         }
         else if (id.ToString().empty())
         {
            //Attach back to the parent scene.
            if (GetParent() != NULL)
            {
               GetGameActorProxy().UnregisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_DELETED, GetParent()->GetUniqueId(), "Detach");
               GetGameActorProxy().UnregisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, GetParent()->GetUniqueId(), "UpdateFromParent");
               SetDeadReckoningAlgorithm(*mOldDRA);

               if(GetGameActorProxy().GetGameManager()->GetEnvironmentActor() != NULL)
               {
                  dtGame::IEnvGameActor &ea = static_cast<dtGame::IEnvGameActor&>(GetGameActorProxy().GetGameManager()->GetEnvironmentActor()->GetGameActor());
                  ea.AddActor(*this);
               }
               else
               {
                  GetGameActorProxy().GetGameManager()->GetScene().AddDrawable(this);
               }
            }
            else
            {
               SetTransform(originalTransform, dtCore::Transformable::ABS_CS);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::UpdateFromParent(const dtGame::Message &msg)
      {
         const dtGame::ActorUpdateMessage &aum = static_cast<const dtGame::ActorUpdateMessage&>(msg);
         dtGame::GameActorProxy *gap = GetGameActorProxy().GetGameManager()->FindGameActorById(aum.GetAboutActorId());
         if(gap == NULL)
            return;

         BaseEntity *entity = dynamic_cast<BaseEntity*>(gap->GetActor());
         if(entity == NULL)
            return;

         SetDeadReckoningAlgorithm(entity->GetDeadReckoningAlgorithm());
         SetVelocityVector(entity->GetVelocityVector());
         SetAngularVelocityVector(entity->GetAngularVelocityVector());
         SetAccelerationVector(entity->GetAccelerationVector());

         GetGameActorProxy().NotifyActorUpdate();
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::AttachToActor(const dtGame::Message& attachMessage)
      {
         AttachOrDetachActor(&static_cast<const SimCore::AttachToActorMessage&>(attachMessage));
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::Detach(const dtGame::Message &msg)
      {
         AttachOrDetachActor(NULL);
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::WarpToPosition(const dtGame::Message& warpToPosMessage)
      {
         AttachOrDetachActor(NULL);
         const SimCore::StealthActorUpdatedMessage& wtpMsg = static_cast<const SimCore::StealthActorUpdatedMessage&>(warpToPosMessage);
         dtCore::Transform xform;
         GetTransform(xform);
         xform.SetTranslation(wtpMsg.GetTranslation());
         SetTransform(xform);
      }

      //////////////////////////////////////////////////////////////////////////////
      bool StealthActor::IsAttachedToActor()
      {
         BaseEntity* entityParent = dynamic_cast<BaseEntity*>(GetParent());
         return (entityParent != NULL);
      }
   }
}
