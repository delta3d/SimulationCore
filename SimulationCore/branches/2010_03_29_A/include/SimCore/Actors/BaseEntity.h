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
*
* @author David Guthrie, Curtiss Murphy
*/
#ifndef BASEENTITY_H_
#define BASEENTITY_H_

#include <SimCore/Export.h>

#include <SimCore/Actors/IGActor.h>

namespace dtGame
{
   class DeadReckoningComponent;
   class DeadReckoningAlgorithm;
   class DeadReckoningHelper;
}

namespace SimCore
{
   class DetonationMessage;

   namespace Actors
   {
      class MunitionTypeActor;


      ///////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BaseEntityActorProxy : public dtGame::GameActorProxy
      {
         public:
            typedef dtGame::GameActorProxy BaseClass;

            static const dtUtil::RefString PROPERTY_LAST_KNOWN_TRANSLATION;
            static const dtUtil::RefString PROPERTY_LAST_KNOWN_ROTATION;
            static const dtUtil::RefString PROPERTY_VELOCITY_VECTOR;
            static const dtUtil::RefString PROPERTY_ACCELERATION_VECTOR;
            static const dtUtil::RefString PROPERTY_ANGULAR_VELOCITY_VECTOR;
            static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_POSITION;
            static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_ON;
            static const dtUtil::RefString PROPERTY_FROZEN;
            static const dtUtil::RefString PROPERTY_FLAMES_PRESENT;
            static const dtUtil::RefString PROPERTY_SMOKE_PLUME_PRESENT;
            static const dtUtil::RefString PROPERTY_ENGINE_POSITION;
            static const dtUtil::RefString PROPERTY_FLYING;
            static const dtUtil::RefString PROPERTY_DAMAGE_STATE;
            static const dtUtil::RefString PROPERTY_MAX_DAMAGE_AMOUNT;
            static const dtUtil::RefString PROPERTY_CUR_DAMAGE_RATIO;
            static const dtUtil::RefString PROPERTY_DEFAULT_SCALE;
            static const dtUtil::RefString PROPERTY_DOMAIN;
            static const dtUtil::RefString PROPERTY_SCALE_MAGNIFICATION_FACTOR;
            static const dtUtil::RefString PROPERTY_MODEL_SCALE;
            static const dtUtil::RefString PROPERTY_MODEL_ROTATION;
            static const dtUtil::RefString PROPERTY_ENTITY_TYPE_ID;
            static const dtUtil::RefString PROPERTY_MAPPING_NAME;
            static const dtUtil::RefString PROPERTY_FORCE;
            static const dtUtil::RefString PROPERTY_GROUND_OFFSET;
            static const dtUtil::RefString PROPERTY_DEAD_RECKONING_ALGORITHM;

            class SIMCORE_EXPORT DomainEnum : public dtUtil::Enumeration
            {
               DECLARE_ENUM(DomainEnum);

               public:
                  static DomainEnum AIR;
                  static DomainEnum AMPHIBIOUS;
                  static DomainEnum GROUND;
                  static DomainEnum SPACE;
                  static DomainEnum SUBMARINE;
                  static DomainEnum SURFACE;
                  static DomainEnum MULTI;

                  const std::string& GetDisplayName();
               private:
                  DomainEnum(const std::string& name, const std::string& displayName);
                  const std::string mDisplayName;
            };

            class SIMCORE_EXPORT DamageStateEnum : public dtUtil::Enumeration
            {
                  DECLARE_ENUM(DamageStateEnum);
               public:
                  static DamageStateEnum NO_DAMAGE;
                  static DamageStateEnum SLIGHT_DAMAGE;
                  static DamageStateEnum MODERATE_DAMAGE;
                  static DamageStateEnum DESTROYED;
               private:
                  DamageStateEnum(const std::string& name);
            };

            class SIMCORE_EXPORT ForceEnum : public dtUtil::Enumeration
            {
                  DECLARE_ENUM(ForceEnum);
               public:
                  static ForceEnum OTHER;
                  static ForceEnum FRIENDLY;
                  static ForceEnum OPPOSING;
                  static ForceEnum NEUTRAL;
                  static ForceEnum INSURGENT;

                  const std::string& GetDisplayName();
               private:
                  ForceEnum(const std::string& name, const std::string displayName);
                  const std::string mDisplayName;
            };

            class SIMCORE_EXPORT ServiceEnum : public dtUtil::Enumeration
            {
                  DECLARE_ENUM(ServiceEnum);
               public:
                  static ServiceEnum OTHER;
                  static ServiceEnum ARMY;
                  static ServiceEnum AIR_FORCE;
                  static ServiceEnum COAST_GUARD;
                  static ServiceEnum MARINES;
                  static ServiceEnum NAVY;
                  static ServiceEnum JOINT;
                  static ServiceEnum CIVILIAN;
                  static ServiceEnum REFUGEE;
               private:
                  ServiceEnum(const std::string& name);
            };

            BaseEntityActorProxy();

            /**
             * All base entity objects will be placeable
             */
            virtual bool IsPlaceable()  { return true; }

            /// Called by tick local when sending a partial update to get a list of the properties to send.
            ///virtual void FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill);
            virtual void GetPartialUpdateProperties(std::vector<dtUtil::RefString>& propNamesToFill);

            /**
             * Build the properties common to all BaseEntity objects
             */
            virtual void BuildPropertyMap();

            /**
             * This is a virtual method on the proxy called from the base actor proxy.
             * It is overridden here to call init dr helper.  If you override this, make sure to call
             * the super version or the actorproxy won't have any properties and create actor won't get called.
             */
            virtual void Init(const dtDAL::ActorType& actorType);

            virtual void OnRemovedFromWorld();

            /// Build the invokables
            virtual void BuildInvokables();

            void SetLastKnownRotation(const osg::Vec3& vec);

            osg::Vec3 GetLastKnownRotation() const;

         protected:
            virtual ~BaseEntityActorProxy();
            virtual void OnEnteredWorld();

            /// Overridden to remove Pos & rotation. See method for explanation.
            virtual void NotifyFullActorUpdate();

         private:
      };

