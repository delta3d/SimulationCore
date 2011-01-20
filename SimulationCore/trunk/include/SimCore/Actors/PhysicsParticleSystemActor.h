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
 * @author Allen Danklefsen, Curtiss Murphy
 */
#ifndef _PHYSICS_PARTICLE_SYSTEM_
#define _PHYSICS_PARTICLE_SYSTEM_
#include <SimCore/Export.h>

#include <dtGame/gameactor.h>

#include <SimCore/PhysicsTypes.h>
#ifdef AGEIA_PHYSICS
#include <NxAgeiaPrimitivePhysicsHelper.h>
#else
#include <dtPhysics/physicsactcomp.h>
#include <dtPhysics/physicsobject.h>
#endif
//#include <dtCore/object.h>

#include <osg/BlendFunc>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Geode>

// Forward declares
namespace dtCore
{
   class Transformable;
}

namespace SimCore
{

   namespace Actors
   {
      class PhysicsParticle;

      ///////////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT PhysicsParticleSystemActor: public dtGame::GameActor
#ifdef AGEIA_PHYSICS
      , public dtAgeiaPhysX::NxAgeiaPhysicsInterface
#endif
      {

      public:
         /// constructor for NxAgeiaBaseActor
         PhysicsParticleSystemActor(dtGame::GameActorProxy& proxy);

         /**
          * This method is an invokable called when an object is local and
          * receives a tick.
          * @param tickMessage A message containing tick related information.
          */
         virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

         /**
          * This method is an invokable called when an object is remote and
          * receives a tick.
          * @param tickMessage A message containing tick related information.
          */
         virtual void OnTickRemote(const dtGame::TickMessage& tickMessage);

         // Called when the actor has been added to the game manager.
         // You can respond to OnEnteredWorld on either the proxy or actor or both.
         virtual void OnEnteredWorld();

         class SIMCORE_EXPORT TwoDOrThreeDTypeEnum : public dtUtil::Enumeration
         {
            DECLARE_ENUM(TwoDOrThreeDTypeEnum);
         public:
            static TwoDOrThreeDTypeEnum TWO_D;
            static TwoDOrThreeDTypeEnum THREE_D;

         private:
            TwoDOrThreeDTypeEnum(const std::string &name);
         };

         //////////////////////////////////////////////////////////////////
         /// /brief only accurate to 100ths place
         static float GetRandBetweenTwoFloats(float max, float min);

         protected:
         // Loads a particle so that the cache works correctly
         void LoadParticleResource(PhysicsParticle &particle, const std::string &resourceFile);

         /// destructor
         virtual ~PhysicsParticleSystemActor(void);

#ifdef AGEIA_PHYSICS
         /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
         virtual void AgeiaPrePhysicsUpdate(){}

         /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
         virtual void AgeiaPostPhysicsUpdate();

         /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
         virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, dtPhysics::PhysicsObject& ourSelf, dtPhysics::PhysicsObject& whatWeHit);

         // You would have to make a new raycast to get this report,
         // so no flag associated with it.
         virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const dtPhysics::PhysicsObject& ourSelf, const dtPhysics::PhysicsObject& whatWeHit){}
#else

         /// dtPhysics post physics callback.
         virtual void PostPhysicsUpdate();
#endif

         public:

         /// turn it on or off from spawning...
         void ToggleEmitter(bool value) {mIsCurrentlyOn = value; mSystemsTimeTotalTimeLength = 0;}

         /// return if the emitter is currently running
         bool GetIsEmitterRunning() {return mIsCurrentlyOn;}

         /// remove all particles, doesnt toggle on or off, so it should keep on spawning if already on.
         void ResetParticleSystem();

         /// return how many particles are in the world through this list....
         unsigned int GetHowManyParticlesThisSystemHasSpawnedCurrently() {return mOurParticleList.size();}

         /// returns the total number of particles that have ever been made by this emitter.
         unsigned int GetHowManyParticlesHaveBeenMadeSinceStart() {return mAmountOfParticlesThatHaveSpawnedTotal;}

