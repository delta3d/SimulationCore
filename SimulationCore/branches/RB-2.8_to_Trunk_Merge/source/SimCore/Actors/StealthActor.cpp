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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/StealthActor.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagefactory.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <dtGame/environmentactor.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtCore/scene.h>
#include <dtCore/transform.h>
#include <dtUtil/log.h>
#include <dtCore/enginepropertytypes.h>

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

         StealthActor* sa = NULL;
         GetDrawable(sa);

         AddInvokable(*new dtGame::Invokable("AttachToActor",
            dtUtil::MakeFunctor(&StealthActor::AttachToActor, *sa)));

         AddInvokable(*new dtGame::Invokable("Detach",
            dtUtil::MakeFunctor(&StealthActor::Detach, *sa)));

         AddInvokable(*new dtGame::Invokable("UpdateFromParent",
            dtUtil::MakeFunctor(&StealthActor::UpdateFromParent, *sa)));

         AddInvokable(*new dtGame::Invokable("WarpToPosition",
            dtUtil::MakeFunctor(&StealthActor::WarpToPosition, *sa)));

      }

      //////////////////////////////////////////////////////
      void StealthActorProxy::BuildPropertyMap()
      {
	   	PlatformActorProxy::BuildPropertyMap();

         StealthActor* sa = NULL;
         GetDrawable(sa);

         AddProperty(new dtCore::Vec3ActorProperty("Attach Offset", "Attach Offset",
             dtCore::Vec3ActorProperty::SetFuncType(sa, &StealthActor::SetAttachOffset),
             dtCore::Vec3ActorProperty::GetFuncType(sa, &StealthActor::GetAttachOffset),
             "Property for the attach offset."));
      }

      //////////////////////////////////////////////////////
       void StealthActorProxy::CreateDrawable()
       {
          StealthActor* pEntity = new StealthActor(*this);
          SetDrawable(*pEntity);
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
      StealthActor::StealthActor(dtGame::GameActorProxy& owner)
         : Platform(owner)
         , mAttachAsThirdPerson(true)
         , mOldDRA(&dtGame::DeadReckoningAlgorithm::NONE)
         , mAttachOffset(0.0f, 0.0f, 1.5f)
      {
         mLogger = &dtUtil::Log::GetInstance("StealthActor.cpp");
         SetAutoRegisterWithMunitionsComponent(false);
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
                  entityParent->SetPlayerAttached(false);
                  entityParent->SetDrawingModel(true);
               }

               GetGameActorProxy().UnregisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_DELETED, entityParent->GetUniqueId(), "Detach");
               GetGameActorProxy().UnregisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, entityParent->GetUniqueId(), "UpdateFromParent");
            }

            GetComponent<dtGame::DeadReckoningHelper>()->SetDeadReckoningAlgorithm(*mOldDRA);
            Emancipate();
         }
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::DoAttach(dtGame::GameActorProxy& ga, const std::string& attachPointNode,
         const osg::Vec3& attachRotationHPR)
      {
         dtCore::UniqueId id = ga.GetId();

         GetGameActorProxy().RegisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_DELETED, id, "Detach");
         GetGameActorProxy().RegisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, id, "UpdateFromParent");

         dtCore::Transform xform;
         xform.SetRotation(attachRotationHPR);

         BaseEntity* entity = NULL;
         ga.GetDrawable(entity);

         if (entity != NULL)
         {
            if (!mAttachAsThirdPerson)
            {
               entity->SetDrawingModel(false);
               entity->SetPlayerAttached(true);
            }
            else
            {
               entity->SetDrawingModel(true);
               entity->SetPlayerAttached(true);
            }
            dtGame::DeadReckoningHelper* drHelper = NULL;
            GetComponent(drHelper);

            mOldDRA = &drHelper->GetDeadReckoningAlgorithm();

            dtGame::DeadReckoningHelper* drHelperEntity = NULL;
            ga.GetComponent(drHelperEntity);

            drHelper->SetDeadReckoningAlgorithm(
               drHelperEntity->GetDeadReckoningAlgorithm());

         }

         bool foundNode = false;
         dtCore::Transformable* xformable = NULL;
         ga.GetDrawable(xformable);
         dtCore::Transform tempXform;

         xformable->GetTransform(tempXform);
         if (!tempXform.IsValid())
         {
             // Bail out if the actor has NAN or what not in it's matrix.
             AttachOrDetachActor(NULL, dtCore::UniqueId(""));
             return;
         }

         if (xformable != NULL)
         {
            //we don't support named parts at his level.
            if (GetParent() != NULL)
            {
               Emancipate();
            }

            IGActor* igActor = NULL;
            ga.GetDrawable(igActor);
            if (igActor != NULL)
            {
               foundNode = igActor->AddChild(this, attachPointNode);
            }
            else
            {
               xformable->AddChild(this);
            }
         }

         // Only set an offset if there is no attach node.
         if (attachPointNode.empty() || !foundNode)
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
         if (ataMsg != NULL)
         {
            AttachOrDetachActor(GetGameActorProxy().GetGameManager()->FindGameActorById(ataMsg->GetAttachToActor()),
               ataMsg->GetAttachToActor(), ataMsg->GetAttachPointNodeName(), 
               ataMsg->GetInitialAttachRotationHPR());
         }
         else
         {
            AttachOrDetachActor(NULL, dtCore::UniqueId(""));
         }

      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::AttachOrDetachActor(dtGame::GameActorProxy* ga, const dtCore::UniqueId& id,
         const std::string& attachPointNode, const osg::Vec3& attachRotationHPR)
      {
         dtCore::Transform originalTransform;
         GetTransform(originalTransform, dtCore::Transformable::ABS_CS);


         DoDetach();

         if (ga != NULL)
         {
            // If we are for some reason attaching to the environment actor.
            if (GetGameActorProxy().GetGameManager()->GetEnvironmentActor() != NULL &&
                GetGameActorProxy().GetGameManager()->GetEnvironmentActor()->GetId() == id)
            {
               dtGame::IEnvGameActor* ea = GetGameActorProxy().GetGameManager()->GetEnvironmentActor()->GetDrawable<dtGame::IEnvGameActor>();
               ea->AddActor(*this);
               SetTransform(originalTransform, dtCore::Transformable::ABS_CS);
            }
            else
            {
               //DoAttach(*ataMsg, *ga);
               DoAttach(*ga, attachPointNode, attachRotationHPR); 
            }
         }
         else if (id.ToString().empty())
         {
            //Attach back to the parent scene.
            if (GetGameActorProxy().GetGameManager()->GetEnvironmentActor() != NULL)
            {
               dtGame::IEnvGameActor* ea = GetGameActorProxy().GetGameManager()->GetEnvironmentActor()->GetDrawable<dtGame::IEnvGameActor>();
               ea->AddActor(*this);
            }
            else
            {
               GetGameActorProxy().GetGameManager()->GetScene().AddChild(this);
            }

            SetTransform(originalTransform, dtCore::Transformable::ABS_CS);
         }
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::UpdateFromParent(const dtGame::Message& msg)
      {
         const dtGame::ActorUpdateMessage& aum = static_cast<const dtGame::ActorUpdateMessage&>(msg);
         dtGame::GameActorProxy* parent = GetGameActorProxy().GetGameManager()->FindGameActorById(aum.GetAboutActorId());
         if(parent == NULL)
         {
            return;
         }

         dtGame::DeadReckoningHelper* drHelper = NULL;
         GetComponent(drHelper);
         dtGame::DeadReckoningHelper* drHelperParent = NULL;
         parent->GetComponent(drHelperParent);

         drHelper->SetDeadReckoningAlgorithm(drHelperParent->GetDeadReckoningAlgorithm());
         drHelper->SetLastKnownVelocity(drHelperParent->GetLastKnownVelocity());
         drHelper->SetLastKnownAngularVelocity(drHelperParent->GetLastKnownAngularVelocity());
         drHelper->SetLastKnownAcceleration(drHelperParent->GetLastKnownAcceleration());

         GetGameActorProxy().NotifyFullActorUpdate();
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::AttachToActor(const dtGame::Message& attachMessage)
      {
         AttachOrDetachActor(&static_cast<const SimCore::AttachToActorMessage&>(attachMessage));
      }

      //////////////////////////////////////////////////////////////////////////////
      void StealthActor::Detach(const dtGame::Message& msg)
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
