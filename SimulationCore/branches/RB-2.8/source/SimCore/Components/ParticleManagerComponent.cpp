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

#include <prefix/SimCorePrefix.h>
#include <dtCore/particlesystem.h>
#include <dtCore/shadermanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/gameactor.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <SimCore/Actors/EntityActorRegistry.h>
//#include <SimCore/Actors/NxAgeiaParticleSystemActor.h>
#include <SimCore/Actors/PhysicsParticleSystemActor.h>
#include <SimCore/Actors/IGEnvironmentActor.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ModularProgram>
#include <osgParticle/Operator>
#include <osgParticle/ForceOperator>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Matrix>
#include <osg/Node>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////
      // Particle Priority code
      //////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(ParticlePriority);
      const ParticlePriority ParticlePriority::LOW("LOW");
      const ParticlePriority ParticlePriority::NORMAL("NORMAL");
      const ParticlePriority ParticlePriority::HIGH("HIGH");

      //////////////////////////////////////////////////////////
      // Particle Info code
      //////////////////////////////////////////////////////////
      const std::string ParticleInfo::FORCE_WIND("Wind");

      //////////////////////////////////////////////////////////
      ParticleInfo::ParticleInfo()
         : dtCore::Base("ParticleInfo"),
         //mPriority(&ParticlePriority::NORMAL),
         mLiveCount(0),
         mDeadCount(0)
      {
         mAttrFlags.mEnableWind = false;
         mAttrFlags.mAddWindToAllLayers = false;
      }

      //////////////////////////////////////////////////////////
      ParticleInfo::ParticleInfo( dtCore::ParticleSystem& particles, const ParticleInfoAttributeFlags* attributes,
         const ParticlePriority& priority )
         :  dtCore::Base("ParticleInfo"),
         mLiveCount(0),
         mDeadCount(0)
      {
         // Using the set function in case more needs to be
         // done to the info based on the passed in particles.
         Set( particles, attributes, priority );
      }


      //////////////////////////////////////////////////////////
      ParticleInfo::~ParticleInfo()
      {
         mWindForces.clear();
      }

      //////////////////////////////////////////////////////////
      bool ParticleInfo::Update()
      {
         if( ! mRef.valid() )
         {
            return false;
         }

         // NOTE: the following comment block is the old code used for
         // a single observer to a Delta3D ParticleSystem. For some reason
         // the particles system does not register the observer. For now
         // this ParticleInfo holds observers to the individual OSG
         // particle systems contained by each Delta ParticleSystem layer.

         /*
         std::list<dtCore::ParticleLayer> &layers = mRef->GetAllLayers();

         // 100 is being used instead of max integer value.
         // This may seem like a hack, BUT total particles layers
         // per particle system should not go up into the tens anyway.
         if( layers.size() > 100 || layers.empty() ) { return false; }

         std::list<dtCore::ParticleLayer>::iterator itor = layers.begin();

         // Reset the last count
         mLiveCount = 0;
         mDeadCount = 0;

         // Loop through all particle layers and sum up
         // their individual particle counts
         unsigned int deadParticles = 0;
         dtCore::RefPtr<const osgParticle::ParticleSystem> ps;
         for( ; itor != layers.end(); ++itor )
         {
            ps = &itor->GetParticleSystem();
            deadParticles = ps->numDeadParticles();
            mDeadCount += deadParticles;
            mLiveCount += ps->numParticles() - deadParticles;
         }
         return true;//*/

         // Reset the last count
         mLiveCount = 0;
         mDeadCount = 0;

         // Loop through all particle layers and sum up
         // their individual particle counts
         const osgParticle::ParticleSystem* ps = NULL;
         unsigned int layersUpdated = 0;
         unsigned int deadParticles = 0;
         unsigned int limit = mLayerRefs.size(); // this avoids multiple function jumps
         for( unsigned int i = 0; i < limit; i++ )
         {
            if( ! mLayerRefs[i].valid() )
            {
               continue;
            }
            ps = mLayerRefs[i].get();
            deadParticles = ps->numDeadParticles();
            mDeadCount += deadParticles;
            mLiveCount += ps->numParticles() - deadParticles;
            layersUpdated++;
         }

         return layersUpdated > 0;
      }

      //////////////////////////////////////////////////////////
      void ParticleInfo::Set( dtCore::ParticleSystem& particles, const ParticleInfoAttributeFlags* attributes,
         const ParticlePriority& priority )
      {
         if( particles.GetAllLayers().empty() )
         {
            return;
         }

         mRef = &particles;

         // Set the attribute flags
         if( attributes != NULL )
         {
            mAttrFlags = *attributes;
         }
         else
         {
            mAttrFlags.mEnableWind = false;
         }

         mLayerRefs.clear();
         ParticleManagerComponent::AttachShaders(particles);

         dtCore::ParticleSystem::LayerList& layers = particles.GetAllLayers();
         dtCore::ParticleSystem::LayerList::iterator itor = layers.begin();
         for( ; itor != layers.end(); ++itor )
         {
            osgParticle::ParticleSystem* ref = &itor->GetParticleSystem();
            mLayerRefs.push_back(ref);
         }

         //mPriority = &priority;
      }

      //////////////////////////////////////////////////////////
      dtCore::ParticleSystem* ParticleInfo::GetParticleSystem()
      {
         return mRef.get();
      }

      //////////////////////////////////////////////////////////
      const dtCore::ParticleSystem* ParticleInfo::GetParticleSystem() const
      {
         return mRef.get();
      }

      //////////////////////////////////////////////////////////
      const ParticlePriority& ParticleInfo::GetPriority() const
      {
         return *mPriority;
      }

      //////////////////////////////////////////////////////////
      unsigned int ParticleInfo::GetLiveCount() const
      {
         return mLiveCount;
      }

      //////////////////////////////////////////////////////////
      unsigned int ParticleInfo::GetDeadCount() const
      {
         return mDeadCount;
      }

      //////////////////////////////////////////////////////////
      unsigned int ParticleInfo::GetAllocatedCount() const
      {
         return mDeadCount+mLiveCount;
      }

      //////////////////////////////////////////////////////////
      ParticleInfoAttributeFlags& ParticleInfo::GetAttributeFlags()
      {
         return mAttrFlags;
      }

      //////////////////////////////////////////////////////////
      const ParticleInfoAttributeFlags& ParticleInfo::GetAttributeFlags() const
      {
         return mAttrFlags;
      }

      //////////////////////////////////////////////////////////
      ParticleInfo::ForceOperatorList& ParticleInfo::GetWindForces()
      {
         return mWindForces;
      }

      //////////////////////////////////////////////////////////
      const ParticleInfo::ForceOperatorList& ParticleInfo::GetWindForces() const
      {
         return mWindForces;
      }



      //////////////////////////////////////////////////////////
      // Particle Manager Component code
      //////////////////////////////////////////////////////////
      const std::string ParticleManagerComponent::DEFAULT_NAME("ParticleManagerComponent");

      //////////////////////////////////////////////////////////
      ParticleManagerComponent::ParticleManagerComponent( const std::string& name )
         : dtGame::GMComponent(name)
         , mGlobalParticleCount(0)
         , mUpdateEnabled(true)
         , mUpdateInterval(3.0)
         , mUpdateTimerName("ParticleMgrComp("+name+"):UpdateTimer")
         , mWindWasUpdated(false)
      {

      }

      //////////////////////////////////////////////////////////
      ParticleManagerComponent::~ParticleManagerComponent()
      {

      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::Clear()
      {
         mGlobalParticleCount = 0;
         mIdToInfoMap.clear();
         mIdToActorMap.clear();

         // Normally the timer would be disabled here but
         // the GameManager will clear the timer when it
         // goes through shutdown. If the timer needs to
         // be stopped or removed prematurely, the user
         // should call SetUpdateEnabled(false).
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::Reset()
      {
         bool wasUpdateEnabled = mUpdateEnabled;
         Clear();

         // Keep the timer rolling, starting back over from 0
         // it had been running before the call to Clear.
         if( wasUpdateEnabled )
         {
            SetUpdateInterval( mUpdateInterval );
         }
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::ProcessMessage( const dtGame::Message& message )
      {
         // Use a local reference to the message type to
         // avoid too many function jumps with GetMassageType.
         const dtGame::MessageType& msgType = message.GetMessageType();

         // Avoid the most common messages that will not need to be processed
         if( msgType == dtGame::MessageType::TICK_LOCAL )
         {
            UpdateParticleForces();
            return;
         }
         if( msgType == dtGame::MessageType::TICK_REMOTE )
         {
            return;
         }

         // Update particle information if the timer has
         // reached a full cycle.
         if( msgType == dtGame::MessageType::INFO_TIMER_ELAPSED )
         {
            const dtGame::TimerElapsedMessage& timerMsg =
               static_cast<const dtGame::TimerElapsedMessage&>(message);

            // Update particle info if the elapsed timer is the
            // same timer controlling this component.
            if(mUpdateTimerName == timerMsg.GetTimerName() )
            {
               UpdateParticleInfo();
               //UpdateActorInfo(); // this is for observer pointer test purposes
            }
         }
         // Check for changes in environmental forces
         else if(
            msgType == dtGame::MessageType::INFO_ACTOR_UPDATED
            || msgType == dtGame::MessageType::INFO_ACTOR_CREATED )
         {
            // Ignore this message if possible
            dtGame::IEnvGameActorProxy* env = GetGameManager()->GetEnvironmentActor();
            if( env == NULL || env->GetId() != message.GetAboutActorId() )
            {
               return;
            }

            SimCore::Actors::IGEnvironmentActorProxy* envProxy =
               dynamic_cast<Actors::IGEnvironmentActorProxy*> (env);

            SimCore::Actors::IGEnvironmentActor* envActor;
            envProxy->GetActor(envActor);

            // Capture the wind force that must be applied to new
            // particle systems registered to this component.
            mWind = envActor->GetWind();
            mWindWasUpdated = true;

            // Update the physics particles wind....
            std::vector<dtCore::ActorProxy*> toFill;
            GetGameManager()->FindActorsByClassName("NxAgeiaParticleSystemActor", toFill);
            if (!toFill.empty())
            {
               std::vector<dtCore::ActorProxy*>::iterator toFillInIter = toFill.begin();
               for (; toFillInIter != toFill.end(); ++toFillInIter)
               {
                   SimCore::Actors::PhysicsParticleSystemActor* currentParticleSystem = NULL;
                   (*toFillInIter)->GetActor(currentParticleSystem);
                   currentParticleSystem->SetOverTimeForceVecMin(mWind);
                   currentParticleSystem->SetOverTimeForceVecMax(mWind);
               }
            }
         }

         // Is this a global application state change?
         else if (msgType == dtGame::MessageType::INFO_MAP_UNLOADED)
         {
            // Release all particle info only, it will not remove the timer.
            // The timer will be removed on GameManager shutdown.
            Clear();
         }
         // After map is loaded, reset our timer. Since map unload deletes all timers
         else if (msgType == dtGame::MessageType::INFO_MAP_LOADED)
         {
            // Resetting the update interval turns the timer back on if appropriate.
            SetUpdateInterval(mUpdateInterval);
            mWindWasUpdated = true; // cause an initial reset, just in case.
         }
         // Restarts is a special state change, slightly different from
         // complete shutdown. Reset is similar to Clear but is intended
         // to re-initialize this component as if it were new.
         else if (msgType == dtGame::MessageType::INFO_RESTARTED)
         {
            Reset();
         }
      }

      //////////////////////////////////////////////////////////
      bool ParticleManagerComponent::Register( dtCore::ParticleSystem& particles,
         const ParticleInfoAttributeFlags* attrFlags,  const ParticlePriority& priority )
      {
         if( HasRegistered(particles.GetUniqueId())
            || particles.GetAllLayers().empty() )
         {
            return false;
         }

		 particles.GetOSGNode()->setNodeMask(SimCore::Components::RenderingSupportComponent::DISABLE_SHADOW_NODE_MASK);

         bool success = mIdToInfoMap.insert(
            std::make_pair( particles.GetUniqueId(), new ParticleInfo( particles, attrFlags, priority ) )
            ).second;

         if( !success )
         {
            LOG_WARNING("Failure registering particle system. Particle system may have already been registered.");
         }

         // Determine if the particle system is effected by wind force
         if( attrFlags != NULL && attrFlags->mEnableWind )
         {
            // Apply the recent wind reported by the environment's update message.
            ApplyForce( ParticleInfo::FORCE_WIND, mWind, particles, attrFlags->mAddWindToAllLayers );
         }

         return success;
      }

      //////////////////////////////////////////////////////////
      bool ParticleManagerComponent::Unregister( const dtCore::ParticleSystem& particles )
      {
         std::map< dtCore::UniqueId, dtCore::RefPtr<ParticleInfo> >::iterator itor =
            mIdToInfoMap.find(particles.GetUniqueId());

         if( itor != mIdToInfoMap.end() )
         {
            mIdToInfoMap.erase(itor);
            return true;
         }
         return false;
      }


      //////////////////////////////////////////////////////////
      bool ParticleManagerComponent::HasRegistered( const dtCore::UniqueId& particlesId) const
      {
         return mIdToInfoMap.find(particlesId) != mIdToInfoMap.end();
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::SetUpdateInterval( double interval )
      {
         // Remove the old timer
         if( mUpdateEnabled )
         {
            GetGameManager()->ClearTimer( mUpdateTimerName, NULL );
         }

         // Set the new update interval (capped at 0 at lower limit)
         mUpdateEnabled = ! (interval <= 0.0);

         // Set the new timer
         if( mUpdateEnabled )
         {
            mUpdateInterval = interval;
            GetGameManager()->SetTimer( mUpdateTimerName, NULL, (float)interval, true );
         }
      }

      //////////////////////////////////////////////////////////
      double ParticleManagerComponent::GetUpdateInterval() const
      {
         return mUpdateInterval;
      }

      //////////////////////////////////////////////////////////
      bool ParticleManagerComponent::GetUpdateEnabled() const
      {
         return mUpdateEnabled;
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::SetUpdateEnabled( bool enabled )
      {
         SetUpdateInterval( enabled ? mUpdateInterval : 0.0 );
      }

      //////////////////////////////////////////////////////////
      unsigned int ParticleManagerComponent::GetGlobalParticleCount() const
      {
         return mGlobalParticleCount;
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::UpdateParticleInfo()
      {
         // Start global count from the beginning
         mGlobalParticleCount = 0;
         unsigned int totalSystems = 0;
         unsigned int totalSystemsRemoved = 0;

         std::map< dtCore::UniqueId, dtCore::RefPtr<ParticleInfo> >::iterator itor = mIdToInfoMap.begin();
         std::vector< dtCore::UniqueId > idsToDelete;

         // Update the component's report data by updating each ParticleInfo.
         // Collect the keys of any infos that fail update
         for( ; itor != mIdToInfoMap.end(); ++itor)
         {
            if( ! itor->second.valid() || ! itor->second->Update() )
            {
               idsToDelete.push_back(itor->first);
            }
            else
            {
               mGlobalParticleCount += itor->second->GetAllocatedCount();
               totalSystems++;
            }
         }

         // Delete the failed info entries
         for( unsigned int i = 0; i < idsToDelete.size(); i ++)
         {
            std::map< dtCore::UniqueId, dtCore::RefPtr<ParticleInfo> >::iterator itor =
               mIdToInfoMap.find(idsToDelete[i]);
            if (itor != mIdToInfoMap.end())
            {
               mIdToInfoMap.erase(itor);
               totalSystemsRemoved++;
            }
         }

         std::ostringstream oss;
         oss << "Total Particles: " << mGlobalParticleCount;
         LOG_INFO(oss.str());
         oss.str("");

         oss << "Total Systems Updated: " << totalSystems;
         LOG_INFO(oss.str());
         oss.str("");

         oss << "Total Systems Removed: " << totalSystemsRemoved;
         LOG_INFO(oss.str());
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::UpdateParticleForces()
      {
         // Nothing to do if the wind hasn't changed. Wind is applied when first registered.
         if (!mWindWasUpdated)
         {
            return;
         }

         //bool forceInfoUpdate = false;
         dtCore::ParticleSystem* curParticles = NULL;
         ParticleInfo::ForceOperatorList* curForces = NULL;
         ParticleInfo* curInfo = NULL;
         ParticleInfoMap::iterator infoIter = mIdToInfoMap.begin();
         for( ; infoIter != mIdToInfoMap.end(); ++infoIter )
         {
            curInfo = infoIter->second.get();
            if( curInfo == NULL || curInfo->GetParticleSystem() == NULL )
            {
               continue;
            }

            curParticles = curInfo->GetParticleSystem();
            curForces = &curInfo->GetWindForces();

            ParticleInfo::ForceOperatorList::iterator forceIter = curForces->begin();
            for( ; forceIter != curForces->end(); ++forceIter )
            {
               if( ! forceIter->valid() )
               {
                  // DEBUG: std::cout << "\tContinue 2" << std::endl;
                  continue;
               }

               // Change the force's
               (*forceIter)->setForce( ConvertWorldToLocalForce( mWind, *curParticles ) );

            } // End Forces Loop
         }// End Particle Infos Loop
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 ParticleManagerComponent::ConvertWorldToLocalForce(
         const osg::Vec3& globalForce, dtCore::Transformable& object, osg::Matrix& outWorldLocalMatrix )
      {
         osg::NodePathList nodePathList = object.GetMatrixNode()->getParentalNodePaths();
         if( ! nodePathList.empty() )
         {
            osg::NodePath& nodePath = nodePathList[0];
            outWorldLocalMatrix = osg::computeWorldToLocal(nodePath);

            osg::Vec3 worldPos = outWorldLocalMatrix.getTrans();

            float forceMag = globalForce.length();
            osg::Vec3 localForce( outWorldLocalMatrix.preMult( globalForce ) );
            localForce -= worldPos;
            localForce.normalize();
            localForce *= forceMag;
            return localForce;
         }
         // No other transforms have been computed so object must be
         // at the root of the scene; thus return the global force.
         return globalForce;
      }

      //////////////////////////////////////////////////////////
      osg::Vec3 ParticleManagerComponent::ConvertWorldToLocalForce(
         const osg::Vec3& globalForce, dtCore::Transformable& object )
      {
         osg::Matrix worldLocalMatrix;
         return ConvertWorldToLocalForce( globalForce, object, worldLocalMatrix );
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::ApplyForce(
         const std::string& forceName, const osg::Vec3& force, dtCore::ParticleSystem& ps, bool addToAllLayers )
      {
         dtCore::ParticleLayer* curLayer = NULL;
         osgParticle::ForceOperator* forceOp = NULL;
         ParticleInfo* info = NULL;

         ParticleInfoMap::iterator foundIter = mIdToInfoMap.find(ps.GetUniqueId());
         if( foundIter != mIdToInfoMap.end() )
         {
            info = foundIter->second.get();
         }

         // Go through all particle layers and apply the force to each
         dtCore::ParticleSystem::LayerList& layers = ps.GetAllLayers();
         dtCore::ParticleSystem::LayerList::iterator itor = layers.begin();
         for( ; itor != layers.end(); ++itor )
         {
            curLayer = &(*itor);

            if( ! curLayer->IsModularProgram() )
            {
               continue;
            }

            // Obtain the particle layer's modular program which
            // contains the force operators of the particle system.
            osgParticle::ModularProgram& program =
               static_cast<osgParticle::ModularProgram&> (curLayer->GetProgram());

            // Find the force operator matching forceName or an unused operator
            osgParticle::ForceOperator* uselessOp = NULL;
            unsigned int numOps = program.numOperators();
            for( unsigned int op = 0; op < numOps; op++ )
            {
               forceOp = dynamic_cast<osgParticle::ForceOperator*>
                  (program.getOperator(op));

               if( forceOp == NULL ) { continue; }

               const std::string& opName = forceOp->getName();
               if(opName != forceName)
               {
                  // Determine if the force operator is useless; this will mostlikely
                  // be a placeholder operator created in the particle editor just for
                  // this effect. Future particle editor will give the ability to apply
                  // names to these operators.
                  if( opName.empty()
                     && (forceOp->getForce().isNaN() || forceOp->getForce().length() == 0.0) )
                  {
                     // Remember this operator in case an operator is not found by the name of forceName
                     uselessOp = forceOp;
                  }
                  // This might not be the target operator, so make it ready for the next loop
                  forceOp = NULL;
               }
               else
               {
                  // The operator was found
                  break;
               }
            }

            bool newForce = false;
            // If a useless operator was found then use it.
            // NOTE: if this effect is not wanted, do not
            // put a 0-force, nameless operator in the particle system
            if( uselessOp != NULL && forceOp == NULL )
            {
               forceOp = uselessOp;
               forceOp->setName( forceName );

               // Even though this force is not actually "new", mark it as
               // new since it has not been used prior to calling this function.
               newForce = true;
            }
            // If the force operator was not found, then add it to the
            // modular program.
            else if( forceOp == NULL )
            {
               if( ! addToAllLayers )
               {
                  // Apply the force only to layers that have an operator
                  // allocated for wind. This allows for a particle system
                  // to be selective on which layers receive the applied
                  // force effect.
                  continue;
               }

               // Add the force since it is requested to be added.
               forceOp = new osgParticle::ForceOperator();
               program.addOperator(forceOp);
               forceOp->setName(forceName);
               newForce = true;
            }

            if( newForce && info != NULL )
            {
               info->GetWindForces().push_back(forceOp);
            }

            forceOp->setForce(force);
            forceOp->setEnabled(true);

            // Get ready for next loop
            forceOp = NULL;
         }
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::UpdateActorInfo()
      {
         // Start global count from the beginning
         unsigned int totalActors = 0;
         unsigned int totalActorsRemoved = 0;

         std::map< dtCore::UniqueId, dtCore::RefPtr<ActorInfo> >::iterator itor = mIdToActorMap.begin();
         std::vector< dtCore::UniqueId > idsToDelete;

         // Update the component's report data by updating each ParticleInfo.
         // Collect the keys of any infos that fail update
         for( ; itor != mIdToActorMap.end(); ++itor)
         {
            if( ! itor->second.valid() || ! itor->second->Update() )
            {
               idsToDelete.push_back(itor->first);
            }
            else
            {
               totalActors++;
            }
         }

         // Delete the failed info entries
         for( unsigned int i = 0; i < idsToDelete.size(); i ++)
         {
            std::map< dtCore::UniqueId, dtCore::RefPtr<ActorInfo> >::iterator itor =
               mIdToActorMap.find(idsToDelete[i]);
            if (itor != mIdToActorMap.end())
            {
               mIdToActorMap.erase(itor);
               totalActorsRemoved++;
            }
         }

         std::ostringstream oss;
         oss << "Total Actors Updated: " << totalActors;
         LOG_INFO(oss.str());
         oss.str("");

         oss << "Total Actors Removed: " << totalActorsRemoved;
         LOG_INFO(oss.str());
      }

      //////////////////////////////////////////////////////////
      void ParticleManagerComponent::AttachShaders(dtCore::ParticleSystem& ps)
      {
         dtCore::ParticleSystem::LayerList& layers = ps.GetAllLayers();
         dtCore::ParticleSystem::LayerList::iterator itor = layers.begin();
         for( ; itor != layers.end(); ++itor )
         {
            //osgParticle::ParticleSystem* ref = &itor->GetParticleSystem();

            //attaching a shader to the particle, one for emmisive particles the other for non emmisive
            dtCore::ParticleLayer& pLayer = *itor;
            osg::StateSet* ss = pLayer.GetParticleSystem().getOrCreateStateSet();
            std::string shaderName = (ss->getMode(GL_LIGHTING) == osg::StateAttribute::ON) ? "NonEmissive" : "Emissive";

            dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
            dtCore::ShaderGroup* sg = sm.FindShaderGroupPrototype("ParticleShaderGroup");
            if(sg)
            {
               dtCore::ShaderProgram* sp = sg->FindShader(shaderName);

               if(sp != NULL)
               {
                  sm.AssignShaderFromPrototype(*sp, pLayer.GetGeode());
               }
               else
               {
                  LOG_ERROR("Unable to find a particle system shader with the name '." + shaderName + "' ");
               }
            }
            else
            {
               LOG_ERROR("Unable to find shader group for particle system manager.");
            }
         }
      }

      //////////////////////////////////////////////////////////
      bool ParticleManagerComponent::RegisterActor( dtGame::GameActorProxy& actor )
      {
         return mIdToActorMap.insert(
            std::make_pair( actor.GetId(), new ActorInfo( actor ) )
            ).second;
      }



      //////////////////////////////////////////////////////////
      // Actor Info code
      //////////////////////////////////////////////////////////
      ActorInfo::ActorInfo()
         : dtCore::Base("ActorInfo")
      {

      }

      //////////////////////////////////////////////////////////
      ActorInfo::ActorInfo( dtGame::GameActorProxy& actor )
         :  dtCore::Base("ActorInfo")
      {
         // Using the set function in case more needs to be
         // done to the info based on the passed in particles.
         Set( actor );
      }


      //////////////////////////////////////////////////////////
      ActorInfo::~ActorInfo()
      {
      }

      //////////////////////////////////////////////////////////
      bool ActorInfo::Update()
      {
         return mRef.valid();
      }

      //////////////////////////////////////////////////////////
      void ActorInfo::Set( dtGame::GameActorProxy& actor )
      {
         mRef = &actor;
      }

      //////////////////////////////////////////////////////////
      const dtGame::GameActorProxy* ActorInfo::GetActor() const
      {
         return mRef.get();
      }

   }
}
