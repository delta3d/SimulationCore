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

#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/MessageType.h>
#include <SimCore/Messages.h>

#include <dtCore/isector.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtGame/actorupdatemessage.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/logcontroller.h>

#include <dtAnim/animationcomponent.h>

#include <dtUtil/mathdefines.h>

#include <dtActors/coordinateconfigactor.h>

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
      ViewerMessageProcessor::ViewerMessageProcessor(): 
         mMagnification(1.0f),
         mTimeSyncLatency(0L)
      {
         srand(unsigned(time(0)));
         mLogger = &dtUtil::Log::GetInstance("ViewerMessageProcessor.cpp");
      }

      ///////////////////////////////////////////////////////////////////////////
      ViewerMessageProcessor::~ViewerMessageProcessor()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
         {
            dtGame::GameManager &gameManager = *GetGameManager();
            std::vector<dtDAL::ActorProxy*> actors;
            
            const dtGame::MapMessage &mlm = static_cast<const dtGame::MapMessage&>(msg);
            dtGame::GameManager::NameVector mapNames;
            mlm.GetMapNames(mapNames);
            
            dtGame::DeadReckoningComponent *drComp;
            gameManager.GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME, drComp);
            dtAnim::AnimationComponent *animComp;
            gameManager.GetComponentByName(dtAnim::AnimationComponent::DEFAULT_NAME, animComp);

            std::vector<dtDAL::ActorProxy*> toFill;
            gameManager.FindActorsByName("Terrain", toFill);
            dtDAL::ActorProxy* terrainProxy = NULL;
            if(!toFill.empty())
            {
               terrainProxy = toFill.front();
               dtCore::Transformable* terrain;
               terrainProxy->GetActor(terrain);
               if(terrain == NULL)
               {
                  LOG_ERROR("The terrain actor is not a transformable. Ignoring.");
               }
               else
               {
                  if (drComp != NULL)
                     drComp->SetTerrainActor(terrain);
                  //if (animComp != NULL)
                     //animComp->SetTerrainActor(terrain);
               }
            }
            else
            {
               LOG_ERROR("No terrain actor was found in the map: " + mapNames[0]);
            }
         }

         dtGame::DefaultMessageProcessor::ProcessMessage(msg);
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::OnAddedToGM()
      {
      }

      ///////////////////////////////////////////////////////////////////////////
      RefPtr<dtGame::GameActorProxy> ViewerMessageProcessor::ProcessRemoteCreateActor(const dtGame::ActorUpdateMessage &msg)
      {
         RefPtr<dtGame::GameActorProxy> ap = dtGame::DefaultMessageProcessor::ProcessRemoteCreateActor(msg);

         if(dynamic_cast<SimCore::Actors:: StealthActorProxy*>(ap.get()) == NULL)
         {
            BaseEntity *eap = dynamic_cast<BaseEntity*>(ap->GetActor());
            if(eap != NULL)
               eap->SetScaleMagnification(osg::Vec3(mMagnification, mMagnification, mMagnification));
         }
         return ap;
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessLocalUpdateActor(const dtGame::ActorUpdateMessage &msg) 
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
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessLocalDeleteActor(const dtGame::ActorDeletedMessage &msg)
      {
         if(mPlayer.valid() && msg.GetAboutActorId() == mPlayer->GetUniqueId())
         {
            mPlayer = NULL;
         }
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessUnhandledLocalMessage(const dtGame::Message &msg)
      {
         if(msg.GetSource() != GetGameManager()->GetMachineInfo())
         {
            if(msg.GetMessageType() == MessageType::MAGNIFICATION)
            {
               mMagnification = static_cast<const MagnificationMessage&>(msg).GetMagnification();
               UpdateMagnification();
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
         
//         if (tvMsg.GetTimeMaster() == mTimeMasterName ||
//               tvMsg.GetSenderName() == mTimeSyncSenderName.ToString())
//         {
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
//         }
//         else
//         {
//            /*
//            // Send our own query.
//            dtCore::RefPtr<TimeQueryMessage> query;
//            GetGameManager()->GetMessageFactory().CreateMessage(MessageType::TIME_QUERY, query);
//            query->SetSenderName(mTimeSyncSenderName.ToString());
//            //Set the transmit time to the sim clock time in seconds (normally in usecs)
//            query->SetQueryTransmitRealTime(GetGameManager()->GetRealClockTime() / SECONDS_TO_USECONDS);
//            GetGameManager()->SendMessage(*query);
//            */
//         }
      }

      ///////////////////////////////////////////////////////////////////////////
      bool ViewerMessageProcessor::AcceptPlayer(dtGame::GameActorProxy& playerProxy)
      {
         return !playerProxy.IsRemote();
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::ProcessPlayerEnteredWorldMessage(const dtGame::Message &msg)
      {
         if(msg.GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD)
         {
            RefPtr<dtGame::GameActorProxy> proxy = GetGameManager()->FindGameActorById(msg.GetAboutActorId());
         
            if (!AcceptPlayer(*proxy))
               return;            

            mPlayer = dynamic_cast<SimCore::Actors::StealthActor*>(proxy->GetActor());
            
            if(mPlayer == NULL)
            {
               LOG_ERROR("Received a player entered world message from an actor that is not a player");
               return;
            }
            else 
            {
               LOG_ALWAYS("Got a valid PlayerEnteredWorld with id: " + msg.GetAboutActorId().ToString());               

               //Need to set the player on the dead reckoning component
               //so that it can use it for the LOD eye point for ground clamping
               //The dr comp listens for delete messages so it will clear the
               //actor itself when it is deleted.
               dtGame::DeadReckoningComponent* drComp = 
                  static_cast<dtGame::DeadReckoningComponent*>(
                  GetGameManager()->GetComponentByName(dtGame::DeadReckoningComponent::DEFAULT_NAME));
               
               if (drComp != NULL)
               {
                  LOG_ALWAYS("Setting eye point on Dead Reckoning Component to the Player Actor: " + msg.GetAboutActorId().ToString());               
                  drComp->SetEyePointActor(mPlayer.get());
               }
               
            }
         }
         else
            LOG_ERROR("Received a player entered world message of the wrong type");
      }

      ///////////////////////////////////////////////////////////////////////////
      void ViewerMessageProcessor::UpdateMagnification()
      {
         //This should be changed to a message.
         if (GetGameManager() != NULL)
         {
            std::vector<dtGame::GameActorProxy*> toFill;
            GetGameManager()->GetAllGameActors(toFill);
            for (size_t i = 0; i < toFill.size(); ++i)
            {
               SimCore::Actors::BaseEntityActorProxy* eap = dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>(toFill[i]);
               if (eap != NULL && dynamic_cast<SimCore::Actors::StealthActorProxy*>(eap) == NULL)
               {
                  SimCore::Actors::BaseEntity& entity = static_cast<SimCore::Actors::BaseEntity&>(eap->GetGameActor());
                  entity.SetScaleMagnification(osg::Vec3(mMagnification, mMagnification, mMagnification)); 
               }
            }
            
         }
      }

   }
}

