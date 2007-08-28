/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#include <prefix/dvteprefix-src.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <dtUtil/stringutils.h>
#include <dtDAL/enginepropertytypes.h>

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
         unsigned int numTokens = tokens.size();

         switch( numTokens )
         {
         case 7: mExtra = (unsigned char) atoi(tokens[6].c_str()); 
         case 6: mSpecific = (unsigned char) atoi(tokens[5].c_str()); 
         case 5: mSubcategory = (unsigned char) atoi(tokens[4].c_str()); 
         case 4: mCategory = (unsigned char) atoi(tokens[3].c_str()); 
         case 3: mCountry = (unsigned short) atoi(tokens[2].c_str()); 
         case 2: mDomain = (unsigned char) atoi(tokens[1].c_str()); 
         case 1: mKind = (unsigned char) atoi(tokens[0].c_str()); 
         default: break;
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
         SetActor( *new MunitionEffectsInfoActor(*this) );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionEffectsInfoActorProxy::BuildPropertyMap()
      {
         MunitionEffectsInfoActor& actor = static_cast<MunitionEffectsInfoActor&>(*GetActor());

         std::string groupFly("Fly Effects");
         std::string groupFire("Fire Effects");
         std::string groupGroundImpact("Ground Impact Effects");
         std::string groupEntityImpact("Entity Impact Effects");
         std::string groupSmoke("Smoke Effects");
         std::string groupTracer("Tracer Effects");

         // MUNITION FLIGHT
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Fly Effect", "Fly Effect", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFlyEffect ),  
            "The file path of the flying visual effect", groupFly));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Fly Sound", "Fly Sound", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFlySound ),  
            "The file path of the flying sound effect", groupFly));

         AddProperty(new dtDAL::FloatActorProperty("Fly Sound Min Distance", "Fly Sound Min Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFlySoundMinDistance ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetFlySoundMinDistance ), 
            "The minimum range in meters for munition flight sounds", groupFly));

         AddProperty(new dtDAL::FloatActorProperty("Fly Sound Max Distance", "Fly Sound Max Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFlySoundMaxDistance ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetFlySoundMaxDistance ), 
            "The maximum range in meters for munition flight sounds", groupFly));

         // SHOT FIRED PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Fire Effect", "Fire Effect", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFireEffect ),  
            "The file path of the firing visual effect", groupFire));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Fire Sound", "Fire Sound", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFireSound ), 
            "The file path of the firing sound effect", groupFire));

         AddProperty(new dtDAL::FloatActorProperty("Fire Sound Min Distance", "Fire Sound Min Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFireSoundMinDistance ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetFireSoundMinDistance ), 
            "The minimum range in meters for shot fired sounds", groupFire));

         AddProperty(new dtDAL::FloatActorProperty("Fire Sound Max Distance", "Fire Sound Max Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFireSoundMaxDistance ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetFireSoundMaxDistance ), 
            "The maximum range in meters for shot fired sounds", groupFire));

         AddProperty(new dtDAL::FloatActorProperty( "Fire Flash Probability", "Fire Flash Probability", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFireFlashProbability ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetFireFlashProbability ), 
            "The probability (0.0 to 1.0) that a visible flash will occur", groupFire));

         AddProperty(new dtDAL::FloatActorProperty( "Fire Flash Time", "Fire Flash Time", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetFireFlashTime ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetFireFlashTime ), 
            "The length of time that a flash effect shall remain visible", groupFire));

         // GROUND IMPACT PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Ground Impact Effect", "Ground Impact Effect", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetGroundImpactEffect ), 
            "The file path of the ground impact visual effect", groupGroundImpact));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Ground Impact Sound", "Ground Impact Sound", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetGroundImpactSound ), 
            "The file path of the ground impact sound effect", groupGroundImpact));

         AddProperty(new dtDAL::FloatActorProperty("Ground Impact Sound Min Distance", "Ground Impact Sound Min Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetGroundImpactSoundMinDistance ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetGroundImpactSoundMinDistance ), 
            "The minimum range in meters for ground impact sounds", groupGroundImpact));

         AddProperty(new dtDAL::FloatActorProperty("Ground Impact Sound Max Distance", "Ground Impact Sound Max Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetGroundImpactSoundMaxDistance ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetGroundImpactSoundMaxDistance),
            "The maximum range in meters for ground impact sounds", groupGroundImpact));

         // ENTITY IMPACT PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Entity Impact Effect", "Entity Impact Effect", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetEntityImpactEffect ), 
            "The file path of the entity impact visual effect", groupEntityImpact));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::SOUND,
            "Entity Impact Sound", "Entity Impact Sound", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetEntityImpactSound ), 
            "The file path of the entity impact sound effect", groupEntityImpact));

         AddProperty(new dtDAL::FloatActorProperty("Entity Impact Sound Min Distance", "Entity Impact Sound Min Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetEntityImpactSoundMinDistance ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetEntityImpactSoundMinDistance ), 
            "The minimum range in meters for entity impact sounds", groupEntityImpact));

         AddProperty(new dtDAL::FloatActorProperty("Entity Impact Sound Max Distance", "Entity Impact Sound Max Distance", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetEntityImpactSoundMaxDistance ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetEntityImpactSoundMaxDistance),
            "The maximum range in meters for entity impact sounds", groupEntityImpact));

         // SMOKE PROPERTIES
         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::PARTICLE_SYSTEM,
            "Smoke Effect", "Smoke Effect", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetSmokeEffect ), 
            "The file path of the post-detonation smoke visual effect", groupSmoke));

         AddProperty(new dtDAL::FloatActorProperty("Smoke Life Time", "Smoke Life Time", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetSmokeLifeTime ), 
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetSmokeLifeTime ), 
            "The time in seconds for the smoke effect to remain active", groupSmoke));

         // TRACER PROPERTIES
         AddProperty(new dtDAL::StringActorProperty("Tracer Shader Name", "Tracer Shader Name", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetTracerShaderName ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetTracerShaderName ),
            "The name of the shader responsible for rendering the tracer", groupTracer));

         AddProperty(new dtDAL::StringActorProperty("Tracer Shader Group", "Tracer Shader Group", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetTracerShaderGroup ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetTracerShaderGroup ),
            "The shader group in which the tracer shader can be found", groupTracer));

         AddProperty(new dtDAL::FloatActorProperty("Tracer Life Time", "Tracer Life Time", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetTracerLifeTime ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetTracerLifeTime ),
            "The maximum time that a tracer will remain visible", groupTracer));

         AddProperty(new dtDAL::FloatActorProperty("Tracer Length", "Tracer Length", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetTracerLength ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetTracerLength ),
            "The visual length of the tracer in meters", groupTracer));

         AddProperty(new dtDAL::FloatActorProperty("Tracer Thickness", "Tracer Thickness", 
            dtDAL::MakeFunctor( actor, &MunitionEffectsInfoActor::SetTracerThickness ),  
            dtDAL::MakeFunctorRet( actor, &MunitionEffectsInfoActor::GetTracerThickness ),
            "The visual thickness of the tracer in meters", groupTracer));
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Effect Actor Code
      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActor::MunitionEffectsInfoActor( MunitionEffectsInfoActorProxy& proxy )
         : dtCore::DeltaDrawable("MunitionEffectsInfoActor"),
         mProbabilityFireFlash(1.0f),
         mSoundFire_Min(10.0f),
         mSoundFire_Max(500.0f),
         mSoundImpactGround_Min(10.0f),
         mSoundImpactGround_Max(500.0f),
         mSoundImpactEntity_Min(10.0f),
         mSoundImpactEntity_Max(500.0f),
         mSoundFly_Min(10.0f),
         mSoundFly_Max(500.0f),
         mTimeSmoke(10.0f),
         mFireFlashTime(0.1f),
         mTracerLifeTime(1.0f),
         mTracerLength(1.0f),
         mTracerThickness(0.15f),
         mProxy(&proxy)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionEffectsInfoActor::~MunitionEffectsInfoActor()
      {
      }



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
         SetActor( *new MunitionTypeActor(*this) );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActorProxy::SetEffectsInfoActor( dtDAL::ActorProxy* proxy )
      {
         MunitionTypeActor* actor = static_cast<MunitionTypeActor*>(GetActor());
         actor->SetEffectsInfoActor( proxy );
      }

      //////////////////////////////////////////////////////////////////////////
      dtCore::DeltaDrawable* MunitionTypeActorProxy::GetEffectsInfoDrawable()
      {
         MunitionTypeActor* actor = static_cast<MunitionTypeActor*>(GetActor());
         return actor->GetEffectsInfoDrawable();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionTypeActorProxy::BuildPropertyMap()
      {
         MunitionTypeActor& actor = static_cast<MunitionTypeActor&>(*GetActor());

         AddProperty(new dtDAL::StringActorProperty("DIS Identifier", "DIS Identifier", 
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetDISIdentifierByString ), 
            dtDAL::MakeFunctorRet( actor, &MunitionTypeActor::GetDISIdentifierString ), 
            "The DIS identifier used by the munition on the network"));

         AddProperty(new dtDAL::ResourceActorProperty( *this, dtDAL::DataType::STATIC_MESH,
            "Model", "Model", 
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetModel ), 
            "The file path of the model that should represent this munition"));

         AddProperty(new dtDAL::StringActorProperty("Damage Type", "Damage Type", 
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetDamageType ), 
            dtDAL::MakeFunctorRet( actor, &MunitionTypeActor::GetDamageTypeString ), 
            "The munition name as found in the MunitionsConfig.xml for damage probabilities"));

         AddProperty(new dtDAL::IntActorProperty("Fuse Type", "Fuse Type", 
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetFuseType ), 
            dtDAL::MakeFunctorRet( actor, &MunitionTypeActor::GetFuseType ), 
            "The fuse type code as found in a fed file."));

         AddProperty(new dtDAL::IntActorProperty("Warhead Type", "Warhead Type", 
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetWarheadType ), 
            dtDAL::MakeFunctorRet( actor, &MunitionTypeActor::GetWarheadType ), 
            "The warhead type code as found in a fed file."));

         AddProperty(new dtDAL::IntActorProperty("Tracer Frequency", "Tracer Frequency", 
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetTracerFrequency ), 
            dtDAL::MakeFunctorRet( actor, &MunitionTypeActor::GetTracerFrequency ), 
            "Every nth round fired that will be a tracer (intended for bullet type munitions)"));

         AddProperty(new dtDAL::EnumActorProperty<MunitionFamily>("Family", "Family",
            dtDAL::MakeFunctor( actor, &MunitionTypeActor::SetFamily ),
            dtDAL::MakeFunctorRet( actor, &MunitionTypeActor::GetFamily ),
            "Generalized munition type grouping based on the munition's overall functionality."));

         AddProperty(new dtDAL::ActorActorProperty( *this, "Effects Info", "Effects Info",
            dtDAL::MakeFunctor( *this, &MunitionTypeActorProxy::SetEffectsInfoActor ), 
            dtDAL::MakeFunctorRet( *this, &MunitionTypeActorProxy::GetEffectsInfoDrawable ), 
            MunitionEffectsInfoActorProxy::CLASS_NAME,
            "A reference to the collection of effect resource file paths, for sound, models and particle systems",
            "Munition Effects"
            ));
      }



      //////////////////////////////////////////////////////////////////////////
      // Munition Type Actor Code
      //////////////////////////////////////////////////////////////////////////
      MunitionTypeActor::MunitionTypeActor( MunitionTypeActorProxy& proxy )
         : dtCore::DeltaDrawable("MunitionTypeActor"),
         mFuse(0),
         mWarhead(0),
         mTracerFrequency(0),
         mFamily(&MunitionFamily::FAMILY_UNKNOWN),
         mProxy(&proxy)
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

   }
}
