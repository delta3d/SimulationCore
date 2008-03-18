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
* @author Allen Danklefsen, Curtis Murphy
*/
#ifndef _NX_MUNITIONS_PARTICLE_SYSTEM_
#define _NX_MUNITIONS_PARTICLE_SYSTEM_

#ifdef AGEIA_PHYSICS

#include <SimCore/Export.h>
#include <SimCore/Actors/NxAgeiaParticleSystemActor.h>

#include <SimCore/Components/RenderingSupportComponent.h>//for dynamic lights, cant be forward declared

namespace dtCore
{
   class Isector;
}

namespace SimCore
{
   namespace Actors
   {
      class WeaponActor;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class SIMCORE_EXPORT MunitionsPhysicsParticle : public PhysicsParticle
{
public:
   /////////////////////////////////////////////////////////////////////////////////////////////////////////
   MunitionsPhysicsParticle(SimCore::Components::RenderingSupportComponent* renderComp, const std::string& name, float ParticleLengthOfTimeOut = 10.0f, float InverseDeletionAlphaTime = 3.0f, float alphaInTime = 0.0f);
   bool IsATracer() const {return mIsTracer;}

   void SetLastPosition(const osg::Vec3& value);
   const osg::Vec3& GetLastPosition() {return mLastPosition;}

protected:
   virtual ~MunitionsPhysicsParticle();

private:
   bool mIsTracer;
   osg::Vec3 mLastPosition;

   dtCore::RefPtr<dtCore::Transformable> mDynamicLight;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////
class SIMCORE_EXPORT NxAgeiaMunitionsPSysActor: public NxAgeiaParticleSystemActor
{
   public:
      /// constructor for NxAgeiaBaseActor
      NxAgeiaMunitionsPSysActor(dtGame::GameActorProxy &proxy);
     
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

   protected:
      /// destructor
      virtual ~NxAgeiaMunitionsPSysActor(void);

      /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
      virtual void AgeiaPrePhysicsUpdate(){}

      /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
      virtual void AgeiaPostPhysicsUpdate();

      /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
      virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, NxActor& ourSelf, NxActor& whatWeHit);

      // You would have to make a new raycast to get this report,
      // so no flag associated with it.
      virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, const NxActor& whatWeHit);

   public:

      // Set the weapon that owns this shooter.
      // This allows the weapon to listen for contact reports.
      void SetWeapon( SimCore::Actors::WeaponActor& weapon ) { mWeapon = &weapon; }
      SimCore::Actors::WeaponActor* GetWeapon() { return mWeapon.get(); }
      const SimCore::Actors::WeaponActor* GetWeapon() const { return mWeapon.get(); }
      // to be called from like a wep apointment
      virtual void Fire();

      //////////////////////////////////////////////////////////////////
      /// Sets and Gets for Properties
      void SetSystemToUseTracers(bool value) {mUseTracers         = value;}
      void SetFrequencyOfTracers(int value)  {mFrequencyOfTracers = value;}

      bool GetSystemToUseTracers()           {return mUseTracers;}
      int  GetFrequencyOfTracers()           {return mFrequencyOfTracers;}

      bool ResolveISectorCollision(MunitionsPhysicsParticle& particleToCheck);
      
   protected:

      //////////////////////////////////////////////////////////////////
      virtual void AddParticle();
      //////////////////////////////////////////////////////////////////
      virtual void RemoveParticle(PhysicsParticle& whichOne);

   private: 
      bool           mUseTracers;               /// Do we use tracers for this particle system?      
      int            mCurrentTracerRoundNumber; /// Current count for knowing when to create a tracer
      int            mFrequencyOfTracers;       /// Everytime it hits this number it will reset mCurrentTracerRoundNumber
                                                /// to 0 and set the curr particle to be tracer
      dtCore::RefPtr<dtCore::Isector>                 mISector;
      dtCore::ObserverPtr<SimCore::Actors::WeaponActor>  mWeapon;
};

////////////////////////////////////////////////////////
// PROXY
////////////////////////////////////////////////////////
class SIMCORE_EXPORT NxAgeiaMunitionsPSysActorProxy : public NxAgeiaParticleSystemActorProxy
{
   public:
      NxAgeiaMunitionsPSysActorProxy();
      virtual void BuildPropertyMap();

   protected:
      virtual ~NxAgeiaMunitionsPSysActorProxy();
      void CreateActor();
      virtual void OnEnteredWorld();
};

#endif
#endif