         //////////////////////////////////////////////////////////////////
         /// Sets and Gets for Properties
         void SetTwoDOrThreeDTypeEnum(TwoDOrThreeDTypeEnum& value)   {mParticleEnumForObjectType = &value;}
         void SetParticleEmitterRateMin(float value)                 {mParticleEmitterRateMin=value;}
         void SetParticleEmitterRateMax(float value)                 {mParticleEmitterRateMax=value;}
         void SetParticleLengthofStay(float value)                   {mParticleLengthOfStay=value;}
         void SetStaticObjectsLifeTime(float value)                  {mStaticObjectsTimeLength=value;}
         void SetNumberOfParticlesWeWantSpawned(int value)           {mAmountofParticlesWeWantSpawned=value;}
         void SetThisAsAnInfiniteParticleSystem(bool value)          {mInfiniteParticleSystem=value;}
         void SetObjectToStatic(bool value)                          {mObjectsStayStatic=value;}
         void SetGravityEnabledOnParticleSystem(bool value)          {mGravityEnabled=value;}
         void SetToApplyForcesToParticlesEveryFrame(bool value)      {mApplyForces=value;}
         void SetCollideWithSelf(bool value)                         {mSelfInteracting=value;}
         void SetStartingPositionMin(const osg::Vec3& value)         {mStartingPositionRandMin=value;}
         void SetStartingPositionMax(const osg::Vec3& value)         {mStartingPositionRandMax=value;}
         void SetLinearVelocityStartMin(const osg::Vec3& value)      {mStartingLinearVelocityScaleMin=value;}
         void SetLinearVelocityStartMax(const osg::Vec3& value)      {mStartingLinearVelocityScaleMax=value;}
         void SetEmitterNoZoneEmitteerCone(const osg::Vec3& value)   {mStartingLinearVelocityScaleInnerConeCap=value;}
         void SetAngularVelocityStartMin(const osg::Vec3& value)     {mStartingAngularVelocityScaleMin=value;}
         void SetAngularVelocityStartMax(const osg::Vec3& value)     {mStartingAngularVelocityScaleMax=value;}
         void SetOverTimeForceVecMin(const osg::Vec3& value)         {mForceVectorMin=value;}
         void SetOverTimeForceVecMax(const osg::Vec3& value)         {mForceVectorMax=value;}
         void SetParticleFadeInTime(float value)                     {mParticleFadeInAmount=value;}
         void SetParticleFadeOutInverseDeletion(float value)         {mParticleFadeOutInverseDeletion=value;}
         void SetFileToLoadOne(const std::string& value)             {mPathOfFileToLoad[0] = value;}
         void SetFileToLoadTwo(const std::string& value)             {mPathOfFileToLoad[1] = value;}
         void SetFileToLoadThree(const std::string& value)           {mPathOfFileToLoad[2] = value;}
         void SetFileToLoadFour(const std::string& value)            {mPathOfFileToLoad[3] = value;}
         void SetFileToLoadFive(const std::string& value)            {mPathOfFileToLoad[4] = value;}
         void SetParentsWorldRelativeVelocityVector(const osg::Vec3& value) {mParentsWorldRelativeVelocityVector = value;}
         void SetObjectsStayStaticWhenHit(bool value)                {mObjectsStayStaticWhenHit = value;}

         TwoDOrThreeDTypeEnum& GetTwoDOrThreeDTypeEnum() const {return *mParticleEnumForObjectType;}
         float GetParticleEmitterRateMin()                     {return mParticleEmitterRateMin;}
         float GetParticleEmitterRateMax()                     {return mParticleEmitterRateMax;}
         float GetParticleLengthofStay()                       {return mParticleLengthOfStay;}
         float GetStaticObjectsLifeTime()                      {return mStaticObjectsTimeLength;}
         int GetNumberOfParticlesWeWantSpawned()               {return mAmountofParticlesWeWantSpawned;}
         bool GetThisAsAnInfiniteParticleSystem()              {return mInfiniteParticleSystem;}
         bool GetObjectToStatic()                              {return mObjectsStayStatic;}
         bool GetGravityEnabledOnParticleSystem()              {return mGravityEnabled;}
         bool GetToApplyForcesToParticlesEveryFrame()          {return mApplyForces;}
         bool GetCollideWithSelf()                             {return mSelfInteracting;}
         osg::Vec3 GetStartingPositionMin()                    {return mStartingPositionRandMin;}
         osg::Vec3 GetStartingPositionMax()                    {return mStartingPositionRandMax;}
         osg::Vec3 GetLinearVelocityStartMin()                 {return mStartingLinearVelocityScaleMin;}
         osg::Vec3 GetLinearVelocityStartMax()                 {return mStartingLinearVelocityScaleMax;}
         osg::Vec3 GetEmitterNoZoneEmitteerCone()              {return mStartingLinearVelocityScaleInnerConeCap;}
         osg::Vec3 GetAngularVelocityStartMin()                {return mStartingAngularVelocityScaleMin;}
         osg::Vec3 GetAngularVelocityStartMax()                {return mStartingAngularVelocityScaleMax;}
         osg::Vec3 GetOverTimeForceVecMin()                    {return mForceVectorMin;}
         osg::Vec3 GetOverTimeForceVecMax()                    {return mForceVectorMax;}
         float GetParticleFadeInTime()                         {return mParticleFadeInAmount;}
         float GetParticleFadeOutInverseDeletion()             {return mParticleFadeOutInverseDeletion;}
         osg::Vec3 GetParentsWorldRelativeVelocityVector()     {return mParentsWorldRelativeVelocityVector;}
         bool GetObjectsStayStaticWhenHit()                    {return mObjectsStayStaticWhenHit;}


#ifdef AGEIA_PHYSICS
         dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper& GetPhysicsActComp() { return *mPhysicsActComp; }
#else
         dtPhysics::PhysicsActComp& GetPhysicsActComp() { return *GetComponent<dtPhysics::PhysicsActComp>(); }
#endif
         protected:

