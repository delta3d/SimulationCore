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
#include <dtDAL/actorproxy.h>
#include <string>

namespace SimCore
{
   namespace Actors
   {
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

            virtual osg::Node* GetOSGNode() { return NULL; }
            virtual const osg::Node* GetOSGNode() const { return NULL; }

            MunitionEffectsInfoActorProxy& GetProxy() { return *mProxy; }
            const MunitionEffectsInfoActorProxy& GetProxy() const { return *mProxy; }

            // MUNITION FLIGHT PROPERTIES
            void SetFlyEffect( const std::string& fileName ) { mEffectFly = fileName; }
            std::string GetFlyEffect() const { return mEffectFly; }

            void SetFlySound( const std::string& fileName ) { mSoundFly = fileName; }
            std::string GetFlySound() const { return mSoundFly; }

            void SetFlySoundMinDistance( float distance ) { mSoundFly_Min = distance; }
            float GetFlySoundMinDistance() const { return mSoundFly_Min; }

            void SetFlySoundMaxDistance( float distance ) { mSoundFly_Max = distance; }
            float GetFlySoundMaxDistance() const { return mSoundFly_Max; }

            // SHOT FIRED PROPERTIES
            void SetFireEffect( const std::string& fileName ) { mEffectFire = fileName; }
            std::string GetFireEffect() const { return mEffectFire; }

            void SetFireSound( const std::string& fileName ) { mSoundFire = fileName; }
            std::string GetFireSound() const { return mSoundFire; }

            void SetFireSoundMinDistance( float distance ) { mSoundFire_Min = distance; }
            float GetFireSoundMinDistance() const { return mSoundFire_Min; }

            void SetFireSoundMaxDistance( float distance ) { mSoundFire_Max = distance; }
            float GetFireSoundMaxDistance() const { return mSoundFire_Max; }

            void SetFireFlashProbability( float probability )
            {
               mProbabilityFireFlash = probability < 0.0f ? 0.0f : probability > 1.0f ? 1.0f : probability;
            }
            float GetFireFlashProbability() const { return mProbabilityFireFlash; }

            void SetFireFlashTime( float flashTime ) { mFireFlashTime = flashTime; }
            float GetFireFlashTime() const { return mFireFlashTime; }

            // GROUND IMPACT PROPERTIES
            void SetGroundImpactEffect( const std::string& fileName ) { mEffectImpactGround = fileName; }
            std::string GetGroundImpactEffect() const { return mEffectImpactGround; }

            void SetGroundImpactSound( const std::string& fileName ) { mSoundImpactGround = fileName; }
            std::string GetGroundImpactSound() const { return mSoundImpactGround; }

            void SetGroundImpactSoundMinDistance( float distance ) { mSoundImpactGround_Min = distance; }
            float GetGroundImpactSoundMinDistance() const { return mSoundImpactGround_Min; }

            void SetGroundImpactSoundMaxDistance( float distance ) { mSoundImpactGround_Max = distance; }
            float GetGroundImpactSoundMaxDistance() const { return mSoundImpactGround_Max; }

            // ENTITY IMPACT PROPERTIES
            void SetEntityImpactEffect( const std::string& fileName ) { mEffectImpactEntity = fileName; }
            std::string GetEntityImpactEffect() const { return mEffectImpactEntity; }
            bool HasEntityImpactEffect() const { return ! mEffectImpactEntity.empty(); }

            void SetEntityImpactSound( const std::string& fileName ) { mSoundImpactEntity = fileName; }
            std::string GetEntityImpactSound() const { return mSoundImpactEntity; }
            bool HasEntityImpactSound() const { return ! mSoundImpactEntity.empty(); }

            void SetEntityImpactSoundMinDistance( float distance ) { mSoundImpactEntity_Min = distance; }
            float GetEntityImpactSoundMinDistance() const { return mSoundImpactEntity_Min; }

            void SetEntityImpactSoundMaxDistance( float distance ) { mSoundImpactEntity_Max = distance; }
            float GetEntityImpactSoundMaxDistance() const { return mSoundImpactEntity_Max; }

            // SMOKE PROPERTIES
            void SetSmokeEffect( const std::string& fileName ) { mEffectSmoke = fileName; }
            std::string GetSmokeEffect() const { return mEffectSmoke; }

            void SetSmokeLifeTime( float lifeTime ) { mTimeSmoke = lifeTime; }
            float GetSmokeLifeTime() const { return mTimeSmoke; }

            // TRACER PROPERTIES
            void SetTracerShaderName( const std::string& shaderName ) { mTracerShaderName = shaderName; }
            std::string GetTracerShaderName() const { return mTracerShaderName; }

            void SetTracerShaderGroup( const std::string& shaderGroup ) { mTracerShaderGroup = shaderGroup; }
            std::string GetTracerShaderGroup() const { return mTracerShaderGroup; }

            void SetTracerLifeTime( float lifeTime ) { mTracerLifeTime = lifeTime; }
            float GetTracerLifeTime() const { return mTracerLifeTime; }

            void SetTracerLength( float tracerLength ) { mTracerLength = tracerLength; }
            float GetTracerLength() const { return mTracerLength; }

            void SetTracerThickness( float tracerThickness ) { mTracerThickness = tracerThickness; }
            float GetTracerThickness() const { return mTracerThickness; }

            // LIGHT PROPERTIES
            void SetGroundImpactLight( const std::string& lightName ) { mLightImpactGround = lightName; }
            std::string GetGroundImpactLight() const { return mLightImpactGround; }

            void SetEntityImpactLight( const std::string& lightName ) { mLightImpactEntity = lightName; }
            std::string GetEntityImpactLight() const { return mLightImpactEntity; }

            void SetFireLight( const std::string& lightName ) { mLightFire = lightName; }
            std::string GetFireLight() const { return mLightFire; }

            void SetTracerLight( const std::string& lightName ) { mLightTracer = lightName; }
            std::string GetTracerLight() const { return mLightTracer; }

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
            dtCore::RefPtr<MunitionEffectsInfoActorProxy> mProxy;
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

            void SetEffectsInfoActor( dtDAL::ActorProxy* proxy )
            { 
               mEffects = proxy != NULL ? 
                  dynamic_cast<MunitionEffectsInfoActor*> (proxy->GetActor()) : NULL;
               mProxy->SetLinkedActor("Effects Info", proxy);
            }
            dtCore::DeltaDrawable* GetEffectsInfoDrawable()
            { 
               dtDAL::ActorProxy* proxy = mProxy->GetLinkedActor("Effects Info");
               return proxy != NULL ? proxy->GetActor() : NULL;
            }

            const MunitionEffectsInfoActor* GetEffectsInfoActor() const { return mEffects.get(); }

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
            dtCore::RefPtr<MunitionEffectsInfoActor> mEffects;
            dtCore::RefPtr<MunitionTypeActorProxy> mProxy;
      };

   }
}

#endif
