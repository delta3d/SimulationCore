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
 * @author David Guthrie
 * @author Eddie Johnson
 * @author Allen Danklefsen
 */

#ifndef _PLATFORM_ACTOR_
#define _PLATFORM_ACTOR_

// entity.h: Declaration of the Platform class.
//
//////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>

#include <SimCore/Actors/BaseEntity.h>
#include <osg/Group>
#include <dtCore/resourcedescriptor.h>

namespace dtCore
{
   class ParticleSystem;
}

namespace dtUtil
{
   class NodeCollector;
}

namespace dtAudio
{
   class Sound;
}

namespace dtCore
{
   class NamedGroupParameter;
   class ActorProxyIcon;
}

namespace osg
{
   class Node;
   class Geode;
   class Group;
   class MatrixTransform;
   class Switch;
}

namespace dtHLAGM
{
   class ArticulatedParameter;
}

namespace SimCore
{
   namespace Components
   {
      class ArticulationHelper;
   }

   namespace Actors
   {
      /**
       * PlatformActorProxy, a class to define the basic behaviors of a vehicle type of entity
       */
      class SIMCORE_EXPORT PlatformActorProxy : public BaseEntityActorProxy
      {
         public:
            typedef BaseEntityActorProxy BaseClass;

            static const dtUtil::RefString PROPERTY_HEAD_LIGHTS_ENABLED;
            static const dtUtil::RefString PROPERTY_MESH_NON_DAMAGED_ACTOR;
            static const dtUtil::RefString PROPERTY_MESH_DAMAGED_ACTOR;
            static const dtUtil::RefString PROPERTY_MESH_DESTROYED_ACTOR;
            static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_POSITION;
            static const dtUtil::RefString PROPERTY_ENGINE_SMOKE_ON;
            static const dtUtil::RefString PROPERTY_ENGINE_POSITION;

            // Constructor
            PlatformActorProxy();

            /**
             * Sometimes one wants all the platforms to have physics enabled, sometimes you want them disabled.
             * This will make any platform created that has an ActorType that extends EntityActorRegistry::PLATFORM_SWITCHABLE_PHYSICS_ACTOR_TYPE
             * to have physics or not.  This only effects the actors at the time of creation and no attempt has been
             * made to make this thread safe.
             *
             * It default to true.
             */
            static void SetPhysicsCreationEnabled(bool);

            /// Reads the value assigned with #SetPhysicsCreationEnabled
            static bool GetPhysicsCreationEnabled();

            /**
             * Initializes the actual actor the derived proxy will represent
             * By default this will create an Platform. This should be
             * overridden in all subclasses of the Platform class
             */
            virtual void CreateDrawable();

