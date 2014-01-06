/* -*-c++-*-
* Simulation Core
* Copyright 2007-2010, Alion Science and Technology
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
* Eddie Johnson, David Guthrie, Chris Rodgers, Curtiss Murphy
*/
#include <prefix/SimCorePrefix.h>

#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

#include <dtCore/isector.h>

#include <SimCore/Actors/BaseWaterActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <SimCore/Components/MultiSurfaceClamper.h>
#include <SimCore/VisibilityOptions.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gamemanager.inl>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/logcontroller.h>
#include <dtGame/messagefactory.h>

#include <dtAnim/animationcomponent.h>

#include <dtUtil/mathdefines.h>

#include <dtDAL/project.h>
#include <dtDAL/map.h>

using dtCore::RefPtr;
using SimCore::Actors::BaseEntityActorProxy;
using SimCore::Actors::BaseEntity;

namespace SimCore
{
   namespace Components
   {
      ///////////////////////////////////////////////////////////////////////////
      ViewerMessageProcessor::ViewerMessageProcessor()
      : mMagnification(1.0f)
      , mVisibilityOptions(new SimCore::VisibilityOptions)
      , mTimeSyncLatency(0L)
      {
         srand(unsigned(time(0)));
         mLogger = &dtUtil::Log::GetInstance("ViewerMessageProcessor.cpp");
      }

      ///////////////////////////////////////////////////////////////////////////
      ViewerMessageProcessor::~ViewerMessageProcessor()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessMessage(const dtGame::Message& msg)
      {
         // Many times, the terrain is part of the map loading
         if (msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            dtGame::GameManager& gameManager = *GetGameManager();
            std::vector<dtDAL::ActorProxy*> actors;

            //dtAnim::AnimationComponent* animComp = NULL;
            //gameManager.GetComponentByName(dtAnim::AnimationComponent::DEFAULT_NAME, animComp);

            // SET THE TERRAIN
            dtDAL::BaseActorObject* terrainAO = NULL;
            gameManager.FindActorByName("Terrain", terrainAO);
            if (!HandleTerrainActor(terrainAO))
            {
               const dtGame::MapMessage& mlm = static_cast<const dtGame::MapMessage&>(msg);
               dtGame::GameManager::NameVector mapNames;
               mlm.GetMapNames(mapNames);

               LOGN_DEBUG("ViewerMessageProcessor.cpp", "No terrain actor was found in the map: " + mapNames[0]);
            }

            // SET THE WATER
            // Get any water actor and assign it to the multi surface ground clamper,
            // which happens to be managed by the Dead Reckoning Component.
            SimCore::Actors::BaseWaterActorProxy* waterProxy = NULL;
            gameManager.FindActorByType(*SimCore::Actors::EntityActorRegistry::BASE_WATER_ACTOR_TYPE, waterProxy);
            HandleWaterActor(waterProxy);
         }

         // Sometimes, the terrain or water come after the map is loaded
         else if (msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED || 
            msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_UPDATED)
         {

            const dtGame::ActorUpdateMessage& updateMessage = static_cast<const dtGame::ActorUpdateMessage&> (msg);
            // SET THE TERRAIN
            dtDAL::BaseActorObject* terrainAO = GetGameManager()->FindActorById(msg.GetAboutActorId());
            if (terrainAO != NULL && terrainAO->GetName() == "Terrain")
            {
               HandleTerrainActor(terrainAO);
            }
            else if (updateMessage.GetActorType() ==  SimCore::Actors::EntityActorRegistry::BASE_WATER_ACTOR_TYPE)
            {
               HandleWaterActor(static_cast<SimCore::Actors::BaseWaterActorProxy*>(terrainAO));
            }
         }

         dtGame::DefaultMessageProcessor::ProcessMessage(msg);
      }

