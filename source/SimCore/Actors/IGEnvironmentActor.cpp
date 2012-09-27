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

#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/IGEnvironmentActor.h>
#include <SimCore/Actors/DayTimeActor.h>
#include <SimCore/Actors/LensFlareDrawable.h>
#include <SimCore/BaseGameEntryPoint.h>

#include <dtABC/application.h>
#include <dtCore/cloudplane.h>
#include <dtCore/ephemeris.h>
#include <dtCore/shadermanager.h>
#include <dtCore/environment.h>
#include <dtCore/camera.h>
#include <dtCore/system.h>
#include <dtUtil/nodecollector.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/log.h>
#include <dtUtil/stringutils.h>
#include <dtDAL/project.h>

#include <osg/Drawable>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Fog>
#include <osg/Depth>
#include <osg/Vec4>
#include <ctime>

////////////////////////////////////////////////////
////////////////////////////////////////////////////

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////
      // Actor code
      ////////////////////////////////////////////////////////////////////////
      IGEnvironmentActor::IGEnvironmentActor(dtGame::GameActorProxy& proxy)
         : BaseClass(proxy)
         , mEnableCloudPlane(true)
         , mEnableLensFlare(false)
         , mInitSystemClock(false)
         , mCurrTime()
         , mWind()
         , mCloudPlane(new dtCore::CloudPlane(1500.0f, "Cloud Plane","./Textures/CloudTexture9.dds"))
         , mEnvironment( new dtCore::Environment("EphemerisEnvironment") )
         , mFog ( new osg::Fog() )
         , mCloudCoverage(0)
      {
         EnableCloudPlane(true);
         AddChild(mEnvironment.get());

         ChangeClouds(3, 2.0f, 2.0f);

         osg::Depth* depthState = new osg::Depth(osg::Depth::ALWAYS, 1.0f , 1.0f );
         osg::StateSet* cloudPlaneSS = mCloudPlane->GetOSGNode()->getOrCreateStateSet();
         cloudPlaneSS->setAttributeAndModes(depthState);
         cloudPlaneSS->setRenderBinDetails( -3, "RenderBin" );

         SetFogEnabled(true);
         //set default fog distance to a clear day
         SetVisibility(SimCore::BaseGameEntryPoint::PLAYER_FAR_CLIP_PLANE - 100.0f);
      }

      /////////////////////////////////////////////////////////////
      IGEnvironmentActor::~IGEnvironmentActor()
      {
         RemoveAllActors();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetSkyDomesCenter(const osg::Vec3& position)
      {
         //mEphemerisModel->setSkyDomeCenter(osg::Vec3(position[0], position[1],0) );
         //mEphemerisModel->update();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetLatitudeAndLongitude( float latitude, float longitude )
      {
         //this updates the fog color
         if(mEnvironment.valid())
         {
            mEnvironment->SetRefLatLong(osg::Vec2(latitude, longitude));
         }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         mEnvironment->Update(999.99f); //passing a large number will force an update

         osg::Vec3 fogColor;
         mEnvironment->GetModFogColor(fogColor);
         SetFogColor(fogColor);         // Seems wierd, but we have to set the clear color to black on the camera or
         // the ephemeris shows stars in the daytime and at night, they are sort of gray instead of white.
         GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->SetClearColor(osg::Vec4(0, 0, 0, 0));

         if(mEnableLensFlare && !mLensFlare.valid())
         {
            InitLensFlare();
         }
      }


      void IGEnvironmentActor::ChangeClouds(int cloudNumber, float windX, float windY)
      {
         //converts wind speed to m/s
         windX = (windX * 0.0000036f);
         windY = (windY * 0.0000036f);

         mCloudPlane->SetWind(windX, windY);

         bool loaded;
         // Look up the cloud texture from the map so we have the right path. Using ./projectassets doesn't work in all cases depending on path
         std::string cloudTextureResource("Textures:CloudTexture" + dtUtil::ToString(cloudNumber) + ".dds");
         std::string texturePath = dtDAL::Project::GetInstance().GetResourcePath(dtDAL::ResourceDescriptor(cloudTextureResource));
         if (texturePath.empty())
         {
            LOG_ERROR("The Cloud File Texture [" + texturePath + "] Could Not be Found");
         }
         else
         {
            loaded = mCloudPlane->LoadTexture(texturePath);
            if(loaded)
               mCloudCoverage = cloudNumber;
            else
               LOG_ERROR("Cloud File Texture was found but had an error while loading [" + texturePath + "].");
         }
      }

      int IGEnvironmentActor::GetCloudCoverage() const
      {
         return mCloudCoverage;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::AddActor( dtCore::DeltaDrawable& actor )
      {
         mEnvironment->AddChild(&actor);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::RemoveActor( dtCore::DeltaDrawable& actor )
      {
         mEnvironment->RemoveChild(&actor);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::RemoveAllActors()
      {
         while( mEnvironment->GetNumChildren() > 0)
         {
            mEnvironment->RemoveChild(mEnvironment->GetChild(0));
         }
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::ContainsActor( dtCore::DeltaDrawable& actor ) const
      {
         return (mEnvironment->GetChildIndex(&actor) < mEnvironment->GetNumChildren());
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::GetAllActors( std::vector<dtCore::DeltaDrawable*>& outActorList )
      {
         outActorList.clear();

         for(unsigned int i = 0; i < mEnvironment->GetNumChildren(); i++)
            outActorList.push_back(mEnvironment->GetChild(i));
      }

      /////////////////////////////////////////////////////////////
      unsigned int IGEnvironmentActor::GetNumEnvironmentChildren() const
      {
         return mEnvironment->GetNumChildren();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetWind( const osg::Vec3& wind )
      {
         mWind = wind;
      }

      /////////////////////////////////////////////////////////////
      const osg::Vec3& IGEnvironmentActor::GetWind() const
      {
         return mWind;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::EnableCloudPlane( bool enable )
      {
         mEnableCloudPlane = enable;

         if( mEnableCloudPlane )
            AddChild( mCloudPlane.get() );
         else
            RemoveChild( mCloudPlane.get() );
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::IsCloudPlaneEnabled() const
      {
         return mEnableCloudPlane;
      }

      /////////////////////////////////////////////////////////////
      const dtUtil::DateTime& IGEnvironmentActor::GetDateTime() const
      {
         return mEnvironment->GetDateTime();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetDateTime(const dtUtil::DateTime& dt)
      {
         dtUtil::Log::GetInstance("IGEnvironmentActor.cpp").LogMessage(dtUtil::Log::LOG_DEBUG, __FILE__, "Sim time set to:%s",
            dt.ToString(dtUtil::DateTime::TimeFormat::CALENDAR_DATE_AND_TIME_FORMAT).c_str());

         mEnvironment->SetDateTime(dt);

         OnTimeChanged();

         dtUtil::Log::GetInstance("IGEnvironmentActor.cpp").LogMessage(dtUtil::Log::LOG_DEBUG, __FILE__, "Sim time set to:%s",
            dt.ToString(dtUtil::DateTime::TimeFormat::CALENDAR_DATE_AND_TIME_FORMAT).c_str());
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetTimeFromSystem()
      {
         dtCore::Timer_t t = dtCore::System::GetInstance().GetSimulationClockTime();
         dtUtil::DateTime dt = GetDateTime();
         dt.SetTime(time_t(t / dtCore::Timer_t(1000000)));
         mEnvironment->SetDateTime(dt);

         OnTimeChanged();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::OnTimeChanged()
      {
         //when the time changes the fog color changes as well so we must update
         //the fog color on our "FogSphere"
         osg::Vec3 fogColor;
         mEnvironment->GetModFogColor(fogColor);
         SetFogColor(fogColor);

         if(mLensFlare.valid())
         {
            //when the time changes we must update the position of the sun
            mLensFlare->Update(GetSunPosition());
         }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetDensity(float density)
      {
         mEnvironment->SetFogDensity(density);
      }

      float IGEnvironmentActor::GetDensity()
      {
         return mEnvironment->GetFogDensity();
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::IsFogEnabled() const
      {
         return mEnvironment->GetFogEnable();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogEnabled(bool enable)
      {
         mEnvironment->SetFogEnable(enable);
      }

      /////////////////////////////////////////////////////////////
      const osg::Vec3 IGEnvironmentActor::GetFogColor()
      {
         osg::Vec3 color;
         mEnvironment->GetFogColor(color);
         return color;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogColor(const osg::Vec3& color)
      {
         mFog->setColor(osg::Vec4 (color, 1.0f) );

         mCloudPlane->SetColor( osg::Vec4(color, 1.0f) );
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogMode(dtCore::Environment::FogMode mode)
      {
         mEnvironment->SetFogMode(mode);
      }


      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogNear(float val )
      {
         mEnvironment->SetFogNear(val);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetVisibility(float distance)
      {
         mFog->setEnd(distance);
         mEnvironment->SetVisibility(distance);
      }

      /////////////////////////////////////////////////////////////
      float IGEnvironmentActor::GetVisibility()
      {
         float result = 0.0;

         if (mEnvironment.valid())
            result = mEnvironment->GetVisibility();

         return result;
      }


      ///////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetTimeAndDateString(const std::string& timeAndDate)
      {
         if(timeAndDate.empty())
            return;

         std::istringstream iss( timeAndDate );
         // The time is stored in the universal format of:
         // yyyy-mm-ddThh:min:ss-some number
         // So we need to use a delimeter to ensure that we don't choke on the seperators
         if( ! SetTimeAndDate( iss ) )
         {
            LOG_ERROR( "The input time and date string: " + timeAndDate
               + " was not formatted correctly. The correct format is: yyyy-mm-ddThh:mm:ss. Ignoring.");
         }
      }

      ///////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::SetTimeAndDate(std::istringstream& iss)
      {
         unsigned year, month, day, hour, min, sec;
         char delimeter;

         iss >> year;
         if( iss.fail() ) { return false; }

         iss >> delimeter;
         if( iss.fail() ) { return false; }

         iss >> month;
         if( iss.fail() ) { return false; }

         iss >> delimeter;
         if( iss.fail() ) { return false; }

         iss >> day;
         if( iss.fail() ) { return false; }

         iss >> delimeter;
         if( iss.fail() ) { return false; }

         iss >> hour;
         if( iss.fail() ) { return false; }

         iss >> delimeter;
         if( iss.fail() ) { return false; }

         iss >> min;
         if( iss.fail() ) { return false; }

         iss >> delimeter;
         if( iss.fail() ) { return false; }

         iss >> sec;
         if( iss.fail() ) { return false; }

         dtUtil::DateTime dt;
         dt.SetTime(year, month, day, hour, min, sec);

         SetDateTime(dt);
         return true;
      }

      std::string IGEnvironmentActor::GetTimeAndDateString() const
      {
         dtUtil::DateTime dt = GetDateTime();
         return dt.ToString();
      }

      /////////////////////////////////////////////////////////////
      dtCore::Environment& IGEnvironmentActor::GetCoreEnvironment()
      {
         return *mEnvironment;
      }

      /////////////////////////////////////////////////////////////
      osg::Fog& IGEnvironmentActor::GetFog()
      {
         return *mFog;
      }

      osg::Vec3d IGEnvironmentActor::GetSunPosition() const
      {
         osg::Vec3d position;

         if(mEnvironment.valid())
         {
            float az, el;
            mEnvironment->GetSunAzEl(az, el);

            //this is the same calculation done in osgEphemeris for azimuth and elevation to position
            position = osg::Vec3d(
               sin(osg::DegreesToRadians(az))*cos(osg::DegreesToRadians(el)),
               cos(osg::DegreesToRadians(az))*cos(osg::DegreesToRadians(el)),
               sin(osg::DegreesToRadians(el)));

            position.normalize();
            // Mean distance to sun  1.496x10^8 km
            // Use 1/2 distance.  In reality, light "goes around corners".  Using half the distance
            // allows us to mimic the light surrounding the moon sphere a bit more.
            position *= (1.496 * 100000000000.0) * 0.5;
         }

         return position;
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::GetEnableLensFlare() const
      {
         return mEnableLensFlare;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetEnableLensFlare( bool b )
      {
         if(!mLensFlare.valid())
         {
            InitLensFlare();
         }

         mLensFlare->GetOSGNode()->setNodeMask(b ? 0xFFFFFFFF : 0x0);
      }

      void IGEnvironmentActor::InitLensFlare()
      {
         //this drawable creates a nice halo/glare effect from the sun
         mLensFlare = new LensFlareDrawable();
         mLensFlare->Init(*GetGameActorProxy().GetGameManager());

         AddChild(mLensFlare.get());
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::GetInitializeSystemClock() const
      {
         return mInitSystemClock;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetInitializeSystemClock(bool enable)
      {
         mInitSystemClock = enable;
      }

      /////////////////////////////////////////////////////////////
      LensFlareDrawable* IGEnvironmentActor::GetLensFlareDrawable()
      {
         return mLensFlare.get();
      }

      /////////////////////////////////////////////////////////////
      const LensFlareDrawable* IGEnvironmentActor::GetLensFlareDrawable() const
      {
         return mLensFlare.get();
      }

      /////////////////////////////////////////////////////////////
      // Actor Proxy Code
      /////////////////////////////////////////////////////////////
      IGEnvironmentActorProxy::IGEnvironmentActorProxy()
      {
         SetClassName("SimCore::Actors::IGEnvironmentActor");
      }

      /////////////////////////////////////////////////////////////
      IGEnvironmentActorProxy::~IGEnvironmentActorProxy()
      {
      }

      /// Creates the actor
      void IGEnvironmentActorProxy::CreateActor()
      {
         SetActor(*new IGEnvironmentActor(*this));
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         IGEnvironmentActor* env = static_cast<IGEnvironmentActor*>(GetActor());

         AddProperty(new dtDAL::BooleanActorProperty("Enable Fog", "Enable Fog",
            dtDAL::BooleanActorProperty::SetFuncType(env, &IGEnvironmentActor::SetFogEnabled),
            dtDAL::BooleanActorProperty::GetFuncType(env, &IGEnvironmentActor::IsFogEnabled),
            "Toggles fog on and off"));

         AddProperty(new dtDAL::BooleanActorProperty("Init System Clock", "Set the system clock to this time and date on startup",
            dtDAL::BooleanActorProperty::SetFuncType(env, &IGEnvironmentActor::SetInitializeSystemClock),
            dtDAL::BooleanActorProperty::GetFuncType(env, &IGEnvironmentActor::GetInitializeSystemClock),
            "Set the system clock to this time and date on startup"));

         AddProperty(new dtDAL::StringActorProperty("Time and Date", "Time and Date",
            dtDAL::StringActorProperty::SetFuncType(env, &IGEnvironmentActor::SetTimeAndDateString),
            dtDAL::StringActorProperty::GetFuncType(env, &IGEnvironmentActor::GetTimeAndDateString),
            "Sets the time and date of the application. This string must be in the following UTC format: yyyy-mm-ddThh:mm:ss"));

         AddProperty(new dtDAL::Vec3ActorProperty("Wind", "Wind",
            dtDAL::Vec3ActorProperty::SetFuncType(env, &IGEnvironmentActor::SetWind),
            dtDAL::Vec3ActorProperty::GetFuncType(env, &IGEnvironmentActor::GetWind),
            "Sets the force of wind, measured in meters per second"));
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::BuildInvokables()
      {
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::OnEnteredWorld()
      {
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActorProxy::IsPlaceable() const
      {
         return false;
      }

   }
}

