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

#include <SimCore/Messages.h>

#include <dtCore/scene.h>

#include <dtGame/messageparameter.h>

#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

namespace SimCore
{
   DT_IMPLEMENT_MESSAGE_BEGIN(BaseWeaponEventMessage)
      DT_ADD_PARAMETER(unsigned short, EventIdentifier)
      DT_ADD_PARAMETER(std::string, MunitionType)
      DT_ADD_PARAMETER(unsigned short, QuantityFired)
      DT_ADD_PARAMETER(unsigned short, RateOfFire)
      DT_ADD_PARAMETER(unsigned short, FuseType)
      DT_ADD_PARAMETER(unsigned short, WarheadType)
   DT_IMPLEMENT_MESSAGE_END()

   DT_IMPLEMENT_MESSAGE_BEGIN(DetonationMessage)
      ///Sets the detonation location of the message parameter
      DT_ADD_PARAMETER(osg::Vec3, DetonationLocation)
      /// Sets the detonation result code message parameter
      DT_ADD_PARAMETER(unsigned char, DetonationResultCode)
      /// Sets the final velocity vector
      DT_ADD_PARAMETER(osg::Vec3, FinalVelocityVector)
      /** Sets the location of the detonation relative to the target, in world units.
       The vector is from the target to the point of detonation. */
      DT_ADD_PARAMETER(osg::Vec3, RelativeDetonationLocation)
   DT_IMPLEMENT_MESSAGE_END()

   /**
    * @class ShotFiredMessage
    * Sent when a weapon is fired.
    */
   DT_IMPLEMENT_MESSAGE_BEGIN(ShotFiredMessage)
      ///the location of the where a shot is fired
      DT_ADD_PARAMETER(osg::Vec3, FiringLocation)
      /// Sets the velocity of the shot at the point it left the weapon.
      DT_ADD_PARAMETER(osg::Vec3, InitialVelocityVector)
   DT_IMPLEMENT_MESSAGE_END()


   DT_IMPLEMENT_MESSAGE_BEGIN(ToolMessage)
      /// Enables the tool
      DT_ADD_PARAMETER(osg::Vec3, Enabled)
   DT_IMPLEMENT_MESSAGE_END()

   /////////////////////////////////////////////////////////////////////////////////////////////
   /////////////////////////////////////////////////////////////////////////////////////////////
   AttachToActorMessage::AttachToActorMessage()
   : mAttachToActorParam(*new dtGame::ActorMessageParameter("AttachToActor"))
   , mAttachPointNodeNameParam(*new dtGame::StringMessageParameter("Attach Point Node Name"))
   , mInitialAttachRotationHPRParam(*new dtGame::Vec3fMessageParameter("Initial Attach Rotation HPR"))
   {
      AddParameter(&mAttachToActorParam);
      AddParameter(&mAttachPointNodeNameParam);
      AddParameter(&mInitialAttachRotationHPRParam);
   }

   void AttachToActorMessage::SetAttachToActor(const dtCore::UniqueId& actorId)
   {
      mAttachToActorParam.SetValue(actorId);
   }

   const dtCore::UniqueId& AttachToActorMessage::GetAttachToActor() const
   {
      return mAttachToActorParam.GetValue();
   }

   void AttachToActorMessage::SetAttachPointNodeName(const std::string& name)
   {
      mAttachPointNodeNameParam.SetValue(name);
   }

   const std::string& AttachToActorMessage::GetAttachPointNodeName() const
   {
      return mAttachPointNodeNameParam.GetValue();
   }

   void AttachToActorMessage::SetInitialAttachRotationHPR(const osg::Vec3& hpr) const
   {
      mInitialAttachRotationHPRParam.SetValue(hpr);
   }

   const osg::Vec3& AttachToActorMessage::GetInitialAttachRotationHPR() const
   {
      return mInitialAttachRotationHPRParam.GetValue();
   }

