/* -*-c++-*-
 * Simulation Core - BaseEntityActorProxyTests (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2006-2008, Alion Science and Technology Corporation
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/actorupdatemessage.h>

#include <dtCore/actorproperty.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/resourcedescriptor.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/refptr.h>
#include <dtCore/observerptr.h>

#include <dtUtil/macros.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/datapathutils.h>

#include <dtCore/timer.h>

#include <dtAudio/audiomanager.h>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/BaseWaterActor.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <dtGame/drpublishingactcomp.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/ViewerMessageProcessor.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/VisibilityOptions.h>

#include <dtCore/transformableactorproxy.h>

#include <dtGame/testcomponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#include <osg/MatrixTransform>
#include <osgSim/DOFTransform>

using dtCore::RefPtr;
using dtCore::ObserverPtr;


class BaseEntityActorProxyTests : public CPPUNIT_NS::TestFixture
{
   CPPUNIT_TEST_SUITE(BaseEntityActorProxyTests);

      CPPUNIT_TEST(TestPlatform);
      CPPUNIT_TEST(TestPlatformDelayedLoading);
      CPPUNIT_TEST(TestPlatformOnEnteredWorldLoading);
      CPPUNIT_TEST(TestPlatformHeadlights);
      CPPUNIT_TEST(TestPlatformSwitchToLocal);
      CPPUNIT_TEST(TestPlatformSwitchToRemote);
      CPPUNIT_TEST(TestHuman);
      CPPUNIT_TEST(TestHumanSwitchToLocal);
      CPPUNIT_TEST(TestHumanSwitchToRemote);
      CPPUNIT_TEST(TestPlatformScaleMagnification);
      CPPUNIT_TEST(TestHumanScaleMagnification);
      CPPUNIT_TEST(TestPlatformActorUpdates);
      CPPUNIT_TEST(TestHumanActorUpdates);
      CPPUNIT_TEST(TestEntityUpdateToFromStream);
      CPPUNIT_TEST(TestPlatformDRRegistration);
      CPPUNIT_TEST(TestHumanDRRegistration);
      CPPUNIT_TEST(TestPlayerActorProxy);
      CPPUNIT_TEST(TestDetonationActorProxy);
      CPPUNIT_TEST(TestDetonationSoundDelay);
      CPPUNIT_TEST(TestBaseWaterActorProxy);
      CPPUNIT_TEST(TestFindMuzzleNodes);
      CPPUNIT_TEST(TestFindMuzzleNodesWithDirection);

   CPPUNIT_TEST_SUITE_END();

public:

   void setUp();
   void tearDown();
   void TestPlatform();
   void TestPlatformDelayedLoading();
   void TestPlatformOnEnteredWorldLoading();
   void TestPlatformHeadlights();
   void TestPlatformSwitchToLocal();
   void TestPlatformSwitchToRemote();
   void TestHuman();
   void TestHumanSwitchToLocal();
   void TestHumanSwitchToRemote();
   void TestPlatformScaleMagnification();
   void TestHumanScaleMagnification();
   void TestPlatformActorUpdates();
   void TestHumanActorUpdates();
   void TestEntityUpdateToFromStream();
   void TestPlatformDRRegistration();
   void TestHumanDRRegistration();
   void TestPlayerActorProxy();
   void TestDetonationActorProxy();
   void TestDetonationSoundDelay();
   void TestBaseWaterActorProxy();
   void TestFindMuzzleNodes();
   void TestFindMuzzleNodesWithDirection();

private:
   void TestScaleMagnification(SimCore::Actors::BaseEntityActorProxy&);
   void TestBaseEntityActorProxy(SimCore::Actors::BaseEntityActorProxy&);
   void TestBaseEntityVisOpts(SimCore::Actors::BaseEntityActorProxy&);
   void TestBaseEntityActorUpdates(SimCore::Actors::BaseEntityActorProxy&);
   void TestBaseEntityDRRegistration(SimCore::Actors::BaseEntityActorProxy&);
   void TestPlatformLoadMesh(SimCore::Actors::Platform* platform,
         SimCore::Actors::BaseEntityActorProxy::DamageStateEnum& state);

   RefPtr<dtGame::GameManager> mGM;
   RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;
   RefPtr<SimCore::Components::TimedDeleterComponent> mTimerDeleterComponent;
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaseEntityActorProxyTests);

void BaseEntityActorProxyTests::setUp()
{
   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();
   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   mGM->SetApplication(GetGlobalApplication());

   mDeadReckoningComponent = new dtGame::DeadReckoningComponent();
   mGM->AddComponent(*mDeadReckoningComponent, dtGame::GameManager::ComponentPriority::NORMAL);

   // add the deleter component so the detonation actor can register itself to be deleted.
   mTimerDeleterComponent = new SimCore::Components::TimedDeleterComponent();
   mGM->AddComponent(*mTimerDeleterComponent, dtGame::GameManager::ComponentPriority::NORMAL);
}

void BaseEntityActorProxyTests::tearDown()
{
   mDeadReckoningComponent = NULL;
   mTimerDeleterComponent = NULL;

   if (mGM.valid())
   {
      mGM->DeleteAllActors(true);
      mGM = NULL;
   }
   dtCore::System::GetInstance().Stop();
}

void BaseEntityActorProxyTests::TestPlatformDelayedLoading()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   CPPUNIT_ASSERT(!eap->GetHasLoadedResources());

   SimCore::Actors::Platform* platform = NULL;
   eap->GetDrawable(platform);

   dtCore::ResourceDescriptor happySphere("StaticMeshes:physics_happy_sphere.ive");

   eap->SetNonDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() == NULL);
   eap->SetDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() == NULL);
   eap->SetDestroyedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetDestroyedFileNode() == NULL);

   CPPUNIT_ASSERT(!eap->GetHasLoadedResources());

   eap->EnsureResourcesAreLoaded();
   CPPUNIT_ASSERT(eap->GetHasLoadedResources());

   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetDestroyedFileNode() != NULL);

   dtCore::RefPtr<osg::Node> backupNode;

   // Now that the resources are loaded, go ahead and change the values to see if they change immediately.
   backupNode = platform->GetNonDamagedFileNode();
   eap->SetNonDamagedResource(dtCore::ResourceDescriptor::NULL_RESOURCE);
   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() == NULL);
   eap->SetNonDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() != backupNode);

   backupNode = platform->GetDamagedFileNode();
   eap->SetDamagedResource(dtCore::ResourceDescriptor::NULL_RESOURCE);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() == NULL);
   eap->SetDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() != backupNode);

   backupNode = platform->GetDamagedFileNode();
   eap->SetDamagedResource(dtCore::ResourceDescriptor::NULL_RESOURCE);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() == NULL);
   eap->SetDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() != backupNode);
}

void BaseEntityActorProxyTests::TestPlatformOnEnteredWorldLoading()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   dtCore::ResourceDescriptor happySphere("StaticMeshes:physics_happy_sphere.ive");

   CPPUNIT_ASSERT(!eap->GetHasLoadedResources());

   SimCore::Actors::Platform* platform = NULL;
   eap->GetDrawable(platform);

   eap->SetNonDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() == NULL);
   eap->SetDamagedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() == NULL);
   eap->SetDestroyedResource(happySphere);
   CPPUNIT_ASSERT(platform->GetDestroyedFileNode() == NULL);

   mGM->AddActor(*eap, false, false);

   CPPUNIT_ASSERT(eap->GetHasLoadedResources());

   CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetDamagedFileNode() != NULL);
   CPPUNIT_ASSERT(platform->GetDestroyedFileNode() != NULL);
}

void BaseEntityActorProxyTests::TestPlatform()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   ObserverPtr<dtCore::DeltaDrawable> edraw = eap->GetDrawable();

   TestBaseEntityActorProxy(*eap);


   SimCore::Actors::Platform* platform = NULL;
   eap->GetDrawable(platform);

   TestPlatformLoadMesh(platform, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE);
   TestPlatformLoadMesh(platform, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE);
   TestPlatformLoadMesh(platform, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE);
   TestPlatformLoadMesh(platform, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED);

   dtCore::RefPtr<SimCore::VisibilityOptions> visOpts = new SimCore::VisibilityOptions;
   SimCore::BasicVisibilityOptions basicVisOpts;
   basicVisOpts.SetAllTrue();
   basicVisOpts.mPlatforms = false;
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(!platform->ShouldBeVisible(*visOpts));
   basicVisOpts.mPlatforms = true;
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(platform->ShouldBeVisible(*visOpts));

   TestBaseEntityVisOpts(*eap);

   dtCore::ActorProperty* prop = NULL;

   osg::Vec3 vec;
   vec.set(1.14, 8.21, 7.85);
   prop = eap->GetProperty("EngineSmokePosition");
   CPPUNIT_ASSERT_MESSAGE("The engine smoke position property should not be NULL", prop != NULL);
   static_cast<dtCore::Vec3ActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::Vec3ActorProperty*>(prop)->GetValue() == vec);

   bool b = true;
   prop = eap->GetProperty("EngineSmokeOn");
   CPPUNIT_ASSERT_MESSAGE("The engine smoke on property should not be NULL", prop != NULL);
   static_cast<dtCore::BooleanActorProperty*>(prop)->SetValue(b);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());

   prop = eap->GetProperty("Engine smoke particles");
   CPPUNIT_ASSERT_MESSAGE("The \"Engine smoke particles\" property should not be NULL", prop != NULL);
   dtCore::ResourceDescriptor rd = static_cast<dtCore::ResourceActorProperty*>(prop)->GetValue();
   CPPUNIT_ASSERT_MESSAGE("\"Engine smoke particles\" value should not be NULL", !rd.IsEmpty());
   CPPUNIT_ASSERT_EQUAL_MESSAGE("\"Engine smoke particles\" value should be", rd.GetResourceIdentifier(), std::string("Particles:smoke.osg"));

   mGM->DeleteActor(*eap);
   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT_EQUAL(1, eap->referenceCount());
   eap = NULL;
   CPPUNIT_ASSERT(!edraw.valid());
   edraw = NULL;

}

void BaseEntityActorProxyTests::TestPlatformLoadMesh(SimCore::Actors::Platform* platform,
      SimCore::Actors::BaseEntityActorProxy::DamageStateEnum& state)
{
   CPPUNIT_ASSERT(platform->GetNodeCollector() == NULL);

   if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
   {
      CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() == NULL);
   }

   dtCore::ResourceDescriptor happySphere("StaticMeshes:physics_happy_sphere.ive");

   platform->LoadDamageableFile(happySphere, state);
   dtCore::RefPtr<osg::Node> currentNode = NULL;

   dtCore::RefPtr<const dtUtil::NodeCollector> nodeCollector = platform->GetNodeCollector();

   if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
   {
      currentNode = platform->GetNonDamagedFileNode();
      CPPUNIT_ASSERT(nodeCollector.valid());
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
   {
      currentNode = platform->GetDamagedFileNode();
      CPPUNIT_ASSERT(!nodeCollector.valid());
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE)
   {
      currentNode = platform->GetDamagedFileNode();
      CPPUNIT_ASSERT(!nodeCollector.valid());
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
   {
      currentNode = platform->GetDestroyedFileNode();
      CPPUNIT_ASSERT(!nodeCollector.valid());
   }

   CPPUNIT_ASSERT(currentNode != NULL);
   CPPUNIT_ASSERT(currentNode->getParent(0)->getUserData() != NULL);
   osg::Node* copiedNode = dynamic_cast<osg::Node*>(currentNode->getParent(0)->getUserData());
   CPPUNIT_ASSERT(copiedNode != NULL);
   CPPUNIT_ASSERT(std::string(dtCore::Project::GetInstance().GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR + "StaticMeshes" + dtUtil::FileUtils::PATH_SEPARATOR + "physics_happy_sphere.ive") ==
         currentNode->getParent(0)->getName() ||
   std::string(dtCore::Project::GetInstance().GetContext(1) + dtUtil::FileUtils::PATH_SEPARATOR + "StaticMeshes" + dtUtil::FileUtils::PATH_SEPARATOR + "physics_happy_sphere.ive") ==
         currentNode->getParent(0)->getName() );
   CPPUNIT_ASSERT_EQUAL(state.GetName(), currentNode->getName());

   platform->LoadDamageableFile(happySphere, state);

   if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
   {
      CPPUNIT_ASSERT_MESSAGE("It should NOT have reloaded the mesh", currentNode == platform->GetNonDamagedFileNode());
      CPPUNIT_ASSERT_MESSAGE("It should NOT have reloaded the node collector", nodeCollector.get() == platform->GetNodeCollector());
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
   {
      CPPUNIT_ASSERT_MESSAGE("It should NOT have reloaded the mesh", currentNode == platform->GetDamagedFileNode());
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE)
   {
      CPPUNIT_ASSERT_MESSAGE("It should NOT have reloaded the mesh", currentNode == platform->GetDamagedFileNode());
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
   {
      CPPUNIT_ASSERT_MESSAGE("It should NOT have reloaded the mesh", currentNode == platform->GetDestroyedFileNode());
   }

   dtCore::ResourceDescriptor crate("StaticMeshes:physics_crate.ive");

   platform->LoadDamageableFile(crate, state);

   if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
   {
      CPPUNIT_ASSERT_MESSAGE("It should have reloaded the mesh", currentNode != platform->GetNonDamagedFileNode());
      CPPUNIT_ASSERT_MESSAGE("It should have reloaded the node collector", nodeCollector.get() != platform->GetNodeCollector());
      currentNode = platform->GetNonDamagedFileNode();
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
   {
      CPPUNIT_ASSERT_MESSAGE("It should have reloaded the mesh", currentNode != platform->GetDamagedFileNode());
      currentNode = platform->GetDamagedFileNode();
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE)
   {
      CPPUNIT_ASSERT_MESSAGE("It should have reloaded the mesh", currentNode != platform->GetDamagedFileNode());
      currentNode = platform->GetDamagedFileNode();
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
   {
      CPPUNIT_ASSERT_MESSAGE("It should have reloaded the mesh", currentNode != platform->GetDestroyedFileNode());
      currentNode = platform->GetDestroyedFileNode();
   }

   CPPUNIT_ASSERT(std::string(dtCore::Project::GetInstance().GetContext(0) + dtUtil::FileUtils::PATH_SEPARATOR + "StaticMeshes" + dtUtil::FileUtils::PATH_SEPARATOR + "physics_crate.ive") ==
            currentNode->getParent(0)->getName() ||
            std::string(dtCore::Project::GetInstance().GetContext(1) + dtUtil::FileUtils::PATH_SEPARATOR + "StaticMeshes" + dtUtil::FileUtils::PATH_SEPARATOR + "physics_crate.ive") ==
                        currentNode->getParent(0)->getName());
   CPPUNIT_ASSERT_EQUAL(state.GetName(), currentNode->getName());

   platform->LoadDamageableFile(dtCore::ResourceDescriptor::NULL_RESOURCE, state);

   if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE)
   {
      CPPUNIT_ASSERT(platform->GetNonDamagedFileNode() == NULL);
      CPPUNIT_ASSERT_MESSAGE("It should have deleted the node collector on model unload", platform->GetNodeCollector() == NULL);
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::SLIGHT_DAMAGE)
   {
      CPPUNIT_ASSERT(platform->GetDamagedFileNode() == NULL);
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::MODERATE_DAMAGE)
   {
      CPPUNIT_ASSERT(platform->GetDamagedFileNode() == NULL);
   }
   else if (state ==  SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
   {
      CPPUNIT_ASSERT(platform->GetDestroyedFileNode() == NULL);
   }
}

void BaseEntityActorProxyTests::TestPlatformHeadlights()
{
   RefPtr<SimCore::Actors::BaseEntityActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   dtCore::BooleanActorProperty* prop = NULL;

   eap->GetProperty(SimCore::Actors::PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED, prop);
   CPPUNIT_ASSERT_MESSAGE("The head lights property should not be NULL", prop != NULL);
   std::stringstream textMessage;
   textMessage << "The default value of \""
         << static_cast<std::string>(SimCore::Actors::PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED) << "\" should be false.";
   CPPUNIT_ASSERT_MESSAGE(textMessage.str(), !prop->GetValue());
   prop->SetValue(true);
   CPPUNIT_ASSERT_MESSAGE("Headlights should NOT turn on with no rendering support component",
         !prop->GetValue());

   mGM->AddComponent(*new SimCore::Components::RenderingSupportComponent, dtGame::GameManager::ComponentPriority::NORMAL);

   prop->SetValue(true);
   CPPUNIT_ASSERT_MESSAGE("Headlights should turn on with a valid rendering support component",
         prop->GetValue());

   prop->SetValue(false);
   CPPUNIT_ASSERT(!prop->GetValue());
   eap->SetGameManager(NULL);
   prop->SetValue(true);
   CPPUNIT_ASSERT_MESSAGE("With no game manager set, you should still be able to set the value to true and have it stick so it will work in stage",
         prop->GetValue());

   eap->SetGameManager(mGM);

}

void BaseEntityActorProxyTests::TestPlatformSwitchToLocal()
{
   RefPtr<SimCore::Actors::BaseEntityActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   mGM->AddActor(*eap, true, false);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(eap->IsRemote());

   mGM->SwitchActorToLocal(*eap);

   CPPUNIT_ASSERT(!eap->IsRemote());
   //TODO somehow this needs to test that the data is not in a bad state.
}

void BaseEntityActorProxyTests::TestPlatformSwitchToRemote()
{
   RefPtr<SimCore::Actors::BaseEntityActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   mGM->AddActor(*eap, false, false);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(!eap->IsRemote());

   mGM->SwitchActorToRemote(*eap);

   CPPUNIT_ASSERT(eap->IsRemote());

   //TODO somehow this needs to test that the data is not in a bad state.

}

void BaseEntityActorProxyTests::TestHuman()
{
   RefPtr<SimCore::Actors::HumanActorProxy> hap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, hap);
   CPPUNIT_ASSERT(hap.valid());

   ObserverPtr<dtCore::DeltaDrawable> edraw = hap->GetDrawable();

   TestBaseEntityActorProxy(*hap);

   SimCore::Actors::Human* human;
   hap->GetDrawable(human);

   CPPUNIT_ASSERT(human->GetNodeCollector() == NULL);
   human->LoadNodeCollector();
   CPPUNIT_ASSERT(human->GetNodeCollector() != NULL);

   dtCore::RefPtr<SimCore::VisibilityOptions> visOpts = new SimCore::VisibilityOptions;
   SimCore::BasicVisibilityOptions basicVisOpts;
   basicVisOpts.SetAllTrue();
   basicVisOpts.mDismountedInfantry = false;
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(!human->ShouldBeVisible(*visOpts));
   basicVisOpts.mDismountedInfantry = true;
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(human->ShouldBeVisible(*visOpts));

   TestBaseEntityVisOpts(*hap);


   mGM->DeleteActor(*hap);
   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT_EQUAL(1, hap->referenceCount());
   hap = NULL;
   CPPUNIT_ASSERT(!edraw.valid());
   edraw = NULL;

}

void BaseEntityActorProxyTests::TestHumanSwitchToLocal()
{
   RefPtr<SimCore::Actors::BaseEntityActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   mGM->AddActor(*eap, true, false);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(eap->IsRemote());

   mGM->SwitchActorToLocal(*eap);

   CPPUNIT_ASSERT(!eap->IsRemote());

   //TODO somehow this needs to test that the data is not in a bad state.
}

void BaseEntityActorProxyTests::TestHumanSwitchToRemote()
{
   RefPtr<SimCore::Actors::BaseEntityActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   mGM->AddActor(*eap, false, false);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(!eap->IsRemote());

   mGM->SwitchActorToRemote(*eap);

   CPPUNIT_ASSERT(eap->IsRemote());

   //TODO somehow this needs to test that the data is not in a bad state.
}


void BaseEntityActorProxyTests::TestBaseEntityVisOpts(SimCore::Actors::BaseEntityActorProxy& eap)
{
   SimCore::Actors::BaseEntity* entity;
   eap.GetDrawable(entity);

   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::GROUND);
   entity->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::FRIENDLY);

   dtCore::RefPtr<SimCore::VisibilityOptions> visOpts = new SimCore::VisibilityOptions;
   SimCore::BasicVisibilityOptions basicVisOpts = visOpts->GetBasicOptions();
   basicVisOpts.SetAllFalse();
   basicVisOpts.mDismountedInfantry = true;
   basicVisOpts.mPlatforms = true;
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::GROUND, true);
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::ForceEnum::FRIENDLY, true);

   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::AIR);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::AIR, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));


   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::AMPHIBIOUS);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::AMPHIBIOUS, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::MULTI);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::MULTI, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::SPACE);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::SPACE, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::SUBMARINE);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::SUBMARINE, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetDomain(SimCore::Actors::BaseEntityActorProxy::DomainEnum::SURFACE);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::DomainEnum::SURFACE, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::INSURGENT);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::ForceEnum::INSURGENT, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::NEUTRAL);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::ForceEnum::NEUTRAL, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OPPOSING);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OPPOSING, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

   entity->SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OTHER);
   CPPUNIT_ASSERT(!entity->ShouldBeVisible(*visOpts));
   basicVisOpts.SetEnumVisible(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OTHER, true);
   visOpts->SetBasicOptions(basicVisOpts);
   CPPUNIT_ASSERT(entity->ShouldBeVisible(*visOpts));

}

void BaseEntityActorProxyTests::TestBaseEntityActorProxy(SimCore::Actors::BaseEntityActorProxy& eap)
{
   using namespace SimCore::Actors;

   //make the actor
   mGM->AddActor(eap, true, false);

   CPPUNIT_ASSERT(eap.GetGameManager() != NULL);

   dtCore::ActorProperty *prop = NULL;
   BaseEntity* entity = NULL;
   eap.GetDrawable( entity );
   CPPUNIT_ASSERT_MESSAGE("BaseEntity should be valid when being accessed from its proxy.",
         entity != NULL);

   prop = eap.GetProperty("Firepower Disabled");
   CPPUNIT_ASSERT_MESSAGE("The firepower property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The default value of \"Firepower Disabled\" should be false.",
         !static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());
   static_cast<dtCore::BooleanActorProperty*>(prop)->SetValue(true);
   CPPUNIT_ASSERT(static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());

   prop = eap.GetProperty("Mobility Disabled");
   CPPUNIT_ASSERT_MESSAGE("The mobility property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The default value of \"Mobility Disabled\" should be false.",
         !static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());
   static_cast<dtCore::BooleanActorProperty*>(prop)->SetValue(true);
   CPPUNIT_ASSERT(static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());

   osg::Vec3 vec(5.0f, 5.0f, 5.0f);
   prop = eap.GetProperty("Angular Velocity Vector");
   CPPUNIT_ASSERT_MESSAGE("The angular velocity vector property should not be NULL", prop != NULL);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set",
         static_cast<dtCore::Vec3fActorProperty*>(prop)->GetValue() == vec);

   prop = eap.GetProperty("Last Known Translation");
   CPPUNIT_ASSERT_MESSAGE("The last known translation property should not be NULL", prop != NULL);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set",
         static_cast<dtCore::Vec3fActorProperty*>(prop)->GetValue() == vec);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(osg::Vec3(0.0f, 0.0f, 0.0f));

   prop = eap.GetProperty("Last Known Rotation");
   CPPUNIT_ASSERT_MESSAGE("The last known rotation property should not be NULL", prop != NULL);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::Vec3fActorProperty*>(prop)->GetValue() == vec);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(osg::Vec3(0.0f, 0.0f, 0.0f));

   vec.set(2.3f, 1.4f, 8.3f);
   prop = eap.GetProperty("Velocity Vector");
   CPPUNIT_ASSERT_MESSAGE("The velocity vector property should not be NULL", prop != NULL);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::Vec3fActorProperty*>(prop)->GetValue() == vec);

   prop = eap.GetProperty("Damage State");
   dtCore::AbstractEnumActorProperty *aep = dynamic_cast<dtCore::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property should not be NULL", aep != NULL);
   aep->SetValueFromString("Destroyed");
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName() == "Destroyed");
   aep->SetValueFromString("No Damage");
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName() == "No Damage");

   prop = eap.GetProperty(BaseEntityActorProxy::PROPERTY_DOMAIN);
   aep = dynamic_cast<dtCore::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property should not be NULL", aep != NULL);
   CPPUNIT_ASSERT_MESSAGE("Property should be defaulted to GROUND domain.", aep->GetEnumValue().GetName()
         == BaseEntityActorProxy::DomainEnum::GROUND.GetName());
   CPPUNIT_ASSERT(entity->GetDomain() == BaseEntityActorProxy::DomainEnum::GROUND);

   aep->SetValueFromString(BaseEntityActorProxy::DomainEnum::SURFACE.GetName());
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName()
         == BaseEntityActorProxy::DomainEnum::SURFACE.GetName());
   CPPUNIT_ASSERT(entity->GetDomain() == BaseEntityActorProxy::DomainEnum::SURFACE);

   entity->SetDomain(BaseEntityActorProxy::DomainEnum::AMPHIBIOUS);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName()
         == BaseEntityActorProxy::DomainEnum::AMPHIBIOUS.GetName());
   CPPUNIT_ASSERT(entity->GetDomain() == BaseEntityActorProxy::DomainEnum::AMPHIBIOUS);

   prop = eap.GetProperty("Force Affiliation");
   aep = dynamic_cast<dtCore::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property for \"Force Affiliation\" should not be NULL", aep != NULL);
   CPPUNIT_ASSERT_MESSAGE("The \"Force Affiliation\" should default to NEUTRAL", aep->GetEnumValue() == SimCore::Actors::BaseEntityActorProxy::ForceEnum::NEUTRAL);
   aep->SetValueFromString("FRIENDLY");
   CPPUNIT_ASSERT_MESSAGE("The \"Force Affiliation\" should now be friendly", aep->GetEnumValue() == SimCore::Actors::BaseEntityActorProxy::ForceEnum::FRIENDLY);

   osg::Vec3 translation(0.0f, 0.0f, 0.0f);
   prop = eap.GetProperty(dtCore::TransformableActorProxy::PROPERTY_TRANSLATION);
   static_cast<dtCore::Vec3ActorProperty*>(prop)->SetValue(translation);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::Vec3ActorProperty*>(prop)->GetValue() == translation);

   prop = eap.GetProperty("GroundClampType");
   aep = dynamic_cast<dtCore::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property for \"GroundClampType\" should not be NULL", aep != NULL);
   CPPUNIT_ASSERT_MESSAGE("The \"GroundClampType\" should default to KEEP_ABOVE", 
         aep->GetEnumValue() == dtGame::GroundClampTypeEnum::KEEP_ABOVE);
   aep->SetValueFromString("Full");
   CPPUNIT_ASSERT_MESSAGE("The \"GroundClampType\" should now be full", 
         aep->GetEnumValue() == dtGame::GroundClampTypeEnum::FULL);

   dtCore::ActorProperty* offsetProp = eap.GetProperty("Ground Offset");
   CPPUNIT_ASSERT_MESSAGE("The ground offset should be close 0.0.", osg::equivalent(static_cast<dtCore::FloatActorProperty*>(offsetProp)->GetValue(), 0.0f, 0.1f));
   static_cast<dtCore::FloatActorProperty*>(offsetProp)->SetValue(1.3f);
   CPPUNIT_ASSERT(static_cast<dtCore::FloatActorProperty*>(offsetProp)->GetValue() == 1.3f);

   dtCore::ActorProperty* drawingProp = eap.GetProperty("DrawingModel");
   CPPUNIT_ASSERT(drawingProp != NULL);
   CPPUNIT_ASSERT_MESSAGE("The default value of drawing should be true.", static_cast<dtCore::BooleanActorProperty*>(drawingProp)->GetValue());
   static_cast<dtCore::BooleanActorProperty*>(drawingProp)->SetValue(false);
   CPPUNIT_ASSERT(!static_cast<dtCore::BooleanActorProperty*>(drawingProp)->GetValue());

   vec.set(4.3f, 8.1f, 7.69f);
   prop = eap.GetProperty("Acceleration Vector");
   CPPUNIT_ASSERT_MESSAGE("The acceleration property property should not be NULL", prop != NULL);
   static_cast<dtCore::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::Vec3fActorProperty*>(prop)->GetValue() == vec);

   SimCore::Actors::HumanActorProxy *hap = dynamic_cast<SimCore::Actors::HumanActorProxy*>(&eap);
   if(hap == NULL)
   {
      bool b = true;
      prop = eap.GetProperty("FlamesPresent");
      CPPUNIT_ASSERT_MESSAGE("The flames present property should not be NULL", prop != NULL);
      static_cast<dtCore::BooleanActorProperty*>(prop)->SetValue(b);
      CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());

      prop = eap.GetProperty("SmokePlumePresent");
      CPPUNIT_ASSERT_MESSAGE("The smoke plume present on property should not be NULL", prop != NULL);
      static_cast<dtCore::BooleanActorProperty*>(prop)->SetValue(b);
      CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::BooleanActorProperty*>(prop)->GetValue());
   }

   dtCore::ActorProperty *ap = eap.GetProperty("Service");
   CPPUNIT_ASSERT(ap != NULL);
   dtCore::AbstractEnumActorProperty *aeap = dynamic_cast<dtCore::AbstractEnumActorProperty*>(ap);
   CPPUNIT_ASSERT(aeap != NULL);
   dtUtil::Enumeration &e = aeap->GetEnumValue();
   CPPUNIT_ASSERT_MESSAGE("The service property should default to Marines", e == SimCore::Actors::BaseEntityActorProxy::ServiceEnum::MARINES);
   aeap->SetEnumValue(SimCore::Actors::BaseEntityActorProxy::ServiceEnum::ARMY);
   dtUtil::Enumeration &newE = aeap->GetEnumValue();
   CPPUNIT_ASSERT_MESSAGE("The service property should return what was set", newE == SimCore::Actors::BaseEntityActorProxy::ServiceEnum::ARMY);

   prop = eap.GetProperty("Smoke plume particles");
   CPPUNIT_ASSERT_MESSAGE("The \"Smoke plume particles\" property should not be NULL", prop != NULL);
   dtCore::ResourceDescriptor rd = static_cast<dtCore::ResourceActorProperty*>(prop)->GetValue();
   CPPUNIT_ASSERT_MESSAGE("\"Smoke plume particles\" value should not be NULL", !rd.IsEmpty());
   CPPUNIT_ASSERT_EQUAL_MESSAGE("\"Smoke plume particles\" value should be", rd.GetResourceIdentifier(), std::string("Particles:smoke.osg"));

   prop = eap.GetProperty("Fire particles");
   CPPUNIT_ASSERT_MESSAGE("The \"Fire particles\" property should not be NULL", prop != NULL);
   rd = static_cast<dtCore::ResourceActorProperty*>(prop)->GetValue();
   CPPUNIT_ASSERT_MESSAGE("\"Fire particles\" value should not be NULL", !rd.IsEmpty());
   CPPUNIT_ASSERT_EQUAL_MESSAGE("\"Fire particles\" value should be", rd.GetResourceIdentifier(), std::string("Particles:fire.osg"));

   std::string munitionTableName("LARGE EXPLOSION");
   prop = eap.GetProperty("Munition Damage Table");
   CPPUNIT_ASSERT_MESSAGE("The \"Munition Damage Table\" property should not be NULL", prop != NULL);
   static_cast<dtCore::StringActorProperty*>(prop)->SetValue(munitionTableName);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtCore::StringActorProperty*>(prop)->GetValue() == munitionTableName);

   std::string testValue("This Is A Test String");
   dtCore::StringActorProperty* strProp = NULL;
   strProp = dynamic_cast<dtCore::StringActorProperty*>(eap.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_ENTITY_TYPE_ID));
   CPPUNIT_ASSERT_MESSAGE("The \"" + SimCore::Actors::BaseEntityActorProxy::PROPERTY_ENTITY_TYPE_ID.Get() + "\" property should not be NULL", strProp != NULL);
   strProp->SetValue(testValue);
   CPPUNIT_ASSERT( strProp->GetValue() == testValue );

   strProp = dynamic_cast<dtCore::StringActorProperty*>(eap.GetProperty(SimCore::Actors::BaseEntityActorProxy::PROPERTY_MAPPING_NAME));
   CPPUNIT_ASSERT_MESSAGE("The \"" + SimCore::Actors::BaseEntityActorProxy::PROPERTY_MAPPING_NAME.Get() + "\" property should not be NULL", strProp != NULL);
   strProp->SetValue(testValue);
   CPPUNIT_ASSERT( strProp->GetValue() == testValue );

   CPPUNIT_ASSERT(eap.GetHideDTCorePhysicsProps());
   CPPUNIT_ASSERT_MESSAGE("dtCore physics properties should be hidden.", eap.GetProperty("Show Collision Geometry") == NULL);
}

void BaseEntityActorProxyTests::TestPlatformScaleMagnification()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   TestScaleMagnification(*eap);
}

void BaseEntityActorProxyTests::TestHumanScaleMagnification()
{
   RefPtr<SimCore::Actors::HumanActorProxy> hap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, hap);
   CPPUNIT_ASSERT(hap.valid());

   TestScaleMagnification(*hap);
}

void BaseEntityActorProxyTests::TestScaleMagnification(SimCore::Actors::BaseEntityActorProxy& eap)
{
   //make the actor
   mGM->AddActor(eap, true, false);

   dtCore::ActorProperty* prop = NULL;
   dtCore::Vec3ActorProperty* v3Prop = NULL;

   osg::Vec3 defScale(4.3f, 8.1f, 7.69f);
   prop = eap.GetProperty("Default Scale");
   CPPUNIT_ASSERT_MESSAGE("The Default Scale property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The Default Scale property type should be Vec3", prop->GetPropertyType() == dtCore::DataType::VEC3);
   v3Prop = static_cast<dtCore::Vec3ActorProperty*>(prop);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Default Scale should default to 1, 1, 1", osg::Vec3(1.0f, 1.0f, 1.0f), v3Prop->GetValue());
   v3Prop->SetValue(defScale);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue for Default Scale should return what was set", defScale, v3Prop->GetValue());

   osg::Vec3 scaleMag(3.0f, 3.7f, 3.9f);
   prop = eap.GetProperty("Scale Magnification Factor");
   CPPUNIT_ASSERT_MESSAGE("The Scale Magnification Factor property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The Scale Magnification Factor property type should be Vec3", prop->GetPropertyType() == dtCore::DataType::VEC3);
   v3Prop = static_cast<dtCore::Vec3ActorProperty*>(prop);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Scale Magnification Factor should default to 1, 1, 1.", osg::Vec3(1.0f, 1.0f, 1.0f), v3Prop->GetValue());
   v3Prop->SetValue(scaleMag);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue for Default Scale should return what was set", scaleMag, v3Prop->GetValue());

   prop = eap.GetProperty("Model Scale");
   v3Prop = static_cast<dtCore::Vec3ActorProperty*>(prop);
   CPPUNIT_ASSERT(prop != NULL);

   osg::Vec3 expectedScale;
   for (int i = 0; i < 3; ++i)
   {
      expectedScale[i] = defScale[i] * scaleMag[i];
   }

   osg::Vec3 value = v3Prop->GetValue();
   CPPUNIT_ASSERT_MESSAGE("The scale should equal the default scale x the scale magnification.",
         osg::equivalent(expectedScale.x(), value.x(), 0.00001f) &&
         osg::equivalent(expectedScale.y(), value.y(), 0.00001f) &&
         osg::equivalent(expectedScale.z(), value.z(), 0.00001f));
}

void BaseEntityActorProxyTests::TestPlatformActorUpdates()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   TestBaseEntityActorUpdates(*eap);
}

void BaseEntityActorProxyTests::TestHumanActorUpdates()
{
   RefPtr<SimCore::Actors::HumanActorProxy> hap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, hap);
   CPPUNIT_ASSERT(hap.valid());

   TestBaseEntityActorUpdates(*hap);
}

void BaseEntityActorProxyTests::TestBaseEntityActorUpdates(SimCore::Actors::BaseEntityActorProxy& eap)
{
   RefPtr<dtGame::TestComponent> tc = new dtGame::TestComponent;
   mGM->AddComponent(*tc, dtGame::GameManager::ComponentPriority::HIGHEST);

   mGM->AddActor(eap, false, true);

   dtGame::DeadReckoningHelper* drHelper = NULL;
   eap.GetComponent(drHelper);

   dtGame::DRPublishingActComp* drPubAC = NULL;
   eap.GetComponent(drPubAC);

   SimCore::Actors::BaseEntity* entityDrawable = NULL;
   eap.GetDrawable(entityDrawable);
   // we now need a DR algorithm other than none, or it will skip updates.
   drHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::STATIC);//VELOCITY_ONLY);
   // Eliminating smoothing causes the entity movements and DR movements to be fully in sync. This
   // prevents too many updates being sent based on 'smoothing' changes.
   drHelper->SetMaxRotationSmoothingTime(0.0f);
   drHelper->SetMaxTranslationSmoothingTime(0.0f);

   drPubAC->SetMaxUpdateSendRate(60.0f); // can update 60 times a second
   drPubAC->SetMaxRotationError(4.0f);
   drPubAC->SetMaxTranslationError(10.0f);

   //double oldTime = dtCore::System::GetInstance().GetSimTimeSinceStartup();

   float initialTimeUntilNextFullUpdate = drPubAC->GetTimeUntilNextFullUpdate();
   float defaultValue = dtGame::DRPublishingActComp::TIME_BETWEEN_UPDATES;
   bool initialValueIsInRange = 
         (initialTimeUntilNextFullUpdate < defaultValue * 1.5001f) &&
         (initialTimeUntilNextFullUpdate > defaultValue * 0.4999f);

   CPPUNIT_ASSERT_MESSAGE("The first full update time should be between 0.5 & 1.5 times the default, but it is \""
         + dtUtil::ToString(initialTimeUntilNextFullUpdate) + "\" with default \"" + dtUtil::ToString(defaultValue) + "\"",
         initialValueIsInRange);
   //CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
   //      "The time until next update should be seeded to the time between complete updates.",
   //      dtGame::DRPublishingActComp::TIME_BETWEEN_UPDATES,
   //      entityActor->GetDRPublishingActComp()->GetTimeUntilNextFullUpdate(), 1e-3f);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT(tc->FindProcessMessageOfType(dtGame::MessageType::INFO_ACTOR_CREATED).valid());

   tc->reset();

   osg::Vec3 smallMovement(0.01f, 0.01f, 0.01f);

   dtCore::Transform xform;
   osg::Vec3 pos;
   entityDrawable->GetTransform(xform, dtCore::Transformable::REL_CS);

   xform.GetTranslation(pos);
   xform.SetTranslation(pos + smallMovement);
   entityDrawable->SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::AppSleep(20);// Give the DR Publisher time to do the actor update. At 60 hz, we technically only need 16.6 ms

   float timeUntilUpdate1 = drPubAC->GetTimeUntilNextFullUpdate();
   dtCore::System::GetInstance().Step();
   float timeUntilUpdate2 = drPubAC->GetTimeUntilNextFullUpdate();

   /// TEST the time Until Update processing
   RefPtr<const dtGame::TickMessage> tickMessage = static_cast<const dtGame::TickMessage*>(
         tc->FindProcessMessageOfType(dtGame::MessageType::TICK_LOCAL).get());
   float tickTime = tickMessage->GetDeltaSimTime();
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The time until the next update should decrement by the step time",
         timeUntilUpdate1 - tickTime, timeUntilUpdate2);


   CPPUNIT_ASSERT_MESSAGE("The translation was small. It should NOT have sent an update.",
         !tc->FindProcessMessageOfType(dtGame::MessageType::INFO_ACTOR_UPDATED).valid());


   osg::Vec3 largeMovement(200.0f, 345.0f, 657.0f);
   xform.SetTranslation(pos + largeMovement);
   entityDrawable->SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::AppSleep(20);// Give the DR Publisher time to do the actor update. At 60 hz, we technically only need 16.6 ms

   dtCore::System::GetInstance().Step();

   RefPtr<const dtGame::ActorUpdateMessage> update1 =
         static_cast<const dtGame::ActorUpdateMessage*>(tc->FindProcessMessageOfType(
               dtGame::MessageType::INFO_ACTOR_UPDATED).get());

   entityDrawable->GetTransform(xform, dtCore::Transformable::REL_CS);

   std::ostringstream ss;
   ss << "The translation was large. It should have sent an update.  New position is: "
         << pos;

   CPPUNIT_ASSERT_MESSAGE(ss.str(),
         update1.valid());

   // The number of params is based on the subclass. It should at least be pos & rot, but might also be vel & ang vel...
   std::vector<const dtGame::MessageParameter*> toFill;
   update1->GetUpdateParameters(toFill);
   CPPUNIT_ASSERT_MESSAGE("The update message should have at least 2 update parameters in it.",
         (int) (toFill.size()) >= 2);

   tc->reset();

   xform.SetRotation(smallMovement);
   entityDrawable->SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::AppSleep(20);// Give the DR Publisher time to do the actor update. At 60 hz, we technically only need 16.6 ms

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT_MESSAGE("The rotation was small. It should NOT have sent an update.",
         !tc->FindProcessMessageOfType(dtGame::MessageType::INFO_ACTOR_UPDATED).valid());

   tc->reset();

   xform.SetRotation(largeMovement);
   entityDrawable->SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::AppSleep(20);// Give the DR Publisher time to do the actor update. At 60 hz, we technically only need 16.6 ms

   dtCore::System::GetInstance().Step();

   RefPtr<const dtGame::ActorUpdateMessage> update2 =
         static_cast<const dtGame::ActorUpdateMessage*>(tc->FindProcessMessageOfType(
               dtGame::MessageType::INFO_ACTOR_UPDATED).get());

   CPPUNIT_ASSERT_MESSAGE("The rotation was large. It should have sent an update.",
         update2.valid());

   update2->GetUpdateParameters(toFill);
   // The number of params is based on the subclass. It should at least be pos & rot, but might also be vel & ang vel...
   CPPUNIT_ASSERT_MESSAGE("The update message should have at least 2 update parameters in it.",
         (int) (toFill.size()) >= 2);

   tc->reset();
}


////////////////////////////////////////////////////////////////
void BaseEntityActorProxyTests::TestEntityUpdateToFromStream()
{
   /// Test the ability to generate an actor update message (with pos/trans)
   /// and convert to & from a stream. Testing an issue where items across a network
   /// or from playback seem to 'blip' with the wrong X or Y pos.

   // create an actor - any entity will do
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   dtGame::DeadReckoningHelper* drHelper = NULL;
   eap->GetComponent(drHelper);


   // Set pos/trans
   //osg::Vec3 basePos(201.0358f, 41.591f, 1.02f);
   osg::Vec3 basePos(196.0123f, 37.2345f, 1.02f);
   osg::Vec3 baseRot(140.01f, -0.84f, -0.13f);
   drHelper->SetLastKnownTranslation(basePos);
   drHelper->SetLastKnownRotation(baseRot);

   // do a loop - 1000 times is better, but if everyone runs this, it will find it eventually.
   for (unsigned int counter = 0; counter < 250; counter ++)
   {
      // multiply pos/trans by some number 1.037
      float randMult = dtUtil::RandFloat(0.001f, 2.003f);
      osg::Vec3 newPos = basePos * randMult;
      osg::Vec3 newRot = baseRot * randMult;
      drHelper->SetLastKnownTranslation(newPos);
      drHelper->SetLastKnownRotation(newRot);

      // generate an update message 
      dtCore::RefPtr<dtGame::Message> updateMsg =
            mGM->GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_ACTOR_UPDATED);
      dtGame::ActorUpdateMessage *message = static_cast<dtGame::ActorUpdateMessage *>(updateMsg.get());
      eap->PopulateActorUpdate(*message);

      // to stream
      dtUtil::DataStream dataStream;
      message->ToDataStream(dataStream);

      // from stream. 
      dtCore::RefPtr<dtGame::ActorUpdateMessage> messageFromStream = new dtGame::ActorUpdateMessage;
      messageFromStream->FromDataStream(dataStream);

      // Get pos & rot from the message
      eap->ApplyActorUpdate(*messageFromStream, false);
      osg::Vec3 posFromMessage, rotFromMessage;
      posFromMessage = drHelper->GetLastKnownTranslation();
      rotFromMessage = drHelper->GetLastKnownRotation();

      // compare values.
      std::ostringstream ss1;
      ss1 << "Test #[" << counter << "] - Pos from the data stream should be [" << newPos << "] but it is [" << posFromMessage << "].";
      CPPUNIT_ASSERT_MESSAGE(ss1.str(), 
            osg::equivalent(posFromMessage.x(), newPos.x(), 1e-2f) &&
            osg::equivalent(posFromMessage.y(), newPos.y(), 1e-2f) &&
            osg::equivalent(posFromMessage.z(), newPos.z(), 1e-2f));

      std::ostringstream ss2;
      ss2 << "Test #[" << counter << "] - Rotation from the data stream should be [" << newRot << "] but it is [" << rotFromMessage << "].";
      CPPUNIT_ASSERT_MESSAGE(ss2.str(), 
            osg::equivalent(rotFromMessage.x(), newRot.x(), 1e-2f) &&
            osg::equivalent(rotFromMessage.y(), newRot.y(), 1e-2f) &&
            osg::equivalent(rotFromMessage.z(), newRot.z(), 1e-2f));

   }

}

void BaseEntityActorProxyTests::TestPlatformDRRegistration()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   mGM->AddActor(*eap, false, false);
   CPPUNIT_ASSERT_MESSAGE("Entity should be added when it's not remote.", mDeadReckoningComponent->IsRegisteredActor(*eap));
   mGM->DeleteActor(*eap);

   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   TestBaseEntityDRRegistration(*eap);
}

void BaseEntityActorProxyTests::TestHumanDRRegistration()
{
   RefPtr<SimCore::Actors::HumanActorProxy> hap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, hap);
   CPPUNIT_ASSERT(hap.valid());

   mGM->AddActor(*hap, false, false);
   CPPUNIT_ASSERT_MESSAGE("Entity should be added when it's not remote.", mDeadReckoningComponent->IsRegisteredActor(*hap));
   mGM->DeleteActor(*hap);

   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, hap);
   CPPUNIT_ASSERT(hap.valid());

   TestBaseEntityDRRegistration(*hap);
}

void BaseEntityActorProxyTests::TestBaseEntityDRRegistration(SimCore::Actors::BaseEntityActorProxy& actor)
{
   mGM->AddActor(actor, true, false);
   CPPUNIT_ASSERT_MESSAGE("Entity Is remote, so it should add itself to the Dead Reckoning component.",
         mDeadReckoningComponent->IsRegisteredActor(actor));

   dtCore::Transform xform;
   actor.GetDrawable<dtCore::Transformable>()->GetTransform(xform);
   osg::Vec3 vec;
   xform.GetTranslation(vec);
   CPPUNIT_ASSERT(osg::equivalent(vec.x(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.y(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.z(), 0.0f, 1e-2f)
   );
   xform.GetRotation(vec);
   CPPUNIT_ASSERT(osg::equivalent(vec.x(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.y(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.z(), 0.0f, 1e-2f)
   );

   SimCore::Actors::BaseEntity& entity = *actor.GetDrawable<SimCore::Actors::BaseEntity>();

   osg::Vec3 setVec = osg::Vec3(1.0, 1.2, 1.3);

   dtGame::DeadReckoningHelper* drHelper = NULL;
   actor.GetComponent(drHelper);

   drHelper->SetLastKnownTranslation(setVec);
   drHelper->SetLastKnownRotation(setVec);
   drHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::NONE);
   drHelper->SetGroundClampType(dtGame::GroundClampTypeEnum::NONE); //SetFlying(true);

   dtCore::System::GetInstance().Step();

   entity.GetTransform(xform);
   xform.GetTranslation(vec);
   CPPUNIT_ASSERT(osg::equivalent(vec.x(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.y(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.z(), 0.0f, 1e-2f)
   );
   xform.GetRotation(vec);
   CPPUNIT_ASSERT(osg::equivalent(vec.x(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.y(), 0.0f, 1e-2f) &&
         osg::equivalent(vec.z(), 0.0f, 1e-2f)
   );

   drHelper->SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::STATIC);
   dtCore::System::GetInstance().Step();

   entity.GetTransform(xform);
   xform.GetTranslation(vec);
   std::ostringstream ss;
   ss << "Translation should be " << setVec << " but it is " << vec << ".";

   CPPUNIT_ASSERT_MESSAGE(ss.str(),
         osg::equivalent(vec.x(), setVec.x(), 1e-2f) &&
         osg::equivalent(vec.y(), setVec.y(), 1e-2f) &&
         osg::equivalent(vec.z(), setVec.z(), 1e-2f)
   );
   xform.GetRotation(vec);

   ss.str("");
   ss << "Rotation should be " << setVec << " but it is " << vec << ".";
   CPPUNIT_ASSERT_MESSAGE(ss.str(),
         osg::equivalent(vec.x(), setVec.x(), 1e-2f) &&
         osg::equivalent(vec.y(), setVec.y(), 1e-2f) &&
         osg::equivalent(vec.z(), setVec.z(), 1e-2f)
   );

   mDeadReckoningComponent->UnregisterActor(actor);
   CPPUNIT_ASSERT(!mDeadReckoningComponent->IsRegisteredActor(actor));
}

void BaseEntityActorProxyTests::TestPlayerActorProxy()
{
   RefPtr<dtGame::TestComponent> tc = new dtGame::TestComponent;
   mGM->AddComponent(*tc, dtGame::GameManager::ComponentPriority::HIGHEST);
   RefPtr<SimCore::Actors::PlayerActorProxy> pa;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, pa);
   CPPUNIT_ASSERT(pa != NULL);

   mGM->AddActor(*pa, false, false);
   dtCore::AppSleep(10);
   dtCore::System::GetInstance().Step();
   RefPtr<const dtGame::Message> msg =
         tc->FindProcessMessageOfType(dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD);

   CPPUNIT_ASSERT_MESSAGE("A player entered the world, a player enter world message should have been found", msg.valid());
   CPPUNIT_ASSERT_MESSAGE("The message found should be of the correct type", msg->GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD);

   tc->reset();
   mGM->DeleteAllActors();
   dtCore::System::GetInstance().Step();

   mGM->AddActor(*pa, true, false);
   dtCore::AppSleep(10);
   dtCore::System::GetInstance().Step();

   msg = tc->FindProcessMessageOfType(dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD);
   CPPUNIT_ASSERT_MESSAGE("The player actor was added remotely. A player entered world message should have been sent.", msg.valid());
}

void BaseEntityActorProxyTests::TestDetonationActorProxy()
{
   //SimCore::Actors::DetonationActor::SetLingeringShotSecs(2);
   RefPtr<SimCore::Actors::DetonationActorProxy> dap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, dap);
   CPPUNIT_ASSERT(dap.valid());
   SimCore::Actors::DetonationActor *da =
         static_cast<SimCore::Actors::DetonationActor*>(&dap->GetGameActor());

   da->SetSmokeLifeTime(0.000001);
   da->SetDeleteActorTimerSecs(0.0001);
   da->SetExplosionTimerSecs(0.0001);

   // TODO Add detonation to GM, set a timer and everything, ensure it fires and deletes
   // itself via its invokable
   mGM->AddActor(*dap, false, false);

   dtCore::AppSleep(1);
   dtCore::System::GetInstance().Step();

   dtCore::AppSleep(1);
   dtCore::System::GetInstance().Step();

   dtCore::AppSleep(1);
   dtCore::System::GetInstance().Step();

   std::vector<dtGame::GameActorProxy*> proxies;
   mGM->GetAllGameActors(proxies);

   CPPUNIT_ASSERT_MESSAGE("The timer has elapsed, the detonation actor should no longer be in the GM", proxies.empty());

   //Make sure these don't ACTUALLY get deleted before the end of test.
   //This is a work around for a bug is sigslot.h
   std::vector<RefPtr<SimCore::Actors::DetonationActorProxy> > detList;
   std::vector<RefPtr<dtAudio::Sound> > soundList;

   SimCore::Actors::DetonationActor::IMPACT_TYPE curType = SimCore::Actors::DetonationActor::IMPACT_ENTITY;

   const unsigned short int numDets = 20;
   for(unsigned int i = 0; i < numDets; i++)
   {
      RefPtr<SimCore::Actors::DetonationActorProxy> d;
      mGM->CreateActor("Effects", "Detonation Actor", d);
      SimCore::Actors::DetonationActor& detActor = static_cast<SimCore::Actors::DetonationActor&>(d->GetGameActor());
      detActor.SetSmokeLifeTime(0.001);
      detActor.SetExplosionTimerSecs(0.001);
      detActor.SetDeleteActorTimerSecs(0.001);

      detActor.SetGroundImpactSound(dtCore::ResourceDescriptor("Sounds:silence.wav"));
      detActor.SetHumanImpactSound(dtCore::ResourceDescriptor("Sounds:tank_fire.wav"));
      detActor.SetEntityImpactSound(dtCore::ResourceDescriptor::NULL_RESOURCE);
      detActor.SetGroundImpactEffect(dtCore::ResourceDescriptor("Particles:DirtHit.osg"));
      detActor.SetHumanImpactEffect(dtCore::ResourceDescriptor("Particles:weapon_flash.osg"));
      detActor.SetEntityImpactEffect(dtCore::ResourceDescriptor::NULL_RESOURCE);

      detActor.SetImpactType(curType);

      mGM->AddActor(*d, false, false);

      switch (curType)
      {
      case SimCore::Actors::DetonationActor::IMPACT_ENTITY:
         // The sound isn't null even though the entity dosen't have one because the ground impact version
         // will take over if it doesn't
         CPPUNIT_ASSERT(detActor.GetSound() != NULL);
         soundList.push_back(detActor.GetSound());
         CPPUNIT_ASSERT(std::string(detActor.GetSound()->GetFilename()).find("silence.wav") != std::string::npos);
         curType = SimCore::Actors::DetonationActor::IMPACT_HUMAN;
         break;
      case SimCore::Actors::DetonationActor::IMPACT_HUMAN:
         CPPUNIT_ASSERT(detActor.GetSound() != NULL);
         soundList.push_back(detActor.GetSound());
         CPPUNIT_ASSERT(std::string(detActor.GetSound()->GetFilename()).find("tank_fire.wav") != std::string::npos);
         curType = SimCore::Actors::DetonationActor::IMPACT_TERRAIN;
         break;
      case SimCore::Actors::DetonationActor::IMPACT_TERRAIN:
         CPPUNIT_ASSERT(detActor.GetSound() != NULL);
         CPPUNIT_ASSERT(std::string(detActor.GetSound()->GetFilename()).find("silence.wav") != std::string::npos);
         soundList.push_back(detActor.GetSound());
         curType = SimCore::Actors::DetonationActor::IMPACT_ENTITY;
         break;
      }

      detList.push_back(d);
   }

   dtCore::AppSleep(1);
   dtCore::System::GetInstance().Step();

   dtCore::AppSleep(1);
   dtCore::System::GetInstance().Step();

   dtCore::AppSleep(1);
   dtCore::System::GetInstance().Step();

   mGM->GetAllGameActors(proxies);

   CPPUNIT_ASSERT_MESSAGE("There should NOT be any detonation actors in the GM after their timers expired",  proxies.empty());

   for (unsigned i = 0; i < detList.size(); ++i)
   {
      SimCore::Actors::DetonationActor* detActor = NULL;
      detList[i]->GetDrawable(detActor);
      CPPUNIT_ASSERT(detActor != NULL);
      // The sound should have been set the NULL.
      CPPUNIT_ASSERT(detActor->GetSound() == NULL);
   }

   for (unsigned i = 0; i < soundList.size(); ++i)
   {
      // Check to see if the sound has been freed by checking if the filename
      CPPUNIT_ASSERT(std::string(soundList[i]->GetFilename()).empty());
   }

   //must delete the actor before shutting down the audio manager.
   dap = NULL;
}

void BaseEntityActorProxyTests::TestDetonationSoundDelay()
{
   mGM->AddComponent(*new SimCore::Components::ViewerMessageProcessor, dtGame::GameManager::ComponentPriority::HIGHEST);

   RefPtr<SimCore::Actors::DetonationActorProxy> dap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::DETONATION_ACTOR_TYPE, dap);
   CPPUNIT_ASSERT(dap.valid());
   SimCore::Actors::DetonationActor &da = static_cast<SimCore::Actors::DetonationActor&>(dap->GetGameActor());

   // The detonation is at 0, 0, 0 and the position is at 0, 350, 0.
   // This should produce somewhere near a 1 second delay
   osg::Vec3 pos(0, 350, 0);
   da.CalculateDelayTime(pos);
   CPPUNIT_ASSERT_MESSAGE("The delay time should be reasonably close to 1 second", osg::equivalent(da.GetDelayTime(), 1.0f, 0.01f));

   da.SetTransform(dtCore::Transform(350, 0, 350));
   pos.set(350, -700, 350);
   da.CalculateDelayTime(pos);
   CPPUNIT_ASSERT_MESSAGE("The delay time should be reasonably close to 2 seconds", osg::equivalent(da.GetDelayTime(), 2.0f, 0.01f));

   pos.set(350, 700, 350);
   da.CalculateDelayTime(pos);
   CPPUNIT_ASSERT_MESSAGE("The delay time should be reasonably close to 2 seconds", osg::equivalent(da.GetDelayTime(), 2.0f, 0.01f));

   da.SetTransform(dtCore::Transform(350, 350, 350));
   pos.set(350, 350, 700);
   da.CalculateDelayTime(pos);
   CPPUNIT_ASSERT_MESSAGE("The delay time should be reasonably close to 1 seconds", osg::equivalent(da.GetDelayTime(), 1.0f, 0.01f));

   pos.set(1050, 350, 350);
   da.CalculateDelayTime(pos);
   CPPUNIT_ASSERT_MESSAGE("The delay time should be reasonably close to 2 seconds", osg::equivalent(da.GetDelayTime(), 2.0f, 0.01f));
}

void BaseEntityActorProxyTests::TestBaseWaterActorProxy()
{
   RefPtr<SimCore::Actors::BaseWaterActorProxy> waterProxy;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::BASE_WATER_ACTOR_TYPE, waterProxy);

   CPPUNIT_ASSERT(waterProxy.valid());
   SimCore::Actors::BaseWaterActor* waterActor = NULL;
   waterProxy->GetDrawable( waterActor );
   const SimCore::Actors::BaseWaterActor* constWaterActor = waterActor;

   float testValue = 1234.56789f;
   float testHeight = 0.0f;
   osg::Vec3 detectPoint;
   osg::Vec3 testNormal;
   const osg::Vec3 worldAxixZ( 0.0f, 0.0f, 1.0f );
   CPPUNIT_ASSERT( constWaterActor->GetHeightAndNormalAtPoint( detectPoint, testHeight, testNormal ) );
   CPPUNIT_ASSERT( testHeight == 0.0f );
   CPPUNIT_ASSERT( testNormal == worldAxixZ );

   CPPUNIT_ASSERT( constWaterActor->GetWaterHeight() == 0.0f );
   waterActor->SetWaterHeight( testValue );
   CPPUNIT_ASSERT( constWaterActor->GetWaterHeight() == testValue );

   testNormal.set( 0.0f, 0.0f, 0.0f );
   CPPUNIT_ASSERT( constWaterActor->GetHeightAndNormalAtPoint( detectPoint, testHeight, testNormal ) );
   CPPUNIT_ASSERT( testHeight == testValue );
   CPPUNIT_ASSERT( testNormal == worldAxixZ );
}

void BaseEntityActorProxyTests::TestFindMuzzleNodes()
{
   RefPtr<SimCore::Actors::PlatformActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   CPPUNIT_ASSERT(!eap->GetHasLoadedResources());

   SimCore::Actors::Platform* platform = NULL;
   eap->GetDrawable(platform);

   dtCore::ResourceDescriptor happySphere("StaticMeshes:physics_happy_sphere.ive");
   dtCore::ResourceDescriptor tank("StaticMeshes:T80:t80u_good.ive");

   platform->LoadDamageableFile(tank, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE);

   std::vector<osg::Group*> muzzles;
   platform->GetWeaponMuzzles(muzzles);

   CPPUNIT_ASSERT_EQUAL(1U, unsigned(muzzles.size()));

   platform->LoadDamageableFile(happySphere, SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::NO_DAMAGE);

   muzzles.clear();
   platform->GetWeaponMuzzles(muzzles);

   CPPUNIT_ASSERT_EQUAL(0U, unsigned(muzzles.size()));

}

class SubBaseEntityManualModel : public SimCore::Actors::BaseEntity
{
public:
   SubBaseEntityManualModel()
   // Don't try this at home.
   : BaseEntity(*new dtGame::GameActorProxy)
   {
      dtCore::RefPtr<osgSim::DOFTransform> dof = new osgSim::DOFTransform();
      dof->setName(SimCore::Actors::BaseEntity::MUZZLE_NODE_PREFIX + "01");
      GetScaleMatrixTransform().addChild(dof);
      dtCore::RefPtr<osg::MatrixTransform> mt = new osg::MatrixTransform();
      mt->setName(SimCore::Actors::BaseEntity::MUZZLE_NODE_PREFIX + "02");
      dof->addChild(mt);
      dtCore::RefPtr<osg::Group> gp = new osg::Group();
      gp->setName(SimCore::Actors::BaseEntity::MUZZLE_NODE_PREFIX + "03");
      dof->addChild(gp);

      dof->setCurrentHPR(osg::Vec3(-25.0, 0.4, 0.0));
      dof->setCurrentScale(osg::Vec3(1.0, 1.0, 1.0));
      dof->setCurrentTranslate(osg::Vec3(0.0, 0.0, 0.0));
      osg::Matrix m;
      dtCore::Transform tx;
      tx.SetTranslation(-5.0f, 5.0, 5.0f);
      tx.SetRotation(-30.0, 0.3, 0.0);
      tx.Get(m);
      mt->setMatrix(m);

      LoadNodeCollector();
   }
};


void BaseEntityActorProxyTests::TestFindMuzzleNodesWithDirection()
{
   dtCore::RefPtr<SubBaseEntityManualModel> entity = new SubBaseEntityManualModel;
   osg::Vec3 dir(0.0, -1.0, 0.0);

   std::vector<osg::Group*> muzzles;
   entity->GetWeaponMuzzles(muzzles);
   CPPUNIT_ASSERT_EQUAL(3U, unsigned(muzzles.size()));

   osg::Group* matchingMuzzle = entity->GetWeaponMuzzleForDirection(dir);
   CPPUNIT_ASSERT(matchingMuzzle != NULL);
   CPPUNIT_ASSERT_EQUAL(SimCore::Actors::BaseEntity::MUZZLE_NODE_PREFIX + "02", matchingMuzzle->getName());

}

