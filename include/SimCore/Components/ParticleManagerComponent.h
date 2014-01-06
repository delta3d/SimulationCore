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
 * @author Chris Rodgers
 */
#ifndef _PARTICLE_MANAGER_COMPONENT_H_
#define _PARTICLE_MANAGER_COMPONENT_H_

#include <string>
#include <SimCore/Export.h>
#include <dtUtil/enumeration.h>
#include <dtCore/particlesystem.h>
#include <dtUtil/refcountedbase.h>
#include <dtGame/gmcomponent.h>
#include <osgParticle/ForceOperator>

namespace dtGame
{
   class Message;
   class GameActorProxy;
}

namespace osgParticle
{
   class ParticleSystem;
}

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////
      // Particle Priority code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ParticlePriority : public dtUtil::Enumeration
      {
         DECLARE_ENUM(ParticlePriority);
         public:
            static const ParticlePriority LOW;
            static const ParticlePriority NORMAL;
            static const ParticlePriority HIGH;
         private:
            ParticlePriority(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
      };

      // Enter new attribute flags here.
      // These determine what forces will be applied to a particle
      // system when the component attempts an update.
      struct ParticleInfoAttributeFlags
      {
         bool mEnableWind;
         bool mAddWindToAllLayers; // forces addition of a wind force operator to particle layers that do not have one
      };

      //////////////////////////////////////////////////////////
      // Particle Info code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ParticleInfo : public dtCore::Base
      {
         public:
            static const std::string FORCE_WIND;

            typedef std::vector<std::weak_ptr<osgParticle::ForceOperator> >
               ForceOperatorList;

            ParticleInfo();

            ParticleInfo( dtCore::ParticleSystem& particles,
               const ParticleInfoAttributeFlags* attrFlags = nullptr,
               const ParticlePriority& priority = ParticlePriority::NORMAL );

            // Updates the info based on the referenced particle system.
            //@return TRUE if update was successful, FALSE if the reference
            // to the particle system is nullptr.
            bool Update();

            void Set( dtCore::ParticleSystem& particles,
               const ParticleInfoAttributeFlags* attrFlags = nullptr,
               const ParticlePriority& priority = ParticlePriority::NORMAL );

            dtCore::ParticleSystem* GetParticleSystem();
            const dtCore::ParticleSystem* GetParticleSystem() const;

            const ParticlePriority& GetPriority() const;

            unsigned int GetLiveCount() const;

            unsigned int GetDeadCount() const;

            // @return Total particles allocated in memory (live + dead)
            unsigned int GetAllocatedCount() const;

            ParticleInfoAttributeFlags& GetAttributeFlags();
            const ParticleInfoAttributeFlags& GetAttributeFlags() const;

            ForceOperatorList& GetWindForces();
            const ForceOperatorList& GetWindForces() const;

         protected:

            virtual ~ParticleInfo();

         private:
            const ParticlePriority* mPriority;
            unsigned int mLiveCount;
            unsigned int mDeadCount;
            ParticleInfoAttributeFlags mAttrFlags;

            std::weak_ptr<dtCore::ParticleSystem> mRef;
            std::vector< std::weak_ptr<osgParticle::ParticleSystem> > mLayerRefs;
            ForceOperatorList mWindForces;
      };

      //////////////////////////////////////////////////////////
      // Particle Info code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ActorInfo : public dtCore::Base
      {
         public:
            ActorInfo();

            ActorInfo( dtGame::GameActorProxy& proxy );

            // Updates the info based on the referenced particle system.
            //@return TRUE if update was successful, FALSE if the reference
            // to the particle system is nullptr.
            bool Update();

            void Set( dtGame::GameActorProxy& proxy );

            const dtGame::GameActorProxy* GetActor() const;

         protected:

            virtual ~ActorInfo();

         private:
            std::weak_ptr<dtGame::GameActorProxy> mRef;

      };

      //////////////////////////////////////////////////////////
      // Particle Manager Component code
      //////////////////////////////////////////////////////////
      /**
      * @class ParticleManagerComponent
      * @brief subclassed component used primarily to track local actors
      * spawned by remote actors. Example: particle systems from missiles
      * need to linger but need a timed delete.
      * All objects are referenced by unique ID.
      */
      class SIMCORE_EXPORT ParticleManagerComponent : public dtGame::GMComponent
      {
      public:
         // The default component name, used when looking it up on the GM.
         static const std::string DEFAULT_NAME;

         // Constructor
         // @param name The name by which this component is called from the GameManager
         ParticleManagerComponent( const std::string& name = DEFAULT_NAME );

         // Clean all allocated memory
         void Clear();

         // Clear and re-initialize
         void Reset();

         // Process certain massages, primarily looking for INFO_TIME_ELAPSED messages.
         // @param message Normal message received via the GameManager
         virtual void ProcessMessage( const dtGame::Message& message );

         // @param particles The particle system to be registered with this component
         // @param attributes Flags that determine what forces will be applied to registered particles
         // @param priority The priority that determines how the particle system should be handled.
         // @return bool TRUE if the particle system was registered
         bool Register( dtCore::ParticleSystem& particles,
            const ParticleInfoAttributeFlags* attrFlags = nullptr,
            const ParticlePriority& priority = ParticlePriority::NORMAL );

         // @param particles The particle system to be unregistered from this component
         // @return TRUE if the particle system had been registered and was removed.
         bool Unregister( const dtCore::ParticleSystem& particles );

         // @param particles The particle system id in question
         // @return bool TRUE if the particle system id is registered with this component
         bool HasRegistered( const dtCore::UniqueId& particlesId) const;

         // Sets a timer to regularly tell this component
         // to update its particle information.
         // Setting the value to 0 or lower will disable the update timer.
         // @param interval The frequency this component should update, measured in seconds.
         void SetUpdateInterval( double interval );

         double GetUpdateInterval() const;

         bool GetUpdateEnabled() const;

         void SetUpdateEnabled( bool enabled );

         // @return total count of all registered particles allocated
         unsigned int GetGlobalParticleCount() const;

         // Changes and/or removes particle info.
         // Info is removed if its weak reference to the particle system is nullptr.
         void UpdateParticleInfo();

         void UpdateParticleForces();

         // Applies a vector force to layers of the particle system that have operators
         // allocated to them for the effect. AddToAllLayers causes this function to
         // add the force operator to all layers that do not have it; otherwise, layers
         // that have relevant operators (like those named "wind" for receiving wind force)
         // will be the only layers affected by the applied force.
         // @param forceName The case-sensitive name of the force operator for which to search;
         //    if addToAllLayers is true, this will be the name of the operator when it is created.
         // @param force The physical vector force to be applied
         // @param ps The particle system that is to be affected by the force
         // @param addToAllLayers Creates an operator and adds the force to all layers that that do not have a relevant operator by the name of forceName.
         void ApplyForce( const std::string& forceName, const osg::Vec3& force, dtCore::ParticleSystem& ps, bool addToAllLayers = false );

         // Changes and/or removes particle info.;
         // Info is removed if its weak reference to the particle system is nullptr.
         void UpdateActorInfo();
         bool RegisterActor( dtGame::GameActorProxy& proxy );

         osg::Vec3 ConvertWorldToLocalForce( const osg::Vec3& globalForce, dtCore::Transformable& object,
            osg::Matrix& outWorldLocalMatrix );
         osg::Vec3 ConvertWorldToLocalForce( const osg::Vec3& globalForce, dtCore::Transformable& object );

         // Convenience function for special shader assignment. This is mostly used for NVG effects.
         static void AttachShaders(dtCore::ParticleSystem& ps);

      protected:

         // Destructor
         virtual ~ParticleManagerComponent();

         const std::string& GetUpdateTimerName() const { return mUpdateTimerName; }

//         void RegisterParticleSystemForceOperator( osgParticle::ForceOperator& );

      private:

         typedef std::map< dtCore::UniqueId, std::shared_ptr<ParticleInfo> > ParticleInfoMap;
         ParticleInfoMap mIdToInfoMap;

         std::map< dtCore::UniqueId, std::shared_ptr<ActorInfo> >     mIdToActorMap;

         unsigned int   mGlobalParticleCount;
         bool           mUpdateEnabled;
         double         mUpdateInterval;
         const std::string mUpdateTimerName;

         // Forces to be applied to registered particles
         osg::Vec3 mWind;
         bool mWindWasUpdated;
      };
   } // namespace
}// namespace

#endif