   AttachToActorMessage::~AttachToActorMessage()
   {
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

   const osg::Vec3& StealthActorUpdatedMessage::GetTranslation() const
   {
      return static_cast<const dtGame::Vec3MessageParameter*>(GetParameter(TRANSLATION))->GetValue();
   }

   void StealthActorUpdatedMessage::SetRotation(const osg::Vec3 &newRotation)
   {
      static_cast<dtGame::Vec3MessageParameter*>(GetParameter(ROTATION))->SetValue(newRotation);
   }

   const osg::Vec3& StealthActorUpdatedMessage::GetRotation() const
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
   ///////////////  Control State Message  /////////////////////////
   /////////////////////////////////////////////////////////////////
   const std::string ControlStateMessage::PARAM_CONTROL_STATE_ID("ControlStateID");
   const std::string ControlStateMessage::PARAM_STATION("Station");

   /////////////////////////////////////////////////////////////////
   ControlStateMessage::ControlStateMessage()
   {
      AddParameter(new dtGame::StringMessageParameter(PARAM_CONTROL_STATE_ID));
      AddParameter(new dtGame::IntMessageParameter(PARAM_STATION));
   }

   /////////////////////////////////////////////////////////////////
   ControlStateMessage::~ControlStateMessage()
   {
   }

   /////////////////////////////////////////////////////////////////
   void ControlStateMessage::SetControlStateID( const std::string& controlStateID )
   {
      static_cast<dtGame::StringMessageParameter*>
         (GetParameter(PARAM_CONTROL_STATE_ID))->FromString(controlStateID);
   }

   /////////////////////////////////////////////////////////////////
   const std::string& ControlStateMessage::GetControlStateID() const
   {
      return static_cast<const dtGame::StringMessageParameter*>
         (GetParameter(PARAM_CONTROL_STATE_ID))->GetValue();
   }

   /////////////////////////////////////////////////////////////////
   void ControlStateMessage::SetStation( int station )
   {
      static_cast<dtGame::IntMessageParameter*>
         (GetParameter(PARAM_STATION))->SetValue(station);
   }

   /////////////////////////////////////////////////////////////////
   int ControlStateMessage::GetStation() const
   {
      return static_cast<const dtGame::IntMessageParameter*>
         (GetParameter(PARAM_STATION))->GetValue();
   }

   ////////////////////////////////////////////////////////
   /////////////////  Binary Data Message  ///////////////
   ////////////////////////////////////////////////////////
   const std::string EmbeddedDataMessage::PARAM_ENCODING_SCHEME("Encoding Scheme");
   const std::string EmbeddedDataMessage::PARAM_DATA_SIZE("Data Size");
   const std::string EmbeddedDataMessage::PARAM_DATA("Data");
   ////////////////////////////////////////////////////////
   EmbeddedDataMessage::EmbeddedDataMessage()
   {
      mEncoding = new dtGame::UnsignedShortIntMessageParameter(PARAM_ENCODING_SCHEME);
      AddParameter(mEncoding);
      mDataSize = new dtGame::UnsignedShortIntMessageParameter(PARAM_DATA_SIZE);
      AddParameter(mDataSize);
      mDataParameter = new dtGame::StringMessageParameter(PARAM_DATA);
      AddParameter(mDataParameter);
   }

   ////////////////////////////////////////////////////////
   EmbeddedDataMessage::~EmbeddedDataMessage()
   {
   }

   void EmbeddedDataMessage::SetEncodingScheme(unsigned short scheme)
   {
      mEncoding->SetValue(scheme);
   }

   unsigned short EmbeddedDataMessage::GetEncodingScheme() const
   {
      return mEncoding->GetValue();
   }

   ////////////////////////////////////////////////////////
   void EmbeddedDataMessage::SetDataSize(unsigned short dataSize)
   {
      mDataSize->SetValue(dataSize);
   }

   ////////////////////////////////////////////////////////
   unsigned short EmbeddedDataMessage::GetDataSize() const
   {
      return mDataSize->GetValue();
   }

   ////////////////////////////////////////////////////////
   void EmbeddedDataMessage::SetData(const std::string& dataBuffer)
   {
      mDataParameter->SetValue(dataBuffer);
   }

   ////////////////////////////////////////////////////////
   void EmbeddedDataMessage::GetData(std::string& dataBufferToFill) const
   {
      dataBufferToFill = mDataParameter->GetValue();
   }

}
