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

#ifndef _MUNITION_TYPE_ACTOR_H_
#define _MUNITION_TYPE_ACTOR_H_

#include <SimCore/Export.h>
#include <dtCore/base.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/observerptr.h>
#include <dtDAL/actorproxy.h>
#include <dtUtil/getsetmacros.h>
#include <string>

namespace SimCore
{
   namespace Actors
   {
      class DetonationActorProxy;

      //////////////////////////////////////////////////////////
      // Enumeration Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionFamily : public dtUtil::Enumeration
      {
         DECLARE_ENUM(MunitionFamily);
         public:
            static MunitionFamily FAMILY_UNKNOWN;
            static MunitionFamily FAMILY_ROUND;
            static MunitionFamily FAMILY_EXPLOSIVE_ROUND;
            static MunitionFamily FAMILY_GRENADE;
            static MunitionFamily FAMILY_MINE;
            static MunitionFamily FAMILY_MISSILE;
            static MunitionFamily FAMILY_GENERIC_EXPLOSIVE;

            bool IsExplosive() const;

         private:
            MunitionFamily(const std::string &name) : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
      };



      //////////////////////////////////////////////////////////
      // DIS Identifier Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DISIdentifier
      {
         public:
            // Constructor
            // NOTE: constructor is explicit to avoid changing a DISIdentifier 
            //       by a single number assignment.
            explicit DISIdentifier( unsigned char kind = 0, unsigned char domain = 0, 
               unsigned short country = 0, unsigned char category = 0, unsigned char subcategory = 0, 
               unsigned char specific = 0, unsigned char extra = 0 );

            virtual ~DISIdentifier() {}

            void Set( unsigned char kind, unsigned char domain, 
               unsigned short country, unsigned char category, unsigned char subcategory, 
               unsigned char specific, unsigned char extra );

            void SetByString( const std::string& disValues );
            std::string ToString() const;

            void SetKind( unsigned char kind ) { mKind = kind; }
            void SetDomain( unsigned char domain ) { mDomain = domain; }
            void SetCountry( unsigned short country ) { mCountry = country; }
            void SetCategory( unsigned char category ) { mCategory = category; }
            void SetSubcategory( unsigned char subCategory ) { mSubcategory = subCategory; }
            void SetSpecific( unsigned char specific ) { mSpecific = specific; }
            void SetExtra( unsigned char extra ) { mExtra = extra; }

            unsigned int GetKind() const { return mKind; }
            unsigned int GetDomain() const { return mDomain; }
            unsigned int GetCountry() const { return mCountry; }
            unsigned int GetCategory() const { return mCategory; }
            unsigned int GetSubcategory() const { return mSubcategory; }
            unsigned int GetSpecific() const { return mSpecific; }
            unsigned int GetExtra() const { return mExtra; }

            unsigned int GetNumber( unsigned int numberIndex ) const;

            // This function rates matches from left to right. The match count
            // stops at the first inequality.
            // NOTE: This ivery different from the inherited function RankMatch,
            // which increases the match count on equalities even after finding
            // an inequality.
            unsigned int GetDegreeOfMatch( const DISIdentifier& dis ) const;

            const DISIdentifier& operator= ( const DISIdentifier& dis );

            bool operator== ( const DISIdentifier& dis ) const;
            bool operator!= ( const DISIdentifier& dis ) const;
            bool operator<= ( const DISIdentifier& dis ) const;
            bool operator>= ( const DISIdentifier& dis ) const;
            bool operator< ( const DISIdentifier& dis ) const;
            bool operator> ( const DISIdentifier& dis ) const;
         private:
            unsigned char  mKind;
            unsigned char  mDomain;
            unsigned short mCountry; // 16 as specified in the OMD file
            unsigned char  mCategory;
            unsigned char  mSubcategory;
            unsigned char  mSpecific;
            unsigned char  mExtra;
      };



      //////////////////////////////////////////////////////////
      // Munition Effect Actor Proxy Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionEffectsInfoActorProxy : public dtDAL::ActorProxy
      {
         public:
            static const std::string CLASS_NAME;

            MunitionEffectsInfoActorProxy();

            virtual void CreateActor();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

         protected:
            virtual ~MunitionEffectsInfoActorProxy();

         private:
      };