         //////////////////////////////////////////////////////////////////
         virtual void AddParticle();
         //////////////////////////////////////////////////////////////////
         virtual void RemoveParticle(PhysicsParticle& whichOne);

         typedef std::list<dtCore::RefPtr<PhysicsParticle> > ParticleList;
         // Data members should NEVER be protected, but this one can't be changed yet.
         ParticleList mOurParticleList;
         //private:
         TwoDOrThreeDTypeEnum*   mParticleEnumForObjectType;      /// The Type of object actor this should be.
         std::string             mPathOfFileToLoad[5];
         float                   mSpawnerParticleTimer;
         float                   mParticleEmitterRateMin;
         float                   mParticleEmitterRateMax;
         float                   mParticleLengthOfStay;
         float                   mStaticObjectsTimeLength;
         float                   mParticleFadeInAmount;
         float                   mParticleFadeOutInverseDeletion;
         float                   mSystemsTimeTotalTimeLength;
         unsigned int            mAmountofParticlesWeWantSpawned;
         unsigned int            mAmountOfParticlesThatHaveSpawnedTotal;
         bool                    mInfiniteParticleSystem;
         bool                    mObjectsStayStatic;
         bool                    mObjectsStayStaticWhenHit;
         bool                    mGravityEnabled;
         bool                    mApplyForces;
         bool                    mSelfInteracting;
         bool                    mHitOutParticleLimitDontSpawnAnymore;
         bool                    mIsCurrentlyOn;
         osg::Vec3               mStartingPositionRandMin;
         osg::Vec3               mStartingPositionRandMax;
         osg::Vec3               mParentsWorldRelativeVelocityVector;
         osg::Vec3               mStartingLinearVelocityScaleMin;
         osg::Vec3               mStartingLinearVelocityScaleMax;
         osg::Vec3               mStartingLinearVelocityScaleInnerConeCap;
         osg::Vec3               mStartingAngularVelocityScaleMin;
         osg::Vec3               mStartingAngularVelocityScaleMax;
         osg::Vec3               mForceVectorMin;
         osg::Vec3               mForceVectorMax;

#ifdef AGEIA_PHYSICS
         dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mPhysicsActComp;
#else
         dtCore::RefPtr<dtPhysics::PhysicsActComp> mPhysicsActComp;
#endif
      };

      ////////////////////////////////////////////////////////
      // PROXY
      ////////////////////////////////////////////////////////
      class SIMCORE_EXPORT PhysicsParticleSystemActorProxy : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;

         PhysicsParticleSystemActorProxy();
         virtual void BuildPropertyMap();

         virtual dtCore::RefPtr<dtDAL::ActorProperty> GetDeprecatedProperty(const std::string& name);

         virtual void BuildActorComponents();

      protected:
         virtual ~PhysicsParticleSystemActorProxy();
         void CreateActor();
         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();
      };

      /////////////////////////////////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT PhysicsParticle : public osg::Referenced
      {
      public:
         PhysicsParticle(const std::string& name, float ParticleLengthOfTimeOut = 10.0f,
                  float InverseDeletionAlphaTime = 3.0f, float alphaInTime = 0.0f);

         void UpdateTime(float elapsedTime);
         bool ShouldBeRemoved();

         void HitReceived();

         void UpdateAlphaAmount();
         void FlagToDelete();

         bool IsFlaggedToDelete();

         const std::string& GetName() const;

         // Get the physics actor - just a pointer, so there is some danger here
         dtPhysics::PhysicsObject* GetPhysicsObject();
         // Set the physics actor - just a pointer, so there is some danger here
         void SetPhysicsObject(dtPhysics::PhysicsObject* physObj);

         dtCore::RefPtr<dtCore::Transformable> mObj;

      protected:
         virtual ~PhysicsParticle() {}

      private:
         float mSpawnTimer;
         float mParticleLengthOfTimeOut;        // how long till it goes away
         float mInverseDeletionAlphaTime;       // when it ends alphaing out before going away
         float mAlphaInStartTime;               // when it starts alphaing out before going away
         bool  mBeenHit;                        // if its been hit start the time out sequence
         bool  mNeedsToBeDeleted ;
         std::string mName;                     // name of the particle.. for removal.
         std::string mFileToLoad;
         dtCore::RefPtr<dtPhysics::PhysicsObject> mPhysicsObject;
      };

   }
}
#endif
