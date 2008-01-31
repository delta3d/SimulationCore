/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

#include <osg/io_utils>
#include <osg/Math>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/actorupdatemessage.h>

#include <dtDAL/actorproperty.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>

#include <dtCore/system.h>
#include <dtCore/scene.h>
#include <dtCore/globals.h>
#include <dtCore/refptr.h>
#include <dtCore/observerptr.h>

#include <dtUtil/macros.h>

#include <dtAudio/audiomanager.h>

#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Actors/Human.h>
#include <SimCore/Actors/Platform.h>
#include <SimCore/Actors/PlayerActor.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/ViewerMaterialActor.h>

#include <dtDAL/transformableactorproxy.h>

#include <SimCore/Components/ViewerMessageProcessor.h>

#include <TestComponent.h>
#include <UnitTestMain.h>
#include <dtABC/application.h>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaWorldComponent.h>
#endif

#ifdef DELTA_WIN32
   #include <Windows.h>
   #define SLEEP(milliseconds) Sleep((milliseconds))
#else
   #include <unistd.h>
   #define SLEEP(milliseconds) usleep(((milliseconds) * 1000))
#endif

using dtCore::RefPtr;
using dtCore::ObserverPtr;


class BaseEntityActorProxyTests : public CPPUNIT_NS::TestFixture 
{
   CPPUNIT_TEST_SUITE(BaseEntityActorProxyTests);

      CPPUNIT_TEST(TestPlatform);
      CPPUNIT_TEST(TestHuman);
      CPPUNIT_TEST(TestPlatformScaleMagnification);
      CPPUNIT_TEST(TestHumanScaleMagnification);
      CPPUNIT_TEST(TestPlatformActorUpdates);
      CPPUNIT_TEST(TestHumanActorUpdates);
      CPPUNIT_TEST(TestPlatformDRRegistration);
      CPPUNIT_TEST(TestHumanDRRegistration);
      CPPUNIT_TEST(TestPlayerActorProxy);
      CPPUNIT_TEST(TestDetonationActorProxy);
      CPPUNIT_TEST(TestDetonationSoundDelay);

   CPPUNIT_TEST_SUITE_END();

   public:

      void setUp();
      void tearDown();
      void TestPlatform();
      void TestHuman();
      void TestPlatformScaleMagnification();
      void TestHumanScaleMagnification();
      void TestPlatformActorUpdates();
      void TestHumanActorUpdates();
      void TestPlatformDRRegistration();
      void TestHumanDRRegistration();
      void TestPlayerActorProxy();
      void TestDetonationActorProxy();
      void TestDetonationSoundDelay();

   private:
      void TestScaleMagnification(SimCore::Actors::BaseEntityActorProxy&);
      void TestBaseEntityActorProxy(SimCore::Actors::BaseEntityActorProxy&);
      void TestBaseEntityActorUpdates(SimCore::Actors::BaseEntityActorProxy&);
      void TestBaseEntityDRRegistration(SimCore::Actors::BaseEntityActorProxy&);

      RefPtr<dtGame::GameManager> mGM;
      RefPtr<dtGame::DeadReckoningComponent> mDeadReckoningComponent;

#ifdef AGEIA_PHYSICS
      RefPtr<dtAgeiaPhysX::NxAgeiaWorldComponent> mWorldComp;
#endif
};

CPPUNIT_TEST_SUITE_REGISTRATION(BaseEntityActorProxyTests);

void BaseEntityActorProxyTests::setUp()
{
   dtCore::System::GetInstance().SetShutdownOnWindowClose(false);
   dtCore::System::GetInstance().Start();
   mGM = new dtGame::GameManager(*GetGlobalApplication().GetScene());
   
   mDeadReckoningComponent = new dtGame::DeadReckoningComponent();
   mGM->AddComponent(*mDeadReckoningComponent, dtGame::GameManager::ComponentPriority::NORMAL);

#ifdef AGEIA_PHYSICS
   mWorldComp = new dtAgeiaPhysX::NxAgeiaWorldComponent;
   mGM->AddComponent(*mWorldComp, dtGame::GameManager::ComponentPriority::NORMAL);
#endif

}

void BaseEntityActorProxyTests::tearDown()
{
   mDeadReckoningComponent = NULL;

#ifdef AGEIA_PHYSICS
   mWorldComp = NULL;
#endif

   if (mGM.valid())
   {
      mGM->DeleteAllActors(true);
      mGM = NULL;
   }
   dtCore::System::GetInstance().Stop();
}