      //////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BaseEntity : public IGActor
      {
         public:

            /**
             * TODO This should be a default and the code should allow changing this value for runtime, and possibly
             *      allow setting the global default in the config xml.
             */
            static const float TIME_BETWEEN_UPDATES;

            BaseEntity(dtGame::GameActorProxy& proxy);

            virtual void OnEnteredWorld();

            virtual void OnRemovedFromWorld();

            /**
             * Sets this entity's DIS/RPR-FOM damage state.
             *
             * @param damageState the damage state
             */
            virtual void SetDamageState(BaseEntityActorProxy::DamageStateEnum& damageState);

            /**
             * Returns this entity's DIS/RPR-FOM damage state.
             *
             * @return the damage state
             */
            BaseEntityActorProxy::DamageStateEnum& GetDamageState() const;

            /**
             * Controls the maximum damage that the munitions component will allow before killing
             * this entity. Only applies to local entities, and only before the munitions component
             * is registered. See MunitionsComponent and DamageHelper for how this is used.
             */
            void SetMaxDamageAmount(float newValue) { mMaxDamageAmount = newValue;  }
            /**
             * Controls the maximum damage that the munitions component will allow before killing
             * this entity. Only applies to local entities, and only before the munitions component
             * is registered. See MunitionsComponent and DamageHelper for how this is used.
             */
            float GetMaxDamageAmount() const { return mMaxDamageAmount; }

            /**
             * The current amount of damage this entity has sustained - between 0.0 (none) and 1.0 (dead).
             * This value is controlled by the munitions component - NEVER set this manually.
             */
            void SetCurDamageRatio(float newValue) { mCurDamageRatio = newValue;  }
            /**
             * The current amount of damage this entity has sustained - between 0.0 (none) and 1.0 (dead).
             * Use this for any HUD or UI displays instead of asking the MunitionsComponent for its DamageHelper.
             */
            float GetCurDamageRatio() const { return mCurDamageRatio; }


            /**
            * Toggles whether a local version of this entity will automatically attempt to register
            * itself with a MunitionsComponent (if one exists).
            * Not currently an Actor Property to avoid publishing this value repeatedly - its for setup only.
            * @param newValue True to enable, false to not. Default is true.
            */
            void SetAutoRegisterWithMunitionsComponent(bool newValue) { mAutoRegisterWithMunitionsComponent = newValue; }

            /**
            * Toggles whether a local version of this entity will automatically attempt to register
            * itself with a MunitionsComponent (if one exists).
            * Not currently an Actor Property to avoid publishing this value repeatedly - its for setup only.
            * @return mAutoRegisterWithMunitionsComponent
            */
            bool IsAutoRegisterWithMunitionsComponent() const { return mAutoRegisterWithMunitionsComponent; }

            /**
             * Set the environment the entity is specialized in navigating.
             * @param domain Type of domain to be set.
             */
            void SetDomain(BaseEntityActorProxy::DomainEnum& domain);

            /**
             * Get the environment the entity is specialized in navigating.
             * @return Type of domain in which this entity is specialized.
             */
            BaseEntityActorProxy::DomainEnum& GetDomain() const;

            /**
             * Sets this entity's minimum Dead Reckoning Algorithm.
             *
             * @param newAlgorithm the new algorithm enum value.
             */
            void SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm& newAlgorithm);

