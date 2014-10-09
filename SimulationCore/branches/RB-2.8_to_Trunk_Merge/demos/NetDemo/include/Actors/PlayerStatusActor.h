/* -*-c++-*-
* Driver Demo - PlayerStatusActor (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
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
* @author Curtiss Murphy
*/
#ifndef _PLAYER_STATUS_ACTOR_
#define _PLAYER_STATUS_ACTOR_

#include <DemoExport.h>

#include <SimCore/Actors/PlayerActor.h>


namespace NetDemo
{
   class PlayerStatusActorProxy;


   ////////////////////////////////////////////////////////////////////////////////
   /* This actor holds the basic settings and status for each real player connected to the world.
    *    It subclasses the PlayerActor from SimCore, which is probably WAY overkill. However, we
    *    will have a PlayerActor anyway, so it simplifies things.
    */
   class NETDEMO_EXPORT PlayerStatusActor : public SimCore::Actors::PlayerActor // dtGame::GameActor
   {
      public:

         ////////////////////////////////////////////////////////////////////////////////
         // This enum describes the state of each player - ie, whether they are connected, in game, etc...
         ////////////////////////////////////////////////////////////////////////////////
         class NETDEMO_EXPORT PlayerStatusEnum : public dtUtil::Enumeration
         {
            DECLARE_ENUM(PlayerStatusEnum);
         public:
            static PlayerStatusEnum UNKNOWN;
            static PlayerStatusEnum IN_LOBBY;
            static PlayerStatusEnum IN_GAME_READYROOM;
            static PlayerStatusEnum IN_GAME_ALIVE;
            static PlayerStatusEnum IN_GAME_DEAD;
            static PlayerStatusEnum IN_GAME_UNKNOWN;
            static PlayerStatusEnum SCORE_SCREEN;
            static PlayerStatusEnum LOADING;
         private:
            PlayerStatusEnum(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
         };

         class NETDEMO_EXPORT VehicleTypeEnum : public dtUtil::Enumeration
         {
            DECLARE_ENUM(VehicleTypeEnum);
         public:
            static VehicleTypeEnum OBSERVER;
            static VehicleTypeEnum HOVER;
            static VehicleTypeEnum FOUR_WHEEL;
         private:
            VehicleTypeEnum(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
         };


         //typedef dtGame::GameActor BaseClass;
         typedef SimCore::Actors::PlayerActor BaseClass;

         /// Constructor
         PlayerStatusActor (PlayerStatusActorProxy &proxy);

      protected:
         /// Destructor
         virtual ~PlayerStatusActor();

      // INHERITED PUBLIC
      public:

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);
         virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

         // Publishes an actor update message for player settings props to let other players know.
         void PublishPlayerSettings();


         /////////////////////////////////////////////////////////////
         // PROPERTIES

         // Player Status - Property
         void SetPlayerStatus(PlayerStatusEnum &playerStatus);
         PlayerStatusEnum& GetPlayerStatus() const;

         // Team Number - Property
         void SetTeamNumber(int newValue);
         int GetTeamNumber() const { return mTeamNumber; }

         // Is Server - Property
         void SetIsServer(bool newValue);
         bool GetIsServer() const { return mIsServer; }

         void SetReady(bool ready);
         bool IsReady() const;

         // Terrain Preference - Property - The server's value here defines what terrain is loaded for all.
         void SetTerrainPreference(const std::string& newValue);
         const std::string& GetTerrainPreference() const { return mTerrainPreference; }

         // Vehicle Preference - Property - The desired startup vehicle type, but not necessarily the current vehicle type.
         void SetVehiclePreference(VehicleTypeEnum &newValue);
         VehicleTypeEnum& GetVehiclePreference() const { return *mVehiclePreference; }

         // Vehicle Actor ID - Property - The id of the vehicle this player is currently driving.
         void SetAttachedVehicleID(const dtCore::UniqueId& newValue);
         const dtCore::UniqueId& GetAttachedVehicleID() const { return mAttachedVehicleID; }

         // IP Address - Property
         void SetIPAddress(const std::string& newValue);
         const std::string& GetIPAddress() const { return mIPAddress; }

         void SetScore(int score);
         int GetScore() const;
         void UpdateScore(int scoreModifier);

      protected:


      // Private vars
      private:

         PlayerStatusEnum* mPlayerStatus;
         int mTeamNumber;
         int mScore;
         bool mIsServer;
         bool mIsReady;
         std::string mTerrainPreference;
         VehicleTypeEnum* mVehiclePreference;
         dtCore::UniqueId mAttachedVehicleID;
         std::string mIPAddress;

         bool mDirtyPlayerSettings;

   };


   ////////////////////////////////////////////////////////////////////////////////
   ////////////////////////////////////////////////////////////////////////////////
   /// This is the proxy for the object.  It needs to build the property map, create the actor, and handle entered world.
   class NETDEMO_EXPORT PlayerStatusActorProxy : public SimCore::Actors::PlayerActorProxy //dtGame::GameActorProxy
   {
      public:
         typedef SimCore::Actors::PlayerActorProxy BaseClass;

         // public strings for the properties
         static const dtUtil::RefString PROP_PLAYER_STATUS;
         static const dtUtil::RefString PROP_TEAM_NUM;
         static const dtUtil::RefString PROP_IS_SERVER;
         static const dtUtil::RefString PROP_IS_READY;
         static const dtUtil::RefString PROP_TERRAIN_PREFERENCE;
         static const dtUtil::RefString PROP_VEHICLE_PREFERENCE;
         static const dtUtil::RefString PROP_ATTACHED_VEHICLE_ID;
         static const dtUtil::RefString PROP_IP_ADDRESS;
         static const dtUtil::RefString PROP_SCORE;

         PlayerStatusActorProxy();
         virtual void BuildPropertyMap();

         /// Returns a useful reference to our actor. If no actor is created yet, this will likley crash.
         PlayerStatusActor &GetActorAsPlayerStatus()
         {
            return *(static_cast<PlayerStatusActor*>(GetDrawable()));
         }


      protected:
         virtual ~PlayerStatusActorProxy();
         void CreateDrawable();
         virtual void OnEnteredWorld();
   };

}

#endif
//#endif
