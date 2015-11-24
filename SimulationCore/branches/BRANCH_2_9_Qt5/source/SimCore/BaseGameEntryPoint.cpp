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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix.h>

#include <SimCore/BaseGameEntryPoint.h>
#include <SimCore/Components/BaseInputComponent.h>
#include <SimCore/Components/ViewerNetworkPublishingComponent.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/MultiSurfaceClamper.h>
#include <SimCore/Components/TimedDeleterComponent.h>
#include <SimCore/Components/ParticleManagerComponent.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>

#include <SimCore/Components/ViewerMaterialComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <SimCore/Actors/DetonationActor.h>

#include <SimCore/Tools/Binoculars.h>

#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/stringutils.h>

#include <dtCore/camera.h>
#include <dtCore/environment.h>
#include <dtCore/isector.h>
#include <dtCore/scene.h>

#include <dtAudio/audiomanager.h>

#include <dtCore/baseactorobject.h>
#include <dtCore/actorproperty.h>
#include <dtCore/project.h>
#include <dtCore/resourcedescriptor.h>
#include <dtCore/map.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/projectconfig.h>

#include <dtGame/gamemanager.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>

#include <dtUtil/datapathutils.h>

#include <dtAnim/animationcomponent.h>
#include <dtAnim/animnodebuilder.h>
#include <dtAnim/modeldatabase.h>
#include <dtABC/application.h>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
//#include <osgUtil/RenderBin>
#include <osgViewer/View>

using dtCore::RefPtr;
using dtCore::ObserverPtr;


namespace SimCore
{
   const float BaseGameEntryPoint::PLAYER_NEAR_CLIP_PLANE = 0.1f;
   const float BaseGameEntryPoint::PLAYER_FAR_CLIP_PLANE  = 10000.0f;

   const std::string BaseGameEntryPoint::LIBRARY_NAME("SimCore");
   const std::string BaseGameEntryPoint::PROJECT_CONTEXT_DIR("ProjectAssets");
   const std::string BaseGameEntryPoint::PROJECT_CONFIG_FILE("ProjectConfig.dtproj");

   const std::string BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE("DeveloperMode");
   const std::string BaseGameEntryPoint::CONFIG_PROP_ASPECT_RATIO("AspectRatio");
   const std::string BaseGameEntryPoint::CONFIG_PROP_MUNITION_MAP("MunitionMap");
   const std::string BaseGameEntryPoint::CONFIG_PROP_MUNITION_CONFIG_FILE("MunitionsConfigFile");
   const std::string BaseGameEntryPoint::CONFIG_PROP_HIGH_RES_GROUND_CLAMP_RANGE("HighResGroundClampingRange");