            /**
             * @return the current minimum Dead Reckoning Algorithm.
             */
            dtGame::DeadReckoningAlgorithm& GetDeadReckoningAlgorithm() const;

            /**
             * Sets the force for which this entity is fighting.
             * @param newForceEnum the new enum value to set.
             */
            virtual void SetForceAffiliation(BaseEntityActorProxy::ForceEnum& newForceEnum);

            /**
             * @return the force for which this entity is fighting.
             */
            BaseEntityActorProxy::ForceEnum& GetForceAffiliation() const;

            /**
             * Sets the service of this entity
             * @param service The new service
             */
            virtual void SetService(BaseEntityActorProxy::ServiceEnum& service);

            /**
             * Gets the service of this entity
             * @return mService
             */
            BaseEntityActorProxy::ServiceEnum& GetService() const;

            /**
             * Toggles engine smoke on and off
             * @param enable True to enable, false to not
             */
            virtual void SetEngineSmokeOn(bool enable);

            /**
             * Gets if engine smoke is currently enabled
             * @return mEngineSmokeOn
             */
            bool IsEngineSmokeOn() const { return mEngineSmokeOn; }

            /**
             * Toggles smoke plumes on and off
             * @param enable True to enable, false to not
             */
            void SetSmokePlumePresent(bool enable);

            /**
             * Gets if smoke plumes are currently enabled
             * @return mSmokePlumePresent
             */
            bool IsSmokePlumePresent() const { return mSmokePlumePresent; }

            /**
             * Toggles fire on and off
             * @param enable True to enable, false to not
             */
            void SetFlamesPresent(bool enable);

            /**
             * Gets if fire is currently enabled
             * @return mFlamesPresent
             */
            bool IsFlamesPresent() const { return mFlamesPresent; }

            /**
             * Sets the position of engine smoke on a model
             * @param pos The position to set
             */
            void SetEngineSmokePos(const osg::Vec3& pos) { mEngineSmokePosition = pos; }

            /**
             * Gets the position of smoke coming from an engine
             * @return The position
             */
            osg::Vec3 GetEngineSmokePos() const { return mEngineSmokePosition; }

            /**
             * Sets The file name of the fire particle system being used
             * @param fileName The name of the file
             */
            void SetFlamesPresentFile(const std::string& fileName) { mFlamesSystemFile = fileName; }

            /**
             * Gets the name of the the file that the particle system is using
             * @return mFlamesSystemFile
             */
            std::string GetFlamesPresentFile() const { return mFlamesSystemFile; }

            /**
             * Sets The file name of the smoke particle system being used
             * @param fileName The name of the file
             */
            void SetSmokePlumesFile(const std::string& fileName) { mSmokePlumesSystemFile = fileName; }

            /**
             * Gets the name of the the file that the particle system is using
             * @return mSmokePlumesSystemFile
             */
            std::string GetSmokePlumesFile() const { return mSmokePlumesSystemFile; }

            /**
             * Sets The file name of the smoke particle system being used
             * @param fileName The name of the file
             */
            void SetEngineSmokeFile(const std::string& fileName) { mEngineSmokeSystemFile = fileName; }

            /**
             * Gets the name of the the file that the particle system is using
             * @return mEngineSmokeSystemFile
             */
            std::string GetEngineSmokeFile() const { return mEngineSmokeSystemFile; }

