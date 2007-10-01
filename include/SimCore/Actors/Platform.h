/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2006 Alion Science and Technology, BMH Operation
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

namespace dtCore
{
   class ParticleSystem;
   class NodeCollector;
}

namespace dtDAL
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

            static const std::string PROPERTY_HEAD_LIGHTS_ENABLED;
            
            // Constructor
            PlatformActorProxy();

            /**
             * Loads in the non damaged model for an entity
             * @param fileName The name of the file to load
             */
            void LoadNonDamagedFile(const std::string &fileName);

            /**
             * Loads in the damaged model for an entity
             * @param fileName The name of the file to load
             */
            void LoadDamagedFile(const std::string &fileName);

            /**
             * Loads in the destroyed model for an entity
             * @param fileName The name of the file to load
             */
            void LoadDestroyedFile(const std::string &fileName);

            /**
             * Initializes the actual actor the derived proxy will represent
             * By default this will create an Platform. This should be 
             * overridden in all subclasses of the Platform class
             */
            virtual void CreateActor();

            /**
             * Build the properties common to all platform objects
             */
            virtual void BuildPropertyMap();

            /// Adds additional invokables for this class.
            virtual void BuildInvokables();
            
            /**
             * Returns the icon to be shown in the level editor
             * for this proxy (PARTICLE_SYSTEM)
             * @return mBillboardIcon
             */
            virtual dtDAL::ActorProxyIcon* GetBillboardIcon();

         protected:
            // Destructor
            virtual ~PlatformActorProxy();

            dtCore::RefPtr<dtDAL::ActorProxyIcon> mBillboardIcon;
      };

      /**
       * A DIS/RPR-FOM entity.
       */
      class SIMCORE_EXPORT Platform : public BaseEntity
      {
         public:
            typedef BaseEntity BaseClass;

            static const std::string INVOKABLE_TICK_CONTROL_STATE;

            /**
             * Constructor.
             *
             * @param name the instance name
             */
            Platform(dtGame::GameActorProxy &proxy);

            /**
             * Generic method for loading damagable files
             * @param fileName The file to load
             * @param state The dtHLA::DamageState that the file represents
             */
            void LoadDamageableFile(const std::string &fileName, PlatformActorProxy::DamageStateEnum &state);

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
            * /brief    Purpose :  to load the dofs for the dofcontainer class
            *           Outs     : filled in mNodeCollector memb vars
            */
            void LoadNodeCollector(osg::Node* dofModel);

            /// Get the node utility class for hotspots and dofs
            dtCore::NodeCollector*  GetNodeCollector();

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
            void SetArticulatedParametersArray(const dtDAL::NamedGroupParameter& newValue);
            /// @return the group parameter for articulations.  This is for supporting the property.
            dtCore::RefPtr<dtDAL::NamedGroupParameter> GetArticulatedParametersArray(); 

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

         protected:

            /**
             * Destructor.
             */
            virtual ~Platform();
            
            /// overridden from base class to setup the nodes to draw or not.
            virtual void HandleModelDrawToggle(bool draw);

            /// Called by tick local to see if an update should be sent and if it is a full or partial.
            virtual bool ShouldForceUpdate(const osg::Vec3& pos, const osg::Vec3& rot, bool& fullUpdate);

            /// Called by tick local when sending a partial update to get a list of the properties to send.
            virtual void FillPartialUpdatePropertyVector(std::vector<std::string>& propNamesToFill);

            osg::Vec3 mMuzzleFlashPosition;

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

            std::string mVehiclesSeatConfigActorName;

            /// @return the z position of intersection of the given terrain node using the point as a line segment going 10000 units up and down.
            double GetTerrainZIntersectionPoint(dtCore::DeltaDrawable& terrainActor, const osg::Vec3& point);
            ///takes the time since an update, the current position and rotation and clamps it to the terrain for ground following.
            void ClampToGround(float timeSinceUpdate, osg::Vec3& position, osg::Matrix& rotation, dtCore::Transform& xform);

            /// @return the engine transform for the smoke
            osg::Vec3 GetEngineTransform();

            /// Internal method that causes the set damage state to actually happen.  Should prevent unnecessary changes to CollisionBox
            void InternalSetDamageState(PlatformActorProxy::DamageStateEnum &damageState);

            /// The entities main dof container that objects get moved around with 
            dtCore::RefPtr<dtCore::NodeCollector> mNodeCollector;

            /// The name of the munition damage table as found in Configs/MunitionsConfig.xml
            std::string mMunitionTableName;

            // The entity type - used by apps to determine what type of behaviors the app should have.
            std::string mEntityType; 

            // The articulation helper used in creating out-going articulation
            // data for entity update messages.
            dtCore::RefPtr<Components::ArticulationHelper> mArticHelper;
            
            // Flag for determining 
            bool mHeadLightsEnabled;
      };

   }
}

#endif // _PLATFORM_ACTOR_
