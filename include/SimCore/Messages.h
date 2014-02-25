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
#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include <dtGame/message.h>
#include <dtGame/messagemacros.h>
#include <SimCore/Export.h>

namespace SimCore
{
   namespace Actors
   {
      class DetonationMunitionType;
   }

   /**
    * Abstract base class for weapon events, weapon fire or detonation
    */
   DT_DECLARE_MESSAGE_BEGIN(BaseWeaponEventMessage, dtGame::Message, SIMCORE_EXPORT)
      DECLARE_PARAMETER_INLINE(unsigned short, EventIdentifier)
      /// the new munition Type name - mapped to an enum in DetonationActor.
      DECLARE_PARAMETER_INLINE(std::string, MunitionType)
      /// Sets the number of shots this message represents.
      DECLARE_PARAMETER_INLINE(unsigned short, QuantityFired)
      /// Rounds per second
      DECLARE_PARAMETER_INLINE(unsigned short, RateOfFire)
      /// Set the raw fuse type code
      DECLARE_PARAMETER_INLINE(unsigned short, FuseType)
      /// Set the raw warhead type code
      DECLARE_PARAMETER_INLINE(unsigned short, WarheadType)
   DT_DECLARE_MESSAGE_END()

   DT_DECLARE_MESSAGE_BEGIN(DetonationMessage,BaseWeaponEventMessage, SIMCORE_EXPORT)
      ///Sets the detonation location of the message parameter
      DECLARE_PARAMETER_INLINE(osg::Vec3, DetonationLocation)
      /// Sets the detonation result code message parameter
      DECLARE_PARAMETER_INLINE(unsigned char, DetonationResultCode)
      /// Sets the final velocity vector
      DECLARE_PARAMETER_INLINE(osg::Vec3, FinalVelocityVector)
      /** Sets the location of the detonation relative to the target, in world units.
       The vector is from the target to the point of detonation. */
      DECLARE_PARAMETER_INLINE(osg::Vec3, RelativeDetonationLocation)
   DT_DECLARE_MESSAGE_END()

   /**
    * @class ShotFiredMessage
    * Sent when a weapon is fired.
    */
   DT_DECLARE_MESSAGE_BEGIN(ShotFiredMessage, BaseWeaponEventMessage, SIMCORE_EXPORT)
      ///the location of the where a shot is fired
      DECLARE_PARAMETER_INLINE(osg::Vec3, FiringLocation)
      /// Sets the velocity of the shot at the point it left the weapon.
      DECLARE_PARAMETER_INLINE(osg::Vec3, InitialVelocityVector)
   DT_DECLARE_MESSAGE_END()


   DT_DECLARE_MESSAGE_BEGIN(ToolMessage, dtGame::Message, SIMCORE_EXPORT)
      /// Enables the tool
      DECLARE_PARAMETER_INLINE(bool, Enabled)
   DT_DECLARE_MESSAGE_END()

   /**
    * @class AttachToActorMessage
    * @brief message class used when attaching to another actor.
    * This is intended to be sent to the player (using the aboutActorId) telling it
    * an actor to attach to.  The recipient will have to know how to perform the action.
    */
   class SIMCORE_EXPORT AttachToActorMessage : public dtGame::Message
   {
   public:

      /// Constructor
      AttachToActorMessage();

      /**
       * Sets Which actor to attach to.
       * @param actorId the actor to attach to.
       */
      void SetAttachToActor(const dtCore::UniqueId& actorId);

      /**
       * @return the actor id the recipient should attach to.
       */
      const dtCore::UniqueId& GetAttachToActor() const;

      /// Sets the node name to use as an attach point.  Empty means attach to the entity as a whole.
      void SetAttachPointNodeName(const std::string& name);

      /// @return the node name to use as an attach point.  Empty means attach to the entity as a whole.
      const std::string& GetAttachPointNodeName() const;

      /// Sets the initial rotation offset in reference to the entity to which to attach.
      void SetInitialAttachRotationHPR(const osg::Vec3& hpr) const;
      /// @return the initial rotation offset in reference to the entity to which to attach.
      const osg::Vec3& GetInitialAttachRotationHPR() const;
   protected:
      /// Destructor
      virtual ~AttachToActorMessage();
      dtCore::NamedActorParameter& mAttachToActorParam;
      dtCore::NamedStringParameter& mAttachPointNodeNameParam;
      dtCore::NamedVec3fParameter& mInitialAttachRotationHPRParam;
   };

   /**
    * @class StealthActorUpdatedMessage
    * @brief This is a message designed to work around the strange way that HLA updates Stealth views.
    */
   class SIMCORE_EXPORT StealthActorUpdatedMessage : public dtGame::Message
   {
      public:

         ///The name parameter name.
         static const std::string NAME;
         ///Translation property name.
         static const std::string TRANSLATION;
         ///Rotation property name.
         static const std::string ROTATION;
         ///Horizontal Field of View property name.
         static const std::string HORIZONTAL_FOV;
         ///Vertical Field of View property name.
         static const std::string VERTICAL_FOV;

         /// Constructor
         StealthActorUpdatedMessage();

         ///@return the name of the stealth actor.
         const std::string& GetName() const;
         ///@param newName the new name of the stealth actor.
         void SetName(const std::string& newName);

         ///@return the translation of a stealth view.
         const osg::Vec3& GetTranslation() const;

         /// Sets the translation of a shot being fired.
         void SetTranslation(const osg::Vec3& newTranslation);

         ///@return the rotation of the stealth view.
         const osg::Vec3& GetRotation() const;

         /// Sets the rotation of the stealth view.
         void SetRotation(const osg::Vec3& newRotation);

         /// Sets the horizontal field of view.
         void SetHorizontalFOV(const float hFOV);

