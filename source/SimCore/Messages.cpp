/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Messages.h>

#include <dtCore/scene.h>

#include <dtGame/messageparameter.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

namespace SimCore
{
   DetonationMessage::DetonationMessage() : Message()
   {
      AddParameter(new dtGame::Vec3MessageParameter("DetonationLocation"));
      AddParameter(new dtGame::UnsignedCharMessageParameter("DetonationResultCode"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("EventIdentifier"));
      AddParameter(new dtGame::Vec3MessageParameter("FinalVelocityVector"));
      AddParameter(new dtGame::Vec3MessageParameter("RelativeDetonationLocation"));
      AddParameter(new dtGame::StringMessageParameter("MunitionType"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("QuantityFired"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("RateOfFire"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("FuseType"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("WarheadType"));
   }

   DetonationMessage::~DetonationMessage()
   {

   }

   void DetonationMessage::SetDetonationLocation(const osg::Vec3 &loc)
   {
   	 static_cast<dtGame::Vec3MessageParameter*>(GetParameter("DetonationLocation"))->SetValue(loc);
   }

   osg::Vec3 DetonationMessage::GetDetonationLocation() const
   {
   	 return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter("DetonationLocation"))->GetValue();
   }

   void DetonationMessage::SetDetonationResultCode(unsigned char code)
   {
   	 static_cast<dtGame::UnsignedCharMessageParameter*>(GetParameter("DetonationResultCode"))->SetValue(code);
   }

   unsigned char DetonationMessage::GetDetonationResultCode() const
   {
   	 return static_cast<const dtGame::UnsignedCharMessageParameter*>(GetParameter("DetonationResultCode"))->GetValue();
   }

   void DetonationMessage::SetFinalVelocityVector(const osg::Vec3 &vec)
   {
   	 static_cast<dtGame::Vec3MessageParameter*>(GetParameter("FinalVelocityVector"))->SetValue(vec);
   }

   osg::Vec3 DetonationMessage::GetFinalVelocityVector() const
   {
   	 return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter("FinalVelocityVector"))->GetValue();
   }

   void DetonationMessage::SetRelativeDetonationLocation(const osg::Vec3 &vec)
   {
      static_cast<dtGame::Vec3MessageParameter*>(GetParameter("RelativeDetonationLocation"))->SetValue(vec);
   }

   osg::Vec3 DetonationMessage::GetRelativeDetonationLocation() const
   {
      return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter("RelativeDetonationLocation"))->GetValue();
   }

   void DetonationMessage::SetEventIdentifier(unsigned short eventID)
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>(GetParameter("EventIdentifier"))->SetValue(eventID);
   }

   unsigned short DetonationMessage::GetEventIdentifier() const
   {
   	 return static_cast<const dtGame::UnsignedShortIntMessageParameter*>(GetParameter("EventIdentifier"))->GetValue();
   }

   const std::string DetonationMessage::GetMunitionType() const
   {
      const dtGame::StringMessageParameter *p =
         static_cast<const dtGame::StringMessageParameter*>(GetParameter("MunitionType"));

      return p->ToString();
   }

   void DetonationMessage::SetMunitionType( const std::string& detType )
   {
      dtGame::StringMessageParameter *p =
         static_cast<dtGame::StringMessageParameter*>(GetParameter("MunitionType"));
      p->FromString( detType );
   }

   void DetonationMessage::SetQuantityFired( unsigned short quantity )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("QuantityFired"))->SetValue(quantity);
   }

   unsigned short DetonationMessage::GetQuantityFired() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("QuantityFired"))->GetValue();
   }

   void DetonationMessage::SetRateOfFire( unsigned short rate )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("RateOfFire"))->SetValue(rate);
   }

   unsigned short DetonationMessage::GetRateOfFire() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("RateOfFire"))->GetValue();
   }

   void DetonationMessage::SetWarheadType( unsigned short warhead )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("WarheadType"))->SetValue(warhead);
   }

   unsigned short DetonationMessage::GetWarheadType() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("WarheadType"))->GetValue();
   }

   void DetonationMessage::SetFuseType( unsigned short fuse )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("FuseType"))->SetValue(fuse);
   }

   unsigned short DetonationMessage::GetFuseType() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("FuseType"))->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////////////////////
   ToolMessage::ToolMessage() : Message()
   {
      AddParameter(new dtGame::BooleanMessageParameter("Enable", false));
   }

   ToolMessage::~ToolMessage()
   {

   }

   void ToolMessage::SetEnabled(bool enable)
   {
      static_cast<dtGame::BooleanMessageParameter*>(GetParameter("Enable"))->SetValue(enable);
   }

   bool ToolMessage::IsEnabled() const
   {
      return static_cast<const dtGame::BooleanMessageParameter*>(GetParameter("Enable"))->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////////////////////
   AttachToActorMessage::AttachToActorMessage()
   {
      AddParameter(new dtGame::ActorMessageParameter("AttachToActor"));
   }

   void AttachToActorMessage::SetAttachToActor(const dtCore::UniqueId& actorId)
   {
      static_cast<dtGame::ActorMessageParameter*>(GetParameter("AttachToActor"))->SetValue(actorId);
   }

   const dtCore::UniqueId& AttachToActorMessage::GetAttachToActor() const
   {
      return static_cast<const dtGame::ActorMessageParameter*>(GetParameter("AttachToActor"))->GetValue();
   }

   AttachToActorMessage::~AttachToActorMessage()
   {
   }

   /////////////////////////////////////////////////////////////////////////////////////////////
   ShotFiredMessage::ShotFiredMessage()
   {
      AddParameter(new dtGame::Vec3MessageParameter("FiringLocation"));
      AddParameter(new dtGame::Vec3MessageParameter("InitialVelocityVector"));
      AddParameter(new dtGame::StringMessageParameter("MunitionType"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("EventIdentifier"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("QuantityFired"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("RateOfFire"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("FuseType"));
      AddParameter(new dtGame::UnsignedShortIntMessageParameter("WarheadType"));
   }

   void ShotFiredMessage::SetEventIdentifier(unsigned short eventID)
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>(GetParameter("EventIdentifier"))->SetValue(eventID);
   }

   unsigned short ShotFiredMessage::GetEventIdentifier() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>(GetParameter("EventIdentifier"))->GetValue();
   }

   osg::Vec3 ShotFiredMessage::GetFiringLocation() const
   {
      return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter("FiringLocation"))->GetValue();
   }

   void ShotFiredMessage::SetFiringLocation(const osg::Vec3 &location)
   {
      static_cast<dtGame::Vec3MessageParameter*>(GetParameter("FiringLocation"))->SetValue(location);
   }

   ShotFiredMessage::~ShotFiredMessage()
   {

   }

   const std::string ShotFiredMessage::GetMunitionType() const
   {
      const dtGame::StringMessageParameter *p =
         static_cast<const dtGame::StringMessageParameter*>(GetParameter("MunitionType"));

      return p->ToString();
   }

   void ShotFiredMessage::SetMunitionType( const std::string& detType )
   {
      dtGame::StringMessageParameter *p =
         static_cast<dtGame::StringMessageParameter*>(GetParameter("MunitionType"));
      p->FromString( detType );
   }

   void ShotFiredMessage::SetQuantityFired( unsigned short quantity )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("QuantityFired"))->SetValue(quantity);
   }

   unsigned short ShotFiredMessage::GetQuantityFired() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("QuantityFired"))->GetValue();
   }

   void ShotFiredMessage::SetRateOfFire( unsigned short rate )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("RateOfFire"))->SetValue(rate);
   }

   unsigned short ShotFiredMessage::GetRateOfFire() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("RateOfFire"))->GetValue();
   }

   void ShotFiredMessage::SetInitialVelocityVector( const osg::Vec3& velocity )
   {
      static_cast<dtGame::Vec3MessageParameter*>
         (GetParameter("InitialVelocityVector"))->SetValue(velocity);
   }

   osg::Vec3 ShotFiredMessage::GetInitialVelocityVector() const
   {
      return static_cast<const dtGame::Vec3MessageParameter*>
         (GetParameter("InitialVelocityVector"))->GetValue();
   }

   void ShotFiredMessage::SetWarheadType( unsigned short warhead )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("WarheadType"))->SetValue(warhead);
   }

   unsigned short ShotFiredMessage::GetWarheadType() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("WarheadType"))->GetValue();
   }

   void ShotFiredMessage::SetFuseType( unsigned short fuse )
   {
      static_cast<dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("FuseType"))->SetValue(fuse);
   }

   unsigned short ShotFiredMessage::GetFuseType() const
   {
      return static_cast<const dtGame::UnsignedShortIntMessageParameter*>
         (GetParameter("FuseType"))->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////////////////////

   ///Name property name.
   const std::string StealthActorUpdatedMessage::NAME("Name");
   ///Translation property name.
   const std::string StealthActorUpdatedMessage::TRANSLATION("Translation");
   ///Rotation property name.
   const std::string StealthActorUpdatedMessage::ROTATION("Rotation");
   ///Horizontal Field of View property name.
   const std::string StealthActorUpdatedMessage::HORIZONTAL_FOV("Horizontal FOV");
   ///Vertical Field of View property name.
   const std::string StealthActorUpdatedMessage::VERTICAL_FOV("Vertical FOV");

   StealthActorUpdatedMessage::StealthActorUpdatedMessage()
   {
      AddParameter(new dtGame::StringMessageParameter(NAME));
      AddParameter(new dtGame::Vec3MessageParameter(TRANSLATION));
      AddParameter(new dtGame::Vec3MessageParameter(ROTATION));
      AddParameter(new dtGame::FloatMessageParameter(HORIZONTAL_FOV));
      AddParameter(new dtGame::FloatMessageParameter(VERTICAL_FOV));
   }

   void StealthActorUpdatedMessage::SetName(const std::string& newName)
   {
      GetParameter(NAME)->FromString(newName);
   }

   const std::string& StealthActorUpdatedMessage::GetName() const
   {
      return static_cast<const dtGame::StringMessageParameter*>(GetParameter(NAME))->GetValue();
   }

   void StealthActorUpdatedMessage::SetTranslation(const osg::Vec3 &newTranslation)
   {
      static_cast<dtGame::Vec3MessageParameter*>(GetParameter(TRANSLATION))->SetValue(newTranslation);
   }

   osg::Vec3 StealthActorUpdatedMessage::GetTranslation() const
   {
      return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter(TRANSLATION))->GetValue();
   }

   void StealthActorUpdatedMessage::SetRotation(const osg::Vec3 &newRotation)
   {
      static_cast<dtGame::Vec3MessageParameter*>(GetParameter(ROTATION))->SetValue(newRotation);
   }

   osg::Vec3 StealthActorUpdatedMessage::GetRotation() const
   {
      return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter(ROTATION))->GetValue();
   }

   void StealthActorUpdatedMessage::SetHorizontalFOV(const float hFOV)
   {
      static_cast<dtGame::FloatMessageParameter*>(GetParameter(HORIZONTAL_FOV))->SetValue(hFOV);
   }

   float StealthActorUpdatedMessage::GetHorizontalFOV() const
   {
      return static_cast<const dtGame::FloatMessageParameter*>(GetParameter(HORIZONTAL_FOV))->GetValue();
   }

   void StealthActorUpdatedMessage::SetVerticalFOV(const float vFOV)
   {
      static_cast<dtGame::FloatMessageParameter*>(GetParameter(VERTICAL_FOV))->SetValue(vFOV);
   }

   float StealthActorUpdatedMessage::GetVerticalFOV() const
   {
      return static_cast<const dtGame::FloatMessageParameter*>(GetParameter(VERTICAL_FOV))->GetValue();
   }

   StealthActorUpdatedMessage::~StealthActorUpdatedMessage()
   {}

   /////////////////////////////////////////////////////////////////
   const std::string TimeQueryMessage::QUERY_TRANSMIT_REAL_TIME("Query Transmit Real Time");
   const std::string TimeQueryMessage::SENDER_NAME("Sender Name");

   TimeQueryMessage::TimeQueryMessage()
   {
      mSenderName = new dtGame::StringMessageParameter(SENDER_NAME);
      AddParameter(mSenderName);
      mQueryTransmitRealTime = new dtGame::UnsignedLongIntMessageParameter(QUERY_TRANSMIT_REAL_TIME);
      AddParameter(mQueryTransmitRealTime);
   }

   const std::string& TimeQueryMessage::GetSenderName() const
   {
      return mSenderName->GetValue();      
   }

   void TimeQueryMessage::SetSenderName(const std::string& newName)
   {
      mSenderName->SetValue(newName);      
   }
   
   unsigned long TimeQueryMessage::GetQueryTransmitRealTime() const
   {
      return mQueryTransmitRealTime->GetValue();
   }
   
   void TimeQueryMessage::SetQueryTransmitRealTime(unsigned long newTime)
   {
      mQueryTransmitRealTime->SetValue(newTime);
   }

   /////////////////////////////////////////////////////////////////
   const float TimeValueMessage::DEFAULT_TIME_SCALE(-1.0f);
   const std::string TimeValueMessage::QUERY_RECEIVED_REAL_TIME("Query Received Real Time");
   const std::string TimeValueMessage::VALUE_TRANSMIT_REAL_TIME("Value Transmit Real Time");
   const std::string TimeValueMessage::SYNCHRONIZED_TIME("Synchronized Time");
   const std::string TimeValueMessage::TIME_SCALE("Time Scale");
   const std::string TimeValueMessage::PAUSED("Paused");
   const std::string TimeValueMessage::TIME_MASTER("Time Master");

   TimeValueMessage::TimeValueMessage()
   {
      mQueryReceivedRealTime = new dtGame::UnsignedLongIntMessageParameter(QUERY_RECEIVED_REAL_TIME);
      AddParameter(mQueryReceivedRealTime);

      mValueTransmitRealTime = new dtGame::UnsignedLongIntMessageParameter(VALUE_TRANSMIT_REAL_TIME);
      AddParameter(mValueTransmitRealTime);

      mSynchronizedTime = new dtGame::UnsignedLongIntMessageParameter(SYNCHRONIZED_TIME);
      AddParameter(mSynchronizedTime);

      mTimeScale = new dtGame::FloatMessageParameter(TIME_SCALE, DEFAULT_TIME_SCALE);
      AddParameter(mTimeScale);

      mPaused = new dtGame::BooleanMessageParameter(PAUSED, false);
      AddParameter(mPaused);

      mTimeMaster = new dtGame::StringMessageParameter(TIME_MASTER);
      AddParameter(mTimeMaster);
   }

   unsigned long TimeValueMessage::GetQueryReceivedRealTime() const
   {
      return mQueryReceivedRealTime->GetValue();
   }
   
   void TimeValueMessage::SetQueryReceivedRealTime(unsigned long newTime)
   {
      mQueryReceivedRealTime->SetValue(newTime);
   }

   unsigned long TimeValueMessage::GetValueTransmitRealTime() const
   {
      return mValueTransmitRealTime->GetValue();
   }
   
   void TimeValueMessage::SetValueTransmitRealTime(unsigned long newTime)
   {
      mValueTransmitRealTime->SetValue(newTime);
   }

   unsigned long TimeValueMessage::GetSynchronizedTime() const
   {
      return mSynchronizedTime->GetValue();
   }
   
   void TimeValueMessage::SetSynchronizedTime(unsigned long newTime)
   {
      mSynchronizedTime->SetValue(newTime);
   }

   float TimeValueMessage::GetTimeScale() const
   {
      return mTimeScale->GetValue();
   }
   
   void TimeValueMessage::SetTimeScale(float newTimeScale)
   {
      mTimeScale->SetValue(newTimeScale);
   }

   bool TimeValueMessage::IsPaused() const
   {
      return mPaused->GetValue(); 
   }

   void TimeValueMessage::SetPaused(bool pause)
   {
      mPaused->SetValue(pause);
   }

   const std::string& TimeValueMessage::GetTimeMaster() const
   {
      return mTimeMaster->GetValue();
   }
   
   void TimeValueMessage::SetTimeMaster(const std::string& newMaster)
   {
      mTimeMaster->SetValue(newMaster);
   }
   
   //////////////////////////////////////////////////////////////////
   MagnificationMessage::MagnificationMessage()
   {
      AddParameter(new dtGame::FloatMessageParameter("Magnification"));
   }

   MagnificationMessage::~MagnificationMessage()
   {
   }

   void MagnificationMessage::SetMagnification(float magnification)
   {
      static_cast<dtGame::FloatMessageParameter&>(*GetParameter("Magnification")).SetValue(magnification);
   }

   float MagnificationMessage::GetMagnification() const
   {
      return static_cast<const dtGame::FloatMessageParameter&>(*GetParameter("Magnification")).GetValue();
   }

   /////////////////////////////////////////////////////////////////
   const std::string ControlStateMessage::PARAM_CONTROL_STATE_ID("ControlStateID");
   const std::string ControlStateMessage::PARAM_STATION("Station");

   ControlStateMessage::ControlStateMessage()
   {
      AddParameter(new dtGame::StringMessageParameter(PARAM_CONTROL_STATE_ID));
      AddParameter(new dtGame::IntMessageParameter(PARAM_STATION));
   }

   ControlStateMessage::~ControlStateMessage()
   {
   }

   void ControlStateMessage::SetControlStateID( const std::string& controlStateID )
   {
      static_cast<dtGame::StringMessageParameter*>
         (GetParameter(PARAM_CONTROL_STATE_ID))->FromString(controlStateID);
   }
   
   const std::string ControlStateMessage::GetControlStateID() const
   {
      return static_cast<const dtGame::StringMessageParameter*>
         (GetParameter(PARAM_CONTROL_STATE_ID))->ToString();
   }

   void ControlStateMessage::SetStation( int station )
   {
      static_cast<dtGame::IntMessageParameter*>
         (GetParameter(PARAM_STATION))->SetValue(station);
   }

   int ControlStateMessage::GetStation() const
   {
      return static_cast<const dtGame::IntMessageParameter*>
         (GetParameter(PARAM_STATION))->GetValue();
   }

}