void BaseEntityActorProxyTests::TestPlatform()
{
   RefPtr<SimCore::Actors::BaseEntityActorProxy> eap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLATFORM_ACTOR_TYPE, eap);
   CPPUNIT_ASSERT(eap.valid());

   ObserverPtr<dtCore::DeltaDrawable> edraw = eap->GetActor();
   
   TestBaseEntityActorProxy(*eap);
   
   dtDAL::ActorProperty *prop = NULL;
   
   prop = eap->GetProperty(SimCore::Actors::PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED);
   CPPUNIT_ASSERT_MESSAGE("The head lights property should not be NULL", prop != NULL);
   std::stringstream textMessage;
   textMessage << "The default value of \"" 
      << SimCore::Actors::PlatformActorProxy::PROPERTY_HEAD_LIGHTS_ENABLED << "\" should be false.";
   CPPUNIT_ASSERT_MESSAGE(textMessage.str(), !static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());
   static_cast<dtDAL::BooleanActorProperty*>(prop)->SetValue(true);
   CPPUNIT_ASSERT(static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());

   CPPUNIT_ASSERT_EQUAL(1, eap->referenceCount());
   eap = NULL;
   CPPUNIT_ASSERT(!edraw.valid());
   edraw = NULL;
}

void BaseEntityActorProxyTests::TestHuman()
{
   RefPtr<SimCore::Actors::HumanActorProxy> hap;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::HUMAN_ACTOR_TYPE, hap);
   CPPUNIT_ASSERT(hap.valid());

   ObserverPtr<dtCore::DeltaDrawable> edraw = hap->GetActor();
   
   TestBaseEntityActorProxy(*hap);
   
   CPPUNIT_ASSERT_EQUAL(1, hap->referenceCount());
   hap = NULL;
   CPPUNIT_ASSERT(!edraw.valid());
   edraw = NULL;
}

