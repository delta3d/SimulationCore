/* -*-c++-*-
* Simulation Core - ParticleManagerComponentTests (.h & .cpp) - Using 'The MIT License'
* Copyright (C) 2007-2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*
* @author Chris Rodgers
*/
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <dtABC/application.h>

#include <dtActors/engineactorregistry.h>

#include <dtCore/system.h>
#include <dtCore/refptr.h>
#include <dtCore/scene.h>
#include <dtCore/uniqueid.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/particlesystem.h>
#include <dtCore/transformable.h>

#include <dtCore/project.h>

#include <dtCore/transform.h>

#include <dtGame/basemessages.h>
#include <dtGame/actorupdatemessage.h>
#include <dtGame/message.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>

#include <dtUtil/exception.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/datapathutils.h>

#include <SimCore/Components/ParticleManagerComponent.h> // includes IGEnvironmentActor
#include <SimCore/MessageType.h>

#include <SimCore/Actors/UniformAtmosphereActor.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/IGEnvironmentActor.h>

#include <osgParticle/ParticleSystem>
#include <osgParticle/ModularProgram>
#include <osgParticle/Operator>
#include <osgParticle/ForceOperator>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <UnitTestMain.h>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // PARTICLE LAYER REF CODE
      //////////////////////////////////////////////////////////////////////////
      class ParticleLayerRef : public dtCore::Base
      {
         public:
            ParticleLayerRef( dtCore::ParticleLayer& layer )
               : mGeode(&layer.GetGeode()),
               mXform(&layer.GetEmitterTransform()),
               mEmitter(&layer.GetModularEmitter()),
               mParticles(&layer.GetParticleSystem())
            {
            }

            const osg::Geode* GetGeode() const { return mGeode.get(); }
            const osg::MatrixTransform* GetTransform() const { return mXform.get(); }
            const osgParticle::ModularEmitter* GetEmitter() const { return mEmitter.get(); }
            const osgParticle::ParticleSystem* GetParticleSystem() const { return mParticles.get(); }

         protected:
            virtual ~ParticleLayerRef() {}

         private:
            dtCore::ObserverPtr<osg::Geode> mGeode;
            dtCore::ObserverPtr<osg::MatrixTransform> mXform;
            dtCore::ObserverPtr<osgParticle::ModularEmitter> mEmitter;
            dtCore::ObserverPtr<osgParticle::ParticleSystem> mParticles;
      };



      //////////////////////////////////////////////////////////////////////////
      // Testable Sub-classed Component
      // (allows public access to protected functions)
      //////////////////////////////////////////////////////////////////////////

      class TestParticleManagerComponent : public ParticleManagerComponent
      {
         public:

            TestParticleManagerComponent();

            const std::string& GetUpdateTimerName() const
            {
               return ParticleManagerComponent::GetUpdateTimerName();
            }

         protected:

            virtual ~TestParticleManagerComponent();

         private:
      };

      //////////////////////////////////////////////////////////////////////////
      TestParticleManagerComponent::TestParticleManagerComponent()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      TestParticleManagerComponent::~TestParticleManagerComponent()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      // Tests Object
      //////////////////////////////////////////////////////////////////////////

      class ParticleManagerComponentTests : public CPPUNIT_NS::TestFixture
      {
         CPPUNIT_TEST_SUITE(ParticleManagerComponentTests);

         CPPUNIT_TEST(TestProperties); // This will also call tests for ParticleInfo.
         CPPUNIT_TEST(TestMessageProcessing);
         CPPUNIT_TEST(TestForceOrientations);

         CPPUNIT_TEST_SUITE_END();

         public:

            void setUp();
            void tearDown();

            // Convenience function for simulating time advancement
            void AdvanceTime( float timeDelta );

            void CreateEnvironmentActor( dtCore::RefPtr<SimCore::Actors::IGEnvironmentActorProxy>& ptr );
            // NOTE: spawnParticles causes a time delay and ticks the system
            // so that the particle system has both live and dead particles.
            // Avoid spawning particles as much as possible because this slows
            // down the unit tests.
            void CreateParticleSystem( dtCore::RefPtr<dtCore::ParticleSystem>& ptr,
               bool spawnParticles = false );

            void AddParticlesToScene( dtCore::ParticleSystem& ps );
            void RemoveParticlesFromScene( dtCore::ParticleSystem& ps );

            bool ParticleSystemHasForce( const std::string& forceName,
               dtCore::ParticleSystem& ps, osg::Vec3* outForce = NULL );

            void TestParticleInfo();
            void TestProperties();
            void TestMessageProcessing();
            void TestForceOrientations();

            typedef std::vector<dtCore::RefPtr<ParticleLayerRef> > ParticleLayerRefList;
            void GetParticleLayerRefs( dtCore::ParticleSystem& particles, ParticleLayerRefList& outLayerRefs );
            void SubTestParticleLayerRefs( const ParticleLayerRefList& layerRefs );

         protected:
         private:

            dtCore::RefPtr<dtGame::GameManager> mGM;
            dtCore::RefPtr<TestParticleManagerComponent> mParticleComp;
            dtCore::RefPtr<dtGame::MachineInfo> mMachineInfo;
            dtCore::RefPtr<dtABC::Application> mApp;

            dtCore::RefPtr<dtCore::ParticleSystem> mPS;
            dtCore::RefPtr<dtCore::ParticleSystem> mPS2;
            dtCore::RefPtr<dtCore::ParticleSystem> mPS3;
      };

      CPPUNIT_TEST_SUITE_REGISTRATION(ParticleManagerComponentTests);


      //////////////////////////////////////////////////////////////////////////
      // Tests code
      //////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::setUp()
      {
         try
         {
            dtCore::System::GetInstance().Start();
            dtCore::System::GetInstance().SetPause(false);

            mApp = &GetGlobalApplication();
            mGM = new dtGame::GameManager(*mApp->GetScene());
            mGM->SetApplication(*mApp);

            mMachineInfo = new dtGame::MachineInfo;
            mParticleComp = new TestParticleManagerComponent;

            mGM->AddComponent(*mParticleComp, dtGame::GameManager::ComponentPriority::NORMAL);
         }
         catch (const dtUtil::Exception& ex)
         {
            CPPUNIT_FAIL(ex.ToString());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::tearDown()
      {
         dtCore::System::GetInstance().Stop();

         mPS = NULL;
         mPS2 = NULL;
         mPS3 = NULL;

         if (mGM.valid())
         {
            mGM->DeleteAllActors(true);
         }

         mGM = NULL;
         mApp = NULL;
         mMachineInfo = NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::AdvanceTime( float timeDelta )
      {
         double targetTime = dtCore::System::GetInstance().GetSimulationTime() + timeDelta;
         //Speed it way up so it this doesn't take long in a real time sense.
         dtCore::System::GetInstance().SetTimeScale(60.0);
         while (dtCore::System::GetInstance().GetSimulationTime() < targetTime)
         {
            dtCore::System::GetInstance().Step();
         }
         dtCore::System::GetInstance().SetTimeScale(1.0);
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::CreateParticleSystem( dtCore::RefPtr<dtCore::ParticleSystem>& ptr, bool spawnParticles )
      {
         ptr = new dtCore::ParticleSystem("TestParticleSystem");
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem must be obtainable from file", ptr.valid() );

         dtCore::Project& project = dtCore::Project::GetInstance();
         std::string path = project.GetResourcePath(dtCore::ResourceDescriptor("Particles:unittestparticles.osg"));

         CPPUNIT_ASSERT(path != (project.GetContext() + "/"));

         CPPUNIT_ASSERT(ptr->LoadFile(path) != NULL);

         CPPUNIT_ASSERT_MESSAGE("Particles should be valid", ptr.valid() );

         // Automatically insert the particles into the scene so that they can be ticked
         AddParticlesToScene(*ptr);

         if( spawnParticles )
         {
            bool success = false;

            const dtCore::ParticleSystem::LayerList& layers = ptr->GetAllLayers();

            CPPUNIT_ASSERT(!layers.empty());

            const osgParticle::ParticleSystem& ps = layers.begin()->GetParticleSystem();
            unsigned int totalAttempts = 20;
            for( unsigned int attempt = 0; attempt < totalAttempts; ++attempt )
            {
               AdvanceTime(0.5f); // tick to generate some particles

               if( ps.numParticles() > 0 )
               {
                  success = true;
                  break;
               }
            }

            if( !success )
            {
               std::cout << "New test particle system did not generate particles. This may cause a test to fail."
                  << std::endl;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::AddParticlesToScene( dtCore::ParticleSystem& ps )
      {
         mGM->GetScene().AddChild(&ps);
         dtCore::RefPtr<dtGame::Message> msg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED,msg);
         msg->SetAboutActorId(ps.GetUniqueId());
         mGM->SendMessage(*msg);
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::RemoveParticlesFromScene( dtCore::ParticleSystem& ps )
      {
         ps.Emancipate();
      }


      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::CreateEnvironmentActor(
         dtCore::RefPtr<SimCore::Actors::IGEnvironmentActorProxy>& ptr )
      {
         mGM->CreateActor( *SimCore::Actors::EntityActorRegistry::ENVIRONMENT_ACTOR_TYPE, ptr );
         CPPUNIT_ASSERT_MESSAGE("IGEnvironmentActor must be obtainable from the EntityActorRegistry", ptr.valid() );
         mGM->SetEnvironmentActor(ptr.get());
         dtCore::System::GetInstance().Step();
      }

      //////////////////////////////////////////////////////////////////////////
      // NOTE: Much of the code in this function has been copied
      // from ParticleManagerComponent::ApplyForce().
      bool ParticleManagerComponentTests::ParticleSystemHasForce(
         const std::string& forceName,
         dtCore::ParticleSystem& ps, osg::Vec3* outForce )
      {
         dtCore::ParticleLayer* curLayer = NULL;
         osgParticle::ForceOperator* forceOp = NULL;

         // Go through all particle layers and apply the force to each
         dtCore::ParticleSystem::LayerList& layers = ps.GetAllLayers();
         dtCore::ParticleSystem::LayerList::iterator itor = layers.begin();
         for( ; itor != layers.end(); ++itor )
         {
            curLayer = &(*itor);

            if( ! curLayer->IsModularProgram() ) { continue; }

            // Obtain the particle layer's modular program which
            // contains the force operators of the particle system.
            osgParticle::ModularProgram& program =
               static_cast<osgParticle::ModularProgram&> (curLayer->GetProgram());

            // Find the force operator matching forceName or an unused operator
            unsigned int numOps = program.numOperators();
            for( unsigned int op = 0; op < numOps; op++ )
            {
               forceOp = dynamic_cast<osgParticle::ForceOperator*>
                  (program.getOperator(op));

               if( forceOp == NULL ) { continue; }

               if(forceOp->getName() == forceName)
               {
                  if( outForce != NULL )
                  {
                     *outForce = forceOp->getForce();
                  }
                  return true;
               }
            }
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::TestParticleInfo()
      {
         // Create the particle system with particles already ticked into existence.
         CreateParticleSystem(mPS,true);

         dtCore::RefPtr<ParticleInfo> info = new ParticleInfo;

         // NOTE: commented out sections are place holders for unused functions
         // that will be implemented.

         // Test default values
         CPPUNIT_ASSERT_MESSAGE("Default particles should be NULL",
            info->GetParticleSystem() == NULL );
//         CPPUNIT_ASSERT_MESSAGE("Default particle priority should be NORMAL",
//            info->GetPriority() == ParticlePriority::NORMAL );
         CPPUNIT_ASSERT_MESSAGE("Default live particle count should be 0",
            info->GetLiveCount() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Default dead particle count should be 0",
            info->GetDeadCount() == 0 );
         CPPUNIT_ASSERT_MESSAGE("Default allocated particle count should be 0",
            info->GetAllocatedCount() == 0 );
         ParticleInfoAttributeFlags* flags = &info->GetAttributeFlags();
         CPPUNIT_ASSERT_MESSAGE("Default flags should be false",
            !flags->mEnableWind );

         // Test set values
         ParticleInfoAttributeFlags flags2 = {true,true};
         info->Set( *mPS, &flags2, ParticlePriority::LOW );
//         CPPUNIT_ASSERT_MESSAGE("ParticleInfo particles should be valid",
//            info->GetParticleSystem() != NULL );
//         CPPUNIT_ASSERT_MESSAGE("Set particle priority should be LOW",
//            info->GetPriority() == ParticlePriority::NORMAL );
         flags = &info->GetAttributeFlags();
         CPPUNIT_ASSERT_MESSAGE("Set flags should be true",
            flags->mEnableWind );

         // Simulate timed particle spawn and death over 0.2 seconds
         AdvanceTime(0.2f);
         // Let the particle info record changes in the particle system.
         info->Update();

         // Advance again just enough to have some particles age
         AdvanceTime(0.05f);
         info->Update();

         // Advance again so that some older particle die
         AdvanceTime(0.05f);
         info->Update();

         unsigned int live = info->GetLiveCount(),
                      dead = info->GetDeadCount();
         CPPUNIT_ASSERT_MESSAGE("Set live particle count should be 0",
            live > 0 );
         // Dead particle count cannot be assumed to be greater than 0; dependent on osg
         CPPUNIT_ASSERT_MESSAGE("Set allocated particle count should be 0",
            info->GetAllocatedCount() == live + dead );
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::TestProperties()
      {
         dtCore::RefPtr<SimCore::Actors::IGEnvironmentActorProxy> envProxy ;
         CreateEnvironmentActor( envProxy );
         SimCore::Actors::IGEnvironmentActor* env = dynamic_cast<SimCore::Actors::IGEnvironmentActor*>
            (envProxy->GetDrawable());
         std::string forceName("Wind");
         osg::Vec3 force(-1.0,2.0,3.4);
         env->SetWind(force);

         // Notify this component that the environment actor's wind has changed.
         dtCore::RefPtr<dtGame::ActorUpdateMessage> msg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED,msg);
         msg->SetAboutActorId(envProxy->GetId());
         mGM->SendMessage(*msg);
         dtCore::System::GetInstance().Step();

         // Test a single particle info, not contained in the component.
         // This call also sets up the initial particle system.
         TestParticleInfo();

         // Verify the particle system is NOT already registered.
         CPPUNIT_ASSERT_MESSAGE("Particles should NOT already be registered.",
            ! mParticleComp->HasRegistered(mPS->GetUniqueId()) );
         CPPUNIT_ASSERT_MESSAGE("Global particle count should be 0.",
            mParticleComp->GetGlobalParticleCount() == 0 );

         // Verify that the particle system has no forces applied to it.
         CPPUNIT_ASSERT_MESSAGE("Particles should NOT have forces.",
            ! ParticleSystemHasForce(forceName,*mPS) );

         // Register the particle system with the component, WITHOUT forces.
         // --- DisableWind && Do NOT AddWindToAllLayers
         ParticleInfoAttributeFlags flags = {false,false};
         CPPUNIT_ASSERT_MESSAGE("Particles registration should be successful.",
            mParticleComp->Register(*mPS,&flags) );
         // --- Make sure registering again fails
         CPPUNIT_ASSERT_MESSAGE("Registering the same particle system again should fail if already registered.",
            ! mParticleComp->Register(*mPS,&flags) );

         // Verify that the particle system IS registered.
         CPPUNIT_ASSERT_MESSAGE("Particles SHOULD already be registered.",
            mParticleComp->HasRegistered(mPS->GetUniqueId()) );

         // Verify that the global count of particles has been calculated.
         mParticleComp->UpdateParticleInfo();
         CPPUNIT_ASSERT_MESSAGE("Global particle count should be greater than 0.",
            mParticleComp->GetGlobalParticleCount() > 0 );

         // Unregister the particle system.
         CPPUNIT_ASSERT_MESSAGE("Particles should NOT be registered.",
            mParticleComp->Unregister(*mPS) );
         // --- Removing again should fail.
         CPPUNIT_ASSERT_MESSAGE("Unregistering particles should fail if the particle system is not currently registered.",
            ! mParticleComp->Unregister(*mPS) );

         // Verify that the particle system was unregistered.
         CPPUNIT_ASSERT_MESSAGE("Particles should NOT be registered.",
            ! mParticleComp->HasRegistered(mPS->GetUniqueId()) );

         // Verify that the global count of particles is now zero.
         mParticleComp->UpdateParticleInfo();
         CPPUNIT_ASSERT_MESSAGE("Global particle count should be 0.",
            mParticleComp->GetGlobalParticleCount() == 0 );

         // Re-register the particle system WITH forces applied.
         // --- EnableWind && AddWindToAllLayers
         flags.mEnableWind = flags.mAddWindToAllLayers = true;
         CPPUNIT_ASSERT_MESSAGE("Particles registration should be successful.",
            mParticleComp->Register(*mPS,&flags) );

         // Verify that the particle system was registered again.
         CPPUNIT_ASSERT_MESSAGE("Particles SHOULD already be registered.",
            mParticleComp->HasRegistered(mPS->GetUniqueId()) );

         // Verify that the particle system has indeed had forces applied
         // and that the force is the same as the one that was specified.
         osg::Vec3 currentForce;
         CPPUNIT_ASSERT_MESSAGE("Particles should have the correct force applied.",
            ParticleSystemHasForce( forceName,*mPS,&currentForce )
            && currentForce == force );
      }

      //////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::TestMessageProcessing()
      {
         // Test the component's update interval timer
         // --- Test default values
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ParticleManagerComponent should have a default update interval of 3.0 seconds.",
                  3.0f, mParticleComp->GetUpdateInterval(), 1e-3f );
         CPPUNIT_ASSERT_MESSAGE("ParticleManagerComponent should have update timer enabled by default.",
            mParticleComp->GetUpdateEnabled() );

         // --- Test enabling by boolean
         mParticleComp->SetUpdateEnabled(true);
         CPPUNIT_ASSERT_MESSAGE("ParticleManagerComponent should now be enabled after setting it to enabled",
            mParticleComp->GetUpdateEnabled() );
         mParticleComp->SetUpdateInterval(0.0f);
         CPPUNIT_ASSERT_MESSAGE("ParticleManagerComponent should now be disabled after setting the update interval to 0.0.",
            ! mParticleComp->GetUpdateEnabled() );

         // --- Test enabling by time
         float interval = 0.25;
         mParticleComp->SetUpdateInterval(interval);
         CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ParticleManagerComponent should have a NEW update interval of 0.25 seconds.",
                  interval, mParticleComp->GetUpdateInterval(), 1e-3f );
         CPPUNIT_ASSERT_MESSAGE("ParticleManagerComponent should now be enabled after setting a valid update interval",
            mParticleComp->GetUpdateEnabled() );


         // NOTE: The following tests will test removal of particle systems as well as
         // the component's update timer functionality. The component does not listen for
         // deletes of the particles individually, but rather waits for the update timer
         // to cycle before it updates all its references to the particles systems.
         // If the weak pointers to the particle systems go NULL, the component will
         // remove the weak references and updates global particle info accordingly.
         // Advancement of time should allow the timer to cycle and send the tick messages.

         // Create the particle system with particles already ticked into existence.
         CreateParticleSystem(mPS,true);
         CreateParticleSystem(mPS2,true);
         CreateParticleSystem(mPS3,true);
         dtCore::UniqueId id1 = mPS->GetUniqueId();
         dtCore::UniqueId id2 = mPS2->GetUniqueId();
         dtCore::UniqueId id3 = mPS3->GetUniqueId();

         AdvanceTime(0.5f);

         // Get references to the particle layers to make sure they get deleted
         // when the entire particle system is deleted.
         ParticleLayerRefList layerRefs1;
         ParticleLayerRefList layerRefs2;
         ParticleLayerRefList layerRefs3;
         GetParticleLayerRefs( *mPS, layerRefs1 );
         GetParticleLayerRefs( *mPS2, layerRefs2 );
         GetParticleLayerRefs( *mPS3, layerRefs3 );


         // Prepare a message that will simulate the update timer elapsing.
         dtCore::RefPtr<dtGame::TimerElapsedMessage> timerMsg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_TIMER_ELAPSED,timerMsg);
         timerMsg->SetTimerName(mParticleComp->GetUpdateTimerName());

         // Add a few particle systems (3) to the component
         mParticleComp->Register(*mPS);
         mParticleComp->Register(*mPS2);
         mParticleComp->Register(*mPS3);

         // Verify that each particle system has been added.
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should be registered.",
            mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should be registered.",
            mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            mParticleComp->HasRegistered(id3));

         // Get the total particle count
         mParticleComp->ProcessMessage(*timerMsg);
         float totalParticles = mParticleComp->GetGlobalParticleCount();
         float lastTotal = totalParticles;
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            totalParticles > 0 );

         // NOTE: About setting the 3 tested particle systems to NULL...
         // With the new OSG 2.2, a system step is required so that the
         // OSG particle systems can be garbage collected when the containing
         // Delta particle system object goes NULL. It seems the OSG garbage
         // collection is either asynchronous or running a frame behind than
         // what is expected.
         // An OSG CullVisitor has been known to temporarily reference the
         // OSG particle systems and thus irregularly prevents them from
         // being deleted in the subsequent tests.

         // Send particle system delete messages out of order
         RemoveParticlesFromScene(*mPS2);
         mPS2 = NULL;
         dtCore::System::GetInstance().Step();
         SubTestParticleLayerRefs( layerRefs2 );
         mParticleComp->ProcessMessage(*timerMsg);
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should NOT be registered.",
            ! mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should be registered.",
            mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            mParticleComp->HasRegistered(id3));

         // --- Get updated global particle total
         mParticleComp->ProcessMessage(*timerMsg);
         totalParticles = mParticleComp->GetGlobalParticleCount();
         CPPUNIT_ASSERT_MESSAGE("Total particle count should have been reduced when a particle system was removed",
            totalParticles < lastTotal );
         lastTotal = totalParticles;

         // --- Remove another particle system
         RemoveParticlesFromScene(*mPS);
         mPS = NULL;
         dtCore::System::GetInstance().Step();
         SubTestParticleLayerRefs( layerRefs1 );
         mParticleComp->ProcessMessage(*timerMsg);
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should NOT be registered.",
            ! mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            mParticleComp->HasRegistered(id3));

         // --- Get updated global particle total
         mParticleComp->ProcessMessage(*timerMsg);
         totalParticles = mParticleComp->GetGlobalParticleCount();
         CPPUNIT_ASSERT_MESSAGE("Total particle count should have been reduced when a particle system was removed",
            totalParticles < lastTotal );
         lastTotal = totalParticles;

         // --- Remove another particle system
         RemoveParticlesFromScene(*mPS3);
         mPS3 = NULL;
         dtCore::System::GetInstance().Step();
         SubTestParticleLayerRefs( layerRefs3 );
         mParticleComp->ProcessMessage(*timerMsg);
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            ! mParticleComp->HasRegistered(id3));

         // --- Get updated global particle total
         mParticleComp->ProcessMessage(*timerMsg);
         totalParticles = mParticleComp->GetGlobalParticleCount();
         CPPUNIT_ASSERT_MESSAGE("Total particle count should be 0 when the last particle system was removed",
            totalParticles == 0 );



         // Re-add all particle systems.
         CreateParticleSystem(mPS,true);
         CreateParticleSystem(mPS2,true);
         CreateParticleSystem(mPS3,true);
         mParticleComp->Register(*mPS);
         mParticleComp->Register(*mPS2);
         mParticleComp->Register(*mPS3);
         id1 = mPS->GetUniqueId();
         id2 = mPS2->GetUniqueId();
         id3 = mPS3->GetUniqueId();
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should be registered.",
            mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should be registered.",
            mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            mParticleComp->HasRegistered(id3));

         // Send a restart message.
         dtCore::RefPtr<dtGame::Message> msg;
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_RESTARTED,msg);
         mGM->SendMessage(*msg);
         dtCore::System::GetInstance().Step();

         // Verify that all particle systems have been removed.
         mPS = NULL;
         mPS2 = NULL;
         mPS3 = NULL;
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should NOT be registered.",
            ! mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should NOT be registered.",
            ! mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should NOT be registered.",
            ! mParticleComp->HasRegistered(id3));

         // --- Test that restart has not compromised the update timer
         CPPUNIT_ASSERT_MESSAGE("ParticleManagerComponent should still have an update interval of 0.25 seconds after restart.",
            mParticleComp->GetUpdateInterval() == interval );
         CPPUNIT_ASSERT_MESSAGE("ParticleManagerComponent should still be enabled after restart.",
            mParticleComp->GetUpdateEnabled() );



         // Re-add all particle systems.
         CreateParticleSystem(mPS,true);
         CreateParticleSystem(mPS2,true);
         CreateParticleSystem(mPS3,true);
         mParticleComp->Register(*mPS);
         mParticleComp->Register(*mPS2);
         mParticleComp->Register(*mPS3);
         id1 = mPS->GetUniqueId();
         id2 = mPS2->GetUniqueId();
         id3 = mPS3->GetUniqueId();
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should be registered.",
            mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should be registered.",
            mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            mParticleComp->HasRegistered(id3));



         // NOTE: Normally the update timer would be disabled after a map unload message.
         // This would be caused by the game manager deleting all timers rather than this
         // component doing so explicitly. Stopping the timer prematurely should be done by
         // the user of this component.

         // Re-add all particle systems.
         CreateParticleSystem(mPS,true);
         CreateParticleSystem(mPS2,true);
         CreateParticleSystem(mPS3,true);
         mParticleComp->Register(*mPS);
         mParticleComp->Register(*mPS2);
         mParticleComp->Register(*mPS3);
         id1 = mPS->GetUniqueId();
         id2 = mPS2->GetUniqueId();
         id3 = mPS3->GetUniqueId();
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should be registered.",
            mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should be registered.",
            mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should be registered.",
            mParticleComp->HasRegistered(id3));

         // Send a map unloaded message.
         mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_MAP_UNLOADED,msg);
         mGM->SendMessage(*msg);
         dtCore::System::GetInstance().Step();

         // Verify that all particle systems have been removed.
         mPS = NULL;
         mPS2 = NULL;
         mPS3 = NULL;
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 1 should NOT be registered.",
            ! mParticleComp->HasRegistered(id1));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 2 should NOT be registered.",
            ! mParticleComp->HasRegistered(id2));
         CPPUNIT_ASSERT_MESSAGE("ParticleSystem 3 should NOT be registered.",
            ! mParticleComp->HasRegistered(id3));
      }

      //////////////////////////////////////////////////////////////////////////
      void SubTestLocalForceToWorldForce( const osg::Vec3& localForce, const osg::Vec3& worldForce, float errorTolerance )
      {
         CPPUNIT_ASSERT_DOUBLES_EQUAL( localForce.x(), worldForce.x(), errorTolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( localForce.y(), worldForce.y(), errorTolerance );
         CPPUNIT_ASSERT_DOUBLES_EQUAL( localForce.z(), worldForce.z(), errorTolerance );
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::TestForceOrientations()
      {
         dtCore::RefPtr<dtCore::Transformable> parentScene = new dtCore::Transformable;
         dtCore::RefPtr<dtCore::Transformable> object1 = new dtCore::Transformable;
         dtCore::RefPtr<dtCore::Transformable> object2 = new dtCore::Transformable;
         dtCore::RefPtr<dtCore::Transformable> object3 = new dtCore::Transformable;
         dtCore::RefPtr<dtCore::Transformable> object4 = new dtCore::Transformable;
         parentScene->AddChild(object1.get());
         parentScene->AddChild(object2.get());
         parentScene->AddChild(object3.get());
         parentScene->AddChild(object4.get());

         dtCore::Transform xform;
         xform.SetTranslation( 500.0f, 1000.0f, -30.0f );
         parentScene->SetTransform( xform );

         xform.SetTranslation( 0.0f, 0.0f, 0.0f );
         xform.SetRotation( 30.0f, 0.0f, 0.0f );
         object1->SetTransform( xform, dtCore::Transformable::REL_CS );

         xform.SetTranslation( 0.0f, 0.0f, 0.0f );
         xform.SetRotation( 120.0f, 0.0f, 0.0f );
         object2->SetTransform( xform, dtCore::Transformable::REL_CS );

         xform.SetTranslation( 0.0f, 0.0f, 0.0f );
         xform.SetRotation( -60.0f, 0.0f, 0.0f );
         object3->SetTransform( xform, dtCore::Transformable::REL_CS );

         xform.SetTranslation( 0.0f, 0.0f, 0.0f );
         xform.SetRotation( -150.0f, 0.0f, 0.0f );
         object4->SetTransform( xform, dtCore::Transformable::REL_CS );

         osg::Matrix worlToLocalMtx1;
         osg::Matrix worlToLocalMtx2;
         osg::Matrix worlToLocalMtx3;
         osg::Matrix worlToLocalMtx4;

         osg::Vec3 globalForce( 128.0f, -64.0f, 32.0f );
         osg::Vec3 localForce1( mParticleComp->ConvertWorldToLocalForce( globalForce, *object1, worlToLocalMtx1 ) );
         osg::Vec3 localForce2( mParticleComp->ConvertWorldToLocalForce( globalForce, *object2, worlToLocalMtx2 ) );
         osg::Vec3 localForce3( mParticleComp->ConvertWorldToLocalForce( globalForce, *object3, worlToLocalMtx3 ) );
         osg::Vec3 localForce4( mParticleComp->ConvertWorldToLocalForce( globalForce, *object4, worlToLocalMtx4 ) );

         CPPUNIT_ASSERT( globalForce != localForce1 );
         CPPUNIT_ASSERT( globalForce != localForce2 );
         CPPUNIT_ASSERT( globalForce != localForce3 );
         CPPUNIT_ASSERT( globalForce != localForce4 );

         CPPUNIT_ASSERT( localForce1 != localForce2 );
         CPPUNIT_ASSERT( localForce1 != localForce3 );
         CPPUNIT_ASSERT( localForce1 != localForce4 );

         CPPUNIT_ASSERT( localForce2 != localForce3 );
         CPPUNIT_ASSERT( localForce2 != localForce4 );

         CPPUNIT_ASSERT( localForce3 != localForce4 );

         float errorTolerance = 0.01f;
         SubTestLocalForceToWorldForce( localForce1,
            worlToLocalMtx1.preMult(globalForce) - worlToLocalMtx1.getTrans(), errorTolerance );
         SubTestLocalForceToWorldForce( localForce2,
            worlToLocalMtx2.preMult(globalForce) - worlToLocalMtx2.getTrans(), errorTolerance );
         SubTestLocalForceToWorldForce( localForce3,
            worlToLocalMtx3.preMult(globalForce) - worlToLocalMtx3.getTrans(), errorTolerance );
         SubTestLocalForceToWorldForce( localForce4,
            worlToLocalMtx4.preMult(globalForce) - worlToLocalMtx4.getTrans(), errorTolerance );
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::GetParticleLayerRefs( dtCore::ParticleSystem& particles, ParticleLayerRefList& outLayerRefs )
      {
         dtCore::ParticleSystem::LayerList& layers = particles.GetAllLayers();
         dtCore::ParticleSystem::LayerList::iterator curLayer = layers.begin();
         for( ; curLayer != layers.end(); ++curLayer )
         {
            outLayerRefs.push_back( new ParticleLayerRef( *curLayer ) );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ParticleManagerComponentTests::SubTestParticleLayerRefs( const ParticleLayerRefList& layerRefs )
      {
         ParticleLayerRefList::const_iterator curLayerRef = layerRefs.begin();
         for( unsigned curLayerIndex = 0;
            curLayerRef != layerRefs.end();
            ++curLayerRef, ++curLayerIndex )
         {
            std::vector<const osg::Referenced*> layerNodes;
            layerNodes.push_back( (*curLayerRef)->GetGeode() );
            layerNodes.push_back( (*curLayerRef)->GetParticleSystem() ); // Not a Node like the rest.
            layerNodes.push_back( (*curLayerRef)->GetEmitter() );
            layerNodes.push_back( (*curLayerRef)->GetTransform() );

            std::vector<const osg::Referenced*>::const_iterator curNode = layerNodes.begin();
            for( ; curNode != layerNodes.end(); ++curNode )
            {
               unsigned parentCount = 0;
               unsigned refCount = 0;
               std::string nodeName;

               if( *curNode != NULL )
               {
                  const osg::Node* node = dynamic_cast<const osg::Node*>(*curNode);
                  if( node != NULL )
                  {
                     nodeName = node->className();
                     parentCount = node->getNumParents();
                  }
                  else
                  {
                     nodeName = "ParticleSystem";
                  }
                  refCount = (*curNode)->referenceCount();
               }

               std::stringstream ss;
               ss << "ParticleLayer[" << curLayerIndex << "] should have a NULL "
                  << nodeName << " but has:\n\tRef Count =\t"
                  << refCount << "\n\tParent Count = \t"
                  << parentCount << std::endl;
               CPPUNIT_ASSERT_MESSAGE( ss.str(), *curNode == NULL );
            }

         }
      }

   }
}
