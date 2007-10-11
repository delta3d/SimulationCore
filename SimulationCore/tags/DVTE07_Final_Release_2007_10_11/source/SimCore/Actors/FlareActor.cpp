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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/particlesystem.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <SimCore/Actors/FlareActor.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // FLARE ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      FlareActor::FlareActor( FlareActorProxy& proxy )
         : BaseEntity(proxy),
         mCell(0),
         mGuise(0),
         mNumberSources(0),
         mTimeSinceDetonation(0), // ms
         mHeight(0.0f),
         mHeightDelta(0.0f),
         mPeakAngle(0.0f),
         mPeakAngleDelta(0.0f),
         mSourceIntensity(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      FlareActor::~FlareActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetCell( int cell )
      {
         mCell = cell;
      }

      //////////////////////////////////////////////////////////////////////////
      int FlareActor::GetCell() const
      {
         return mCell;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetGuise( int guise )
      {
         mGuise = guise;
      }

      //////////////////////////////////////////////////////////////////////////
      int FlareActor::GetGuise() const
      {
         return mGuise;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetNumberOfSources( int numberSources )
      {
         mNumberSources = numberSources;
      }

      //////////////////////////////////////////////////////////////////////////
      int FlareActor::GetNumberOfSources() const
      {
         return mNumberSources;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetTimeSinceDetonation( int timeSinceDetonation )
      {
         mTimeSinceDetonation = timeSinceDetonation;
      }

      //////////////////////////////////////////////////////////////////////////
      int FlareActor::GetTimeSinceDetonation() const
      {
         return mTimeSinceDetonation;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetHeight( float height )
      {
         mHeight = height;
      }

      //////////////////////////////////////////////////////////////////////////
      float FlareActor::GetHeight() const
      {
         return mHeight;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetHeightDelta( float heightDelta )
      {
         mHeightDelta = heightDelta;
      }

      //////////////////////////////////////////////////////////////////////////
      float FlareActor::GetHeightDelta() const
      {
         return mHeightDelta;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetPeakAngle( float angle )
      {
         mPeakAngle = angle;
      }

      //////////////////////////////////////////////////////////////////////////
      float FlareActor::GetPeakAngle()const
      {
         return mPeakAngle;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetPeakAngleDelta( float angleDelta )
      {
         mPeakAngleDelta = angleDelta;
      }

      //////////////////////////////////////////////////////////////////////////
      float FlareActor::GetPeakAngleDelta()const
      {
         return mPeakAngleDelta;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetSourceIntensity( float intensity )
      {
         mSourceIntensity = intensity;
      }

      //////////////////////////////////////////////////////////////////////////
      float FlareActor::GetSourceIntensity() const
      {
         return mSourceIntensity;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetModelType( const std::string& modelType )
      {
         mModelType = modelType;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& FlareActor::GetModelType() const
      {
         return mModelType;
      }

      //////////////////////////////////////////////////////////////////////////
      std::string FlareActor::GetModelTypeString() const
      {
         return mModelType;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::SetParticleFileName( const std::string& fileName )
      {
         mParticleFileName = fileName;
      }

      //////////////////////////////////////////////////////////////////////////
      std::string FlareActor::GetParticleFileName() const
      {
         return mParticleFileName;
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::LoadParticlesFile( const std::string& fileName )
      {
         mParticles = new dtCore::ParticleSystem("FlareParticles");
         // Flare particles designed to keep particles together at one point
         // to give the effect of shimmer and brightness (via additive blending).
         mParticles->SetParentRelative(true);
         mParticles->LoadFile(fileName);
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::HandleModelDrawToggle( bool draw )
      {
         // Do nothing for now.
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActor::OnEnteredWorld()
      {
         BaseEntity::OnEnteredWorld();

         LoadParticlesFile( mParticleFileName );

         if( mParticles.valid() )
         {
            AddChild( mParticles.get() );
            RegisterParticleSystem( *mParticles );

            RegisterWithDeadReckoningComponent();
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // FLARE ACTOR PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      FlareActorProxy::FlareActorProxy()
      {
         SetClassName("dvte::actors::FlareActor");
      }

      //////////////////////////////////////////////////////////////////////////
      FlareActorProxy::~FlareActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActorProxy::BuildPropertyMap()
      {
         BaseEntityActorProxy::BuildPropertyMap();

         FlareActor& actor = static_cast<FlareActor&>(GetGameActor());

         AddProperty(new dtDAL::IntActorProperty("Cell", "Cell", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetCell), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetCell), 
            "Assumed to be the regional atmospheric position"));

         AddProperty(new dtDAL::IntActorProperty("Guise", "Guise", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetGuise), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetGuise), 
            "")); // ?

         AddProperty(new dtDAL::IntActorProperty("Number Of Sources", "Number Of Sources", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetNumberOfSources), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetNumberOfSources), 
            "Assumed to be the number of sub-components that produce light"));

         AddProperty(new dtDAL::IntActorProperty("Time Since Detonation", "Time Since Detonation", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetTimeSinceDetonation), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetTimeSinceDetonation), 
            "Length of time that the flare has been luminous, measured in milliseconds"));

         AddProperty(new dtDAL::FloatActorProperty("Height", "Height", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetHeight), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetHeight), 
            "Assumed to be height above ground level measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Height Delta", "Height Delta", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetHeightDelta), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetHeightDelta), 
            "Assumed to be the change in height since the last actor update, measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty("Peak Angle", "Peak Angle", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetPeakAngle), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetPeakAngle), 
            "")); // ?

         AddProperty(new dtDAL::FloatActorProperty("Peak Angle Delta", "Peak Angle Delta", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetPeakAngleDelta), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetPeakAngleDelta), 
            "")); // ?

         AddProperty(new dtDAL::FloatActorProperty("Source Intensity", "Source Intensity", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetSourceIntensity), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetSourceIntensity), 
            "")); // ?

         AddProperty(new dtDAL::StringActorProperty("Model Type", "Model Type", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetModelType), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetModelTypeString), 
            "The type of flare being modeled by this actor"));

         AddProperty(new dtDAL::StringActorProperty("Particle File", "Particle File", 
            dtDAL::MakeFunctor(actor, &FlareActor::SetParticleFileName), 
            dtDAL::MakeFunctorRet(actor, &FlareActor::GetParticleFileName), 
            "The type of flare being modeled by this actor"));
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActorProxy::CreateActor()
      {
         SetActor( *new FlareActor(*this) );

         (static_cast<FlareActor*>(GetActor()))->InitDeadReckoningHelper();
      }

   }
}
