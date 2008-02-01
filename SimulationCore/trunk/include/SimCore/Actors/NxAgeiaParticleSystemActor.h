/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen, Curtiss Murphy
*/

#ifdef AGEIA_PHYSICS
#ifndef _NX_PARTICLE_SYSTEM_
#define _NX_PARTICLE_SYSTEM_

#include <SimCore/Export.h>

#include <dtGame/gameactor.h>

#include <NxAgeiaPrimitivePhysicsHelper.h>
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
class PhysicsParticle;


///////////////////////////////////////////////////////////////////////////////////
class SIMCORE_EXPORT NxAgeiaParticleSystemActor: public dtGame::GameActor, public dtAgeiaPhysX::NxAgeiaPhysicsInterface
{
   public:
      /// constructor for NxAgeiaBaseActor
      NxAgeiaParticleSystemActor(dtGame::GameActorProxy &proxy);
     
      /**
      * This method is an invokable called when an object is local and
      * receives a tick.
      * @param tickMessage A message containing tick related information.
      */
      virtual void TickLocal(const dtGame::Message &tickMessage);

      /**
      * This method is an invokable called when an object is remote and
      * receives a tick.
      * @param tickMessage A message containing tick related information.
      */
      virtual void TickRemote(const dtGame::Message &tickMessage);

      // Called when the actor has been added to the game manager.
      // You can respond to OnEnteredWorld on either the proxy or actor or both.
      virtual void OnEnteredWorld();

      class SIMCORE_EXPORT TwoDOrThreeDTypeEnum : public dtUtil::Enumeration 
      {
         DECLARE_ENUM(TwoDOrThreeDTypeEnum);
         public:
            static TwoDOrThreeDTypeEnum TWOD;
            static TwoDOrThreeDTypeEnum THREED;

         private:
            TwoDOrThreeDTypeEnum(const std::string &name) : dtUtil::Enumeration(name) 
            {
               AddInstance(this);
            }
      };

   protected:
      // Loads a particle so that the cache works correctly
      void LoadParticleResource(PhysicsParticle &particle, const std::string &resourceFile);

      /// destructor
      virtual ~NxAgeiaParticleSystemActor(void);

      /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
      virtual void AgeiaPrePhysicsUpdate(){}

      /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
      virtual void AgeiaPostPhysicsUpdate();

      /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
      virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit);

      // You would have to make a new raycast to get this report,
      // so no flag associated with it.
      virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, const NxActor& whatWeHit){}

     
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
      void SetTwoDOrThreeDTypeEnum(TwoDOrThreeDTypeEnum &value)   {mParticleEnumForObjectType = &value;}
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
      //////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mPhysicsHelper;

   protected:

      //////////////////////////////////////////////////////////////////
      virtual void AddParticle();
      //////////////////////////////////////////////////////////////////
      virtual void RemoveParticle(PhysicsParticle& whichOne);
      //////////////////////////////////////////////////////////////////
      /// /brief only accurate to 100ths place
      inline float GetRandBetweenTwoFloats(float max, float min)
      {
         if(min > max)
            min = max;

         if(max == min)
            return max;

         int Max = (int)(max * 100);
         int Min = (int)(min * 100);

         int result = (rand() % (Max - Min + 1) + Min);
         float Result = (float)result / 100;
         return Result; 
      }

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
      bool                    mGravityEnabled;
      bool                    mApplyForces;
      bool                    mSelfInteracting;
      bool                    mHitOutParticleLimitDontSpawnAnymore;
      bool                    mIsCurrentlyOn;
      bool                    mObjectsStayStaticWhenHit;
      osg::Vec3               mStartingPositionRandMin;
      osg::Vec3               mParentsWorldRelativeVelocityVector;
      osg::Vec3               mStartingPositionRandMax;
      osg::Vec3               mStartingLinearVelocityScaleMin;
      osg::Vec3               mStartingLinearVelocityScaleMax;
      osg::Vec3               mStartingLinearVelocityScaleInnerConeCap;
      osg::Vec3               mStartingAngularVelocityScaleMin;
      osg::Vec3               mStartingAngularVelocityScaleMax;
      osg::Vec3               mForceVectorMin;
      osg::Vec3               mForceVectorMax;
      std::list<dtCore::RefPtr<PhysicsParticle> > mOurParticleList;
};

////////////////////////////////////////////////////////
// PROXY
////////////////////////////////////////////////////////
class SIMCORE_EXPORT NxAgeiaParticleSystemActorProxy : public dtGame::GameActorProxy
{
   public:
      NxAgeiaParticleSystemActorProxy();
      virtual void BuildPropertyMap();

   protected:
      virtual ~NxAgeiaParticleSystemActorProxy();
      void CreateActor();
      virtual void OnEnteredWorld();
      virtual void OnRemovedFromWorld();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class SIMCORE_EXPORT PhysicsParticle : public osg::Referenced
{
public:
   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   PhysicsParticle(const std::string& name, float ParticleLengthOfTimeOut = 10.0f, float InverseDeletionAlphaTime = 3.0f, float alphaInTime = 0.0f) 
   {
      mSpawnTimer = 0; 
      mParticleLengthOfTimeOut = ParticleLengthOfTimeOut;
      mInverseDeletionAlphaTime = InverseDeletionAlphaTime;
      mAlphaInStartTime = alphaInTime;
      mBeenHit = false;
      mName = name;
      mNeedsToBeDeleted = false;
      mNxActor = NULL;
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   void UpdateTime(float elapsedTime) {mSpawnTimer += elapsedTime;}
   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   bool ShouldBeRemoved() 
   {
      if(mSpawnTimer > mParticleLengthOfTimeOut || mNeedsToBeDeleted == true) 
         return true; 
      return false;
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   void HitReceived() 
   {
      if(mBeenHit == false)
      {
         mSpawnTimer = mParticleLengthOfTimeOut - mInverseDeletionAlphaTime;
      }
      mBeenHit = true;
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   void UpdateAlphaAmount();    
   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   void FlagToDelete(){mNeedsToBeDeleted = true;}
   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   const std::string& GetName(){return mName;}
   // Get the physics actor - just a pointer, so there is some danger here
   NxActor* GetPhysicsActor() { return mNxActor; }
   // Set the physics actor - just a pointer, so there is some danger here
   void SetPhysicsActor(NxActor *nxActor) { mNxActor = nxActor; }

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
   // Note - we hold the actual pointer to the nxactor, because the helper holds the real reference
   NxActor *mNxActor; // The Nx actor created by the physics helper. We hold it so we can find it later
};


#endif
#endif
