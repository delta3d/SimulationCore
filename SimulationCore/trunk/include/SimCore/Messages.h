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
#include <SimCore/Export.h>

namespace dtDAL
{
   class NamedVec3Parameter;
   class NamedStringParameter;
   class NamedActorParameter;
}

namespace SimCore
{
   namespace Actors
   {
      class DetonationMunitionType;
   }

   class SIMCORE_EXPORT DetonationMessage : public dtGame::Message
   {
      public:

         /// Constructor
         DetonationMessage();

         /**
          * Sets the detonation location of the message parameter
          * @param loc The new location
          */
         void SetDetonationLocation(const osg::Vec3 &loc);

         /**
          * Get the detonation location of the message parameter
          * @return The location
          */
         osg::Vec3 GetDetonationLocation() const;

         /**
          * Sets the detonation result code message parameter
          * @param code The new code
          */
         void SetDetonationResultCode(unsigned char code);

         /**
          * Gets the detonation result code
          * @return The code
          */
         unsigned char GetDetonationResultCode() const;

         /**
          * Sets the final velocity vector
          * @param vec The new vector
          */
         void SetFinalVelocityVector(const osg::Vec3 &vec);

         /**
          * Gets the final velocity vector
          * @return The velocity vector
          */
         osg::Vec3 GetFinalVelocityVector() const;

         /**
          * Sets the location of the detonation relative to the target, in world units.
          * The vector is from the target to the point of detonation.
          * @param vec The relative detonation offset from the target.
          */
         void SetRelativeDetonationLocation(const osg::Vec3 &vec);

         /**
          * Gets the location of the detonation relative to the target, in world units.
          * The vector is from the target to the point of detonation.
          * @return The relative detonation offset from the target.
          */
         osg::Vec3 GetRelativeDetonationLocation() const;

         /**
          * Sets the event identifier
          * @param eventID The event identifier
          */
         void SetEventIdentifier(unsigned short eventID);

         /**
          * Gets the event identifier
          * @return The event identifier
          */
         unsigned short GetEventIdentifier() const;

         /**
          * Gets the new munition Type name - mapped to an enum in DetonationActor.
          * @return A string containing the munition type.
          */
         const std::string GetMunitionType() const;

         /**
          * Sets the new munition Type name - mapped to an enum in DetonationActor.
          * @param munitionType The name of the munition type enum in DetonationActor.
          */
         void SetMunitionType( const std::string& detType );

         /**
          * Sets the number of shots this message represents.
          * Quantity would be greater than 1 for cluster bombs.
          */
         void SetQuantityFired( unsigned short quantity );
         unsigned short GetQuantityFired() const;

         void SetRateOfFire( unsigned short rate );
         unsigned short GetRateOfFire() const;

         /**
          * Set the raw warhead type code
          */
         void SetWarheadType( unsigned short warhead );
         unsigned short GetWarheadType() const;

         /**
          * Set the raw fuse type code
          */
         void SetFuseType( unsigned short fuse );
         unsigned short GetFuseType() const;

      protected:
         /// Destructor
         virtual ~DetonationMessage();

      private:

   };

   class SIMCORE_EXPORT ToolMessage : public dtGame::Message
   {
      public:

         /// Constructor
         ToolMessage();

         /**
          * Enables the tool
          * @param enable True to enable, false to disable
          */
         void SetEnabled(bool enable);

         /**
          * Returns true if the tool is enabled
          */
         bool IsEnabled() const;

      protected:
         /// Destructor
         virtual ~ToolMessage();
   };


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
         dtDAL::NamedActorParameter& mAttachToActorParam;
         dtDAL::NamedStringParameter& mAttachPointNodeNameParam;
         dtDAL::NamedVec3Parameter& mInitialAttachRotationHPRParam;
   };

   /**
    * @class FireMessage
    * @brief This class will map to a fire message from JSAF
    */
   class SIMCORE_EXPORT ShotFiredMessage : public dtGame::Message
   {
      public:

         /// Constructor
         ShotFiredMessage();

         // Sets the event identifier
         // @param eventID The event identifier
         void SetEventIdentifier(unsigned short eventID);
         unsigned short GetEventIdentifier() const;

         // Returns the location of the where a shot is fired
         osg::Vec3 GetFiringLocation() const;

         // Sets the location of a shot being fired
         void SetFiringLocation(const osg::Vec3 &location);

         // Gets the new munition Type name - mapped to an enum in DetonationActor.
         // @return A string containing the munition type.
         const std::string GetMunitionType() const;

         // Sets the new munition Type name - mapped to an enum in DetonationActor.
         // @param munitionType The name of the munition type enum in DetonationActor.
         void SetMunitionType( const std::string& detType );

         // Sets the number of shots this message represents.
         void SetQuantityFired( unsigned short quantity );
         unsigned short GetQuantityFired() const;

         void SetRateOfFire( unsigned short rate );
         unsigned short GetRateOfFire() const;

         // Sets the velocity of the shot at the point it left the weapon.
         void SetInitialVelocityVector( const osg::Vec3& velocity );
         osg::Vec3 GetInitialVelocityVector() const;

         // Set the raw warhead type code
         void SetWarheadType( unsigned short warhead );
         unsigned short GetWarheadType() const;

         // Set the raw fuse type code
         void SetFuseType( unsigned short fuse );
         unsigned short GetFuseType() const;

      protected:

         /// Destructor
         virtual ~ShotFiredMessage();
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
         osg::Vec3 GetTranslation() const;

         /// Sets the translation of a shot being fired.
         void SetTranslation(const osg::Vec3 &newTranslation);

         ///@return the rotation of the stealth view.
         osg::Vec3 GetRotation() const;

         /// Sets the rotation of the stealth view.
         void SetRotation(const osg::Vec3 &newRotation);

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
         unsigned long GetQueryTransmitRealTime() const;
         /// Sets the real time the query was sent.
         void SetQueryTransmitRealTime(unsigned long newTime);

      protected:

         /// Destructor
         virtual ~TimeQueryMessage() {}

      private:
         dtGame::StringMessageParameter* mSenderName;
         dtGame::UnsignedLongIntMessageParameter* mQueryTransmitRealTime;
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
         unsigned long GetQueryReceivedRealTime() const;
         /// Set the real time the query was received by the server.
         void SetQueryReceivedRealTime(unsigned long newTime);

         /// @return the real time the server sent this message.
         unsigned long GetValueTransmitRealTime() const;
         /// Sets the time the server sent this message.
         void SetValueTransmitRealTime(unsigned long newTime);

         /// @return the synchronized time value when the server sent the message.
         unsigned long GetSynchronizedTime() const;
         /// Sets the synchronized time value when the server sent the message.
         void SetSynchronizedTime(unsigned long newTime);

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
         dtGame::UnsignedLongIntMessageParameter* mQueryReceivedRealTime;
         dtGame::UnsignedLongIntMessageParameter* mValueTransmitRealTime;
         dtGame::UnsignedLongIntMessageParameter* mSynchronizedTime;
         dtGame::FloatMessageParameter*           mTimeScale;
         dtGame::BooleanMessageParameter*         mPaused;

         dtGame::StringMessageParameter*          mTimeMaster;
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
         dtGame::UnsignedShortIntMessageParameter* mEncoding;
         dtGame::UnsignedShortIntMessageParameter* mDataSize;
         dtGame::StringMessageParameter* mDataParameter;
   };
}
#endif