         ///@return the horizontal field of view of the stealth view.
         float GetHorizontalFOV() const;

         /// Sets the vertical field of view.
         void SetVerticalFOV(const float vFOV);

         ///@return the Vertical field of view of the stealth view.
         float GetVerticalFOV() const;
      protected:

         /// Destructor
         virtual ~StealthActorUpdatedMessage();
   };

   class SIMCORE_EXPORT TimeQueryMessage : public dtGame::Message
   {
      public:

         /// Defines the string name for the time the query the sent.
         static const std::string QUERY_TRANSMIT_REAL_TIME;
         /// The name of the sender.
         static const std::string SENDER_NAME;

         /// Constructor
         TimeQueryMessage();

         const std::string& GetSenderName() const;
         void SetSenderName(const std::string& newName);

         /// @return the real time the query was sent.
         unsigned int GetQueryTransmitRealTime() const;
         /// Sets the real time the query was sent.
         void SetQueryTransmitRealTime(unsigned int newTime);

      protected:

         /// Destructor
         virtual ~TimeQueryMessage() {}

      private:
         dtCore::NamedStringParameter* mSenderName;
         dtCore::NamedUnsignedIntParameter* mQueryTransmitRealTime;
   };

   class SIMCORE_EXPORT TimeValueMessage : public TimeQueryMessage
   {
      public:
         // We need a default scale so that we don't override the scale time in playback
         // when a time message wasn't sent with a value in the first place.
         // This is usually set to something like -1.0 or 999999.0
         static const float DEFAULT_TIME_SCALE;

         /// The parameter name of the real time the query was received by the time master.
         static const std::string QUERY_RECEIVED_REAL_TIME;
         /// The parameter name of the real time the time value message was sent by the time master.
         static const std::string VALUE_TRANSMIT_REAL_TIME;
         /// The parameter name of the synchronized clock value at the time the message was sent.
         static const std::string SYNCHRONIZED_TIME;
         /// The parameter name of the time scale base on real time (Time rate multiplier)
         static const std::string TIME_SCALE;
         /// The parameter name of the paused flag.
         static const std::string PAUSED;
         /// the parameter name of the time master actor.
         static const std::string TIME_MASTER;

         /// Constructor
         TimeValueMessage();

         /// @return the real time the server received the time query.
         unsigned int GetQueryReceivedRealTime() const;
         /// Set the real time the query was received by the server.
         void SetQueryReceivedRealTime(unsigned int newTime);

         /// @return the real time the server sent this message.
         unsigned int GetValueTransmitRealTime() const;
         /// Sets the time the server sent this message.
         void SetValueTransmitRealTime(unsigned int newTime);

         /// @return the synchronized time value when the server sent the message.
         unsigned int GetSynchronizedTime() const;
         /// Sets the synchronized time value when the server sent the message.
         void SetSynchronizedTime(unsigned int newTime);

         /// @return the time scale factor (Multiple of real time)
         float GetTimeScale() const;
         /// Set the time scale factor (Multiple of real time)
         void SetTimeScale(float newTimeScale);

         /// @return true if the simulation is paused
         bool IsPaused() const;

         /// Sets if the simulation is paused
         void SetPaused(bool pause);

         /// @return the id of the time master.
         const std::string& GetTimeMaster() const;
         /// Sets the id of the time master.
         void SetTimeMaster(const std::string& newId);

      protected:

         /// Destructor
         virtual ~TimeValueMessage() {}

      private:
         dtCore::NamedUnsignedIntParameter* mQueryReceivedRealTime;
         dtCore::NamedUnsignedIntParameter* mValueTransmitRealTime;
         dtCore::NamedUnsignedIntParameter* mSynchronizedTime;
         dtCore::NamedFloatParameter*           mTimeScale;
         dtCore::NamedBooleanParameter*         mPaused;

         dtCore::NamedStringParameter*          mTimeMaster;
   };


   class SIMCORE_EXPORT MagnificationMessage : public dtGame::Message
   {
      public:

         /// Constructor
         MagnificationMessage();

         /// Sets the magnification factor
         void SetMagnification(float magnification);

         /// Gets the magnification factor
         float GetMagnification() const;

      protected:

         virtual ~MagnificationMessage();
   };


   class SIMCORE_EXPORT ControlStateMessage : public dtGame::Message
   {
      public:
         // NOTE: About Id will point to the vehicle.
         static const std::string PARAM_CONTROL_STATE_ID;
         static const std::string PARAM_STATION;

         ControlStateMessage();

         void SetControlStateID( const std::string& controlStateID );
         const std::string& GetControlStateID() const;

         void SetStation( int station );
         int GetStation() const;

      protected:
         virtual ~ControlStateMessage();
   };


   /**
    * Message that transmits a block of binary data.
    * It could be an embedded message or a radio transmission.
    */
   class SIMCORE_EXPORT EmbeddedDataMessage : public dtGame::Message
   {
      public:
         static const std::string PARAM_ENCODING_SCHEME;
         static const std::string PARAM_DATA_SIZE;
         static const std::string PARAM_DATA;

         /// Constructor
         EmbeddedDataMessage();

         void SetEncodingScheme(unsigned short scheme);
         unsigned short GetEncodingScheme() const;

         void SetDataSize(unsigned short dataSize);
         unsigned short GetDataSize() const;

         void SetData(const std::string& dataBuffer);
         void GetData(std::string& dataBufferToFill) const;

      protected:
         /// Destructor
         virtual ~EmbeddedDataMessage();

      private:
         dtCore::NamedUnsignedShortIntParameter* mEncoding;
         dtCore::NamedUnsignedShortIntParameter* mDataSize;
         dtCore::NamedStringParameter* mDataParameter;
   };
}
#endif
