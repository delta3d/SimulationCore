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
#ifndef STEALTH_INPUT_COMPONENT_H_
#define STEALTH_INPUT_COMPONENT_H_

#include <SimCore/Components/BaseInputComponent.h>
#include <SimCore/StealthMotionModel.h>
#include <StealthViewer/GMApp/StealthHUD.h>
#include <StealthViewer/GMApp/Export.h>

namespace dtGame
{
   class Message;
   class LogKeyframe;
}

namespace dtHLAGM
{
   class HLAComponent;
}

namespace SimCore
{
   class MessageType;
   
   namespace Tools
   {
      class Tool;
   }
}

namespace StealthGM
{
   class STEALTH_GAME_EXPORT StealthInputComponent : public SimCore::Components::BaseInputComponent
   {
      public:

         static const std::string DEFAULT_NAME;

         /// Constructor
         StealthInputComponent(bool enableLogging  = false, 
                               bool enablePlayback = false, 
                               const std::string& name = DEFAULT_NAME, 
                               bool hasUI = false);

         void ProcessMessage(const dtGame::Message &message);

         virtual bool HandleKeyPressed(const dtCore::Keyboard* keyboard, int key);

         dtDAL::ActorProxy* GetTerrainActor();

         void SetConnectionParameters(const std::string& executionName,
            const std::string &fedFile,
            const std::string &federateName, 
            const std::string &ridFile = "RTI.rid");
   
         SimCore::Tools::Tool* GetTool(SimCore::MessageType &type);
         void AddTool(SimCore::Tools::Tool &tool, SimCore::MessageType &type);
         void RemoveTool(SimCore::MessageType &type);
         bool IsToolEnabled(SimCore::MessageType &type) const;
         void UpdateTools( float timeDelta );
         void ToggleTool(SimCore::MessageType& msgType);
         void DisableAllTools();
         SimCore::MessageType& GetEnabledTool() const;
         void SetToolEnabled(SimCore::MessageType& toolType, bool enable);

         bool IsConnected() const;
   
         bool IsInPlayback() const;
         bool IsInRecord() const;
         bool IsIdle() const;
   
         void JoinFederation( bool updateSystem = true );
         void LeaveFederation();

         void EnableIdle();
         void EnableRecord();
         void EnablePlayback();
         void HandlePause();
         void HandleSpeedChange(bool higherSpeed);
         void HandleSpeedChange(float newSpeed);
         void HandleGotoKeyFrame(const std::string &name);
         void HandleGotoKeyFrame(bool nextKeyFrame);
         void HandleSetAutoKeyFrameInterval(double mins);
         void HandleAddKeyFrame(const dtGame::LogKeyframe &kf);
         void HandleGetKeyFrames();

         void ChangeFlyMotionModelSpeed(bool higher);

         void SetLogFileName(const std::string &fileName);

         void ChangeMotionModels(bool firstPerson);

         void EnableCameraCollision(bool enable);

         virtual void OnAddedToGM();

         void GotoFirstKeyframe();
         void GotoPreviousKeyframe();
         void GotoNextKeyframe();

         void SetReconnectOnIdle( bool reconnect ) { mReconnectOnIdle = reconnect; }
         bool GetReconnectOnIdle() const { return mReconnectOnIdle; }
   
      protected:

         /// Destructor
         virtual ~StealthInputComponent();

         void Cycle(bool forward, bool attach);
         void ToggleEntityShaders();
         void ChangeAARState( const dtGame::LogStateEnumeration& targetState );
         
      private:
         void HandleHelpPressed();
         StealthHUD* GetHUDComponent();
   
         dtCore::RefPtr<SimCore::StealthMotionModel> mStealthMM;
      
         dtCore::RefPtr<StealthHUD>    mHUDComponent;
         dtCore::UniqueId              mCurrentActorId;
         unsigned int                  mCycleIndex;
         
         // logging methods and vars
         void SetupLogging();
         bool mEnableLogging;
         bool mEnablePlayback;
         bool mWasConnected;
         bool mReconnectOnIdle;
         int mTicksToLogStateChange;
         const dtGame::LogStateEnumeration* mTargetLogState;
         dtCore::RefPtr<dtGame::LogController> mLogController;
         
         // JSAF vars needed for joining and leaving a federation
         std::string mExecutionName;
         std::string mFedFile;
         std::string mFederateName;
         std::string mRidFile;
         dtCore::RefPtr<dtHLAGM::HLAComponent> mHLA;

         std::map<SimCore::MessageType*, dtCore::RefPtr<SimCore::Tools::Tool> > mToolList;
         bool mFirstPersonAttachMode;
         bool mHasUI;
         bool mCollideWithGround;

         // Ensures stealth actor persists between map changes.
         dtCore::RefPtr<SimCore::Actors::StealthActorProxy> mStealthActorProxy;
   };
}
#endif
