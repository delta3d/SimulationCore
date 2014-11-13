/*
* Copyright, 2009, Alion Science and Technology Corporation, all rights reserved.
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
#include <prefix/SimCorePrefix.h>

#include <dtUtil/mswin.h>
#include <Actors/PlayerStatusActor.h>

#include <dtCore/enginepropertytypes.h>
#include <dtGame/messagetype.h>
#include <dtABC/application.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagefactory.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/gamemanager.h>
//#include <SimCore/Actors/EntityActorRegistry.h>



namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(PlayerStatusActor::PlayerStatusEnum);
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::UNKNOWN("UNKNOWN");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::IN_LOBBY("IN_LOBBY");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::IN_GAME_READYROOM("IN_GAME_READYROOM");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::IN_GAME_ALIVE("IN_GAME_ALIVE");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::IN_GAME_DEAD("IN_GAME_DEAD");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::IN_GAME_UNKNOWN("IN_GAME_UNKNOWN");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::SCORE_SCREEN("SCORE_SCREEN");
   PlayerStatusActor::PlayerStatusEnum PlayerStatusActor::PlayerStatusEnum::LOADING("LOADING");

   IMPLEMENT_ENUM(PlayerStatusActor::VehicleTypeEnum);
   PlayerStatusActor::VehicleTypeEnum PlayerStatusActor::VehicleTypeEnum::OBSERVER("OBSERVER");
   PlayerStatusActor::VehicleTypeEnum PlayerStatusActor::VehicleTypeEnum::HOVER("HOVER");
   PlayerStatusActor::VehicleTypeEnum PlayerStatusActor::VehicleTypeEnum::FOUR_WHEEL("FOUR_WHEEL");
   PlayerStatusActor::VehicleTypeEnum PlayerStatusActor::VehicleTypeEnum::SURFACE_VESSEL("SURFACE_VESSEL");

   ///////////////////////////////////////////////////////////////////////////////////
   PlayerStatusActor::PlayerStatusActor(PlayerStatusActorProxy &proxy)
      : BaseClass(proxy)
      , mPlayerStatus(&PlayerStatusEnum::UNKNOWN)
      , mTeamNumber(0)
      , mScore(0)
      , mIsServer(false)
      , mIsReady(false)
      , mTerrainPreference("")
      , mVehiclePreference(&VehicleTypeEnum::FOUR_WHEEL)
      //, mVehiclePreference(&VehicleTypeEnum::HOVER)
      , mAttachedVehicleID("")
      , mIPAddress("")
      , mDirtyPlayerSettings(false)
   {
      SetAttachAsThirdPerson(true);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   PlayerStatusActor::~PlayerStatusActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      if(!IsRemote())
      {
      }
   }

   //////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickLocal(tickMessage);

      if (mDirtyPlayerSettings)
      {
         PublishPlayerSettings();
      }

      static float timeToForceTest = 10.0f;
      timeToForceTest -= tickMessage.GetDeltaRealTime();
      if (timeToForceTest <= 0.0f)
      {
         timeToForceTest = 10.0f;
         PublishPlayerSettings();

      }

   }

   //////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::OnTickRemote(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickRemote(tickMessage);

   }

   //////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::PublishPlayerSettings()
   {
      dtCore::RefPtr<dtGame::Message> msg = GetGameActorProxy().GetGameManager()->
         GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED);

      std::vector<dtUtil::RefString> propNames;
      propNames.push_back(PlayerStatusActorProxy::PROP_PLAYER_STATUS);
      propNames.push_back(PlayerStatusActorProxy::PROP_TEAM_NUM);
      propNames.push_back(PlayerStatusActorProxy::PROP_IS_SERVER);
      propNames.push_back(PlayerStatusActorProxy::PROP_TERRAIN_PREFERENCE);
      propNames.push_back(PlayerStatusActorProxy::PROP_VEHICLE_PREFERENCE);
      propNames.push_back(PlayerStatusActorProxy::PROP_ATTACHED_VEHICLE_ID);
      propNames.push_back(PlayerStatusActorProxy::PROP_IP_ADDRESS);
      GetGameActorProxy().PopulateActorUpdate(static_cast<dtGame::ActorUpdateMessage&>(*msg), propNames);

      GetGameActorProxy().GetGameManager()->SendMessage(*msg);
      //GetGameActorProxy().GetGameManager()->SendNetworkMessage(*msg);

      // Clears the dirty flag. Allows others to call this and keep flag in sync.
      mDirtyPlayerSettings = false;
   }


   ////////////////////////////////////////////////////////////////////////////////////
   PlayerStatusActor::PlayerStatusEnum& PlayerStatusActor::GetPlayerStatus() const
   {
      return *mPlayerStatus;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetPlayerStatus(PlayerStatusActor::PlayerStatusEnum &playerStatus)
   {
      mDirtyPlayerSettings = true;
      mPlayerStatus = &playerStatus;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetTeamNumber(int newValue)
   {
      mDirtyPlayerSettings = true;
      mTeamNumber = newValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetIsServer(bool newValue)
   {
      mDirtyPlayerSettings = true;
      mIsServer = newValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetTerrainPreference(const std::string& newValue)
   {
      mDirtyPlayerSettings = true;
      mTerrainPreference = newValue;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetVehiclePreference(PlayerStatusActor::VehicleTypeEnum &preference)
   {
      mDirtyPlayerSettings = true;
      mVehiclePreference = &preference;
   }

   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetAttachedVehicleID(const dtCore::UniqueId& newValue)
   {
      mDirtyPlayerSettings = true;
      mAttachedVehicleID = newValue;
   }


   ////////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetIPAddress(const std::string& newValue)
   {
      mDirtyPlayerSettings = true;
      mIPAddress = newValue;
   }

   //////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetReady(bool ready)
   {
      mIsReady = ready;
   }

   //////////////////////////////////////////////////////////////////////
   bool PlayerStatusActor::IsReady() const
   {
      return mIsReady;
   }

   //////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::SetScore(int score)
   {
      mScore = score;
   }
   
   //////////////////////////////////////////////////////////////////////
   int PlayerStatusActor::GetScore() const
   {
      return mScore;
   }
   
   //////////////////////////////////////////////////////////////////////
   void PlayerStatusActor::UpdateScore(int scoreModifier)
   {
      mScore += scoreModifier;
   }



   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   PlayerStatusActorProxy::PlayerStatusActorProxy()
   {
      SetClassName("PlayerStatusActor");
   }


   const dtUtil::RefString PlayerStatusActorProxy::PROP_PLAYER_STATUS("Player Status");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_TEAM_NUM("Team Number");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_IS_SERVER("Is Server");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_IS_READY("Is Ready");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_TERRAIN_PREFERENCE("Terrain Preference");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_VEHICLE_PREFERENCE("Vehicle Preference");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_ATTACHED_VEHICLE_ID("Attached Vehicle ID");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_IP_ADDRESS("IP Address");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_SCORE("Score");


   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActorProxy::BuildPropertyMap()
   {
      const std::string& GROUP = "Player Status";

      BaseClass::BuildPropertyMap();

      PlayerStatusActor* drawable = GetDrawable<PlayerStatusActor>();


      // OTHER POSSIBLE PROPERTIES

      // COLOR

      static const dtUtil::RefString PROP_PLAYER_STATUS_DESC("Indicates the current status of this network player.");
      AddProperty(new dtCore::EnumActorProperty<PlayerStatusActor::PlayerStatusEnum>(PROP_PLAYER_STATUS, PROP_PLAYER_STATUS,
         dtCore::EnumActorProperty<PlayerStatusActor::PlayerStatusEnum>::SetFuncType(drawable, &PlayerStatusActor::SetPlayerStatus),
         dtCore::EnumActorProperty<PlayerStatusActor::PlayerStatusEnum>::GetFuncType(drawable, &PlayerStatusActor::GetPlayerStatus),
         PROP_PLAYER_STATUS_DESC, GROUP));

      static const dtUtil::RefString PROP_TEAM_NUM_DESC("The player's Team number (1 or 2).");
      AddProperty(new dtCore::IntActorProperty(PROP_TEAM_NUM, PROP_TEAM_NUM,
         dtCore::IntActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetTeamNumber),
         dtCore::IntActorProperty::GetFuncType(drawable, &PlayerStatusActor::GetTeamNumber),
         PROP_TEAM_NUM_DESC, GROUP));

      static const dtUtil::RefString PROP_IS_SERVER_DESC("Only the server may publish true here. Everyone else is 0.");
      AddProperty(new dtCore::BooleanActorProperty(PROP_IS_SERVER, PROP_IS_SERVER,
         dtCore::BooleanActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetIsServer),
         dtCore::BooleanActorProperty::GetFuncType(drawable, &PlayerStatusActor::GetIsServer),
         PROP_IS_SERVER_DESC, GROUP));

      static const dtUtil::RefString PROP_IS_READY_DESC("Simple communication flag to allow clients to know if other clients are ready to start a game.");
      AddProperty(new dtCore::BooleanActorProperty(PROP_IS_READY, PROP_IS_READY,
         dtCore::BooleanActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetReady),
         dtCore::BooleanActorProperty::GetFuncType(drawable, &PlayerStatusActor::IsReady),
         PROP_IS_READY_DESC, GROUP));

      static const dtUtil::RefString PROP_TERRAIN_PREFERENCE_DESC("The desired terrain to load. The server's value will be the actual terrain people load.");
      AddProperty(new dtCore::StringActorProperty(PROP_TERRAIN_PREFERENCE, PROP_TERRAIN_PREFERENCE,
         dtCore::StringActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetTerrainPreference),
         dtCore::StringActorProperty::GetFuncType(drawable, &PlayerStatusActor::GetTerrainPreference),
         PROP_TERRAIN_PREFERENCE_DESC, GROUP));

      static const dtUtil::RefString PROP_VEHICLE_PREFERENCE_DESC("The desired startup vehicle type for this player. Not necessarily the current vehicle type for the player.");
      AddProperty(new dtCore::EnumActorProperty<PlayerStatusActor::VehicleTypeEnum>(PROP_VEHICLE_PREFERENCE, PROP_VEHICLE_PREFERENCE,
         dtCore::EnumActorProperty<PlayerStatusActor::VehicleTypeEnum>::SetFuncType(drawable, &PlayerStatusActor::SetVehiclePreference),
         dtCore::EnumActorProperty<PlayerStatusActor::VehicleTypeEnum>::GetFuncType(drawable, &PlayerStatusActor::GetVehiclePreference),
         PROP_VEHICLE_PREFERENCE_DESC, GROUP));

      static const dtUtil::RefString PROP_ATTACHED_VEHICLE_ID_DESC("The ID of the vehicle the player is driving, if any. This property is controlled by the GameLogicComponent, do not set.");
      AddProperty(new dtCore::ActorIDActorProperty(*this, PROP_ATTACHED_VEHICLE_ID, PROP_ATTACHED_VEHICLE_ID,
         dtCore::ActorIDActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetAttachedVehicleID),
         dtCore::ActorIDActorProperty::GetFuncType(drawable, &PlayerStatusActor::GetAttachedVehicleID),
         PROP_ATTACHED_VEHICLE_ID_DESC, GROUP));

      static const dtUtil::RefString PROP_IP_ADDRESS_DESC("The IP Address for this player.");
      AddProperty(new dtCore::StringActorProperty(PROP_IP_ADDRESS, PROP_IP_ADDRESS,
         dtCore::StringActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetIPAddress),
         dtCore::StringActorProperty::GetFuncType(drawable, &PlayerStatusActor::GetIPAddress),
         PROP_IP_ADDRESS_DESC, GROUP));

      static const dtUtil::RefString PROP_SCORE_DESC("Total points accumulated from destroying enemies.");
      AddProperty(new dtCore::IntActorProperty(PROP_SCORE, PROP_SCORE,
         dtCore::IntActorProperty::SetFuncType(drawable, &PlayerStatusActor::SetScore),
         dtCore::IntActorProperty::GetFuncType(drawable, &PlayerStatusActor::GetScore),
         PROP_SCORE_DESC, GROUP));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   PlayerStatusActorProxy::~PlayerStatusActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActorProxy::CreateDrawable()
   {
      PlayerStatusActor* playerStatusActor = new PlayerStatusActor(*this);
      SetDrawable(*playerStatusActor);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
//#endif