            /**
             * Sets this entity's last known translation.  This should
             * only be set for remote actors.
             *
             * @param vec the new last position.
             */
            virtual void SetLastKnownTranslation(const osg::Vec3& vec);

            /**
             * @return the last known position for this if it's a remote entity.
             */
            osg::Vec3 GetLastKnownTranslation() const;

            /**
             * Sets this entity's last known rotation.  This should
             * only be set for remote actors.
             *
             * @param vec the new last rotation as yaw, pitch, roll.
             */
            virtual void SetLastKnownRotation(const osg::Vec3& vec);

            /**
             * @return the last known rotation for this if it's a remote entity as yaw, pitch, roll.
             */
            osg::Vec3 GetLastKnownRotation() const;

            /**
             * Sets this entity's DIS/RPR-FOM velocity vector.
             *
             * @param vec the velocity vector to copy
             */
            void SetLastKnownVelocity(const osg::Vec3& vec);

            /**
             * Retrieves this entity's DIS/RPR-FOM velocity vector.
             *
             * @return the velocity vector
             */
            osg::Vec3 GetLastKnownVelocity() const;

            /**
             * Sets this entity's DIS/RPR-FOM acceleration vector.
             *
             * @param accelerationVector the acceleration vector to copy
             */
            void SetLastKnownAcceleration(const osg::Vec3& vec);

            /**
             * Retrieves this entity's DIS/RPR-FOM acceleration vector.
             *
             * @return the acceleration vector
             */
            osg::Vec3 GetLastKnownAcceleration() const;

            /**
             * Sets this entity's DIS/RPR-FOM angular velocity vector.
             *
             * @param angularVelocityVector the angular velocity vector to copy
             */
            void SetLastKnownAngularVelocity(const osg::Vec3& vec);

            /**
             * Retrieves this entity's DIS/RPR-FOM angular velocity vector.
             *
             * @return the angular velocity vector
             */
            osg::Vec3 GetLastKnownAngularVelocity() const;

            /**
            * Sets the CURRENT velocity. This should be updated each frame and is different
            * from the LastKnownVelocityVector (which is used for DeadReckoning).
            * If you perform your own physics, you should set this at the start of the frame,
            * BEFORE you perform physics for the current frame. BaseEntity sets this as the
            * LastKnownVelocity right before it publishes.
            * Since the physics component runs before entities get ticked, the timing works.
            * @param vec the velocity vector to copy
            * @see #SetLastKnownVelocity
            */
            void SetCurrentVelocity(const osg::Vec3& vec) { mCurrentVelocity = vec; }
            /**
            * Gets the CURRENT velocity. This is different from the LastKnownVelocity.
            * @see #SetCurrentVelocity()
            * @see #SetLastKnownVelocity()
            * @return the current instantaneous velocity.
            */
            osg::Vec3 GetCurrentVelocity() const { return mCurrentVelocity; }

            /**
            * Sets the CURRENT acceleration. This is sort of hard to compute accurately,
            * but is provided since it is published and will be used for dead reckoning
            * by entities. If you CAN compute this for a local entity, you should set this at the
            * start of the frame, BEFORE you perform physics for the current frame. BaseEntity
            * will set this as the LastKnownAcceleration right before it publishes.
            * Since the physics component runs before entities get ticked, the timing works.
            * @param vec the new value
            * @see #SetLastKnownAcceleration
            */
            void SetCurrentAcceleration(const osg::Vec3& vec) { mCurrentAcceleration = vec; }
            /**
            * Gets the CURRENT acceleration. This is different from the LastKnownAcceleration.
            * @see #SetCurrentAcceleration()
            * @see #SetLastKnownAcceleration()
            * @return the current acceleration from the start of the frame.
            */
            osg::Vec3 GetCurrentAcceleration() const { return mCurrentAcceleration; }

