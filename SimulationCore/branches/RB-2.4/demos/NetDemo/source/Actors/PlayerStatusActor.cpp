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
#include <prefix/SimCorePrefix-src.h>

#include <dtUtil/mswin.h>
#include <Actors/PlayerStatusActor.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtGame/basemessages.h>
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

   ///////////////////////////////////////////////////////////////////////////////////
   PlayerStatusActor::PlayerStatusActor(PlayerStatusActorProxy &proxy)
      : BaseClass(proxy)
      , mPlayerStatus(&PlayerStatusEnum::UNKNOWN)
      , mTeamNumber(0)
      , mIsServer(false)
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

      std::vector<std::string> propNames;
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
   // PROXY
   //////////////////////////////////////////////////////////////////////
   PlayerStatusActorProxy::PlayerStatusActorProxy()
   {
      SetClassName("PlayerStatusActor");
   }


   const dtUtil::RefString PlayerStatusActorProxy::PROP_PLAYER_STATUS("Player Status");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_TEAM_NUM("Team Number");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_IS_SERVER("Is Server");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_TERRAIN_PREFERENCE("Terrain Preference");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_VEHICLE_PREFERENCE("Vehicle Preference");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_ATTACHED_VEHICLE_ID("Attached Vehicle ID");
   const dtUtil::RefString PlayerStatusActorProxy::PROP_IP_ADDRESS("IP Address");


   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActorProxy::BuildPropertyMap()
   {
      const std::string& GROUP = "Player Status";

      BaseClass::BuildPropertyMap();

      PlayerStatusActor &actor = static_cast<PlayerStatusActor &>(GetGameActor());


      // OTHER POSSIBLE PROPERTIES

      // COLOR
      // SCORE


      static const dtUtil::RefString PROP_PLAYER_STATUS_DESC("Indicates the current status of this network player.");
      AddProperty(new dtDAL::EnumActorProperty<PlayerStatusActor::PlayerStatusEnum>(PROP_PLAYER_STATUS, PROP_PLAYER_STATUS,
         dtDAL::EnumActorProperty<PlayerStatusActor::PlayerStatusEnum>::SetFuncType(&actor, &PlayerStatusActor::SetPlayerStatus),
         dtDAL::EnumActorProperty<PlayerStatusActor::PlayerStatusEnum>::GetFuncType(&actor, &PlayerStatusActor::GetPlayerStatus),
         PROP_PLAYER_STATUS_DESC, GROUP));

      static const dtUtil::RefString PROP_TEAM_NUM_DESC("The player's Team number (1 or 2).");
      AddProperty(new dtDAL::IntActorProperty(PROP_TEAM_NUM, PROP_TEAM_NUM,
         dtDAL::IntActorProperty::SetFuncType(&actor, &PlayerStatusActor::SetTeamNumber),
         dtDAL::IntActorProperty::GetFuncType(&actor, &PlayerStatusActor::GetTeamNumber),
         PROP_TEAM_NUM_DESC, GROUP));

      static const dtUtil::RefString PROP_IS_SERVER_DESC("Only the server may publish true here. Everyone else is 0.");
      AddProperty(new dtDAL::BooleanActorProperty(PROP_IS_SERVER, PROP_IS_SERVER,
         dtDAL::BooleanActorProperty::SetFuncType(&actor, &PlayerStatusActor::SetIsServer),
         dtDAL::BooleanActorProperty::GetFuncType(&actor, &PlayerStatusActor::GetIsServer),
         PROP_IS_SERVER_DESC, GROUP));

      static const dtUtil::RefString PROP_TERRAIN_PREFERENCE_DESC("The desired terrain to load. The server's value will be the actual terrain people load.");
      AddProperty(new dtDAL::StringActorProperty(PROP_TERRAIN_PREFERENCE, PROP_TERRAIN_PREFERENCE,
         dtDAL::StringActorProperty::SetFuncType(&actor, &PlayerStatusActor::SetTerrainPreference),
         dtDAL::StringActorProperty::GetFuncType(&actor, &PlayerStatusActor::GetTerrainPreference),
         PROP_TERRAIN_PREFERENCE_DESC, GROUP));

      static const dtUtil::RefString PROP_VEHICLE_PREFERENCE_DESC("The desired startup vehicle type for this player. Not necessarily the current vehicle type for the player.");
      AddProperty(new dtDAL::EnumActorProperty<PlayerStatusActor::VehicleTypeEnum>(PROP_VEHICLE_PREFERENCE, PROP_VEHICLE_PREFERENCE,
         dtDAL::EnumActorProperty<PlayerStatusActor::VehicleTypeEnum>::SetFuncType(&actor, &PlayerStatusActor::SetVehiclePreference),
         dtDAL::EnumActorProperty<PlayerStatusActor::VehicleTypeEnum>::GetFuncType(&actor, &PlayerStatusActor::GetVehiclePreference),
         PROP_VEHICLE_PREFERENCE_DESC, GROUP));

      static const dtUtil::RefString PROP_ATTACHED_VEHICLE_ID_DESC("The ID of the vehicle the player is driving, if any. This property is controlled by the GameLogicComponent, do not set.");
      AddProperty(new dtDAL::ActorIDActorProperty(*this, PROP_ATTACHED_VEHICLE_ID, PROP_ATTACHED_VEHICLE_ID,
         dtDAL::ActorIDActorProperty::SetFuncType(&actor, &PlayerStatusActor::SetAttachedVehicleID),
         dtDAL::ActorIDActorProperty::GetFuncType(&actor, &PlayerStatusActor::GetAttachedVehicleID),
         PROP_ATTACHED_VEHICLE_ID_DESC, GROUP));

      static const dtUtil::RefString PROP_IP_ADDRESS_DESC("The IP Address for this player.");
      AddProperty(new dtDAL::StringActorProperty(PROP_IP_ADDRESS, PROP_IP_ADDRESS,
         dtDAL::StringActorProperty::SetFuncType(&actor, &PlayerStatusActor::SetIPAddress),
         dtDAL::StringActorProperty::GetFuncType(&actor, &PlayerStatusActor::GetIPAddress),
         PROP_IP_ADDRESS_DESC, GROUP));

   }

   ///////////////////////////////////////////////////////////////////////////////////
   PlayerStatusActorProxy::~PlayerStatusActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActorProxy::CreateActor()
   {
      PlayerStatusActor* playerStatusActor = new PlayerStatusActor(*this);
      SetActor(*playerStatusActor);

      playerStatusActor->InitDeadReckoningHelper();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void PlayerStatusActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
//#endif