      //////////////////////////////////////////////////////////
      // Munition Effect Actor Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionEffectsInfoActor : public dtCore::DeltaDrawable
      {
         public:
            MunitionEffectsInfoActor( MunitionEffectsInfoActorProxy& proxy );

            virtual osg::Node* GetOSGNode();
            virtual const osg::Node* GetOSGNode() const;

            MunitionEffectsInfoActorProxy& GetProxy();
            const MunitionEffectsInfoActorProxy& GetProxy() const;

            // MUNITION FLIGHT PROPERTIES
            void SetFlyEffect( const std::string& fileName );
            const std::string& GetFlyEffect() const;

            void SetFlySound( const std::string& fileName );
            const std::string& GetFlySound() const;

            void SetFlySoundMaxDistance( float distance );
            float GetFlySoundMaxDistance() const;

            // SHOT FIRED PROPERTIES
            void SetFireEffect( const std::string& fileName );
            const std::string& GetFireEffect() const;

            void SetFireSound( const std::string& fileName );
            const std::string& GetFireSound() const;

            void SetFireSoundMaxDistance( float distance );
            float GetFireSoundMaxDistance() const;

            void SetFireFlashProbability( float probability );
            float GetFireFlashProbability() const;

            void SetFireFlashTime( float flashTime );
            float GetFireFlashTime() const;

            // GROUND IMPACT PROPERTIES
            void SetGroundImpactEffect( const std::string& fileName );
            const std::string& GetGroundImpactEffect() const;

            void SetGroundImpactSound( const std::string& fileName );
            const std::string& GetGroundImpactSound() const;

            void SetGroundImpactSoundMaxDistance( float distance );
            float GetGroundImpactSoundMaxDistance() const;

            // ENTITY IMPACT PROPERTIES
            void SetEntityImpactEffect( const std::string& fileName );
            const std::string& GetEntityImpactEffect() const;
            bool HasEntityImpactEffect() const;

            void SetEntityImpactSound( const std::string& fileName );
            const std::string& GetEntityImpactSound() const;
            bool HasEntityImpactSound() const;

            void SetEntityImpactSoundMaxDistance( float distance );
            float GetEntityImpactSoundMaxDistance() const;

            // SMOKE PROPERTIES
            void SetSmokeEffect( const std::string& fileName );
            const std::string& GetSmokeEffect() const;

            void SetSmokeLifeTime( float lifeTime );
            float GetSmokeLifeTime() const;

            // TRACER PROPERTIES
            void SetTracerShaderName( const std::string& shaderName );
            const std::string& GetTracerShaderName() const;

            void SetTracerShaderGroup( const std::string& shaderGroup );
            const std::string& GetTracerShaderGroup() const;

            void SetTracerLifeTime( float lifeTime );
            float GetTracerLifeTime() const;

            void SetTracerLength( float tracerLength );
            float GetTracerLength() const;

            void SetTracerThickness( float tracerThickness );
            float GetTracerThickness() const;

            // LIGHT PROPERTIES
            void SetGroundImpactLight( const std::string& lightName );
            const std::string& GetGroundImpactLight() const;

            void SetEntityImpactLight( const std::string& lightName );
            const std::string& GetEntityImpactLight() const;

            void SetFireLight( const std::string& lightName );
            const std::string& GetFireLight() const;

            void SetTracerLight( const std::string& lightName );
            const std::string& GetTracerLight() const;

         protected:
            virtual ~MunitionEffectsInfoActor();

         private:
            float mProbabilityFireFlash;
            float mSoundFire_Min;
            float mSoundFire_Max;
            float mSoundImpactGround_Min;
            float mSoundImpactGround_Max;
            float mSoundImpactEntity_Min;
            float mSoundImpactEntity_Max;
            float mSoundFly_Min;
            float mSoundFly_Max;
            float mTimeSmoke;
            float mFireFlashTime;
            float mTracerLifeTime;
            float mTracerLength;
            float mTracerThickness;
            std::string mSoundFly;
            std::string mSoundFire;
            std::string mSoundImpactGround;
            std::string mSoundImpactEntity;
            std::string mEffectFire;
            std::string mEffectImpactEntity;
            std::string mEffectImpactGround;
            std::string mEffectFly;
            std::string mEffectSmoke;
            std::string mTracerShaderName;
            std::string mTracerShaderGroup;
            std::string mLightImpactGround;
            std::string mLightImpactEntity;
            std::string mLightFire;
            std::string mLightTracer;
            dtCore::ObserverPtr<MunitionEffectsInfoActorProxy> mProxy;
      };



