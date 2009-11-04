/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
*
* See the .h file for complete licensing information.
*
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
*
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>

//#ifdef AGEIA_PHYSICS
#include <Actors/BaseEnemyActor.h>
#include <dtPhysics/physicshelper.h>
#include <dtPhysics/physicsobject.h>
//#include <NxAgeiaWorldComponent.h>
//#include <NxAgeiaRaycastReport.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtABC/application.h>
#include <dtAudio/audiomanager.h>
#include <dtAudio/sound.h>
#include <dtUtil/matrixutil.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/keyboard.h>
#include <dtGame/basemessages.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Actors/BaseEntity.h>
#include <SimCore/Components/RenderingSupportComponent.h>
//#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/CollisionGroupEnum.h>

//#include <dtUtil/nodeprintout.h>

#include <AIState.h>
#include <AIEvent.h>
#include <dtGame/gameactor.h>
#include <ActorRegistry.h>
#include <dtGame/gamemanager.h>


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   BaseEnemyActor::BaseEnemyActor(SimCore::Actors::BasePhysicsVehicleActorProxy &proxy)
      : SimCore::Actors::BasePhysicsVehicleActor(proxy)
      , mTimeSinceBorn(0.0f)
      , mTimeToExistAfterDead(20.0f)
      , mTimeSinceKilled(0.0f)
   {
      /////////////////////////////////////////////////////////////////
      // Set a bunch of initial conditions - Note, some of these may be
      // changed if the actor is loaded from a map or when received as a remote actor
      /////////////////////////////////////////////////////////////////

      SetMaxUpdateSendRate(2.0f);

      SetPublishLinearVelocity(true);
      SetPublishAngularVelocity(true);

      // create my unique physics helper.  almost all of the physics is on the helper.
      // The actor just manages properties and key presses mostly.
      dtPhysics::PhysicsHelper *helper = new dtPhysics::PhysicsHelper(proxy);
      //helper->SetBaseInterfaceClass(this);
      SetPhysicsHelper(helper);

      dtCore::RefPtr<dtPhysics::PhysicsObject> physicsObject = new dtPhysics::PhysicsObject("VehicleBody");
      helper->AddPhysicsObject(*physicsObject);
      helper->SetMass(500.0f);
      physicsObject->SetPrimitiveType(dtPhysics::PrimitiveType::CONVEX_HULL);
      physicsObject->SetMass(100.0f);
      //physicsObject->SetExtents(osg::Vec3(1.5f, 1.5f, 1.5f));
      physicsObject->SetMechanicsType(dtPhysics::MechanicsType::STATIC);

      // Preset some Entity values
      SetForceAffiliation(SimCore::Actors::BaseEntityActorProxy::ForceEnum::OPPOSING); // Shows up 'red' in Stealth Viewer
      SetEntityType("Enemy");
      SetMunitionDamageTableName("StandardDamageType");

      // Make a semi-unique name - mostly only useful for Stealth Viewer
      static int uniqueCounter = 0;
      uniqueCounter ++;
      SetName("Enemy " + dtUtil::ToString(uniqueCounter));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   BaseEnemyActor::~BaseEnemyActor(void)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   float BaseEnemyActor::ValidateIncomingDamage(float incomingDamage, const SimCore::DetonationMessage& message, const SimCore::Actors::MunitionTypeActor& munition)
   {
      dtGame::GameActorProxy* gap = GetGameActorProxy().GetGameManager()->FindGameActorById(message.GetSendingActorId());

      //if is enemy true it will multiply incoming damage by a 0
      return incomingDamage * float(!IsEnemyActor(gap));
   }


   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::OnEnteredWorld()
   {
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);

      // TODO - Maybe use sphere instead???

      dtPhysics::PhysicsObject *physObj = GetPhysicsHelper()->GetMainPhysicsObject();
      physObj->CreateFromProperties(GetNonDamagedFileNode());
      physObj->SetTransform(ourTransform);
      physObj->SetActive(true);

      BaseClass::OnEnteredWorld();

   }


   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::DoExplosion(float)
   {
      //const osg::Vec3& finalVelocity, const osg::Vec3& location, const dtCore::Transformable* target )
      //printf("Sending DETONATION\r\n");

      dtGame::GameManager* gm = GetGameActorProxy().GetGameManager();
      dtCore::Transform ourTransform;
      GetTransform(ourTransform);
      osg::Vec3 trans = ourTransform.GetTranslation();

      // Prepare a detonation message
      dtCore::RefPtr<SimCore::DetonationMessage> msg;
      gm->GetMessageFactory().CreateMessage( SimCore::MessageType::DETONATION, msg );

      // Required Parameters:
      msg->SetEventIdentifier( 1 );
      msg->SetDetonationLocation(trans);
      // --- DetonationResultCode 1 == Entity Impact, 3 == Ground Impact, 5 == Detonation
      msg->SetDetonationResultCode( 5 ); // TO BE DYNAMIC
      msg->SetMunitionType("Generic Explosive");
      msg->SetFuseType(0);
      msg->SetWarheadType(0);
      msg->SetQuantityFired(1);
      msg->SetSendingActorId(GetGameActorProxy().GetId());
      //msg->SetFinalVelocityVector( finalVelocity );
      msg->SetRateOfFire(1);

      gm->SendMessage( *msg );
      gm->SendNetworkMessage( *msg );

      GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::RespondToHit(const SimCore::DetonationMessage& message,
      const SimCore::Actors::MunitionTypeActor& munition, const osg::Vec3& force, 
      const osg::Vec3& location)
   {
      dtGame::GameActorProxy* gap = GetGameActorProxy().GetGameManager()->FindGameActorById(message.GetSendingActorId());

      // the base class applies an impulse
      if(!IsEnemyActor(gap))
      {
         BaseClass::RespondToHit(message, munition, force, location);

         if(IsMobilityDisabled())
         {
            mAIHelper->GetStateMachine().MakeCurrent(&AIStateType::AI_STATE_DIE);
         }

         if(GetDamageState() == SimCore::Actors::BaseEntityActorProxy::DamageStateEnum::DESTROYED)
         {       
            //this lets the AI respond to being hit
            mAIHelper->GetStateMachine().HandleEvent(&AIEvent::AI_EVENT_TOOK_DAMAGE);
         }
      }
   } 


   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::UpdateVehicleTorquesAndAngles(float deltaTime)
   {
      mTimeSinceBorn += deltaTime;

      if( ! IsMobilityDisabled())
      {
         // Do physics/pathing/AI of vehicle
      }
      else
      {
         // Self-terminate after we're been dead for a bit - allows time for animations, fire, falling from physics, etc...
         mTimeSinceKilled += deltaTime;
         if (mTimeSinceKilled > mTimeToExistAfterDead)
         {
            GetGameActorProxy().GetGameManager()->DeleteActor(GetGameActorProxy());
         }
      }

   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::UpdateSoundEffects(float deltaTime)
   {
      BaseClass::UpdateSoundEffects(deltaTime);
   }

  ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::UpdateRotationDOFS(float deltaTime, bool insideVehicle)
   {
   }

   //////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::OnTickLocal( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickLocal( tickMessage );
   }

   //////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::OnTickRemote( const dtGame::TickMessage& tickMessage )
   {
      BaseClass::OnTickRemote( tickMessage );
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyAIHelper* BaseEnemyActor::GetAIHelper()
   {
      return mAIHelper.get();
   }

   ///////////////////////////////////////////////////////////////////////////////////
   const EnemyAIHelper* BaseEnemyActor::GetAIHelper() const
   {
      return mAIHelper.get();
   }

   //////////////////////////////////////////////////////////////////////
   void BaseEnemyActor::InitAI( const EnemyDescriptionActor* desc )
   {
      mAIHelper->Init(desc);
   }

   //////////////////////////////////////////////////////////////////////
   bool BaseEnemyActor::IsEnemyActor( dtGame::GameActorProxy* gap ) const
   {
      if(gap != NULL)
      {
         const dtDAL::ActorType& atype = gap->GetActorType();
         if( atype == *NetDemoActorRegistry::ENEMY_HELIX_ACTOR_TYPE || 
            atype == * NetDemoActorRegistry::ENEMY_MINE_ACTOR_TYPE ||
            atype == * NetDemoActorRegistry::ENEMY_MOTHERSHIP_ACTOR_TYPE )
         {
            return true;
         }
      }
      return false;
   }
   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   BaseEnemyActorProxy::BaseEnemyActorProxy()
   {
      SetClassName("BaseEnemyActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActorProxy::BuildPropertyMap()
   {
      const std::string GROUP = "Enemy Props";
      BaseClass::BuildPropertyMap();

      //BaseEnemyActor& actor = *static_cast<BaseEnemyActor*>(GetActor());

      // Add properties
   }

   ///////////////////////////////////////////////////////////////////////////////////
   BaseEnemyActorProxy::~BaseEnemyActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActorProxy::CreateActor()
   {
      SetActor(*new BaseEnemyActor(*this));
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void BaseEnemyActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();
   }

} // namespace
//#endif