            /**
            * Sets the CURRENT angular velocity. This is often hard to compute accurately,
            * but is provided since it is published and can be used for dead reckoning
            * remote entities. If you CAN compute this for a local entity, you should set this at the
            * start of the frame, BEFORE you perform physics for the current frame. BaseEntity
            * will set this as the LastKnownAcceleration right before it publishes.
            * Since the physics component runs before entities get ticked, the timing works.
            * Note - Angular Velocity is future 'rate' of change in the rotation.
            * @param vec the new value
            * @see #SetLastKnownAngularVelocity()
            */
            void SetCurrentAngularVelocity(const osg::Vec3& vec) { mCurrentAngularVelocity = vec; }
            /**
            * Gets the CURRENT angular velocity. This is different from the LastKnownAngularVelocity.
            * @see #SetCurrentAngularVelocity()
            * @see #SetLastKnownAngularVelocity()
            * @return the current angular velocity that was set at the start of the frame.
            */
            osg::Vec3 GetCurrentAngularVelocity() const { return mCurrentAngularVelocity; }

            /**
             * When publishing pos/rot updates with dead reckoning - do we send velocity/acceleration or not?
             * Allows tightly controlled entities that do motion model stuff to not mess up velocity
             * If false, linear velocity will be zero. Should be made into a property...
             * @param newValue True to enable, false to not. Default is true.
             */
            void SetPublishLinearVelocity(bool newValue) { mPublishLinearVelocity = newValue; }
            /**
             * Are we set to publish linear velocity&  acceleration.
             * @see #SetPublishLinearVelocity()
             * @return mPublishLinearVelocity
             */
            bool IsPublishLinearVelocity() const { return mPublishLinearVelocity; }

            /**
            * When publishing pos/rot updates with dead reckoning - do we send velocity/acceleration or not?
            * Allows tightly controlled entities that do motion model stuff to not mess up velocity
            * If false, linear velocity will be zero. Should be made into a property...
            * @param newValue True to enable, false to not. Default is true.
            */
            void SetPublishAngularVelocity(bool newValue) { mPublishAngularVelocity = newValue; }
            /**
             * Are we set to publish angular velocity.
             * @see #SetPublishAngularVelocity()
             * @return mPublishAngularVelocity
             */
            bool IsPublishAngularVelocity() const { return mPublishAngularVelocity; }


            void SetFrozen(bool frozen);

            bool GetFrozen() const;


            /**
             * @return true if no ground following should be performed on this actor.
             */
            bool IsFlying() const;

            /**
             * True means no ground following should be performed on this actor.  False
             * it will follow the ground as it moves.
             * @param newFlying the new value to set.
             */
            void SetFlying(bool newFlying);

            /**
             * @return true if the model is being drawn.
             */
            bool IsDrawingModel() const;

            /**
             * Set this to make the model for this entity show or not.
             * @param newDrawing the new value to set.
             */
            void SetDrawingModel(bool newDrawing);

            /**
             * @return true a player is attached to this entity
             */
            bool IsPlayerAttached() const;

            /**
             * Set this to when a player attaches to this entity
             * @param attach the new value to set.
             */
            void SetIsPlayerAttached(bool attach);

            /// Set the mobility state of this entity. This is intended for use by
            /// the entity's damage helper.
            bool IsMobilityDisabled() const { return mDisabledMobility; }
            void SetMobilityDisabled( bool disabled ) { mDisabledMobility = disabled; }

            /// Set the mobility state of this entity. This is intended for use by
            /// the entity's damage helper.
            bool IsFirepowerDisabled() const { return mDisabledFirepower; }
            void SetFirepowerDisabled( bool disabled ) { mDisabledFirepower = disabled; }

            /**
             * Sets the offset from the ground the actor should be clamped to.
             * This only matters if flying is set to false.
             * @param newOffset the new offset value.
             */
            void SetGroundOffset(float newOffset);

            ///@return The distance from the ground that the actor should be.
            float GetGroundOffset() const;

            /**
             * Set the name of the munition damage table found in the Configs/MunitionsConfig.xml
             * @param tableName The name of the table, usually the same name as the entity class
             */
            void SetMunitionDamageTableName( const std::string& tableName );
            ///@ return The name of the referenced munition damage table, usually the same name as the entity class
            std::string GetMunitionDamageTableName() const;

            /**
             * Sets the maximum translation offset from the last update translation this
             * actor may have before forcing an update message to be sent.
             */
            void SetMaxTranslationError(float distance)
            {
               mMaxTranslationError = distance;
               mMaxTranslationError2 = distance * distance;
            }

            ///@return the max translation error allowed before forcing an update.
            float GetMaxTranslationError() const { return mMaxTranslationError; }