      //////////////////////////////////////////////////////////
      // Munition Type Actor Proxy Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionTypeActorProxy : public dtDAL::ActorProxy
      {
         public:
            static const std::string CLASS_NAME;

            MunitionTypeActorProxy();

            virtual void CreateActor();

            virtual void BuildPropertyMap();

            virtual bool IsPlaceable() const { return false; }

            void SetEffectsInfoActor( dtDAL::ActorProxy* proxy );
            dtCore::DeltaDrawable* GetEffectsInfoDrawable();

         protected:
            virtual ~MunitionTypeActorProxy();

         private:
      };



      //////////////////////////////////////////////////////////
      // Munition Type Actor Code
      //////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionTypeActor : public dtCore::DeltaDrawable
      {
         public:
            MunitionTypeActor( MunitionTypeActorProxy& proxy );

            virtual osg::Node* GetOSGNode() { return NULL; }
            virtual const osg::Node* GetOSGNode() const { return NULL; }

            MunitionTypeActorProxy& GetProxy() { return *mProxy; }
            const MunitionTypeActorProxy& GetProxy() const { return *mProxy; }

            void SetFamily( MunitionFamily& family ) { mFamily = &family; }
            MunitionFamily& GetFamily() const { return *mFamily; }

            void SetFuseType( int fuse ) { mFuse = fuse; }
            int GetFuseType() const { return mFuse; }

            void SetWarheadType( int warhead ) { mWarhead = warhead; }
            int GetWarheadType() const { return mWarhead; }

            void SetTracerFrequency( int frequency ) { mTracerFrequency = frequency; }
            int GetTracerFrequency() const { return mTracerFrequency; }

            // OTHER PROPERTIES
            void SetModel( const std::string& fileName ) { mModel = fileName; }
            const std::string& GetModel() const { return mModel; }

            void SetDamageType( const std::string& name ) { mDamageType = name; }
            const std::string& GetDamageType() const { return mDamageType; }
            std::string GetDamageTypeString() const { return mDamageType; }

            void SetDISIdentifier( const DISIdentifier& dis );
            void SetDISIdentifierByString( const std::string& dis );
            const DISIdentifier& GetDISIdentifier() const { return mDIS; }
            std::string GetDISIdentifierString() const;

            void SetEffectsInfoActor(dtDAL::ActorProxy* proxy);
            dtCore::DeltaDrawable* GetEffectsInfoDrawable();
            const MunitionEffectsInfoActor* GetEffectsInfoActor() const;

            void SetDetonationActorPrototype(dtDAL::ActorProxy* proxy);
            DetonationActorProxy* GetDetonationActorPrototype();
            dtCore::DeltaDrawable* GetDetonationActorPrototypeDrawable();

            DT_DECLARE_ACCESSOR_INLINE(dtCore::UniqueId, DetonationActor);

         protected:
            virtual ~MunitionTypeActor();

         private:
            int mFuse;
            int mWarhead;
            int mTracerFrequency;
            std::string mModel;
            std::string mDamageType;
            MunitionFamily* mFamily;
            DISIdentifier mDIS;
            dtCore::RefPtr<DetonationActorProxy> mDetonationPrototype;
            dtCore::RefPtr<MunitionEffectsInfoActor> mEffects;
            dtCore::ObserverPtr<MunitionTypeActorProxy> mProxy;
      };

   }
}

#endif
