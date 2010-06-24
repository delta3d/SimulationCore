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

#include <dtUtil/getsetmacros.h>

namespace dtGame
{
   class DeadReckoningComponent;
   //class DeadReckoningAlgorithm;
   class DeadReckoningHelper;
}

namespace SimCore
{
   class DetonationMessage;

   namespace Actors
   {
      class MunitionTypeActor;
      class DRPublishingActComp;


      ///////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BaseEntityActorProxy : public dtGame::GameActorProxy
      {
         public:
            typedef dtGame::GameActorProxy BaseClass;

            static const dtUtil::RefString PROPERTY_FROZEN;
            static const dtUtil::RefString PROPERTY_FLAMES_PRESENT;
            static const dtUtil::RefString PROPERTY_SMOKE_PLUME_PRESENT;
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

            BaseEntity(dtGame::GameActorProxy& proxy);

            /// Override this to add your own components or to init values on the ones that are already added.
            virtual void BuildActorComponents();

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
            DECLARE_PROPERTY(float, MaxDamageAmount);

            /**
             * The current amount of damage this entity has sustained - between 0.0 (none) and 1.0 (dead).
             * This value is controlled by the munitions component - NEVER set this manually.
             */
            DECLARE_PROPERTY(float, CurDamageRatio);

            DECLARE_PROPERTY(std::string, MappingName);
            DECLARE_PROPERTY(std::string, EntityTypeId);

            /// Set the environment the entity is specialized in navigating.
            DECLARE_PROPERTY(dtUtil::EnumerationPointer<BaseEntityActorProxy::DomainEnum>, Domain);

            #undef  PROPERTY_MODIFIERS_SETTER
            #define PROPERTY_MODIFIERS_SETTER virtual
            /// Sets the force for which this entity is fighting.
            DECLARE_PROPERTY(dtUtil::EnumerationPointer<BaseEntityActorProxy::ForceEnum>, ForceAffiliation);
            #undef  PROPERTY_MODIFIERS_SETTER
            #define PROPERTY_MODIFIERS_SETTER

            /// Sets the service of this entity
            DECLARE_PROPERTY(dtUtil::EnumerationPointer<BaseEntityActorProxy::ServiceEnum>, Service);

            /// Toggles smoke plumes on and off
            DECLARE_PROPERTY(bool, SmokePlumePresent);
            /// Toggles fire on and off
            DECLARE_PROPERTY(bool, FlamesPresent);
            bool IsFlamesPresent() const { return GetFlamesPresent(); }

            /// Set this to make the model for this entity show or not.
            DECLARE_PROPERTY(bool, DrawingModel);
            bool IsDrawingModel() const { return GetDrawingModel(); }

            /// Set this to when a player attaches to this entity
            DECLARE_PROPERTY(bool, PlayerAttached);
            bool IsPlayerAttached() const { return GetPlayerAttached(); }

            /// Set the mobility state of this entity. This is intended for use by the entity's damage helper.
            DECLARE_PROPERTY(bool, MobilityDisabled);
            bool IsMobilityDisabled() const { return GetMobilityDisabled(); }

            /// Set the mobility state of this entity. This is intended for use by the entity's damage helper.
            DECLARE_PROPERTY(bool, FirepowerDisabled);
            bool IsFirepowerDisabled() const { return GetFirepowerDisabled(); }

            DECLARE_PROPERTY(bool, Frozen);

            DECLARE_PROPERTY(bool, AutoRegisterWithMunitionsComponent);
            /// Not an actual property. Defaults to true. Set to false if for some bizarre reason you don't want the DR Component
            DECLARE_PROPERTY(bool, AutoRegisterWithDeadReckoningComponent);


            /**
             * Set the name of the munition damage table found in the Configs/MunitionsConfig.xml
             * @param tableName The name of the table, usually the same name as the entity class
             */
            DECLARE_PROPERTY(std::string, MunitionDamageTableName);


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
             * Sets the default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            void SetDefaultScale(const osg::Vec3& newDefScale);

            /**
             * @return The default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            const osg::Vec3& GetDefaultScale() const { return mDefaultScale; }

            /**
             * Sets the default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            void SetScaleMagnification(const osg::Vec3& newScaleMag);

            /**
             * @return The default scale of the entity.  The actual scale is the default scale x scale magnification
             */
            const osg::Vec3& GetScaleMagnification() const { return mScaleMagnification; }

            ///@return the model scale result of the default scale and magnification.
            osg::Vec3 GetModelScale() const;

            ///Changes the rotation on the model matrix
            void SetModelRotation(const osg::Vec3& hpr);

            ///@return the rotation on the model matrix
            osg::Vec3 GetModelRotation() const;

            /// Ticks are registered for on all local entities. Over ride for custom stuff.
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

            virtual void ProcessMessage(const dtGame::Message& message);

            //this was made public so the proxy could call it.. -bga
            dtGame::DeadReckoningHelper& GetDeadReckoningHelper() { return *mDeadReckoningHelper; }
            const dtGame::DeadReckoningHelper& GetDeadReckoningHelper() const { return *mDeadReckoningHelper; }
            bool IsDeadReckoningHelperValid() const { return (mDeadReckoningHelper.valid()); }

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

            /// Call when an important property changes that requires a full update. The update is not sent immediately, but later during the DR Publishing
            void CauseFullUpdate();

            /// Accessor for the dr publishing component. Allows setting properties, changing behaviors, forcing updates, unit tests, etc. 
            DRPublishingActComp* GetDRPublishingActComp();

            /**
             * Get the bounding sphere information for this Entity.
             * Overridden to exclude particles systems and other effects.
             * @param center : pointer to fill out with the sphere's center position
             * @param radius : float pointer to fill out with the sphere's radius
             */
            virtual void GetBoundingSphere(osg::Vec3& center, float& radius);

            /**
             * Get the bounding box information for this Drawable.
             * Overridden to exclude particles systems and other effects.
             * @return BoundingBox that encloses the Drawable.
             */
            virtual osg::BoundingBox GetBoundingBox();

         protected:
            virtual ~BaseEntity();

            dtUtil::Log* mLogger;

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

            /// Called in OnEnteredWorld. A basic part of entities.
            void RegisterWithDeadReckoningComponent();

         private:
            ///a sub-matrix node just for doing scale on the model.
            dtCore::RefPtr<osg::MatrixTransform> mScaleMatrixNode;

            //////////////// DeadReckoning related values
            // Note that the LastKnown values are stored on the DR helper, but the CURRENT
            // values are part of the entity. See the get/set methods for info.
            dtCore::RefPtr<dtGame::DeadReckoningHelper> mDeadReckoningHelper;
            
            //This is stored on both the entity and the helper because the
            //two values are not always the same.
            //dtGame::DeadReckoningAlgorithm* mDRAlgorithm;

            /// The particle systems used for fire and smoke
            dtCore::RefPtr<dtCore::ParticleSystem> mSmokePlumesSystem, mFlamesSystem;

            /// The file names that are loaded into the above particle systems
            std::string mSmokePlumesSystemFile, mFlamesSystemFile;
            ///The entity's DIS/RPR-FOM damage state.
            BaseEntityActorProxy::DamageStateEnum* mDamageState;

            /// the default xyz scale of the model
            osg::Vec3 mDefaultScale;

            /// the xyz magnification of default scale to display.
            osg::Vec3 mScaleMagnification;

            unsigned mFireLightID;

            // held onto since we use this fairly often.
            dtCore::RefPtr<DRPublishingActComp> mDRPublishingActComp;
      };

   }
}

#endif /*BASEENTITY_H_*/