   //////////////////////////////////////////////////////////////////////////
   BaseGameEntryPoint::BaseGameEntryPoint()
   : mAspectRatio(0.0f)
   , mLingeringShotEffectSecs(300.0f)
   , mMissingRequiredCommandLineOption(false)
   , mStartedAudio(false)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   BaseGameEntryPoint::~BaseGameEntryPoint()
   {
      // This has to be in the destructor because
      // OnShutdown happens before the GM shutdown, and sounds can get freed after that.
      if (mStartedAudio)
      {
         dtAudio::AudioManager::Destroy();
         mStartedAudio = false;
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::Initialize(dtABC::BaseABC& app, int argc, char **argv)
   {
      osg::ArgumentParser* parser = GetOrCreateArgParser(argc, argv);

      parser->getApplicationUsage()->addCommandLineOption("--UI", "Specify this to disable old functionality in favor of the UI");
      parser->getApplicationUsage()->addCommandLineOption("--aspectRatio", "The aspect ratio to use for the camera [1.33 or 1.6]");
      parser->getApplicationUsage()->addCommandLineOption("--lingeringShotSecs", "The number of seconds for a shot to linger after impact. The default value is 300 (5 minutes)");

      // Only change the value if a command line option is received.
      int tempBool = 0;
      if(parser->read("--UI", tempBool))
      {
         // note: bool is intentionally reversed
         SetMapIsRequired(tempBool == 1 ? false : true);
      }

      BaseClass::Initialize(app, argc, argv);
      mMissingRequiredCommandLineOption = false;

      parser->read("--aspectRatio", mAspectRatio);

      parser->read("--lingeringShotSecs", mLingeringShotEffectSecs);

      //if (!parser->read("--statisticsInterval", mStatisticsInterval))
      //{
      //   mStatisticsInterval = 0;
      //}

      mStartedAudio = false;

      try
      {
         if (!dtAudio::AudioManager::IsInitialized())
         {
            dtAudio::AudioManager::Instantiate();
            mStartedAudio = true;
         }
      }
      catch(const dtUtil::Exception& e)
      {
         dtAudio::AudioManager::Destroy();
         throw e;
      }
      catch(const std::exception& e)
      {
         std::ostringstream ss;
         ss << "std::exception caught of type: \"" << typeid(e).name() << ". Its message is: "
            << e.what() << "\" Aborting.";
         LOG_ERROR(ss.str());
         dtAudio::AudioManager::Destroy();
         throw dtGame::GameApplicationConfigException(
            ss.str(), __FILE__, __LINE__);
      }
   }

   void BaseGameEntryPoint::SetupProjectContext()
   {
      dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();

      if (GetProjectPath().empty())
      {
         if (fileUtils.FileExists(PROJECT_CONFIG_FILE))
         {
            SetProjectPath(PROJECT_CONFIG_FILE);
         }
         else if (fileUtils.DirExists(PROJECT_CONTEXT_DIR))
         {
            SetProjectPath(PROJECT_CONTEXT_DIR);
         }
         else if (fileUtils.DirExists(std::string("../") + PROJECT_CONTEXT_DIR))
         {
            SetProjectPath(std::string("../") + PROJECT_CONTEXT_DIR);
         }
      }

      BaseClass::SetupProjectContext();
   }

   //////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::AssignAspectRatio(dtGame::GameManager& gm)
   {
      dtABC::Application& app = gm.GetApplication();
      app.GetCamera()->SetProjectionResizePolicy(osg::Camera::FIXED);

      //The command line arg takes precidence, followed by the config file option.
      if(mAspectRatio == 0.0f)
      {
         std::string aspectString = app.GetConfigPropertyValue(CONFIG_PROP_ASPECT_RATIO, "1.6");

         if (aspectString == "auto")
         {
            app.GetCamera()->SetProjectionResizePolicy(osg::Camera::HORIZONTAL);
         }
         else
         {
            std::istringstream iss(aspectString);
            iss >> mAspectRatio;
            if (mAspectRatio == 0.0f)
            {
               double defaultAspectRatio = app.GetCamera()->GetAspectRatio();
               app.GetCamera()->SetAspectRatio(defaultAspectRatio < 1.47 ? 1.33 : 1.6);
            }
            else
            {
               app.GetCamera()->SetAspectRatio(mAspectRatio);
            }
         }

      }
      else
      {
         app.GetCamera()->SetAspectRatio(mAspectRatio);
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::OnStartup(dtABC::BaseABC& app, dtGame::GameManager& gameManager)
   {
      BaseClass::OnStartup(app, gameManager);

      dtCore::Camera* camera = gameManager.GetApplication().GetCamera();

      osg::Camera* cam = camera->GetOSGCamera();

      cam->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
         //setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
         //setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);

      cam->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);
      //camera->GetSceneHandler()->GetSceneView()->
      //   setCullingMode(osg::CullSettings::SMALL_FEATURE_CULLING | osg::CullSettings::SHADOW_OCCLUSION_CULLING |
      //      osg::CullSettings::CLUSTER_CULLING | osg::CullSettings::FAR_PLANE_CULLING |
      //      osg::CullSettings::VIEW_FRUSTUM_SIDES_CULLING);
      //camera->GetSceneHandler()->GetSceneView()->setSmallFeatureCullingPixelSize(250.0f);

      //camera->GetSceneHandler()->GetSceneView()->setNearFarRatio( PLAYER_NEAR_CLIP_PLANE / PLAYER_FAR_CLIP_PLANE);

      //cam->setNearFarRatio(PLAYER_NEAR_CLIP_PLANE / PLAYER_FAR_CLIP_PLANE);

      camera->SetPerspectiveParams(60.0f, 1.6,
                             PLAYER_NEAR_CLIP_PLANE,
                             PLAYER_FAR_CLIP_PLANE);
      AssignAspectRatio(gameManager);

	   camera->AddChild(dtAudio::AudioManager::GetListener());

      gameManager.LoadActorRegistry(LIBRARY_NAME);
      
      RefPtr<dtGame::DeadReckoningComponent>               drComp                     = new dtGame::DeadReckoningComponent;
      RefPtr<Components::ViewerNetworkPublishingComponent> rulesComp                  = new Components::ViewerNetworkPublishingComponent;
      RefPtr<Components::TimedDeleterComponent>            mTimedDeleterComp          = new Components::TimedDeleterComponent;
      RefPtr<Components::ParticleManagerComponent>         mParticleComp              = new Components::ParticleManagerComponent;
      RefPtr<Components::WeatherComponent>                 weatherComp                = new Components::WeatherComponent;
      RefPtr<Components::MunitionsComponent>               munitionsComp              = new Components::MunitionsComponent;
      RefPtr<Components::ViewerMaterialComponent>          viewerMaterialComponent    = new Components::ViewerMaterialComponent;
      RefPtr<dtAnim::AnimationComponent>                   animationComponent         = new dtAnim::AnimationComponent;

      gameManager.AddComponent(*weatherComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*drComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*rulesComp, dtGame::GameManager::ComponentPriority::HIGHER);
      gameManager.AddComponent(*mTimedDeleterComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*mParticleComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*viewerMaterialComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*munitionsComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*animationComponent, dtGame::GameManager::ComponentPriority::NORMAL);

      std::string highResGroundClampingRange = gameManager.GetConfiguration().GetConfigPropertyValue(
         CONFIG_PROP_HIGH_RES_GROUND_CLAMP_RANGE, "200");
 
      // Setup the DR Component.
      dtCore::RefPtr<Components::MultiSurfaceClamper> clamper = new Components::MultiSurfaceClamper;
      
      clamper->SetHighResGroundClampingRange( dtUtil::ToFloat(highResGroundClampingRange) );
      drComp->SetGroundClamper( *clamper );

      // Setup the Weather Component.
      weatherComp->SetUpdatesEnabled(!IsUIRunning());
      //WeatherComp->SetUseEphemeris(mIsUIRunning);
      weatherComp->SetNearFarClipPlanes( PLAYER_NEAR_CLIP_PLANE, PLAYER_FAR_CLIP_PLANE );
      //WeatherComp->SetUseEphemeris(true);
      weatherComp->UpdateFog();

      munitionsComp->SetMunitionConfigFileName(
         gameManager.GetConfiguration().GetConfigPropertyValue(CONFIG_PROP_MUNITION_CONFIG_FILE, "Configs:MunitionsConfig.xml"));

      InitializeComponents(gameManager);

   }

   /////////////////////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::OnShutdown(dtABC::BaseABC& app, dtGame::GameManager& gameManager)
   {
   }
}
