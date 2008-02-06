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

#include <prefix/SimCorePrefix-src.h>

#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Actors/IGEnvironmentActor.h>

#include <dtABC/application.h>
#include <dtCore/cloudplane.h>
#include <dtCore/ephemeris.h>
#include <dtCore/shadermanager.h>
#include <dtCore/environment.h>
#include <dtCore/camera.h>
#include <dtCore/nodecollector.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtUtil/log.h>
#include <dtDAL/project.h>

#include <osg/Drawable>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Fog>
#include <osg/Vec4>
#include <osg/Depth>
#include <osg/Fog>
#include <osg/StateSet>

#include <osgEphemeris/EphemerisData>

#include <ctime>

////////////////////////////////////////////////////
////////////////////////////////////////////////////

struct FogBoundingBoxCallback: public osg::Drawable::ComputeBoundingBoxCallback
{
   virtual osg::BoundingBox computeBound(const osg::Drawable& d) const 
   {
      return osg::BoundingBox(); 
   }
};


class BBVisitor : public osg::NodeVisitor
{
public:

   BBVisitor(): osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
   {

   }

   virtual void apply(osg::Geode& geode)
   {
      unsigned pNumDrawables = geode.getNumDrawables();
      for(unsigned i = 0; i < pNumDrawables; ++i)
      {
         osg::Drawable* draw = geode.getDrawable(i);
         draw->setComputeBoundingBoxCallback(new FogBoundingBoxCallback());
      }


   }
};

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////
      // Actor code
      ////////////////////////////////////////////////////////////////////////
      IGEnvironmentActor::IGEnvironmentActor(dtGame::GameActorProxy &proxy)
         : BaseClass(proxy)
         , mEnableCloudPlane(true)
         , mCurrTime()
         , mWind()
         , mCloudPlane(new dtCore::CloudPlane(1500.0f, "Cloud Plane","./Textures/CloudTexture9.dds"))
         , mEnvironment( new dtCore::Environment("EphemerisEnvironment") )
         , mEphemerisModel(new osgEphemeris::EphemerisModel())
         , mFogSphere(0)
         , mFogSphereEyePointTransform(new MoveWithEyePointTransform())
         , mFog ( new osg::Fog() )
         , mCloudCoverage(0)
      {
         EnableCloudPlane(true);
         AddChild(mEnvironment.get());
         
         mEphemerisModel->setSkyDomeRadius( 9000.0f );
         mEphemerisModel->setSunLightNum(0);
         mEphemerisModel->setMoveWithEyePoint(true);

         //FogSphere SetUp
         mFogSphere = new osgEphemeris::Sphere( 8500.0f,
            osgEphemeris::Sphere::TessLow,
            osgEphemeris::Sphere::OuterOrientation,
            osgEphemeris::Sphere::BothHemispheres,
            false
            );
            
         // Change render order and depth writing.
         osg::StateSet* states = mEphemerisModel->getOrCreateStateSet();
         osg::Depth* depthState = new osg::Depth(osg::Depth::ALWAYS, 1.0f , 1.0f );
         states->setAttributeAndModes(depthState);

         osg::StateSet* cloudPlaneSS = mCloudPlane->GetOSGNode()->getOrCreateStateSet();
         cloudPlaneSS->setAttributeAndModes(depthState);
         cloudPlaneSS->setRenderBinDetails( -1, "RenderBin" );

         states->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
         states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
         states->setRenderBinDetails( SimCore::Components::RenderingSupportComponent::RENDER_BIN_ENVIRONMENT, "RenderBin" );
         mEphemerisModel->setStateSet(states);


         //Set up the Fog Sphere so that it can be rendered
         osg::StateSet* fogSphereStates = mFogSphere->getOrCreateStateSet();
         osg::Depth* depthFogState = new osg::Depth(osg::Depth::ALWAYS, 1.0f , 1.0f );
         fogSphereStates->setAttributeAndModes(depthFogState);
         fogSphereStates->setMode(GL_FOG, osg::StateAttribute::ON);
         fogSphereStates->setMode(GL_LIGHTING, osg::StateAttribute::ON);
         fogSphereStates->setMode(GL_BLEND, osg::StateAttribute::ON);
         fogSphereStates->setRenderBinDetails( -1, "RenderBin" );

         mFogSphere->setStateSet(fogSphereStates);         
         //set default fog distance to a clear day
         mFog->setEnd( 600000 );
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
         mEphemerisModel->setLatitudeLongitude(latitude, longitude);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         //GetGameActorProxy().GetGameManager()->GetScene().UseSceneLight(true);
         mEnvironment->GetOSGNode()->asGroup()->addChild(mEphemerisModel.get());

         //here we setup the FogSphere's state set and add it to the scene
         mEnvironment->GetOSGNode()->asGroup()->addChild(mFogSphereEyePointTransform.get());
         mFogSphereEyePointTransform->addChild(mFogSphere.get());
                  
         mEnvironment->Update(999.99f); //passing a large number will force an update

         osg::Vec3 fogColor;         
         mEnvironment->GetModFogColor(fogColor);
         SetFogColor(fogColor);


         //we give all the drawables on the fog sphere a large bounding box to ensure it is always rendered
         size_t sizeOfNH = mFogSphere->getNorthernHemisphere()->getNumDrawables();
         for(size_t i = 0; i < sizeOfNH; ++i)
         {
            mFogSphere->getNorthernHemisphere()->getDrawable(i)->setComputeBoundingBoxCallback(new FogBoundingBoxCallback());
         }

         //we give all the drawables on the fog sphere a large bounding box to ensure it is always rendered
         size_t sizeOfSH = mFogSphere->getSouthernHemisphere()->getNumDrawables();
         for(size_t i = 0; i < sizeOfSH; ++i)
         {
            mFogSphere->getSouthernHemisphere()->getDrawable(i)->setComputeBoundingBoxCallback(new FogBoundingBoxCallback());
         }
          
         mFogSphereEyePointTransform->setCenter(mEphemerisModel->getSkyDomeCenter());

         //this little hack will create a large bounding volume for the ephemeris to ensure it doesn't 
         //get culled out
         BBVisitor bbv;
         mEphemerisModel->traverse(bbv);


         dtCore::ShaderManager::GetInstance().UnassignShaderFromNode(*mFogSphere.get());

         //First get the shader group assigned to this actor.
         const dtCore::ShaderGroup *shaderGroup =
         dtCore::ShaderManager::GetInstance().FindShaderGroupPrototype("EphemerisFogGroup");

         if (shaderGroup == NULL)
         {
            LOG_INFO("Could not find shader group EphemerisFogGroup");
            return;
         }

         const dtCore::ShaderProgram *defaultShader = shaderGroup->GetDefaultShader();

         try
         {
            if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *mFogSphere.get());
            }
            else
            {
               LOG_WARNING("Could not find a default shader in shader group: EphemerisFogGroup");
               return;
            }
         }
         catch (const dtUtil::Exception &e)
         {
            LOG_WARNING("Caught Exception while assigning shader: " + e.ToString());
            return;
         }

         //setup the weather component
         /*SimCore::Components::WeatherComponent* weatherComp = 
            static_cast<SimCore::Components::WeatherComponent*>
            (GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME));
         if(weatherComp != NULL)
         {            
            weatherComp->SetUseEphemeris(true);
         }*/
         // Seems wierd, but we have to set the clear color to black on the camera or 
         // the ephemeris shows stars in the daytime and at night, they are sort of gray instead of white.
         GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->SetClearColor(osg::Vec4(0, 0, 0, 0));
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
      void IGEnvironmentActor::SetTimeAndDate( const int iyear, const int imonth, const int iday, 
         const int ihour, const int iminute, const int isecond )
      {
         //printf("Hour = %d\n", hour);
         //printf("Minute = %d\n", minute);

         // Why do primitive parameters have const on them? It is flat out useless.
         int year = iyear;
         int month = imonth;
         int day = iday;
         int hour = ihour;
         int minute = iminute;
         int second = isecond;

         if ((year <= -1) || (month <= -1) || (day <= -1) ||
            (hour <= -1) || (minute <= -1) || (second <= -1) || mCurrTime <= 0 )
         {
            mCurrTime = time(NULL);//dtCore::GetGMT(year-1900, month-1, day, hour, minute, second);
            tm* expandedTime = gmtime( &mCurrTime );
            if( expandedTime != NULL )
            {
               year = expandedTime->tm_year+1900;
               month = expandedTime->tm_mon+1;
               day = expandedTime->tm_mday;
               hour = expandedTime->tm_hour;
               minute = expandedTime->tm_min;
               second = expandedTime->tm_sec;
            }
         }

         // Time being set by GetGMT might have failed mCurrTime is < 0.
         if( mCurrTime < 0 ) { mCurrTime = 0; }

         dtUtil::Log::GetInstance().LogMessage(dtUtil::Log::LOG_DEBUG, __FILE__, "Sim time set to:%s",
            asctime( localtime(&mCurrTime) ) );
         
         mEphemerisModel->setAutoDateTime( false );

         osgEphemeris::EphemerisData *data = mEphemerisModel->getEphemerisData();

         //set the environment time here to?
         mEnvironment->SetDateTime(year, month, day, hour, minute, second);                  
         mEnvironment->Update(999.99f); //passing a large number will force an update

         osg::Vec3 fogColor;         
         mEnvironment->GetModFogColor(fogColor);
         SetFogColor(fogColor);

         osg::StateSet* fogSphereStates = mFogSphere->getOrCreateStateSet();
         fogSphereStates->setAttributeAndModes(mFog.get());

         mFogSphere->setStateSet(fogSphereStates);


         data->dateTime.setYear( year ); // DateTime uses _actual_ year (not since 1900)
         data->dateTime.setMonth( month );    // DateTime numbers months from 1 to 12, not 0 to 11
         data->dateTime.setDayOfMonth( day ); // DateTime numbers days from 1 to 31, not 0 to 30
         data->dateTime.setHour( hour );
         data->dateTime.setMinute( minute );
         data->dateTime.setSecond( second );
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::GetTimeAndDate( int& year, int& month, int& day, 
         int& hour, int& minute, int& second ) const
      {
         if( mCurrTime < 0 )
         {
            year = month = day = hour = minute = second = 0;
            return;
         }
         struct tm *tmp = localtime(&mCurrTime);
         year     = tmp->tm_year+1900;
         month    = tmp->tm_mon+1;
         day      = tmp->tm_mday;
         hour     = tmp->tm_hour;
         minute   = tmp->tm_min;
         second   = tmp->tm_sec;
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::EnableFog(bool enable)
      {
         mEnvironment->SetFogEnable(enable);
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


      void IGEnvironmentActor::SetEphemerisFog(bool fog_toggle ) 
      {
         if(fog_toggle == true)
         {
            mFogSphere->setNodeMask(0xFFFFFFFF);
         }
         else
         {   
            mFogSphere->setNodeMask(0);
         }
      }

      /////////////////////////////////////////////////////////////
      osgEphemeris::EphemerisModel* IGEnvironmentActor::GetEphemerisModel()
      {
         return mEphemerisModel.get();
      }


      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogColor( const osg::Vec3& color )
      {
         mFog->setColor(osg::Vec4 (color, 1.0f) );

         osg::StateSet* fogSphereStates = mFogSphere->getOrCreateStateSet();
         fogSphereStates->setAttributeAndModes(mFog.get());

         mFogSphere->setStateSet(fogSphereStates);
         mCloudPlane->SetColor( osg::Vec4(color, 1.0f) );
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogMode( dtCore::Environment::FogMode mode )
      {
         mEnvironment->SetFogMode(mode);
      }


      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetFogNear( float val )
      {
         mEnvironment->SetFogNear(val);
      }

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetVisibility( float distance )
      {
         mFog->setEnd( distance );
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


      /////////////////////////////////////////////////////////////
      void IGEnvironmentActor::SetTimeAndDateString( const std::string &timeAndDate )
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

      /////////////////////////////////////////////////////////////
      std::string IGEnvironmentActor::GetTimeAndDateString() const
      {
         std::ostringstream oss;
         int year, month, day, hour, min, sec;
         GetTimeAndDate(year, month, day, hour, min, sec);
         oss << year << '-';
         if(month < 10)
            oss << '0' << month << '-';
         else
            oss << month << '-';

         if(day < 10)
            oss << '0' << day << 'T';
         else
            oss << day << 'T';

         if(hour < 10)
            oss << '0' << hour << ':';
         else
            oss << hour << ':';

         if(min < 10)
            oss << '0' << min << ':';
         else
            oss << min << ':';

         if(sec < 10)
            oss << '0' << sec;
         else
            oss << sec;

         return oss.str();
      }

      /////////////////////////////////////////////////////////////
      std::string IGEnvironmentActor::GetCurrentTimeAndDateString() const
      {
         time_t currentTime;
         time(&currentTime);
         return dtUtil::TimeAsUTC(currentTime);
      }

      /////////////////////////////////////////////////////////////
      bool IGEnvironmentActor::SetTimeAndDate( std::istringstream& iss )
      {
         int year, month, day, hour, min, sec;
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

         IGEnvironmentActor::SetTimeAndDate(year, month, day, hour, min, sec);
         return true;
      }


      bool IGEnvironmentActor::MoveWithEyePointTransform::computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
      {
         if( _enabled )
         {
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cv)
            {
               osg::Vec3 eyePointLocal = cv->getEyeLocal();
               matrix.preMult(osg::Matrix::translate(
                  osg::Vec3( eyePointLocal.x(),eyePointLocal.y(),0.0f) - _center));
            }
         }
         return true;
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

      /////////////////////////////////////////////////////////////
      void IGEnvironmentActorProxy::BuildPropertyMap()
      {
         dtGame::GameActorProxy::BuildPropertyMap();

         IGEnvironmentActor *env = static_cast<IGEnvironmentActor*>(GetActor());

         AddProperty(new dtDAL::BooleanActorProperty("Enable Fog", "Enable Fog",
            dtDAL::MakeFunctor(*env, &IGEnvironmentActor::EnableFog),
            dtDAL::MakeFunctorRet(*env, &IGEnvironmentActor::IsFogEnabled),
            "Toggles fog on and off"));

         AddProperty(new dtDAL::StringActorProperty("Time and Date", "Time and Date",
            dtDAL::MakeFunctor(*env, &IGEnvironmentActor::SetTimeAndDateString),
            dtDAL::MakeFunctorRet(*env, &IGEnvironmentActor::GetTimeAndDateString),
            "Sets the time and date of the application. This string must be in the following UTC format: yyyy-mm-ddThh:mm:ss"));

         AddProperty(new dtDAL::Vec3ActorProperty("Wind", "Wind",
            dtDAL::MakeFunctor(*env, &IGEnvironmentActor::SetWind),
            dtDAL::MakeFunctorRet(*env, &IGEnvironmentActor::GetWind),
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
   }
}