      ///////////////////////////////////////////////////////////////////////////
      bool ViewerMessageProcessor::HandleTerrainActor(dtDAL::ActorProxy* terrainProxy)
      {
         bool result = false;

         if(terrainProxy != NULL)
         {
            dtGame::DeadReckoningComponent* drComp = NULL;
            GetGameManager()->GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drComp);

            dtCore::Transformable* terrain;
            terrainProxy->GetActor(terrain);
            if(terrain == NULL)
            {
               LOG_ERROR("The terrain actor is not a transformable. Ignoring.");
            }
            else if (drComp != NULL)
            {
               result = true;
               drComp->SetTerrainActor(terrain);
               //if (animComp != NULL)
               //animComp->SetTerrainActor(terrain);
            }
         }
         return result;
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::HandleWaterActor(dtDAL::ActorProxy* waterProxy)
      {
         if (waterProxy != NULL)
         {
            dtGame::DeadReckoningComponent* drComp = NULL;
            GetGameManager()->GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drComp);

            if (drComp != NULL)
            {
               SimCore::Actors::BaseWaterActor* water = NULL;
               waterProxy->GetActor(water);

               // Assign the water surface to the clamper for water based entities to clamp to.
               SimCore::Components::MultiSurfaceClamper* clamper
                  = dynamic_cast<SimCore::Components::MultiSurfaceClamper*>(&drComp->GetGroundClamper());
               if(clamper != NULL)
               {
                  clamper->SetWaterSurface(water);
               }
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::OnAddedToGM()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      RefPtr<dtGame::GameActorProxy> ViewerMessageProcessor::ProcessRemoteCreateActor(const dtGame::ActorUpdateMessage& msg)
      {
         RefPtr<dtGame::GameActorProxy> ap = dtGame::DefaultMessageProcessor::ProcessRemoteCreateActor(msg);

         if(ap.valid() && dynamic_cast<SimCore::Actors:: StealthActorProxy*>(ap.get()) == NULL)
         {
            //Must dynamic cast here because the GetActor template does a static cast.
            BaseEntity* eap = dynamic_cast<BaseEntity*>(ap->GetDrawable());
            if (eap != NULL)
            {
               eap->SetScaleMagnification(osg::Vec3(mMagnification, mMagnification, mMagnification));
            }

         }
         return ap;
      }

      ////////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessRemoteUpdateActor(const dtGame::ActorUpdateMessage& msg,
               dtGame::GameActorProxy* ap)
      {
         dtGame::DefaultMessageProcessor::ProcessRemoteUpdateActor(msg, ap);

         if (ap == NULL)
         {
            return;
         }

         //Must dynamic cast here because the GetActor template does a static cast.
         SimCore::Actors::IGActor* ig = dynamic_cast<SimCore::Actors::IGActor*>(ap->GetDrawable());
         if (ig != NULL)
         {
            // The happens every time we get an update, but it must happen after the properties
            // are set because a property set can change if it should be visible or not.
            ig->SetVisible(ig->ShouldBeVisible(*mVisibilityOptions));
         }

      }


      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessLocalUpdateActor(const dtGame::ActorUpdateMessage& msg)
      {
         dtGame::GameActorProxy *ap = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
         if (ap == NULL)
         {
            LOG_ERROR("The about actor id is invalid");
            return;
         }

         if(msg.GetMessageType() == dtGame::MessageType::INFO_ACTOR_CREATED)
         {
            if(dynamic_cast<SimCore::Actors:: StealthActorProxy*>(ap) == NULL)
            {
               BaseEntityActorProxy *eap = dynamic_cast<BaseEntityActorProxy*>(ap);
               if(eap != NULL)
                  static_cast<BaseEntity&>(eap->GetGameActor()).SetScaleMagnification(osg::Vec3(mMagnification, mMagnification, mMagnification));
            }
            //Must dynamic cast here because the GetActor template does a static cast.
            SimCore::Actors::IGActor* ig = dynamic_cast<SimCore::Actors::IGActor*>(ap->GetDrawable());
            if (ig != NULL)
            {
               ig->SetVisible(ig->ShouldBeVisible(*mVisibilityOptions));
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessLocalDeleteActor(const dtGame::Message& msg)
      {
         if(mPlayer.valid() && msg.GetAboutActorId() == mPlayer->GetUniqueId())
         {
            mPlayer = NULL;
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessUnhandledLocalMessage(const dtGame::Message& msg)
      {
         if(msg.GetSource() != GetGameManager()->GetMachineInfo())
         {
            if(msg.GetMessageType() == MessageType::MAGNIFICATION)
            {
               mMagnification = static_cast<const MagnificationMessage&>(msg).GetMagnification();
               UpdateMagnificationAndVisibilityOptions();
            }
            else if(msg.GetMessageType() == MessageType::TIME_VALUE)
            {
               UpdateSyncTime(static_cast<const SimCore::TimeValueMessage&>(msg));
            }
            else
            {
               LOG_DEBUG("Received a message of unknown type: " + msg.GetMessageType().GetName());
            }
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::UpdateSyncTime(const SimCore::TimeValueMessage& tvMsg)
      {
         const unsigned long MILLISECONDS_TO_USECONDS = 1000UL;

         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            std::string msgString;
            tvMsg.ToString(msgString);
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__, "Received a time value message: \n%s", msgString.c_str());
         }

         if (true)//tvMsg.GetTimeMaster() == mTimeMasterName ||
             //     tvMsg.GetSenderName() == mTimeSyncSenderName.ToString())
         {
            mTimeMasterName = tvMsg.GetTimeMaster();

            if (tvMsg.GetSenderName() == mTimeSyncSenderName.ToString())
            {
               long transmitLatency = tvMsg.GetQueryReceivedRealTime() - tvMsg.GetQueryTransmitRealTime();
               long receiveLatency  = (long)(GetGameManager()->GetRealClockTime()/MILLISECONDS_TO_USECONDS) - tvMsg.GetValueTransmitRealTime();

               mTimeSyncLatency = (unsigned long)(abs((transmitLatency + receiveLatency) / 2));
            }

            unsigned long timeOffset = mTimeSyncLatency * int(tvMsg.GetTimeScale());

            // The time offset is mute if the simulation is paused.
            if (tvMsg.IsPaused())
            {
               timeOffset = 0L;
            }

            // Figure out which time scale to use. If the message has the default, then don't
            // change the timescale.  Allows time scale to be set in playback.
            float timeScale = tvMsg.GetTimeScale();
            if (timeScale == TimeValueMessage::DEFAULT_TIME_SCALE)
            {
               timeScale = GetGameManager()->GetTimeScale();
            }
            else
            {
               // We also have to check to see if the ServerLoggerComponent is alive and active.
               dtGame::LogController *logController;
               GetGameManager()->GetComponentByName(dtGame::LogController::DEFAULT_NAME, logController);

               if (logController != NULL)
               {
                  const dtGame::LogStatus &logStatus = logController->GetLastKnownStatus();
                  if (logStatus.GetStateEnum() == dtGame::LogStateEnumeration::LOGGER_STATE_PLAYBACK)
                     timeScale = GetGameManager()->GetTimeScale();
               }
            }

            dtCore::Timer_t newTime = dtCore::Timer_t(tvMsg.GetSynchronizedTime() + timeOffset);
            GetGameManager()->ChangeTimeSettings(double(newTime) / 1000.0, timeScale, GetGameManager()->GetSimulationClockTime());
            GetGameManager()->SetPaused(tvMsg.IsPaused());

            if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            {
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                        "Changed simulation clock time to match time value \"%lld\", scale \"%f\", and paused \"%s\"",
                        newTime, tvMsg.GetTimeScale(), tvMsg.IsPaused() ? "true" : "false");
            }
         }
         else
         {
            // Send our own query.
            dtCore::RefPtr<TimeQueryMessage> query;
            GetGameManager()->GetMessageFactory().CreateMessage(MessageType::TIME_QUERY, query);
            query->SetSenderName(mTimeSyncSenderName.ToString());
            //Set the transmit time to the sim clock time in seconds (normally in usecs)
            query->SetQueryTransmitRealTime(unsigned(GetGameManager()->GetRealClockTime() / 1000ULL));
            //GetGameManager()->SendMessage(*query);
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      bool ViewerMessageProcessor::AcceptPlayer(dtGame::GameActorProxy& playerProxy)
      {
         return !playerProxy.IsRemote();
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessPlayerEnteredWorldMessage(const dtGame::Message& msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD)
         {
            RefPtr<dtGame::GameActorProxy> proxy = GetGameManager()->FindGameActorById(msg.GetAboutActorId());

            if (!AcceptPlayer(*proxy))
               return;

            mPlayer = dynamic_cast<SimCore::Actors::StealthActor*>(proxy->GetDrawable());

            if(mPlayer == NULL)
            {
               LOG_ERROR("Received a player entered world message from an actor that is not a player");
               return;
            }
            else
            {
               LOG_INFO("Got a valid PlayerEnteredWorld with id: " + msg.GetAboutActorId().ToString());

               //Need to set the player on the dead reckoning component
               //so that it can use it for the LOD eye point for ground clamping
               //The dr comp listens for delete messages so it will clear the
               //actor itself when it is deleted.
               dtGame::DeadReckoningComponent* drComp =
                  static_cast<dtGame::DeadReckoningComponent*>(
                  GetGameManager()->GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME));

               if (drComp != NULL)
               {
                  LOG_INFO("Setting eye point on Dead Reckoning Component to the Player Actor: " + msg.GetAboutActorId().ToString());
                  drComp->SetEyePointActor(mPlayer.get());
               }

            }
         }
         else
            LOG_ERROR("Received a player entered world message of the wrong type");
      }


      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::SetVisibilityOptions(SimCore::VisibilityOptions& options)
      {
         mVisibilityOptions = &options;
         //It may be better here to set a flag and do in on the next tick.
         if (GetGameManager() != NULL)
         {
            UpdateMagnificationAndVisibilityOptions();
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      const SimCore::VisibilityOptions& ViewerMessageProcessor::GetVisibilityOptions() const
      {
         return *mVisibilityOptions;
      }

      ///////////////////////////////////////////////////////////////////////////
      SimCore::VisibilityOptions& ViewerMessageProcessor::GetVisibilityOptions()
      {
         return *mVisibilityOptions;
      }

      ///////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////
      class VMPUpdateMagVisFunctor
      {
      public:
         VMPUpdateMagVisFunctor(float magnification, SimCore::VisibilityOptions& visOpts)
         : mMagnification(magnification)
         , mVisibilityOptions(visOpts)
         {
         }

         void operator()(dtDAL::ActorProxy& ap)
         {
            SimCore::Actors::BaseEntityActorProxy* eap = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>(&ap);
            if (eap != NULL && dynamic_cast<SimCore::Actors::StealthActorProxy*>(eap) == NULL)
            {
               SimCore::Actors::BaseEntity* entity = NULL;
               eap->GetActor(entity);
               entity->SetScaleMagnification(osg::Vec3(mMagnification, mMagnification, mMagnification));
            }

            SimCore::Actors::IGActor* ig = dynamic_cast<SimCore::Actors::IGActor*>(ap.GetDrawable());
            if (ig != NULL)
            {
               ig->SetVisible(ig->ShouldBeVisible(mVisibilityOptions));
            }
         }
         float mMagnification;
         SimCore::VisibilityOptions& mVisibilityOptions;
      };
      ///////////////////////////////////////////////////////////////////////////
      ///////////////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::UpdateMagnificationAndVisibilityOptions()
      {
         VMPUpdateMagVisFunctor mag(mMagnification, *mVisibilityOptions);
         GetGameManager()->ForEachActor(mag);
      }

   }
}

