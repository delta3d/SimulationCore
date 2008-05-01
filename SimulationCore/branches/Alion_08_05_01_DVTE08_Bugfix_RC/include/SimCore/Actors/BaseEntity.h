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
*/
#ifndef BASEENTITY_H_
#define BASEENTITY_H_

#include <SimCore/Export.h>

#include <dtGame/gameactor.h>

#include <SimCore/Actors/IGActor.h>

namespace dtGame
{
   class DeadReckoningComponent;
   class DeadReckoningAlgorithm;
   class DeadReckoningHelper;
}

namespace SimCore
{
   namespace Actors
   {

      class SIMCORE_EXPORT BaseEntityActorProxy : public dtGame::GameActorProxy
      {
         public:
            typedef dtGame::GameActorProxy BaseClass;
            
            static const std::string PROPERTY_LAST_KNOWN_TRANSLATION;
            static const std::string PROPERTY_LAST_KNOWN_ROTATION;
            static const std::string PROPERTY_VELOCITY_VECTOR;
            static const std::string PROPERTY_ACCELERATION_VECTOR;
            static const std::string PROPERTY_ANGULAR_VELOCITY_VECTOR;
            static const std::string PROPERTY_ENGINE_SMOKE_POSITION;
            static const std::string PROPERTY_ENGINE_SMOKE_ON;
            static const std::string PROPERTY_FLAMES_PRESENT;
            static const std::string PROPERTY_SMOKE_PLUME_PRESENT;
            static const std::string PROPERTY_ENGINE_POSITION;
            static const std::string PROPERTY_FLYING;
            static const std::string PROPERTY_DAMAGE_STATE;
            static const std::string PROPERTY_DEFAULT_SCALE;
            static const std::string PROPERTY_SCALE_MAGNIFICATION_FACTOR;
            static const std::string PROPERTY_MODEL_SCALE;
            static const std::string PROPERTY_MODEL_ROTATION;

            class SIMCORE_EXPORT DamageStateEnum : public dtUtil::Enumeration
            {
                  DECLARE_ENUM(DamageStateEnum);
               public:
                  static DamageStateEnum NO_DAMAGE;
                  static DamageStateEnum SLIGHT_DAMAGE;
                  static DamageStateEnum MODERATE_DAMAGE;
                  static DamageStateEnum DESTROYED;
               private:
                  DamageStateEnum(const std::string& name) : dtUtil::Enumeration(name)
                  {
                     AddInstance(this);
                  }
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
               private:
                  ForceEnum(const std::string& name) : dtUtil::Enumeration(name)
                  {
                     AddInstance(this);
                  }
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
                  ServiceEnum(const std::string& name) : dtUtil::Enumeration(name)
                  {
                     AddInstance(this);
                  }
            };

            BaseEntityActorProxy();

            /**
             * All base entity objects will be placeable
             */
            virtual bool IsPlaceable()  { return true; }

            /**
             * Build the properties common to all BaseEntity objects
             */
            virtual void BuildPropertyMap();

            virtual void OnRemovedFromWorld();

            /// Build the invokables
            virtual void BuildInvokables();

            void SetLastKnownRotation(const osg::Vec3 &vec);

            osg::Vec3 GetLastKnownRotation() const;

         protected:
            virtual ~BaseEntityActorProxy();
            virtual void OnEnteredWorld();
            ///This is a dummy method to make the readonly property work.
            void SetModelScale(const osg::Vec3& newValue) {} 
         private:
      };

      class SIMCORE_EXPORT BaseEntity : public IGActor
      {
         public:

            static const float TIME_BETWEEN_UPDATES;

            BaseEntity(dtGame::GameActorProxy& proxy);

            virtual void OnEnteredWorld();

            virtual void OnRemovedFromWorld();

            /**
             * Sets this entity's DIS/RPR-FOM damage state.
             *
             * @param damageState the damage state
             */
            virtual void SetDamageState(BaseEntityActorProxy::DamageStateEnum &damageState);

            /**
             * Returns this entity's DIS/RPR-FOM damage state.
             *
             * @return the damage state
             */
            BaseEntityActorProxy::DamageStateEnum& GetDamageState() const;

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
            void SetForceAffiliation(BaseEntityActorProxy::ForceEnum& newForceEnum);

            /**
             * @return the force for which this entity is fighting.
             */
            BaseEntityActorProxy::ForceEnum& GetForceAffiliation() const;

            /**
             * Sets the service of this entity
             * @param service The new service
             */
            void SetService(BaseEntityActorProxy::ServiceEnum &service);

            /**
             * Gets the service of this entity
             * @return mService
             */
            BaseEntityActorProxy::ServiceEnum& GetService() const;

            /**
             * Toggles engine smoke on and off
             * @param enable True to enable, false to not
             */
            void SetEngineSmokeOn(bool enable);

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
            void SetEngineSmokePos(const osg::Vec3 &pos) { mEngineSmokePosition = pos; }

            /**
             * Gets the position of smoke coming from an engine
             * @return The position
             */
            osg::Vec3 GetEngineSmokePos() const { return mEngineSmokePosition; }

            /**
             * Sets The file name of the fire particle system being used
             * @param fileName The name of the file
             */
            void SetFlamesPresentFile(const std::string &fileName) { mFlamesSystemFile = fileName; }