            /// mesh Resource to use for the non-damaged state.  @see #EnsureResourcesAreLoaded
            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, NonDamagedResource);
            /// mesh Resource to use for the damaged state. @see #EnsureResourcesAreLoaded
            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, DamagedResource);
            /// mesh Resource to use for the destroyed state. @see #EnsureResourcesAreLoaded
            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, DestroyedResource);

            /// Called by tick local when sending a partial update to get a list of the properties to send.
            virtual void GetPartialUpdateProperties(std::vector<dtUtil::RefString>& propNamesToFill);

            /**
             * Build the properties common to all platform objects
             */
            virtual void BuildPropertyMap();

            /// Adds additional invokables for this class.
            virtual void BuildInvokables();

            /**
             * Override method to add actor components needed by this class.
             */
            virtual void BuildActorComponents();

            /**
             * Returns the icon to be shown in the level editor
             * for this proxy (PARTICLE_SYSTEM)
             * @return mBillboardIcon
             */
            virtual dtCore::ActorProxyIcon* GetBillboardIcon();

            /**
             * This call exists so that the resources can be loaded when the developer needs them to be.
             * It is called from OnEnteredWorld; however, a subclass may want to call this earlier.
             * For example, before it calls the baseclass OnEnteredWorld, or before the actor is even added to the GM.
             * This will only do anything once per time the actor is added to the GM, so multiple calls to it will work fine.
             *
             * Once this has been called, setting any resource values such as #SetNonDamagedResource
             * will trigger an immediate model load.
             *
             * As some background, the platform now no longer loads resources as soon as the value is set
             * so that prototype actors don't take up too much memory.  This call will make them load.
             *
             * If the actor is in STAGE or is alreading in the world, the models will load as soon as the
             * property is set.
             *
             * @note a version of this function exists on the drawable to make it easier to call
             *       this from there if needed.  In the future, hopefully that won't be necessary.
             */
            virtual void EnsureResourcesAreLoaded();

            /// @return true if EnsureResourcesAreLoaded has been called.
            bool GetHasLoadedResources() const;
            /// Sets HasLoadedResources back to false so if the actor is re-added to the GM it will work.
            virtual void OnRemovedFromWorld();
         protected:
            // Destructor
            virtual ~PlatformActorProxy();

         private:
            dtCore::RefPtr<dtCore::ActorProxyIcon> mBillboardIcon;
            bool mHasLoadedResources;

            static bool mPhysicsCreationEnabled;
      };

      /**
       * A DIS/RPR-FOM entity.
       */
      class SIMCORE_EXPORT Platform : public BaseEntity
      {
         public:
            typedef BaseEntity BaseClass;

            static const dtUtil::RefString INVOKABLE_TICK_CONTROL_STATE;
            static const std::string DOF_NAME_HEAD_LIGHTS;

            /**
             * Constructor.
             *
             * @param name the instance name
             */
            Platform(dtGame::GameActorProxy& parent);

            /**
             * Generic method for loading damageable resources.  It is called automatically
             * when the #NonDamagedResource #DamagedResource and #DestroyedResource files are
             * set.
             * @param rd The resource descriptor to load.
             * @param state The damage state enum that the resource represents
             */
            void LoadDamageableFile(const dtCore::ResourceDescriptor& rd, PlatformActorProxy::DamageStateEnum& state);

            /**
             * Returns the switch node of this entity
             * @return mSwitchNode
             */
            dtCore::RefPtr<osg::Switch> GetSwitchNode();

            /**
             * Sets the shader group on this entity.  The shader group is
             * specified in the entity mapping.  If the group contains shaders
             * for any of the damage states they are mapped to the appropriate
             * state.  Otherwise, if a default shader has been specified in them
             * group, it is assigned to the remaining damage states.
             * @param groupName The name of the shader group.
             * @note Shaders and Shader groups are managed within the ShaderManger.
             *  These shaders and shader groups are defined in an external XML
             *  file which must be loaded before any of these properties are set.
             */
            virtual void OnShaderGroupChanged();

            /**
             * Overridden to handle the model switching
             * /brief    Purpose :  to load the dofs for the dofcontainer class
             *           Outs     : filled in mNodeCollector memb vars
             */
            virtual void LoadNodeCollector();

            /**
             * Called when this entity is added to the game manager.
             */
            virtual void OnEnteredWorld();

            /**
             * Sets the muzzle position on an entity
             * @param newPos The new position
             */
            void SetMuzzleFlashPosition(const osg::Vec3 &newPos) { mMuzzleFlashPosition = newPos; }

            /**
             * Gets the muzzle position of an entity
             * @return mMuzzleFlashPosition
             */
            osg::Vec3 GetMuzzleFlashPosition() const { return mMuzzleFlashPosition; }

            /// For the different physics models
            osg::Node* GetNonDamagedFileNode();
            /// For the different physics models
            osg::Node* GetDamagedFileNode();
            /// For the different physics models
            osg::Node* GetDestroyedFileNode();

            /**
             * Sets this entity's DIS/RPR-FOM damage state.
             * Overridden to allow the for changing the model when the damage state changes.
             * @param damageState the damage state
             */
            /*virtual*/ void SetDamageState(BaseEntityActorProxy::DamageStateEnum &damageState);

            /// Sets the group parameter of articulations.  This is for supporting the property.
            void SetArticulatedParametersArray(const dtCore::NamedGroupParameter& newValue);
            /// @return the group parameter for articulations.  This is for supporting the property.
            dtCore::RefPtr<dtCore::NamedGroupParameter> GetArticulatedParametersArray();

            /**
             * Sets the entity type.  Entity Type is a string that is used by the Input component
             * to determine what types of behaviors the app should have such as gunner positions, seats, weapons, ...
             */
            void SetEntityType( const std::string& entityType) { mEntityType = entityType; }

            /**
             * Gets the entity type.  Entity Type is a string that is used by the Input component
             * to determine what types of behaviors the app should have such as gunner positions, seats, weapons, ...
             */
            std::string GetEntityType() { return mEntityType; }

            void SetVehiclesSeatConfigActorName(const std::string& value) {mVehiclesSeatConfigActorName = value;}
            std::string GetVehiclesSeatConfigActorName() {return mVehiclesSeatConfigActorName;}

            /**
             * Set the articulation helper responsible for creating articulation
             * data used in entity update messages.
             */
            void SetArticulationHelper( Components::ArticulationHelper* articHelper );
            Components::ArticulationHelper* GetArticulationHelper() { return mArticHelper.get(); }
            const Components::ArticulationHelper* GetArticulationHelper() const { return mArticHelper.get(); }

            /**
             * Set the minimum time in seconds between control state updates.
             * This prevents flooding the network with many control state
             * messages spawning from slight movements.
             */
            void SetTimeBetweenControlStateUpdates( float timeBetweenUpdates ) { mTimeBetweenControlStateUpdates = timeBetweenUpdates; }
            float GetTimeBetweenControlStateUpdates() const { return mTimeBetweenControlStateUpdates; }

            /**
             * Set whether the headlights of the vehicle are on or off.
             */
            void SetHeadLightsEnabled( bool enabled );
            bool IsHeadLightsEnabled() const;

            /**
             * This function advances the update time and determines if an update
             * regarding a control state should be sent out to the network.
             * @param tickMessage Tick message sent to this object via registration
             *        of this function in an invokable mapped to TickRemote messages.
             */
            void TickControlState( const dtGame::Message& tickMessage );

            // for the engine idle sound effect
            void SetSFXEngineIdleLoop(const std::string& soundFX);
            void SetMaxDistanceIdleSound(float value) {mMaxIdleSoundDistance = value;}

            float GetMaxDistanceIdleSound() const {return mMaxIdleSoundDistance;}

            void TickTimerMessage(const dtGame::Message& tickMessage);

            /// @return true if this platform should be visible based on the options given.
            bool ShouldBeVisible(const SimCore::VisibilityOptions& options);

            /// Ticks are registered for on all local entities. Over ride for custom stuff.
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

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


            DT_DECLARE_ACCESSOR(osg::Vec3, EngineSmokePos);
            /// Toggles engine smoke on and off
            DT_DECLARE_ACCESSOR(bool, EngineSmokeOn);

           unsigned GetHeadlightId() const{ return mHeadLightID;}

           /// since it derives off vehicle interface
           virtual float GetMPH() const {return 0.0f;}

         protected:

            /**
             * Destructor.
             */
            virtual ~Platform();

            /// Called by tick local to see if an update should be sent and if it is a full or partial.
            //virtual bool ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            osg::Vec3 mMuzzleFlashPosition;

            // tests to see if we are in range, if so play the sound effect if its playing
            // else dont.
            void UpdateEngineIdleSoundEffect();

            void LoadSFXEngineIdleLoop();

            /// Does the work of loading a model file name into a node, assigning it to a parent group, and populating the node collector.
            bool LoadModelNodeInternal(osg::Group& modelNode, const std::string& fileName, const std::string& copiedNodeName);

            // Allows a sub-class to set the engine smoke value without doing all the engine
            // smoke 'stuff'.
            void InnerSetEngineSmokeOn(bool enable) { mEngineSmokeOn = enable; }

            // Helper function to call the one on the proxy.
            void EnsureResourcesAreLoaded();

         private:
            /// The minimum time allowed between control state updates
            float mTimeBetweenControlStateUpdates;

            /// The current time remaining before another control state update can be sent
            float mTimeUntilControlStateUpdate;

            /// Nodes representing the damagable model file nodes
            dtCore::RefPtr<osg::Group> mNonDamagedFileNode;
            dtCore::RefPtr<osg::Group> mDamagedFileNode;
            dtCore::RefPtr<osg::Group> mDestroyedFileNode;

            /// Node that switched model damagable states
            dtCore::RefPtr<osg::Switch> mSwitchNode;

            /// The particle systems used for fire and smoke
            dtCore::RefPtr<dtCore::ParticleSystem> mEngineSmokeSystem;
            /// The file names that are loaded into the above particle systems
            std::string mEngineSmokeSystemFile;

            std::string mVehiclesSeatConfigActorName;

            /// @return the z position of intersection of the given terrain node using the point as a line segment going 10000 units up and down.
            double GetTerrainZIntersectionPoint(dtCore::DeltaDrawable& terrainActor, const osg::Vec3& point);
            ///takes the time since an update, the current position and rotation and clamps it to the terrain for ground following.
            void ClampToGround(float timeSinceUpdate, osg::Vec3& position, osg::Matrix& rotation, dtCore::Transform& xform);

            /// @return the engine transform for the smoke
            osg::Vec3 GetEngineTransform();

            /// Internal method that causes the set damage state to actually happen.  Should prevent unnecessary changes to CollisionBox
            void InternalSetDamageState(PlatformActorProxy::DamageStateEnum &damageState);

            /// The name of the munition damage table as found in Configs/MunitionsConfig.xml
            std::string mMunitionTableName;

            // The entity type - used by apps to determine what type of behaviors the app should have.
            std::string mEntityType;

            // The articulation helper used in creating out-going articulation
            // data for entity update messages.
            dtCore::RefPtr<Components::ArticulationHelper> mArticHelper;

            // Flag for determining if the head light effect should be enabled or disabled
            bool mHeadLightsEnabled;

            // Handle to the light effect
            unsigned mHeadLightID;

            // For idle engine sounds, great for hearing things coming!
            dtCore::RefPtr<dtAudio::Sound>   mSndIdleLoop;
            std::string                      mSFXSoundIdleEffect; /// What is the filepath / string of the sound effect
            float                            mMinIdleSoundDistance;
            float                            mMaxIdleSoundDistance;

      };

   }
}

#endif // _PLATFORM_ACTOR_
