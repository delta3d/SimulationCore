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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <dtActors/engineactorregistry.h>
#include <dtCore/particlesystem.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtGame/gamemanager.h>
#include <dtGame/invokable.h>
#include <SimCore/Actors/FlareActor.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <osg/Vec3>


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
         mSourceIntensity(0.0f),
         mLightName(SimCore::Components::RenderingSupportComponent::DEFAULT_LIGHT_NAME)
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
      void FlareActor::SetLightName( const std::string& lightName )
      {
         mLightName = lightName;
      }

      //////////////////////////////////////////////////////////////////////////
      std::string FlareActor::GetLightName() const
      {
         return mLightName;
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
      void FlareActor::OnEnteredWorld()
      {
         BaseEntity::OnEnteredWorld();

         LoadParticlesFile( mParticleFileName );

         if( mParticles.valid() )
         {
            AddChild( mParticles.get() );
            RegisterParticleSystem( *mParticles );

            // Now handled in base class.
            //RegisterWithDeadReckoningComponent();


            //to make an illumination round dynamic light we must note that
            //these are dropped at 600meters and will light the ground directly below within
            //a radius of 1km
            SimCore::Components::RenderingSupportComponent* renderComp;
            GetGameActorProxy().GetGameManager()->GetComponentByName(
                  SimCore::Components::RenderingSupportComponent::DEFAULT_NAME,
                  renderComp);

            if( renderComp != nullptr )
            {
               SimCore::Components::RenderingSupportComponent::DynamicLight* dl =
                  renderComp->AddDynamicLightByPrototypeName( GetLightName() );
               dl->mTarget = this;

               // DEBUG:
               //SimCore::Components::RenderingSupportComponent::DynamicLight* dl = new SimCore::Components::RenderingSupportComponent::DynamicLight();
               //dl->mSaturationIntensity = 1.0f;
               //dl->mIntensity = 1.0f;//flare->GetSourceIntensity();
               //dl->mColor.set(osg::Vec3(1.0f, 1.0f, 1.0f));//flare->GetColor();
               //dl->mAttenuation.set(0.1, 0.005, 0.00002);
               //dl->mTarget = this;
               //dl->mAutoDeleteLightOnTargetNull = true;
               ////  //dl->mAutoDeleteAfterMaxTime = true;
               ////  //dl->mMaxTime = 20.0f;
               //dl->mFadeOut = true;
               //dl->mFadeOutTime = 5.0f;
               //dl->mFlicker = true;
               //dl->mFlickerScale = 0.1f;
               //dl->mRadius = 100.0f;
               //renderComp->AddDynamicLight(dl);
            }
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // FLARE ACTOR PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string& FlareActorProxy::PROPERTY_CELL = "Cell";
      const std::string& FlareActorProxy::PROPERTY_GUISE = "Guise";
      const std::string& FlareActorProxy::PROPERTY_NUMBER_OF_SOURCES = "Number Of Sources";
      const std::string& FlareActorProxy::PROPERTY_TIME_SINCE_DETONATION = "Time Since Detonation";
      const std::string& FlareActorProxy::PROPERTY_HEIGHT = "Height";
      const std::string& FlareActorProxy::PROPERTY_HEIGHT_DELTA = "Height Delta";
      const std::string& FlareActorProxy::PROPERTY_PEAK_ANGLE = "Peak Angle";
      const std::string& FlareActorProxy::PROPERTY_PEAK_ANGLE_DELTA = "Peak Angle Delta";
      const std::string& FlareActorProxy::PROPERTY_SOURCE_INTENSITY = "Source Intensity";
      const std::string& FlareActorProxy::PROPERTY_MODEL_TYPE = "Model Type";
      const std::string& FlareActorProxy::PROPERTY_LIGHT_NAME = "Light Name";
      const std::string& FlareActorProxy::PROPERTY_PARTICLE_FILE = "Particle File";

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

         FlareActor* actor = nullptr;
         GetActor(actor);

         AddProperty(new dtDAL::IntActorProperty(
            PROPERTY_CELL,
            PROPERTY_CELL,
            dtDAL::IntActorProperty::SetFuncType(actor, &FlareActor::SetCell),
            dtDAL::IntActorProperty::GetFuncType(actor, &FlareActor::GetCell),
            "Assumed to be the regional atmospheric position"));

         AddProperty(new dtDAL::IntActorProperty(
            PROPERTY_GUISE,
            PROPERTY_GUISE,
            dtDAL::IntActorProperty::SetFuncType(actor, &FlareActor::SetGuise),
            dtDAL::IntActorProperty::GetFuncType(actor, &FlareActor::GetGuise),
            "")); // ?

         AddProperty(new dtDAL::IntActorProperty(
            PROPERTY_NUMBER_OF_SOURCES,
            PROPERTY_NUMBER_OF_SOURCES,
            dtDAL::IntActorProperty::SetFuncType(actor, &FlareActor::SetNumberOfSources),
            dtDAL::IntActorProperty::GetFuncType(actor, &FlareActor::GetNumberOfSources),
            "Assumed to be the number of sub-components that produce light"));

         AddProperty(new dtDAL::IntActorProperty(
            PROPERTY_TIME_SINCE_DETONATION,
            PROPERTY_TIME_SINCE_DETONATION,
            dtDAL::IntActorProperty::SetFuncType(actor, &FlareActor::SetTimeSinceDetonation),
            dtDAL::IntActorProperty::GetFuncType(actor, &FlareActor::GetTimeSinceDetonation),
            "Length of time that the flare has been luminous, measured in milliseconds"));

         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_HEIGHT,
            PROPERTY_HEIGHT,
            dtDAL::FloatActorProperty::SetFuncType(actor, &FlareActor::SetHeight),
            dtDAL::FloatActorProperty::GetFuncType(actor, &FlareActor::GetHeight),
            "Assumed to be height above ground level measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_HEIGHT_DELTA,
            PROPERTY_HEIGHT_DELTA,
            dtDAL::FloatActorProperty::SetFuncType(actor, &FlareActor::SetHeightDelta),
            dtDAL::FloatActorProperty::GetFuncType(actor, &FlareActor::GetHeightDelta),
            "Assumed to be the change in height since the last actor update, measured in meters"));

         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_PEAK_ANGLE,
            PROPERTY_PEAK_ANGLE,
            dtDAL::FloatActorProperty::SetFuncType(actor, &FlareActor::SetPeakAngle),
            dtDAL::FloatActorProperty::GetFuncType(actor, &FlareActor::GetPeakAngle),
            "")); // ?

         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_PEAK_ANGLE_DELTA,
            PROPERTY_PEAK_ANGLE_DELTA,
            dtDAL::FloatActorProperty::SetFuncType(actor, &FlareActor::SetPeakAngleDelta),
            dtDAL::FloatActorProperty::GetFuncType(actor, &FlareActor::GetPeakAngleDelta),
            "")); // ?

         AddProperty(new dtDAL::FloatActorProperty(
            PROPERTY_SOURCE_INTENSITY,
            PROPERTY_SOURCE_INTENSITY,
            dtDAL::FloatActorProperty::SetFuncType(actor, &FlareActor::SetSourceIntensity),
            dtDAL::FloatActorProperty::GetFuncType(actor, &FlareActor::GetSourceIntensity),
            "")); // ?

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_MODEL_TYPE,
            PROPERTY_MODEL_TYPE,
            dtDAL::StringActorProperty::SetFuncType(actor, &FlareActor::SetModelType),
            dtDAL::StringActorProperty::GetFuncType(actor, &FlareActor::GetModelTypeString),
            "The type of flare being modeled by this actor"));

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_LIGHT_NAME,
            PROPERTY_LIGHT_NAME,
            dtDAL::StringActorProperty::SetFuncType(actor, &FlareActor::SetLightName),
            dtDAL::StringActorProperty::GetFuncType(actor, &FlareActor::GetLightName),
            "The name of the light prototype actor that describes the light effect used by this flare actor"));

         AddProperty(new dtDAL::StringActorProperty(
            PROPERTY_PARTICLE_FILE,
            PROPERTY_PARTICLE_FILE,
            dtDAL::StringActorProperty::SetFuncType(actor, &FlareActor::SetParticleFileName),
            dtDAL::StringActorProperty::GetFuncType(actor, &FlareActor::GetParticleFileName),
            "The type of flare being modeled by this actor"));
      }

      //////////////////////////////////////////////////////////////////////////
      void FlareActorProxy::CreateDrawable()
      {
         SetDrawable( *new FlareActor(*this) );
      }

   }
}
