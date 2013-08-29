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

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <dtUtil/stringutils.h>
#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>
#include <SimCore/Actors/DetonationActor.h>
#include <SimCore/Actors/ViewerMaterialActor.h>
#include <dtDAL/propertymacros.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // Enumeration Code
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(MunitionFamily);
      MunitionFamily MunitionFamily::FAMILY_UNKNOWN("UNKNOWN");
      MunitionFamily MunitionFamily::FAMILY_ROUND("ROUND");
      MunitionFamily MunitionFamily::FAMILY_EXPLOSIVE_ROUND("EXPLOSIVE ROUND");
      MunitionFamily MunitionFamily::FAMILY_GRENADE("GRENADE");
      MunitionFamily MunitionFamily::FAMILY_MINE("MINE");
      MunitionFamily MunitionFamily::FAMILY_MISSILE("MISSILE");
      MunitionFamily MunitionFamily::FAMILY_GENERIC_EXPLOSIVE("GENERIC EXPLOSIVE");

      //////////////////////////////////////////////////////////////////////////
      bool MunitionFamily::IsExplosive() const
      {
         // Anything other than a round or UNKNOWN is treated as an explosive.
         return this != &SimCore::Actors::MunitionFamily::FAMILY_ROUND
            && this != &SimCore::Actors::MunitionFamily::FAMILY_UNKNOWN;
      }



      //////////////////////////////////////////////////////////////////////////
      // DIS Identifier Code
      //////////////////////////////////////////////////////////////////////////
      DISIdentifier::DISIdentifier( unsigned char kind, unsigned char domain,
         unsigned short country, unsigned char category, unsigned char subcategory,
         unsigned char specific, unsigned char extra )
         : mKind(kind),
         mDomain(domain),
         mCountry(country),
         mCategory(category),
         mSubcategory(subcategory),
         mSpecific(specific),
         mExtra(extra)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void DISIdentifier::Set( unsigned char kind, unsigned char domain,
         unsigned short country, unsigned char category, unsigned char subcategory,
         unsigned char specific, unsigned char extra )
      {
         mKind = kind;
         mDomain = domain;
         mCountry = country;
         mCategory = category;
         mSubcategory = subcategory;
         mSpecific = specific;
         mExtra = extra;
      }

      //////////////////////////////////////////////////////////////////////////
      void DISIdentifier::SetByString( const std::string& disValues )
      {
         std::vector<std::string> tokens;
         dtUtil::StringTokenizer<dtUtil::IsSpace>::tokenize( tokens, disValues );

         // Zero out all values to make ready for the new parsed values.
         Set(0,0,0,0,0,0,0);

         unsigned char value = 0;
         std::vector<std::string>::iterator curToken = tokens.begin();
         std::vector<std::string>::iterator endTokenList = tokens.end();

         for( size_t loop = 0; curToken != endTokenList; ++loop, ++curToken )
         {
            // This should use dtUtil::ToType, but toType returns garbage if it doesn't
            // find an int, which makes the unit tests fail. atoi return 0.
            value = (unsigned char)(atoi(curToken->c_str()));
            switch( loop )
            {
               case 6: mExtra = value;
                  break;
               case 5: mSpecific = value;
                  break;
               case 4: mSubcategory = value;
                  break;
               case 3: mCategory = value;
                  break;
               case 2: mCountry = value;
                  break;
               case 1: mDomain = value;
                  break;
               case 0: mKind = value;
                  break;
               default: break;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      std::string DISIdentifier::ToString() const
      {
         std::stringstream ss;
         ss << (unsigned int)mKind << " " << (unsigned int)mDomain << " "
            << (unsigned int)mCountry << " " << (unsigned int)mCategory << " "
            << (unsigned int)mSubcategory << " " << (unsigned int)mSpecific << " "
            << (unsigned int)mExtra;
         return ss.str();
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int DISIdentifier::GetNumber( unsigned int numberIndex ) const
      {
         switch( numberIndex )
         {
            case 0: return mKind;
            case 1: return mDomain;
            case 2: return mCountry;
            case 3: return mCategory;
            case 4: return mSubcategory;
            case 5: return mSpecific;
            case 6: return mExtra;
            default: return 0;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int DISIdentifier::GetDegreeOfMatch( const DISIdentifier& dis ) const
      {
         unsigned int matchDegree = 0;
         bool stillMatches = GetKind() == dis.GetKind();
         if( stillMatches ) { ++matchDegree; stillMatches = mDomain == dis.mDomain; }
         if( stillMatches ) { ++matchDegree; stillMatches = mCountry == dis.mCountry; }
         if( stillMatches ) { ++matchDegree; stillMatches = mCategory == dis.mCategory; }
         if( stillMatches ) { ++matchDegree; stillMatches = mSubcategory == dis.mSubcategory; }
         if( stillMatches ) { ++matchDegree; stillMatches = mSpecific == dis.mSpecific; }
         if( stillMatches ) { ++matchDegree; stillMatches = mExtra == dis.mExtra; }
         if( stillMatches ) { ++matchDegree; }
         return matchDegree;
      }

      //////////////////////////////////////////////////////////////////////////
      const DISIdentifier& DISIdentifier::operator= ( const DISIdentifier& dis )
      {
         if( &dis != this )
         {
            mKind = dis.mKind;
            mDomain = dis.mDomain;
            mCountry = dis.mCountry;
            mCategory = dis.mCategory;
            mSubcategory = dis.mSubcategory;
            mSpecific = dis.mSpecific;
            mExtra = dis.mExtra;
         }
         return *this;
      }

      //////////////////////////////////////////////////////////////////////////
      bool DISIdentifier::operator== ( const DISIdentifier& dis ) const
      {
         if( &dis == this ) { return true; }
         return mKind == dis.mKind && mDomain == dis.mDomain && mCountry == dis.mCountry
            && mCategory == dis.mCategory && mSubcategory == dis.mSubcategory
            && mSpecific == dis.mSpecific && mExtra == dis.mExtra;
      }

      //////////////////////////////////////////////////////////////////////////
      bool DISIdentifier::operator!= ( const DISIdentifier& dis ) const
      {
         return ! ( dis == *this );
      }

      //////////////////////////////////////////////////////////////////////////
      bool DISIdentifier::operator< ( const DISIdentifier& dis ) const
      {
         switch( GetDegreeOfMatch( dis ) )
         {
            case 1: return mDomain < dis.mDomain;
            case 2: return mCountry < dis.mCountry;
            case 3: return mCategory < dis.mCategory;
            case 4: return mSubcategory < dis.mSubcategory;
            case 5: return mSpecific < dis.mSpecific;
            case 6: return mExtra < dis.mExtra;
            case 7: return false; // Full match
            default: return mKind < dis.mKind;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool DISIdentifier::operator> ( const DISIdentifier& dis ) const
      {
         switch( GetDegreeOfMatch( dis ) )
         {
            case 1: return mDomain > dis.mDomain;
            case 2: return mCountry > dis.mCountry;
            case 3: return mCategory > dis.mCategory;
            case 4: return mSubcategory > dis.mSubcategory;
            case 5: return mSpecific > dis.mSpecific;
            case 6: return mExtra > dis.mExtra;
            case 7: return false; // Full match
            default: return mKind > dis.mKind;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool DISIdentifier::operator<= ( const DISIdentifier& dis ) const
      {
         return (*this) == dis || (*this) < dis;
      }

      //////////////////////////////////////////////////////////////////////////
      bool DISIdentifier::operator>= ( const DISIdentifier& dis ) const
      {
         return (*this) == dis || (*this) > dis;
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Effect Actor Proxy Code
      //////////////////////////////////////////////////////////////////////////
      const std::string MunitionEffectsInfoActorProxy::CLASS_NAME("SimCore::Actors::MunitionEffectsInfoActor");

      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActorProxy::MunitionEffectsInfoActorProxy()
      {
         SetClassName( CLASS_NAME );
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActorProxy::~MunitionEffectsInfoActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActorProxy::CreateActor()
      {
         SetDrawable( *new MunitionEffectsInfoActor(*this) );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActorProxy::BuildPropertyMap()
      {
         MunitionEffectsInfoActor& actor = static_cast<MunitionEffectsInfoActor&>(*GetDrawable());

         static const dtUtil::RefString groupFly("Fly Effects");
         static const dtUtil::RefString groupFire("Fire Effects");
         static const dtUtil::RefString groupGroundImpact("Ground Impact Effects");
         static const dtUtil::RefString groupEntityImpact("Entity Impact Effects");
         static const dtUtil::RefString groupSmoke("Smoke Effects");
         static const dtUtil::RefString groupTracer("Tracer Effects");

         // MUNITION FLIGHT
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Fly Effect", "Fly Effect",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFlyEffect ),
            "The file path of the flying visual effect", groupFly));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Fly Sound", "Fly Sound",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFlySound ),
            "The file path of the flying sound effect", groupFly));

         AddProperty(new dtDAL::FloatActorProperty("Fly Sound Max Distance", "Fly Sound Max Distance",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFlySoundMaxDistance ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetFlySoundMaxDistance ),
            "The maximum range in meters for munition flight sounds", groupFly));

         // SHOT FIRED PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Fire Effect", "Fire Effect",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFireEffect ),
            "The file path of the firing visual effect", groupFire));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Fire Sound", "Fire Sound",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFireSound ),
            "The file path of the firing sound effect", groupFire));

         AddProperty(new dtDAL::FloatActorProperty("Fire Sound Max Distance", "Fire Sound Max Distance",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFireSoundMaxDistance ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetFireSoundMaxDistance ),
            "The maximum range in meters for shot fired sounds", groupFire));

         AddProperty(new dtDAL::FloatActorProperty( "Fire Flash Probability", "Fire Flash Probability",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFireFlashProbability ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetFireFlashProbability ),
            "The probability (0.0 to 1.0) that a visible flash will occur", groupFire));

         AddProperty(new dtDAL::FloatActorProperty( "Fire Flash Time", "Fire Flash Time",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFireFlashTime ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetFireFlashTime ),
            "The length of time that a flash effect shall remain visible", groupFire));

         AddProperty(new dtDAL::StringActorProperty("Fire Light", "Fire Light",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetFireLight ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetFireLight ),
            "The name of light effect for the weapon fire effect", groupFire));

         // GROUND IMPACT PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Ground Impact Effect", "Ground Impact Effect",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetGroundImpactEffect ),
            "The file path of the ground impact visual effect", groupGroundImpact));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Ground Impact Sound", "Ground Impact Sound",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetGroundImpactSound ),
            "The file path of the ground impact sound effect", groupGroundImpact));

         AddProperty(new dtDAL::FloatActorProperty("Ground Impact Sound Max Distance", "Ground Impact Sound Max Distance",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetGroundImpactSoundMaxDistance ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetGroundImpactSoundMaxDistance),
            "The maximum range in meters for ground impact sounds", groupGroundImpact));

         AddProperty(new dtDAL::StringActorProperty("Ground Impact Light", "Ground Impact Light",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetGroundImpactLight ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetGroundImpactLight ),
            "The name of light effect for the ground impact effect", groupGroundImpact));

         // ENTITY IMPACT PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Entity Impact Effect", "Entity Impact Effect",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetEntityImpactEffect ),
            "The file path of the entity impact visual effect", groupEntityImpact));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Entity Impact Sound", "Entity Impact Sound",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetEntityImpactSound ),
            "The file path of the entity impact sound effect", groupEntityImpact));

         AddProperty(new dtDAL::FloatActorProperty("Entity Impact Sound Max Distance", "Entity Impact Sound Max Distance",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetEntityImpactSoundMaxDistance ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetEntityImpactSoundMaxDistance),
            "The maximum range in meters for entity impact sounds", groupEntityImpact));

         AddProperty(new dtDAL::StringActorProperty("Entity Impact Light", "Entity Impact Light",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetEntityImpactLight ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetEntityImpactLight ),
            "The name of light effect for the entity impact effect", groupEntityImpact));

         // SMOKE PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Smoke Effect", "Smoke Effect",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetSmokeEffect ),
            "The file path of the post-detonation smoke visual effect", groupSmoke));

         AddProperty(new dtDAL::FloatActorProperty("Smoke Life Time", "Smoke Life Time",
                  dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetSmokeLifeTime ),
                  dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetSmokeLifeTime ),
            "The time in seconds for the smoke effect to remain active", groupSmoke));

         // TRACER PROPERTIES
         AddProperty(new dtDAL::StringActorProperty("Tracer Shader Name", "Tracer Shader Name",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetTracerShaderName ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetTracerShaderName ),
            "The name of the shader responsible for rendering the tracer", groupTracer));

         AddProperty(new dtDAL::StringActorProperty("Tracer Shader Group", "Tracer Shader Group",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetTracerShaderGroup ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetTracerShaderGroup ),
            "The shader group in which the tracer shader can be found", groupTracer));

         AddProperty(new dtDAL::FloatActorProperty("Tracer Life Time", "Tracer Life Time",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetTracerLifeTime ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetTracerLifeTime ),
            "The maximum time that a tracer will remain visible", groupTracer));

         AddProperty(new dtDAL::FloatActorProperty("Tracer Length", "Tracer Length",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetTracerLength ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetTracerLength ),
            "The visual length of the tracer in meters", groupTracer));

         AddProperty(new dtDAL::FloatActorProperty("Tracer Thickness", "Tracer Thickness",
            dtDAL::FloatActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetTracerThickness ),
            dtDAL::FloatActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetTracerThickness ),
            "The visual thickness of the tracer in meters", groupTracer));

         AddProperty(new dtDAL::StringActorProperty("Tracer Light", "Tracer Light",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionEffectsInfoActor::SetTracerLight ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionEffectsInfoActor::GetTracerLight ),
            "The name of light effect for the tracer effect", groupTracer));
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Effect Actor Code
      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActor::MunitionEffectsInfoActor( MunitionEffectsInfoActorProxy& proxy )
         : dtCore::DeltaDrawable("MunitionEffectsInfoActor")
         , mProbabilityFireFlash(1.0f)
         , mSoundFire_Min(10.0f)
         , mSoundFire_Max(500.0f)
         , mSoundImpactGround_Min(10.0f)
         , mSoundImpactGround_Max(500.0f)
         , mSoundImpactEntity_Min(10.0f)
         , mSoundImpactEntity_Max(500.0f)
         , mSoundFly_Min(10.0f)
         , mSoundFly_Max(500.0f)
         , mTimeSmoke(10.0f)
         , mFireFlashTime(0.1f)
         , mTracerLifeTime(1.0f)
         , mTracerLength(1.0f)
         , mTracerThickness(0.15f)
         , mProxy(&proxy)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActor::~MunitionEffectsInfoActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Node* MunitionEffectsInfoActor::GetOSGNode() { return NULL; }
      const osg::Node* MunitionEffectsInfoActor::GetOSGNode() const { return NULL; }

      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActorProxy& MunitionEffectsInfoActor::GetProxy() { return *mProxy; }
      const MunitionEffectsInfoActorProxy& MunitionEffectsInfoActor::GetProxy() const { return *mProxy; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFlyEffect( const std::string& fileName ) { mEffectFly = fileName; }
      const std::string& MunitionEffectsInfoActor::GetFlyEffect() const { return mEffectFly; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFlySound( const std::string& fileName ) { mSoundFly = fileName; }
      const std::string& MunitionEffectsInfoActor::GetFlySound() const { return mSoundFly; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFlySoundMaxDistance( float distance ) { mSoundFly_Max = distance; }
      float MunitionEffectsInfoActor::GetFlySoundMaxDistance() const { return mSoundFly_Max; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFireEffect( const std::string& fileName ) { mEffectFire = fileName; }
      const std::string& MunitionEffectsInfoActor::GetFireEffect() const { return mEffectFire; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFireSound( const std::string& fileName ) { mSoundFire = fileName; }
      const std::string& MunitionEffectsInfoActor::GetFireSound() const { return mSoundFire; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFireSoundMaxDistance( float distance ) { mSoundFire_Max = distance; }
      float MunitionEffectsInfoActor::GetFireSoundMaxDistance() const { return mSoundFire_Max; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFireFlashProbability( float probability )
      {
         mProbabilityFireFlash = probability < 0.0f ? 0.0f : probability > 1.0f ? 1.0f : probability;
      }
      float MunitionEffectsInfoActor::GetFireFlashProbability() const { return mProbabilityFireFlash; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFireFlashTime( float flashTime ) { mFireFlashTime = flashTime; }
      float MunitionEffectsInfoActor::GetFireFlashTime() const { return mFireFlashTime; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetGroundImpactEffect( const std::string& fileName ) { mEffectImpactGround = fileName; }
      const std::string& MunitionEffectsInfoActor::GetGroundImpactEffect() const { return mEffectImpactGround; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetGroundImpactSound( const std::string& fileName ) { mSoundImpactGround = fileName; }
      const std::string& MunitionEffectsInfoActor::GetGroundImpactSound() const { return mSoundImpactGround; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetGroundImpactSoundMaxDistance( float distance ) { mSoundImpactGround_Max = distance; }
      float MunitionEffectsInfoActor::GetGroundImpactSoundMaxDistance() const { return mSoundImpactGround_Max; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetEntityImpactEffect( const std::string& fileName ) { mEffectImpactEntity = fileName; }
      const std::string& MunitionEffectsInfoActor::GetEntityImpactEffect() const { return mEffectImpactEntity; }
      bool MunitionEffectsInfoActor::HasEntityImpactEffect() const { return ! mEffectImpactEntity.empty(); }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetEntityImpactSound( const std::string& fileName ) { mSoundImpactEntity = fileName; }
      const std::string& MunitionEffectsInfoActor::GetEntityImpactSound() const { return mSoundImpactEntity; }
      bool MunitionEffectsInfoActor::HasEntityImpactSound() const { return ! mSoundImpactEntity.empty(); }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetEntityImpactSoundMaxDistance( float distance ) { mSoundImpactEntity_Max = distance; }
      float MunitionEffectsInfoActor::GetEntityImpactSoundMaxDistance() const { return mSoundImpactEntity_Max; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetSmokeEffect( const std::string& fileName ) { mEffectSmoke = fileName; }
      const std::string& MunitionEffectsInfoActor::GetSmokeEffect() const { return mEffectSmoke; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetSmokeLifeTime( float lifeTime ) { mTimeSmoke = lifeTime; }
      float MunitionEffectsInfoActor::GetSmokeLifeTime() const { return mTimeSmoke; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetTracerShaderName( const std::string& shaderName ) { mTracerShaderName = shaderName; }
      const std::string& MunitionEffectsInfoActor::GetTracerShaderName() const { return mTracerShaderName; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetTracerShaderGroup( const std::string& shaderGroup ) { mTracerShaderGroup = shaderGroup; }
      const std::string& MunitionEffectsInfoActor::GetTracerShaderGroup() const { return mTracerShaderGroup; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetTracerLifeTime( float lifeTime ) { mTracerLifeTime = lifeTime; }
      float MunitionEffectsInfoActor::GetTracerLifeTime() const { return mTracerLifeTime; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetTracerLength( float tracerLength ) { mTracerLength = tracerLength; }
      float MunitionEffectsInfoActor::GetTracerLength() const { return mTracerLength; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetTracerThickness( float tracerThickness ) { mTracerThickness = tracerThickness; }
      float MunitionEffectsInfoActor::GetTracerThickness() const { return mTracerThickness; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetGroundImpactLight( const std::string& lightName ) { mLightImpactGround = lightName; }
      const std::string& MunitionEffectsInfoActor::GetGroundImpactLight() const { return mLightImpactGround; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetEntityImpactLight( const std::string& lightName ) { mLightImpactEntity = lightName; }
      const std::string& MunitionEffectsInfoActor::GetEntityImpactLight() const { return mLightImpactEntity; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetFireLight( const std::string& lightName ) { mLightFire = lightName; }
      const std::string& MunitionEffectsInfoActor::GetFireLight() const { return mLightFire; }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActor::SetTracerLight( const std::string& lightName ) { mLightTracer = lightName; }
      const std::string& MunitionEffectsInfoActor::GetTracerLight() const { return mLightTracer; }



      //////////////////////////////////////////////////////////////////////////
      // Munition Type Actor Proxy Code
      //////////////////////////////////////////////////////////////////////////
      const std::string MunitionTypeActorProxy::CLASS_NAME("SimCore::Actors::MunitionTypeActor");

      //////////////////////////////////////////////////////////////////////////
      MunitionTypeActorProxy::MunitionTypeActorProxy()
      {
         SetClassName(CLASS_NAME);
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionTypeActorProxy::~MunitionTypeActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActorProxy::CreateActor()
      {
         SetDrawable( *new MunitionTypeActor(*this) );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActorProxy::SetEffectsInfoActor( dtDAL::ActorProxy* proxy )
      {
         MunitionTypeActor* actor = static_cast<MunitionTypeActor*>(GetDrawable());
         actor->SetEffectsInfoActor( proxy );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActorProxy::BuildPropertyMap()
      {
         MunitionTypeActor& actor = static_cast<MunitionTypeActor&>(*GetDrawable());

         typedef dtDAL::PropertyRegHelper<MunitionTypeActorProxy&, MunitionTypeActor> RegHelperType;
         RegHelperType propReg(*this, &actor, "Detonation");

         DT_REGISTER_ACTOR_ID_PROPERTY(DetonationActorProxy::CLASS_NAME, DetonationActor, "Detonation Prototype",
                                       "This is the class that will be spawned to handle the detonation for this munition type", RegHelperType, propReg);

         AddProperty(new dtDAL::StringActorProperty("DIS Identifier", "DIS Identifier",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionTypeActor::SetDISIdentifierByString ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionTypeActor::GetDISIdentifierString ),
            "The DIS identifier used by the munition on the network"));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::STATIC_MESH,
            "Model", "Model",
            dtDAL::ResourceActorProperty::SetFuncType( &actor, &MunitionTypeActor::SetModel ),
            "The file path of the model that should represent this munition"));

         AddProperty(new dtDAL::StringActorProperty("Damage Type", "Damage Type",
            dtDAL::StringActorProperty::SetFuncType( &actor, &MunitionTypeActor::SetDamageType ),
            dtDAL::StringActorProperty::GetFuncType( &actor, &MunitionTypeActor::GetDamageTypeString ),
            "The munition name as found in the MunitionsConfig.xml for damage probabilities"));

         AddProperty(new dtDAL::IntActorProperty("Fuse Type", "Fuse Type",
            dtDAL::IntActorProperty::SetFuncType( &actor, &MunitionTypeActor::SetFuseType ),
            dtDAL::IntActorProperty::GetFuncType( &actor, &MunitionTypeActor::GetFuseType ),
            "The fuse type code as found in a fed file."));

         AddProperty(new dtDAL::IntActorProperty("Warhead Type", "Warhead Type",
            dtDAL::IntActorProperty::SetFuncType( &actor, &MunitionTypeActor::SetWarheadType ),
            dtDAL::IntActorProperty::GetFuncType( &actor, &MunitionTypeActor::GetWarheadType ),
            "The warhead type code as found in a fed file."));

         AddProperty(new dtDAL::IntActorProperty("Tracer Frequency", "Tracer Frequency",
            dtDAL::IntActorProperty::SetFuncType( &actor, &MunitionTypeActor::SetTracerFrequency ),
            dtDAL::IntActorProperty::GetFuncType( &actor, &MunitionTypeActor::GetTracerFrequency ),
            "Every nth round fired that will be a tracer (intended for bullet type munitions)"));

         AddProperty(new dtDAL::EnumActorProperty<MunitionFamily>("Family", "Family",
            dtDAL::EnumActorProperty<MunitionFamily>::SetFuncType( &actor, &MunitionTypeActor::SetFamily ),
            dtDAL::EnumActorProperty<MunitionFamily>::GetFuncType( &actor, &MunitionTypeActor::GetFamily ),
            "Generalized munition type grouping based on the munition's overall functionality."));

         AddProperty(new dtDAL::ActorActorProperty( *this, "Effects Info", "Effects Info",
            dtDAL::ActorActorProperty::SetFuncType( this, &MunitionTypeActorProxy::SetEffectsInfoActor ),
            dtDAL::ActorActorProperty::GetFuncType( ),
            MunitionEffectsInfoActorProxy::CLASS_NAME,
            "A reference to the collection of effect resource file paths, for sound, models and particle systems",
            "Munition Effects"
            ));
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Type Actor Code
      //////////////////////////////////////////////////////////////////////////
      MunitionTypeActor::MunitionTypeActor( MunitionTypeActorProxy& proxy )
         : dtCore::DeltaDrawable("MunitionTypeActor")
         , mFuse(0)
         , mWarhead(0)
         , mTracerFrequency(0)
         , mModel()
         , mDamageType()
         , mFamily(&MunitionFamily::FAMILY_UNKNOWN)
         , mDIS()
         , mDetonationPrototype()
         , mEffects()
         , mProxy(&proxy)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionTypeActor::~MunitionTypeActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActor::SetDISIdentifier( const DISIdentifier& dis )
      {
         mDIS = dis;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActor::SetDISIdentifierByString( const std::string& dis )
      {
         mDIS.SetByString( dis );
      }

      //////////////////////////////////////////////////////////////////////////
      std::string MunitionTypeActor::GetDISIdentifierString() const
      {
         return mDIS.ToString();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActor::SetEffectsInfoActor( dtDAL::ActorProxy* proxy )
      {
         mEffects = proxy != NULL ?
            dynamic_cast<MunitionEffectsInfoActor*> (proxy->GetDrawable()) : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const MunitionEffectsInfoActor* MunitionTypeActor::GetEffectsInfoActor() const
      {
         return mEffects.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActor::SetDetonationActorPrototype(dtDAL::ActorProxy* proxy)
      {
         if(proxy != NULL)
         {
            DetonationActorProxy* detonProxy = dynamic_cast<DetonationActorProxy*>(proxy);
            mDetonationPrototype = detonProxy;
         }
         else
         {
            mDetonationPrototype = NULL;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      DetonationActorProxy* MunitionTypeActor::GetDetonationActorPrototype()
      {
         return mDetonationPrototype.get();
      }
   }
}