            /**
             * Sets the maximum rotation offset (in degrees) from the last update rotation this
             * actor may have before forcing an update message to be sent.
             */
            void SetMaxRotationError(float rotation)
            {
               mMaxRotationError = rotation;
               mMaxRotationError2 = rotation * rotation;
            }

            ///@return the max rotation (in degrees) error allowed before forcing an update.
            float GetMaxRotationError() const { return mMaxRotationError; }

            /**
             * Sets the default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            void SetDefaultScale(const osg::Vec3& newDefScale);

            /**
             * @return The default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            osg::Vec3 GetDefaultScale() const { return mDefaultScale; }

            /**
             * Sets the default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            void SetScaleMagnification(const osg::Vec3& newScaleMag);

            /**
             * @return The default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            osg::Vec3 GetScaleMagnification() const { return mScaleMagnification; }

            ///@return the model scale result of the default scale and magnification.
            osg::Vec3 GetModelScale() const;

            ///Changes the rotation on the model matrix
            void SetModelRotation(const osg::Vec3& hpr);

            ///@return the rotation on the model matrix
            osg::Vec3 GetModelRotation() const;

            /**
             * Overridden to handle dead-reckoning and other such issues related to rendering a local actor.
             */
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

            virtual void ProcessMessage(const dtGame::Message& message);

            //this was made public so the proxy could call it.. -bga
            dtGame::DeadReckoningHelper& GetDeadReckoningHelper() { return *mDeadReckoningHelper; }
            const dtGame::DeadReckoningHelper& GetDeadReckoningHelper() const { return *mDeadReckoningHelper; }
            bool IsDeadReckoningHelperValid() const { return (mDeadReckoningHelper.valid()); }

            //this is used to create the dead reckoning helper and initialize the default options
            //made virtual  for supporting custom dr helpers
            virtual void InitDeadReckoningHelper();

            /**
             * This function is intended for use by entities that implement physics
             * simulation behaviors.
             * @param force The force direction and magnitude emitted from the location
             *              to the center of the entity. The entity will have to determine
             *              the exact force at specific points relative to its center.
             *              Force is measured in Newtons: mass (1kg) * acceleration (1meter/1sec^2).
             * @param location The location in world space from which the force was generated.
             *              The location components will be measured in meters.
             * @param isImpulse Impulses are not applied each tick, they are fully applied
             *              instantaneous forces. For most physics engines, that means the impulse
             *              should not be scaled down to be relative to the current tick timeslice.
             */
            virtual void ApplyForce(const osg::Vec3& force, const osg::Vec3& location, bool IsImpulse = false) {}

            /// Getter for the time until next update value.  Used for testing
            float GetTimeUntilNextUpdate() const { return mTimeUntilNextUpdate; }

            void SetMappingName( const std::string& name );
            const std::string& GetMappingName() const;

            void SetEntityTypeId( const std::string& entityType );
            const std::string& GetEntityTypeId() const;

            /**
             * Gives a local entity an opportunity to respond to damage from a munition.
             * Called from the MunitionsComponent via the DamageHelper. Typically called
             * from ProcessDetonationMessage().
             * NOTE - Damage has already been applied to mCurDamageRatio and published
             * on the network. This method must apply forces manually.
             */
            virtual void RespondToHit(const SimCore::DetonationMessage& message,
               const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force,
               const osg::Vec3& location);

            /**
             * Gives a local entity an opportunity to filter out certain types of damage or
             * take less/more damage from specific munition types. The incoming damage is 'normalized'
             * between 0.0 (none) to 1.0 (all). Called from MunitionsComonent::ProcessDetonationMessage()
             * via the DamageHelper in response to a local entity being hit by a direct or indirect fire.
             * Called before damage or forces have been applied. See RespondToHit().
             */
            virtual float ValidateIncomingDamage(float incomingDamage, const DetonationMessage& message,
               const SimCore::Actors::MunitionTypeActor& munition)
            {
               return incomingDamage;
            }

            /// @return true if this entity should be visible based on the options given.
            bool ShouldBeVisible(const SimCore::VisibilityOptions& options);

         protected:
            virtual ~BaseEntity();

            dtUtil::Log* mLogger;
            /// TODO this needs to private, and subclasses should call a method to set this to 0 so it forces an update.
            float mTimeUntilNextUpdate;

