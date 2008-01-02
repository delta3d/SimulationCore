/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Chris Rodgers
 */

#include <prefix/SimCorePrefix-src.h>
#include <dtCore/particlesystem.h>
#include <dtCore/shadermanager.h>
#include <dtGame/basemessages.h>
#include <dtGame/gameactor.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/NxAgeiaParticleSystemActor.h>
#include <SimCore/Actors/IGEnvironmentActor.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ModularProgram>
#include <osgParticle/Operator>
#include <osgParticle/ForceOperator>
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
      ParticleInfo::ParticleInfo( dtCore::ParticleSystem& particles, const AttributeFlags* attributes,
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
      void ParticleInfo::Set( dtCore::ParticleSystem& particles, const AttributeFlags* attributes,
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
         std::list<dtCore::ParticleLayer>& layers = particles.GetAllLayers();
         std::list<dtCore::ParticleLayer>::iterator itor = layers.begin();
         for( ; itor != layers.end(); ++itor )
         {
            osgParticle::ParticleSystem* ref = &itor->GetParticleSystem();
            mLayerRefs.push_back(ref);

            //attaching a shader to the particle, one for emmisive particles the other for non emmisive
            dtCore::ParticleLayer& pLayer = *itor;
            osg::StateSet* ss = pLayer.GetParticleSystem().getOrCreateStateSet();
            std::string shaderName;
            (ss->getMode(GL_LIGHTING) == osg::StateAttribute::ON) ? shaderName = "NonEmissive" : shaderName = "Emissive";
            AttachShader(shaderName, pLayer.GetGeode());


            //osg::Uniform* lightArray = ss->getOrCreateUniform("dynamicLights", osg::Uniform::FLOAT_VEC4, 60);

            //int count = 0;
            //for(;count < 20 * 3; count += 3)
            //{ 
            //   //else we turn the light off by setting the intensity to 0
            //   lightArray->setElement(count, osg::Vec4(1000.0f, 1000.0f, 1000000.0f, 10.0f));
            //   lightArray->setElement(count + 1, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
            //   lightArray->setElement(count + 2, osg::Vec4(0.00001f, 0.0000001f, 0.00000001f, 1.0f));
            //
            //}

         }

         //mPriority = &priority;
      }

      void ParticleInfo::AttachShader(const std::string& name, osg::Node& node)
      {
         dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();
         dtCore::ShaderGroup* sg = sm.FindShaderGroupPrototype("ParticleShaderGroup");
         if(sg)
         {
            dtCore::ShaderProgram* sp = sg->FindShader(name);

            if(sp != NULL)
            {
               sm.AssignShaderFromPrototype(*sp, node);
            }
            else
            {
               LOG_ERROR("Unable to find a particle system shader with the name '." + name + "' ");
            }
         }
         else
         {
            LOG_ERROR("Unable to find shader group for particle system manager.");
         }            
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
      ParticleInfo::AttributeFlags& ParticleInfo::GetAttributeFlags()
      {
         return mAttrFlags;
      }

      //////////////////////////////////////////////////////////
      const ParticleInfo::AttributeFlags& ParticleInfo::GetAttributeFlags() const
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
         : dtGame::GMComponent(name),
         mGlobalParticleCount(0),
         mUpdateEnabled(false),
         mUpdateInterval(5.0),
         mUpdateTimerName("ParticleMgrComp("+name+"):UpdateTimer")
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

            Actors::IGEnvironmentActorProxy* envProxy = 
               static_cast<Actors::IGEnvironmentActorProxy*> (env);

            Actors::IGEnvironmentActor* envActor = static_cast<Actors::IGEnvironmentActor*>
               (envProxy->GetActor());

            // Capture the wind force that must be applied to new
            // particle systems registered to this component.
            mWind = envActor->GetWind();

            // Update the physics particles wind....
#ifdef AGEIA_PHYSICS
            std::vector<dtDAL::ActorProxy*> toFill;
            GetGameManager()->FindActorsByClassName("NxAgeiaParticleSystemActor", toFill);
            if(!toFill.empty())
            {
               std::vector<dtDAL::ActorProxy*>::iterator toFillInIter = toFill.begin();
               for(; toFillInIter != toFill.end(); ++toFillInIter)
               {
                   NxAgeiaParticleSystemActor* currentParticleSystem = static_cast<NxAgeiaParticleSystemActor*>((*toFillInIter)->GetActor());
                   currentParticleSystem->SetOverTimeForceVecMin(mWind);
                   currentParticleSystem->SetOverTimeForceVecMax(mWind);
               }
            }
#endif
         }

         // Is this a global application state change?
         else if( msgType == dtGame::MessageType::INFO_MAP_UNLOADED )
         {
            // Release all particle info only, it will not remove the timer.
            // The timer will be removed on GameManager shutdown.
            Clear();
         }
         // Restarts is a special state change, slightly different from
         // complete shutdown. Reset is similar to Clear but is intended
         // to re-initialize this component as if it were new.
         else if( msgType == dtGame::MessageType::INFO_RESTARTED )
         {
            Reset();
         }
      }

      //////////////////////////////////////////////////////////
      bool ParticleManagerComponent::Register( dtCore::ParticleSystem& particles,
         const ParticleInfo::AttributeFlags* attrFlags,  const ParticlePriority& priority )
      {
         if( HasRegistered(particles.GetUniqueId()) 
            || particles.GetAllLayers().empty() )
         {
            return false;
         }

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
         bool forceInfoUpdate = false;
         dtCore::ParticleSystem* curParticles = NULL;
         ParticleInfo::ForceOperatorList* curForces = NULL;
         ParticleInfo* curInfo = NULL;
         ParticleInfoMap::iterator infoIter = mIdToInfoMap.begin();
         for( ; infoIter != mIdToInfoMap.end(); ++infoIter )
         {
            curInfo = infoIter->second.get();
            if( curInfo == NULL || curInfo->GetParticleSystem() == NULL )
            {
               // Ensure that the invalid particle info is removed by calling
               // UpdateParticleInfo at the end of this function. The update timer
               // may not have been activated and thus would not call updates on
               // all particle infos and not remove invalid ones. This makes certain
               // that any and all invalid infos are removed from the manager.
               forceInfoUpdate = true;

               // DEBUG:
               // std::cout << "\tINFO: " << (curInfo==NULL?"NULL":"OK")
               //   << "\tParticlePtr: " << (curInfo==NULL || curInfo->GetParticleSystem()==NULL?"NULL":"OK") << std::endl;
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

         // Force an update on particle infos if any were found to be invalid.
         if( forceInfoUpdate )
         {
            UpdateParticleInfo();
         }
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
         std::list<dtCore::ParticleLayer>& layers = ps.GetAllLayers();
         std::list<dtCore::ParticleLayer>::iterator itor = layers.begin();
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
      bool ParticleManagerComponent::RegisterActor( dtGame::GameActorProxy& proxy )
      {
         return mIdToActorMap.insert( 
            std::make_pair( proxy.GetId(), new ActorInfo( proxy ) ) 
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
      ActorInfo::ActorInfo( dtGame::GameActorProxy& proxy )
         :  dtCore::Base("ActorInfo")
      {
         // Using the set function in case more needs to be
         // done to the info based on the passed in particles.
         Set( proxy );
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
      void ActorInfo::Set( dtGame::GameActorProxy& proxy )
      {
         mRef = &proxy;
      }

      //////////////////////////////////////////////////////////
      const dtGame::GameActorProxy* ActorInfo::GetActor() const
      {
         return mRef.get();
      }

   }
}