            /**
             * Gets the name of the the file that the particle system is using
             * @return mFlamesSystemFile
             */
            std::string GetFlamesPresentFile() const { return mFlamesSystemFile; }

            /**
             * Sets The file name of the smoke particle system being used
             * @param fileName The name of the file
             */
            void SetSmokePlumesFile(const std::string &fileName) { mSmokePlumesSystemFile = fileName; }

            /**
             * Gets the name of the the file that the particle system is using
             * @return mSmokePlumesSystemFile
             */
            std::string GetSmokePlumesFile() const { return mSmokePlumesSystemFile; }

            /**
             * Sets The file name of the smoke particle system being used
             * @param fileName The name of the file
             */
            void SetEngineSmokeFile(const std::string &fileName) { mEngineSmokeSystemFile = fileName; }

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
            virtual void SetLastKnownTranslation(const osg::Vec3 &vec);

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
            virtual void SetLastKnownRotation(const osg::Vec3 &vec);

            /**
             * @return the last known rotation for this if it's a remote entity as yaw, pitch, roll.
             */
            osg::Vec3 GetLastKnownRotation() const;

            /**
             * Sets this entity's DIS/RPR-FOM velocity vector.
             *
             * @param vec the velocity vector to copy
             */
            void SetVelocityVector(const osg::Vec3 &vec);

            /**
             * Retrieves this entity's DIS/RPR-FOM velocity vector.
             *
             * @return the velocity vector
             */
            osg::Vec3 GetVelocityVector() const;

            /**
             * Sets this entity's DIS/RPR-FOM acceleration vector.
             *
             * @param accelerationVector the acceleration vector to copy
             */
            void SetAccelerationVector(const osg::Vec3 &vec);

            /**
             * Retrieves this entity's DIS/RPR-FOM acceleration vector.
             *
             * @return the acceleration vector
             */
            osg::Vec3 GetAccelerationVector() const;

            /**
             * Sets this entity's DIS/RPR-FOM angular velocity vector.
             *
             * @param angularVelocityVector the angular velocity vector to copy
             */
            void SetAngularVelocityVector(const osg::Vec3 &vec);

            /**
             * Retrieves this entity's DIS/RPR-FOM angular velocity vector.
             *
             * @return the angular velocity vector
             */
            osg::Vec3 GetAngularVelocityVector() const;


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
             * Overridden to handle dead-reckoning and other such issues related to rendering a remote actor.
             */
            virtual void TickRemote(const dtGame::Message& tickMessage);

            /**
             * Overridden to handle dead-reckoning and other such issues related to rendering a local actor.
             */
            virtual void TickLocal(const dtGame::Message& tickMessage);

            //this was made public so the proxy could call it.. -bga
            dtGame::DeadReckoningHelper& GetDeadReckoningHelper() { return *mDeadReckoningHelper; }
            const dtGame::DeadReckoningHelper& GetDeadReckoningHelper() const { return *mDeadReckoningHelper; }

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
             */
            virtual void ApplyForce( const osg::Vec3& force, const osg::Vec3& location ) {}
            
            /// Getter for the time until next update value.  Used for testing
            float GetTimeUntilNextUpdate() const { return mTimeUntilNextUpdate; }
            
         protected:
            virtual ~BaseEntity();

            dtUtil::Log* mLogger;
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
            
            virtual void HandleModelDrawToggle(bool draw) = 0;

            ///updates the scale of the model base on the default scale and magnification.
            void UpdateModelScale();
            
            //this was made public so the proxy could call it.. see above  -bga
            //dtGame::DeadReckoningHelper& GetDeadReckoningHelper() {return *mDeadReckoningHelper.get();}

            void SetDeadReckoningHelper(dtGame::DeadReckoningHelper* pHelper);
         
            void RegisterWithDeadReckoningComponent();
            
            /// Called by tick local to see if an update should be sent and if it is a full or partial.
            virtual bool ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            /// Called by tick local when sending a partial update to get a list of the properties to send.
            virtual void FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill);

         private:
            ///a sub-matrix node just for doing scale on the model.
            dtCore::RefPtr<osg::MatrixTransform> mScaleMatrixNode;

            dtCore::RefPtr<dtGame::DeadReckoningHelper> mDeadReckoningHelper;

            //This is stored on both the entity and the helper because the
            //two values are not always the same.
            dtGame::DeadReckoningAlgorithm* mDRAlgorithm;

            ///The force for which this entity is fighting.
            BaseEntityActorProxy::ForceEnum* mForceAffiliation;

            /// The particle systems used for fire and smoke
            dtCore::RefPtr<dtCore::ParticleSystem> mEngineSmokeSystem, mSmokePlumesSystem, mFlamesSystem;

            /// The entity's service
            BaseEntityActorProxy::ServiceEnum *mService;

            /// The file names that are loaded into the above particle systems
            std::string mEngineSmokeSystemFile, mSmokePlumesSystemFile, mFlamesSystemFile;
            ///The entity's DIS/RPR-FOM damage state.
            BaseEntityActorProxy::DamageStateEnum* mDamageState;

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

            /// Flags for particle effects
            bool mEngineSmokeOn, mSmokePlumePresent, mFlamesPresent;
            bool mFlying;
            bool mDrawing;
            bool mIsPlayerAttached;
            bool mDisabledFirepower;
            bool mDisabledMobility;
            bool mIsFrozen;

            unsigned mFireLightID;

      };

   }
}

#endif /*BASEENTITY_H_*/