void BaseEntityActorProxyTests::TestBaseEntityActorProxy(SimCore::Actors::BaseEntityActorProxy& eap)
{
   //make the actor 
   mGM->AddActor(eap, true, false);

   dtDAL::ActorProperty *prop = NULL;
   
   prop = eap.GetProperty("Firepower Disabled");
   CPPUNIT_ASSERT_MESSAGE("The firepower property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The default value of \"Firepower Disabled\" should be false.", 
            !static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());
   static_cast<dtDAL::BooleanActorProperty*>(prop)->SetValue(true);
   CPPUNIT_ASSERT(static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());

   prop = eap.GetProperty("Mobility Disabled");
   CPPUNIT_ASSERT_MESSAGE("The mobility property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The default value of \"Mobility Disabled\" should be false.", 
            !static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());
   static_cast<dtDAL::BooleanActorProperty*>(prop)->SetValue(true);
   CPPUNIT_ASSERT(static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());

   osg::Vec3 vec(5.0f, 5.0f, 5.0f);
   prop = eap.GetProperty("Angular Velocity Vector");
   CPPUNIT_ASSERT_MESSAGE("The angular velocity vector property should not be NULL", prop != NULL);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", 
            static_cast<dtDAL::Vec3fActorProperty*>(prop)->GetValue() == vec);

   prop = eap.GetProperty("Last Known Translation");
   CPPUNIT_ASSERT_MESSAGE("The last known translation property should not be NULL", prop != NULL);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", 
            static_cast<dtDAL::Vec3fActorProperty*>(prop)->GetValue() == vec);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(osg::Vec3(0.0f, 0.0f, 0.0f));

   prop = eap.GetProperty("Last Known Rotation");
   CPPUNIT_ASSERT_MESSAGE("The last known rotation property should not be NULL", prop != NULL);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::Vec3fActorProperty*>(prop)->GetValue() == vec);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(osg::Vec3(0.0f, 0.0f, 0.0f));

   vec.set(2.3f, 1.4f, 8.3f);
   prop = eap.GetProperty("Velocity Vector");
   CPPUNIT_ASSERT_MESSAGE("The velocity vector property should not be NULL", prop != NULL);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::Vec3fActorProperty*>(prop)->GetValue() == vec);

   prop = eap.GetProperty("Damage State");
   dtDAL::AbstractEnumActorProperty *aep = dynamic_cast<dtDAL::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property should not be NULL", aep != NULL);
   aep->SetValueFromString("Destroyed");
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName() == "Destroyed");
   aep->SetValueFromString("No Damage");
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName() == "No Damage");

   prop = eap.GetProperty("Dead Reckoning Algorithm");
   aep = dynamic_cast<dtDAL::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property should not be NULL", aep != NULL);
   CPPUNIT_ASSERT_MESSAGE("The \"Dead Reckoning Algorithm\" should default to NONE", aep->GetEnumValue().GetName() == "None");
   aep->SetValueFromString("Velocity Only");
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", aep->GetEnumValue().GetName() == "Velocity Only");

   prop = eap.GetProperty("Force Affiliation");
   aep = dynamic_cast<dtDAL::AbstractEnumActorProperty*>(prop);
   CPPUNIT_ASSERT_MESSAGE("The abstract enum property for \"Force Affiliation\" should not be NULL", aep != NULL);
   CPPUNIT_ASSERT_MESSAGE("The \"Force Affilition\" should default to NEUTRAL", aep->GetEnumValue() == SimCore::Actors::BaseEntityActorProxy::ForceEnum::NEUTRAL);
   aep->SetValueFromString("FRIENDLY");
   CPPUNIT_ASSERT_MESSAGE("The \"Force Affilition\" should now be friendly", aep->GetEnumValue() == SimCore::Actors::BaseEntityActorProxy::ForceEnum::FRIENDLY);

   osg::Vec3 translation(0.0f, 0.0f, 0.0f);
   prop = eap.GetProperty(dtDAL::TransformableActorProxy::PROPERTY_TRANSLATION);
   static_cast<dtDAL::Vec3ActorProperty*>(prop)->SetValue(translation);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::Vec3ActorProperty*>(prop)->GetValue() == translation);

   dtDAL::ActorProperty* flyingProp = eap.GetProperty("Flying");
   CPPUNIT_ASSERT_MESSAGE("The default value of falling should be false.", !static_cast<dtDAL::BooleanActorProperty*>(flyingProp)->GetValue());
   static_cast<dtDAL::BooleanActorProperty*>(flyingProp)->SetValue(true);
   CPPUNIT_ASSERT(static_cast<dtDAL::BooleanActorProperty*>(flyingProp)->GetValue());

   dtDAL::ActorProperty* offsetProp = eap.GetProperty("Ground Offset");
   CPPUNIT_ASSERT_MESSAGE("The ground offset should be close 0.0.", osg::equivalent(static_cast<dtDAL::FloatActorProperty*>(offsetProp)->GetValue(), 0.0f, 0.1f));
   static_cast<dtDAL::FloatActorProperty*>(offsetProp)->SetValue(1.3f);
   CPPUNIT_ASSERT(static_cast<dtDAL::FloatActorProperty*>(offsetProp)->GetValue() == 1.3f);
   
   dtDAL::ActorProperty* drawingProp = eap.GetProperty("DrawingModel");
   CPPUNIT_ASSERT(drawingProp != NULL);
   CPPUNIT_ASSERT_MESSAGE("The default value of drawing should be true.", static_cast<dtDAL::BooleanActorProperty*>(drawingProp)->GetValue());
   static_cast<dtDAL::BooleanActorProperty*>(flyingProp)->SetValue(false);
   CPPUNIT_ASSERT(!static_cast<dtDAL::BooleanActorProperty*>(flyingProp)->GetValue());
      
   vec.set(4.3f, 8.1f, 7.69f);
   prop = eap.GetProperty("Acceleration Vector");
   CPPUNIT_ASSERT_MESSAGE("The acceleration property property should not be NULL", prop != NULL);
   static_cast<dtDAL::Vec3fActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::Vec3fActorProperty*>(prop)->GetValue() == vec);

   vec.set(1.14, 8.21, 7.85);
   prop = eap.GetProperty("EngineSmokePosition");
   CPPUNIT_ASSERT_MESSAGE("The engine smoke position property should not be NULL", prop != NULL);
   static_cast<dtDAL::Vec3ActorProperty*>(prop)->SetValue(vec);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::Vec3ActorProperty*>(prop)->GetValue() == vec);

   bool b = true;
   prop = eap.GetProperty("EngineSmokeOn");
   CPPUNIT_ASSERT_MESSAGE("The engine smoke on property should not be NULL", prop != NULL);
   static_cast<dtDAL::BooleanActorProperty*>(prop)->SetValue(b);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());

   SimCore::Actors::HumanActorProxy *hap = dynamic_cast<SimCore::Actors::HumanActorProxy*>(&eap);
   if(hap == NULL)
   {
      prop = eap.GetProperty("FlamesPresent");
      CPPUNIT_ASSERT_MESSAGE("The flames present property should not be NULL", prop != NULL);
      static_cast<dtDAL::BooleanActorProperty*>(prop)->SetValue(b);
      CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());

      prop = eap.GetProperty("SmokePlumePresent");
      CPPUNIT_ASSERT_MESSAGE("The smoke plume present on property should not be NULL", prop != NULL);
      static_cast<dtDAL::BooleanActorProperty*>(prop)->SetValue(b);
      CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::BooleanActorProperty*>(prop)->GetValue());
   }

   dtDAL::ActorProperty *ap = eap.GetProperty("Service");
   CPPUNIT_ASSERT(ap != NULL);
   dtDAL::AbstractEnumActorProperty *aeap = dynamic_cast<dtDAL::AbstractEnumActorProperty*>(ap);
   CPPUNIT_ASSERT(aeap != NULL);
   dtUtil::Enumeration &e = aeap->GetEnumValue();
   CPPUNIT_ASSERT_MESSAGE("The service property should default to Marines", e == SimCore::Actors::BaseEntityActorProxy::ServiceEnum::MARINES);
   aeap->SetEnumValue(SimCore::Actors::BaseEntityActorProxy::ServiceEnum::ARMY);
   dtUtil::Enumeration &newE = aeap->GetEnumValue();
   CPPUNIT_ASSERT_MESSAGE("The service property should return what was set", newE == SimCore::Actors::BaseEntityActorProxy::ServiceEnum::ARMY);

   prop = eap.GetProperty("Smoke plume particles");
   CPPUNIT_ASSERT_MESSAGE("The \"Smoke plume particles\" property should not be NULL", prop != NULL);
   dtDAL::ResourceDescriptor* rd = static_cast<dtDAL::ResourceActorProperty*>(prop)->GetValue();
   CPPUNIT_ASSERT_MESSAGE("\"Smoke plume particles\" value should not be NULL", rd != NULL);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("\"Smoke plume particles\" value should be", rd->GetResourceIdentifier(), std::string("Particles:smoke.osg"));

   prop = eap.GetProperty("Fire particles");
   CPPUNIT_ASSERT_MESSAGE("The \"Fire particles\" property should not be NULL", prop != NULL);
   rd = static_cast<dtDAL::ResourceActorProperty*>(prop)->GetValue();
   CPPUNIT_ASSERT_MESSAGE("\"Fire particles\" value should not be NULL", rd != NULL);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("\"Fire particles\" value should be", rd->GetResourceIdentifier(), std::string("Particles:fire.osg"));

   prop = eap.GetProperty("Engine smoke particles");
   CPPUNIT_ASSERT_MESSAGE("The \"Engine smoke particles\" property should not be NULL", prop != NULL);
   rd = static_cast<dtDAL::ResourceActorProperty*>(prop)->GetValue();
   CPPUNIT_ASSERT_MESSAGE("\"Engine smoke particles\" value should not be NULL", rd != NULL);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("\"Engine smoke particles\" value should be", rd->GetResourceIdentifier(), std::string("Particles:smoke.osg"));

   std::string munitionTableName("LARGE EXPLOSION");
   prop = eap.GetProperty("Munition Damage Table");
   CPPUNIT_ASSERT_MESSAGE("The \"Munition Damage Table\" property should not be NULL", prop != NULL);
   static_cast<dtDAL::StringActorProperty*>(prop)->SetValue(munitionTableName);
   CPPUNIT_ASSERT_MESSAGE("GetValue should return what was set", static_cast<dtDAL::StringActorProperty*>(prop)->GetValue() == munitionTableName);

   mGM->DeleteActor(eap);
   dtCore::System::GetInstance().Step();

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
   
   dtDAL::ActorProperty* prop = NULL;
   dtDAL::Vec3ActorProperty* v3Prop = NULL;
   
   osg::Vec3 defScale(4.3f, 8.1f, 7.69f);
   prop = eap.GetProperty("Default Scale");
   CPPUNIT_ASSERT_MESSAGE("The Default Scale property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The Default Scale property type should be Vec3", prop->GetPropertyType() == dtDAL::DataType::VEC3);
   v3Prop = static_cast<dtDAL::Vec3ActorProperty*>(prop);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Default Scale should default to 1, 1, 1", osg::Vec3(1.0f, 1.0f, 1.0f), v3Prop->GetValue());
   v3Prop->SetValue(defScale);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue for Default Scale should return what was set", defScale, v3Prop->GetValue());

   osg::Vec3 scaleMag(3.0f, 3.7f, 3.9f);
   prop = eap.GetProperty("Scale Magnification Factor");
   CPPUNIT_ASSERT_MESSAGE("The Scale Magnification Factor property should not be NULL", prop != NULL);
   CPPUNIT_ASSERT_MESSAGE("The Scale Magnification Factor property type should be Vec3", prop->GetPropertyType() == dtDAL::DataType::VEC3);
   v3Prop = static_cast<dtDAL::Vec3ActorProperty*>(prop);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("Scale Magnification Factor should default to 1, 1, 1.", osg::Vec3(1.0f, 1.0f, 1.0f), v3Prop->GetValue());
   v3Prop->SetValue(scaleMag);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("GetValue for Default Scale should return what was set", scaleMag, v3Prop->GetValue());

   prop = eap.GetProperty("Model Scale");
   v3Prop = static_cast<dtDAL::Vec3ActorProperty*>(prop);
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
   RefPtr<TestComponent> tc = new TestComponent;
   mGM->AddComponent(*tc, dtGame::GameManager::ComponentPriority::HIGHEST);

   mGM->AddActor(eap, false, true);
   
   SimCore::Actors::BaseEntity& entityActor = static_cast<SimCore::Actors::BaseEntity&>(eap.GetGameActor());

   double oldTime = dtCore::System::GetInstance().GetSimulationTime();
   
   CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
         "The time until next update should be seeded to the time between complete updates.",
         SimCore::Actors::BaseEntity::TIME_BETWEEN_UPDATES, 
         entityActor.GetTimeUntilNextUpdate(), 1e-3f);

   dtCore::System::GetInstance().Step();

   double newTime = dtCore::System::GetInstance().GetSimulationTime();

   CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
         "The time until the next update should decrement by the step time",
         SimCore::Actors::BaseEntity::TIME_BETWEEN_UPDATES - (newTime - oldTime), 
         entityActor.GetTimeUntilNextUpdate(), 1e-3f);
   
   CPPUNIT_ASSERT(tc->FindProcessMessageOfType(dtGame::MessageType::INFO_ACTOR_CREATED).valid());
   
   osg::Vec3 smallMovement(0.1f, 0.3f, 0.4f);
   
   dtCore::Transform xform;
   entityActor.GetTransform(xform, dtCore::Transformable::REL_CS);
   xform.SetTranslation(xform.GetTranslation() + smallMovement);
   entityActor.SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT_MESSAGE("The translation was small. It should NOT have sent an update.",
      !tc->FindProcessMessageOfType(dtGame::MessageType::INFO_ACTOR_UPDATED).valid());


   osg::Vec3 largeMovement(200.0f, 345.0f, 657.0f);
   xform.SetTranslation(xform.GetTranslation() + largeMovement);
   entityActor.SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::System::GetInstance().Step();

   RefPtr<const dtGame::ActorUpdateMessage> update1 = 
      static_cast<const dtGame::ActorUpdateMessage*>(tc->FindProcessMessageOfType(
                                               dtGame::MessageType::INFO_ACTOR_UPDATED).get());

   entityActor.GetTransform(xform, dtCore::Transformable::REL_CS);

   std::ostringstream ss;
   ss << "The translation was large. It should have sent an update.  New position is: " 
      << xform.GetTranslation();
   
   CPPUNIT_ASSERT_MESSAGE(ss.str(),
                          update1.valid());
                          
   std::vector<const dtGame::MessageParameter*> toFill;
   update1->GetUpdateParameters(toFill);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The update message should exactly have 2 update parameters in it.",
                                size_t(2), toFill.size()); 

   tc->reset();

   xform.SetRotation(smallMovement);
   entityActor.SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::System::GetInstance().Step();

   CPPUNIT_ASSERT_MESSAGE("The rotation was small. It should NOT have sent an update.",
      !tc->FindProcessMessageOfType(dtGame::MessageType::INFO_ACTOR_UPDATED).valid());

   tc->reset();

   xform.SetRotation(largeMovement);
   entityActor.SetTransform(xform, dtCore::Transformable::REL_CS);

   dtCore::System::GetInstance().Step();

   RefPtr<const dtGame::ActorUpdateMessage> update2 = 
      static_cast<const dtGame::ActorUpdateMessage*>(tc->FindProcessMessageOfType(
                                               dtGame::MessageType::INFO_ACTOR_UPDATED).get());
   
   CPPUNIT_ASSERT_MESSAGE("The rotation was large. It should have sent an update.",
                          update2.valid());
                          
   update2->GetUpdateParameters(toFill);
   CPPUNIT_ASSERT_EQUAL_MESSAGE("The update message should only have 2 update parameters in it.",
                                size_t(2), toFill.size()); 

   tc->reset();
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
   actor.GetGameActor().GetTransform(xform);
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
                  
   SimCore::Actors::BaseEntity& entity = static_cast<SimCore::Actors::BaseEntity&>(actor.GetGameActor());

   osg::Vec3 setVec = osg::Vec3(1.0, 1.2, 1.3);
   
   entity.SetLastKnownTranslation(setVec);
   entity.SetLastKnownRotation(setVec);
   entity.SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::NONE);
   entity.SetFlying(true);
   
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

   entity.SetDeadReckoningAlgorithm(dtGame::DeadReckoningAlgorithm::STATIC);
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
   RefPtr<TestComponent> tc = new TestComponent;
   mGM->AddComponent(*tc, dtGame::GameManager::ComponentPriority::HIGHEST);
   RefPtr<SimCore::Actors::PlayerActorProxy> pa;
   mGM->CreateActor(*SimCore::Actors::EntityActorRegistry::PLAYER_ACTOR_TYPE, pa);
   CPPUNIT_ASSERT(pa != NULL);

   mGM->AddActor(*pa, false, false);
   SLEEP(10);
   dtCore::System::GetInstance().Step();
   RefPtr<const dtGame::Message> msg = 
      tc->FindProcessMessageOfType(dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD);

   CPPUNIT_ASSERT_MESSAGE("A player entered the world, a player enter world message should have been found", msg.valid());
   CPPUNIT_ASSERT_MESSAGE("The message found should be of the correct type", msg->GetMessageType() == dtGame::MessageType::INFO_PLAYER_ENTERED_WORLD);

   tc->reset();
   mGM->DeleteAllActors();
   dtCore::System::GetInstance().Step();

   mGM->AddActor(*pa, true, false);
   SLEEP(10);
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

   da->SetLingeringSmokeSecs(0.000001);
   da->SetDeleteActorTimerSecs(0.0001);
   da->SetExplosionTimerSecs(0.0001);
   da->LoadSoundFile("Sounds/silence.wav");

   // TODO Add detonation to GM, set a timer and everything, ensure it fires and deletes
   // itself via its invokable
   mGM->AddActor(*dap, false, false);
   SLEEP(1);
   dtCore::System::GetInstance().Step();

   SLEEP(1);
   dtCore::System::GetInstance().Step();

   SLEEP(1);
   dtCore::System::GetInstance().Step();

   std::vector<dtGame::GameActorProxy*> proxies;
   mGM->GetAllGameActors(proxies);

   CPPUNIT_ASSERT_MESSAGE("The timer has elapsed, the detonation actor should no longer be in the GM", proxies.empty());

   //Make sure these don't ACTUALLY get deleted before the end of test.
   //This is a work around for a bug is sigslot.h
   std::vector<RefPtr<SimCore::Actors::DetonationActorProxy> > detList;
 
   const unsigned short int numDets = 20;
   for(unsigned int i = 0; i < numDets; i++)
   {
      RefPtr<SimCore::Actors::DetonationActorProxy> d; 
      mGM->CreateActor("Effects", "Detonation Actor", d);
      SimCore::Actors::DetonationActor& detActor = static_cast<SimCore::Actors::DetonationActor&>(d->GetGameActor());
      detActor.SetLingeringSmokeSecs(0.001);
      detActor.SetExplosionTimerSecs(0.001);
      detActor.SetDeleteActorTimerSecs(0.001);
      detActor.LoadSoundFile("Sounds/silence.wav");
      mGM->AddActor(*d, false, false);
      detList.push_back(d);
   }
   
   SLEEP(1);
   dtCore::System::GetInstance().Step();

   SLEEP(1);
   dtCore::System::GetInstance().Step();

   SLEEP(1);
   dtCore::System::GetInstance().Step();

   mGM->GetAllGameActors(proxies);
   
   CPPUNIT_ASSERT_MESSAGE("There should NOT be any detonation actors in the GM after their timers expired",  proxies.empty());
   
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