            /**
             * @return the matrix transform node that holds the scale.  This should be the parent
             *         of all other nodes, not the one returned by GetOSGNode().
             */
            osg::MatrixTransform& GetScaleMatrixTransform();

            /**
             * @return the matrix transform node that holds the scale.  This should be the parent
             *         of all other nodes, not the one returned by GetOSGNode().
             */
            const osg::MatrixTransform& GetScaleMatrixTransform() const;

            ///updates the scale of the model base on the default scale and magnification.
            void UpdateModelScale();

            void SetDeadReckoningHelper(dtGame::DeadReckoningHelper* pHelper);

            void RegisterWithDeadReckoningComponent();

            /// Called by tick local to see if an update should be sent and if it is a full or partial.
            virtual bool ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            // Allows a sub-class to set the engine smoke value without doing all the engine
            // smoke 'stuff'.
            void InnerSetEngineSmokeOn(bool enable) { mEngineSmokeOn = enable; }

            /// Called once we decided to publish - push our last known values onto the DR helper
            virtual void SetLastKnownValuesBeforePublish(const osg::Vec3& pos, const osg::Vec3& rot);

         private:
            ///a sub-matrix node just for doing scale on the model.
            dtCore::RefPtr<osg::MatrixTransform> mScaleMatrixNode;

            //////////////// DeadReckoning related values
            // Note that the LastKnown values are stored on the DR helper, but the CURRENT
            // values are part of the entity. See the get/set methods for info.
            dtCore::RefPtr<dtGame::DeadReckoningHelper> mDeadReckoningHelper;
            //This is stored on both the entity and the helper because the
            //two values are not always the same.
            dtGame::DeadReckoningAlgorithm* mDRAlgorithm;
            osg::Vec3 mCurrentVelocity;
            osg::Vec3 mCurrentAcceleration;
            osg::Vec3 mCurrentAngularVelocity;

            ///The force for which this entity is fighting.
            BaseEntityActorProxy::ForceEnum* mForceAffiliation;

            /// The particle systems used for fire and smoke
            dtCore::RefPtr<dtCore::ParticleSystem> mEngineSmokeSystem, mSmokePlumesSystem, mFlamesSystem;

            /// The entity's service
            BaseEntityActorProxy::ServiceEnum* mService;

            /// The file names that are loaded into the above particle systems
            std::string mEngineSmokeSystemFile, mSmokePlumesSystemFile, mFlamesSystemFile;
            ///The entity's DIS/RPR-FOM damage state.
            BaseEntityActorProxy::DamageStateEnum* mDamageState;

            /// The max amount of damage before dead - use when registering local actors with the MunitionsComponent
            float mMaxDamageAmount;
            /// The cur damage ratio (0.0 to 1.0 dead). Do not set manually - controlled by the MunitionsComonent when a local entity takes damage
            float mCurDamageRatio;
            /// If true, local actors will automatically attempt to register with the Munitions Component (if one exists). True by default
            bool mAutoRegisterWithMunitionsComponent;

            /// Environment the entity is specialized in navigating.
            BaseEntityActorProxy::DomainEnum* mDomain;

            /// The name of the munition damage table as found in Configs/MunitionsConfig.xml
            std::string mMunitionTableName;

            /// Position of the engine for smoke
            osg::Vec3 mEngineSmokePosition;

            /// the default xyz scale of the model
            osg::Vec3 mDefaultScale;

            /// the xyz magnification of default scale to display.
            osg::Vec3 mScaleMagnification;

            //members for sending updates on a local actor.
            float mMaxRotationError;
            float mMaxRotationError2;
            float mMaxTranslationError;
            float mMaxTranslationError2;

            unsigned mFireLightID;

            std::string mEntityType;
            std::string mMappingName;

            /// Flags for particle effects
            bool mEngineSmokeOn: 1;
            bool mSmokePlumePresent: 1;
            bool mFlamesPresent: 1;
            bool mFlying: 1;
            bool mDrawing: 1;
            bool mIsPlayerAttached: 1;
            bool mDisabledFirepower: 1;
            bool mDisabledMobility: 1;
            bool mIsFrozen: 1;

            bool mPublishLinearVelocity: 1;
            bool mPublishAngularVelocity: 1;


      };

   }
}

#endif /*BASEENTITY_H_*/