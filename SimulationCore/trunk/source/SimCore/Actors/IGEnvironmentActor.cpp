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

#include <dtUtil/log.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/nodemask.h>
#include <dtUtil/nodecollector.h>

#include <dtCore/actorfactory.h>
#include <dtCore/cloudplane.h>
#include <dtCore/ephemeris.h>
#include <dtCore/shadermanager.h>
#include <dtCore/environment.h>
#include <dtCore/camera.h>
#include <dtCore/system.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/project.h>
#include <dtCore/propertymacros.h>

#include <dtRender/dtrenderactorregistry.h>


#include <dtABC/application.h>

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
      IGEnvironmentActor::IGEnvironmentActor(dtGame::GameActorProxy& owner)
         : BaseClass(owner)
         , mEnableCloudPlane(false)
         , mEnableLensFlare(false)
         , mInitSystemClock(false)
         , mCloudCoverage(0)
         , mWind(0.0f, 0.0f, 0.0f)
         , mCloudPlane()
         , mLensFlare()
         , mEphemeris()
      {
         
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
         if(mEphemeris.valid())
         {
             mEphemeris->SetLatitudeLongitude(latitude, longitude);
         }
         else
         {
             LOG_ERROR("No valid EphemerisScene found, cannot set lat/long");
         }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::OnEnteredWorld()
      {
         //we should have a valid ephemeris scene
         if (mEphemeris.valid())
         {
             if (!ContainsActor(*mEphemeris))
             {
                 AddActor(*mEphemeris);
                 IGEnvironmentActorProxy& igProxy = *dynamic_cast<IGEnvironmentActorProxy*>(GetOwner());
                 
                 //setup clouds
                 mCloudPlane = new dtCore::CloudPlane(igProxy.GetCloudHeight(), "Cloud Plane", "./Textures/CloudTexture9.dds");
                 
                 osg::Depth* depthState = new osg::Depth(osg::Depth::ALWAYS, 1.0f, 1.0f);
                 osg::StateSet* cloudPlaneSS = mCloudPlane->GetOSGNode()->getOrCreateStateSet();
                 cloudPlaneSS->setAttributeAndModes(depthState);
                 cloudPlaneSS->setRenderBinDetails(-3, "RenderBin");
                 
                 ChangeClouds(igProxy.GetCloudNum(), mWind[0], mWind[1]);

                 if (igProxy.GetInitialCloudState())
                 {
                     EnableCloudPlane(true);
                 }
                 
                 if (igProxy.GetInitialFogState())
                 {
                     SetFogEnabled(true);
                 }

                 if (GetInitializeSystemClock())
                 {
                     SetDateTime(igProxy.GetInitialDateTime());
                 }
                 
                 //set default visibility to go to the far plane
                 SetVisibility(SimCore::BaseGameEntryPoint::PLAYER_FAR_CLIP_PLANE - 100.0f);
             }
             else
             {
                 LOG_ERROR("Ephemeris already added to scene.");
             }
         }
         else
         {
             LOG_ERROR("Ephemeris Scene should be created and added by the proxy class.");
         }

         //add lens flare if necessary
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
         std::string texturePath = dtCore::Project::GetInstance().GetResourcePath(dtCore::ResourceDescriptor(cloudTextureResource));
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
         BaseClass::AddActor(actor);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::RemoveActor( dtCore::DeltaDrawable& actor )
      {
		  BaseClass::RemoveActor(actor);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::RemoveAllActors()
      {
		  BaseClass::RemoveAllActors();
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::ContainsActor( dtCore::DeltaDrawable& actor ) const
      {
		 return BaseClass::ContainsActor(actor);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::GetAllActors( std::vector<dtCore::DeltaDrawable*>& outActorList )
      {
		 BaseClass::GetAllActors(outActorList);
      }

      /////////////////////////////////////////////////////////////
      unsigned int IGEnvironmentActor::GetNumEnvironmentChildren() const
      {
		 return BaseClass::GetNumEnvironmentChildren();
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
      dtUtil::DateTime IGEnvironmentActor::GetDateTime() const
      {
         if(mEphemeris.valid())
         {
             return mEphemeris->GetDateTime();
         }
         else
         {
             LOG_ERROR("No valid EphemerisScene found, returning default DateTime.");
             return dtUtil::DateTime();
         }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetDateTime(const dtUtil::DateTime& dt)
      {
         dtUtil::Log::GetInstance("IGEnvironmentActor.cpp").LogMessage(dtUtil::Log::LOG_DEBUG, __FILE__, "Sim time set to:%s",
            dt.ToString(dtUtil::DateTime::TimeFormat::CALENDAR_DATE_AND_TIME_FORMAT).c_str());

         if (mEphemeris.valid())
         {
             mEphemeris->SetDateTime(dt);

             OnTimeChanged();
         }
         else
         {
             LOG_ERROR("Invalid EphemerisScene unable to SetDateTime().");
         }

         dtUtil::Log::GetInstance("IGEnvironmentActor.cpp").LogMessage(dtUtil::Log::LOG_DEBUG, __FILE__, "Sim time set to:%s",
            dt.ToString(dtUtil::DateTime::TimeFormat::CALENDAR_DATE_AND_TIME_FORMAT).c_str());
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetTimeFromSystem()
      {
          if (mEphemeris.valid())
          {
              mEphemeris->SetTimeFromSystem();

              OnTimeChanged();
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to SetTimeFromSystem().");
          }

      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::OnTimeChanged()
      {
        //this functionality is currently not supported in the upgrade to dtRender
        //temporarily commenting out as a reminder

         //when the time changes the fog color changes as well so we must update
         //the fog color on our "FogSphere"
         //osg::Vec3 fogColor;
         //mEnvironment->GetModFogColor(fogColor);
         //SetFogColor(fogColor);

         if(mLensFlare.valid())
         {
            //when the time changes we must update the position of the sun
            mLensFlare->Update(GetSunPosition());
         }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetDensity(float density)
      {
          if (mEphemeris.valid())
          {
              mEphemeris->SetFogDensity(density);
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Set Fog Density.");
          }
      }

      float IGEnvironmentActor::GetDensity()
      {
          if (mEphemeris.valid())
          {
              return mEphemeris->GetFogDensity();
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Get Fog Density.");
              return 0.0f;
          }
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::IsFogEnabled() const
      {
          if (mEphemeris.valid())
          {
              return mEphemeris->GetFogEnable();
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Get Fog Enabled.");
              return false;
          }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogEnabled(bool enable)
      {
          if (mEphemeris.valid())
          {
              mEphemeris->SetFogEnable(enable);
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Set Fog Enabled.");
          }
      }

      /////////////////////////////////////////////////////////////
      const osg::Vec3 IGEnvironmentActor::GetFogColor()
      {
          osg::Vec4 color;

          if (mEphemeris.valid())
          {
              color = mEphemeris->GetFogColor();
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Get Fog Color.");
          }

          return osg::Vec3(color.x(), color.y(), color.z());
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogColor(const osg::Vec3& color)
      {
          if (mEphemeris.valid())
          {
              mEphemeris->SetFogColor(osg::Vec4(color, 1.0f));
              mCloudPlane->SetColor(osg::Vec4(color, 1.0f));
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Set Fog Color.");
          }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogMode(dtRender::EphemerisScene::FogMode mode)
      {
         if (mEphemeris.valid())
         {
             mEphemeris->SetFogMode(mode);
         }
         else
         {
             LOG_ERROR("Invalid EphemerisScene unable to Set Fog Mode.");
         }
      }


      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogNear(float val )
      {
          if (mEphemeris.valid())
          {
              mEphemeris->SetFogNear(val);
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Set Fog Near.");
          }
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetVisibility(float distance)
      {
          if (mEphemeris.valid())
          {
              mEphemeris->SetVisibility(distance);
          }
          else
          {
              LOG_ERROR("Invalid EphemerisScene unable to Set Visibility.");
          }
      }

      /////////////////////////////////////////////////////////////
      float IGEnvironmentActor::GetVisibility()
      {
         float result = 0.0;

         if (mEphemeris.valid())
         {
             result = mEphemeris->GetVisibility();
         }
         else
         {
             LOG_ERROR("Invalid EphemerisScene unable to GetVisibility().");
         }

         return result;
      }

      ///////////////////////////////////////////////////////////////
      osg::Vec3d IGEnvironmentActor::GetSunPosition() const
      {
         osg::Vec3d position;

         if (mEphemeris.valid())
         {
             position = mEphemeris->GetSunPosition();
         }
         else
         {
             LOG_ERROR("Invalid EphemerisScene unable to GetSunPosition().");
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

         mLensFlare->GetOSGNode()->setNodeMask(b ? dtUtil::NodeMask::BACKGROUND : dtUtil::NodeMask::NOTHING);
      }

      void IGEnvironmentActor::InitLensFlare()
      {
         //this drawable creates a nice halo/glare effect from the sun
         mLensFlare = new LensFlareDrawable();
         mLensFlare->Init(*GetOwner()->GetGameManager());

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
	  void IGEnvironmentActor::SetEphemerisScene(dtRender::EphemerisScene& ephScene)
	  {
		  mEphemeris = &ephScene;
	  }

	  /////////////////////////////////////////////////////////////
	  dtRender::EphemerisScene* IGEnvironmentActor::GetEphemerisScene()
	  {
		  return mEphemeris.get();
	  }

	  /////////////////////////////////////////////////////////////
	  const dtRender::EphemerisScene* IGEnvironmentActor::GetEphemerisScene() const
	  {
		  return mEphemeris.get();
	  }

      /////////////////////////////////////////////////////////////
      // Actor Proxy Code
      /////////////////////////////////////////////////////////////
      IGEnvironmentActorProxy::IGEnvironmentActorProxy()
          : mInitialFogState(true)
          , mInitialCloudState(true)
          , mCloudNum(3)
          , mCloudHeight(1500.0f)
      {
         SetClassName("SimCore::Actors::IGEnvironmentActor");
      }

      /////////////////////////////////////////////////////////////
      IGEnvironmentActorProxy::~IGEnvironmentActorProxy()
      {
      }

	  /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::Init(const dtCore::ActorType& actorType)
	  {
		  BaseClass::Init(actorType);

		  dtCore::RefPtr<IGEnvironmentActor> envActor = GetDrawable<IGEnvironmentActor>();
		  if (envActor != NULL)
		  {
              //inherited from scene manager
			  envActor->CreateScene();

              dtCore::RefPtr<dtRender::EphemerisSceneActor> ephActor = CreateEphemeris();
              
              dtCore::RefPtr<dtRender::EphemerisScene> ephScene = ephActor->GetDrawable<dtRender::EphemerisScene>();             

              if (ephScene.valid())
              {
                  envActor->SetEphemerisScene(*ephScene);
              }
              else
              {
                  LOG_ERROR("Error creating EphemerisScene");
              }
		  }
		  else
		  {
			  LOG_ERROR("Error creating drawable.");
		  }

	  }

	  /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::CreateDrawable()
      {		 
		 dtCore::RefPtr<IGEnvironmentActor> envActor = new IGEnvironmentActor(*this);		 
		 SetDrawable(*envActor);
      }

	  /////////////////////////////////////////////////////////////
	  dtCore::RefPtr<dtRender::EphemerisSceneActor> IGEnvironmentActorProxy::CreateEphemeris()
	  {
        dtCore::RefPtr<dtCore::BaseActorObject> ap = dtCore::ActorFactory::GetInstance().CreateActor(*dtRender::RenderActorRegistry::EPHEMERIS_SCENE_ACTOR_TYPE);
        dtCore::RefPtr<dtRender::EphemerisSceneActor> ephActor = dynamic_cast<dtRender::EphemerisSceneActor*>(ap.get());

		if(ephActor == NULL)
        {
            LOG_ERROR("Error creating EphemerisScene.");
        }
		 
        return ephActor;
	  }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         const dtUtil::RefString GROUPNAME("IGEnvironmentActorProxy");
         typedef dtCore::PropertyRegHelper<IGEnvironmentActorProxy&, IGEnvironmentActorProxy> PropertyRegType;
         PropertyRegType propertyRegHelper(*this, this, GROUPNAME);


         DT_REGISTER_PROPERTY(
             InitialFogState,
             "Setting this enables fog at startup.",
             PropertyRegType, propertyRegHelper );

         DT_REGISTER_PROPERTY(
             InitialCloudState,
             "Setting this enables the cloud plane at startup.",
             PropertyRegType, propertyRegHelper);

         DT_REGISTER_PROPERTY(
             CloudNum,
             "An integer from 1-10 specifying the amount cloud coverage.",
             PropertyRegType, propertyRegHelper);

         DT_REGISTER_PROPERTY(
             CloudHeight,
             "The height in meters to place the cloud plane, default is 1500.",
             PropertyRegType, propertyRegHelper);


         IGEnvironmentActor* env = static_cast<IGEnvironmentActor*>(GetDrawable());


         AddProperty(new dtCore::BooleanActorProperty("Init System Clock", "Set the system clock to this time and date on startup",
            dtCore::BooleanActorProperty::SetFuncType(env, &IGEnvironmentActor::SetInitializeSystemClock),
            dtCore::BooleanActorProperty::GetFuncType(env, &IGEnvironmentActor::GetInitializeSystemClock),
            "Set the system clock to this time and date on startup"));

         AddProperty(new dtCore::StringActorProperty("Time and Date", "Time and Date",
             dtCore::StringActorProperty::SetFuncType(this, &IGEnvironmentActorProxy::SetInitialDateTimeAsString),
             dtCore::StringActorProperty::GetFuncType(this, &IGEnvironmentActorProxy::GetInitialDateTimeAsString),
            "Sets the time and date of the application. This string must be in the following UTC format: yyyy-mm-ddThh:mm:ss"));

         AddProperty(new dtCore::Vec3ActorProperty("Wind", "Wind",
            dtCore::Vec3ActorProperty::SetFuncType(env, &IGEnvironmentActor::SetWind),
            dtCore::Vec3ActorProperty::GetFuncType(env, &IGEnvironmentActor::GetWind),
            "Sets the force of wind, measured in meters per second"));
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::BuildInvokables()
      {
          BaseClass::BuildInvokables();
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         // For backward compatibility.
         GetDrawable<IGEnvironmentActor>()->OnEnteredWorld();
         // the ephemeris shows stars in the daytime and at night, they are sort of gray instead of white.
         GetGameManager()->GetApplication().GetCamera()->SetClearColor(osg::Vec4(0, 0, 0, 0));
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActorProxy::IsPlaceable() const
      {
         return false;
      }

      ///////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::SetInitialDateTimeAsString(const std::string& timeAndDate)
      {
          std::istringstream iss(timeAndDate);
          // The time is stored in the universal format of:
          // yyyy-mm-ddThh:min:ss-some number
          // So we need to use a delimeter to ensure that we don't choke on the separators
          bool result = SetTimeAndDateFromStringStream(iss);

          if (!result)
          {
              LOG_ERROR("The input time and date string: " + timeAndDate
                  + " was not formatted correctly. The correct format is: yyyy-mm-ddThh:mm:ss. Ignoring.");
          }
      }

      ///////////////////////////////////////////////////////////////
      bool IGEnvironmentActorProxy::SetTimeAndDateFromStringStream(std::istringstream& iss)
      {
          unsigned year, month, day, hour, min, sec;
          char delimeter;

          iss >> year;
          if (iss.fail()) { return false; }

          iss >> delimeter;
          if (iss.fail()) { return false; }

          iss >> month;
          if (iss.fail()) { return false; }

          iss >> delimeter;
          if (iss.fail()) { return false; }

          iss >> day;
          if (iss.fail()) { return false; }

          iss >> delimeter;
          if (iss.fail()) { return false; }

          iss >> hour;
          if (iss.fail()) { return false; }

          iss >> delimeter;
          if (iss.fail()) { return false; }

          iss >> min;
          if (iss.fail()) { return false; }

          iss >> delimeter;
          if (iss.fail()) { return false; }

          iss >> sec;
          if (iss.fail()) { return false; }

          dtUtil::DateTime dt;
          dt.SetTime(year, month, day, hour, min, sec);

          SetInitialDateTime(dt);
          return true;
      }

      /////////////////////////////////////////////////////////////
      const dtUtil::DateTime& IGEnvironmentActorProxy::GetInitialDateTime() const
      {
          return mStartTime;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::SetInitialDateTime(const dtUtil::DateTime& dt)
      {
          mStartTime = dt;
      }

      /////////////////////////////////////////////////////////////
      std::string IGEnvironmentActorProxy::GetInitialDateTimeAsString() const
      {
          return mStartTime.ToString();
      }
   }
}

