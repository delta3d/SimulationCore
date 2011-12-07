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
#include <dtUtil/stringutils.h>

#include <dtCore/camera.h>
#include <dtCore/environment.h>
#include <dtCore/isector.h>
#include <dtCore/scene.h>

#include <dtAudio/audiomanager.h>

#include <dtDAL/actorproxy.h>
#include <dtDAL/actorproperty.h>
#include <dtDAL/project.h>
#include <dtDAL/resourcedescriptor.h>
#include <dtDAL/map.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/projectconfig.h>

#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>
#include <dtGame/deadreckoningcomponent.h>
#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>

#include <dtUtil/datapathutils.h>

#include <dtAnim/animationcomponent.h>
#include <dtAnim/cal3ddatabase.h>
#include <dtAnim/animnodebuilder.h>

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

   const std::string BaseGameEntryPoint::CONFIG_PROP_PROJECT_CONTEXT_PATH("ProjectPath");
   const std::string BaseGameEntryPoint::CONFIG_PROP_USE_GPU_CHARACTER_SKINNING("UseGPUCharacterSkinning");
   const std::string BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE("DeveloperMode");
   const std::string BaseGameEntryPoint::CONFIG_PROP_ASPECT_RATIO("AspectRatio");
   const std::string BaseGameEntryPoint::CONFIG_PROP_MUNITION_MAP("MunitionMap");
   const std::string BaseGameEntryPoint::CONFIG_PROP_MUNITION_CONFIG_FILE("MunitionsConfigFile");
   const std::string BaseGameEntryPoint::CONFIG_PROP_HIGH_RES_GROUND_CLAMP_RANGE("HighResGroundClampingRange");

   //////////////////////////////////////////////////////////////////////////
   BaseGameEntryPoint::BaseGameEntryPoint()
   : parser(NULL)
   , mMissingRequiredCommandLineOption(false)
   , mIsUIRunning(false)
   {
   }

   //////////////////////////////////////////////////////////////////////////
   BaseGameEntryPoint::~BaseGameEntryPoint()
   {
      if (parser != NULL)
      {
         delete parser;
         parser = NULL;
      }

      //this needs to be moved
      dtAudio::AudioManager::Destroy();
   }

   //////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::Initialize(dtGame::GameApplication& app, int argc, char **argv)
   {
      mMissingRequiredCommandLineOption = false;

      if(parser == NULL)
         parser = new osg::ArgumentParser(&argc, argv);

      parser->getApplicationUsage()->addCommandLineOption("-h or --help","Display command line options");
      parser->getApplicationUsage()->addCommandLineOption("--UI", "Specify this to disable old functionality in favor of the UI");
      parser->getApplicationUsage()->addCommandLineOption("--projectPath", "The path (either relative or absolute) to the project context you wish to use. This defaults to the current working directory.");
      parser->getApplicationUsage()->addCommandLineOption("--mapName", "The name of the map to load in. This must be a map that is located within the project path specified");
      parser->getApplicationUsage()->addCommandLineOption("--aspectRatio", "The aspect ratio to use for the camera [1.33 or 1.6]");
      parser->getApplicationUsage()->addCommandLineOption("--lingeringShotSecs", "The number of seconds for a shot to linger after impact. The default value is 300 (5 minutes)");
      //parser->getApplicationUsage()->addCommandLineOption("--statisticsInterval", "The interval the game manager will use to print statistics, in seconds");

      if (parser->read("-h") || parser->read("--help") || parser->read("-?") || parser->read("--?") ||
         parser->argc() == 0)
      {
         parser->getApplicationUsage()->write(std::cerr);
         throw dtGame::GameApplicationConfigException(
            "Command Line Error.", __FILE__, __LINE__);
      }

      // Only change the value if a command line option is received.
      int tempBool = 0;
      if(parser->read("--UI", tempBool))
      {
         mIsUIRunning = tempBool == 1 ? true : false;
      }

      if (!parser->read("--projectPath", mProjectPath))
      {
         mProjectPath = "";
      }

      if (!parser->read("--aspectRatio", mAspectRatio))
      {
         mAspectRatio = 0.0f;
      }

      if (!parser->read("--lingeringShotSecs", mLingeringShotEffectSecs))
      {
         mLingeringShotEffectSecs = 300.0f;
      }

      //if (!parser->read("--statisticsInterval", mStatisticsInterval))
      //{
      //   mStatisticsInterval = 0;
      //}

      if(!mIsUIRunning)
      {
         if (!parser->read("--mapName", mMapName) && mMapName.empty())
         {
            std::cerr << "Please specify the map file to be used with the --mapName option.\n";
            mMissingRequiredCommandLineOption = true;
         }
      }

      try
      {
         dtAudio::AudioManager::Instantiate();
      }
      catch(const dtUtil::Exception& e)
      {
         dtAudio::AudioManager::Destroy();
         throw e;
      }
      catch(const std::exception &e)
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

   //////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::FinalizeParser()
   {
      if(parser == NULL)
      {
         LOG_DEBUG("FinalizeParser was called when the parser was null");
         return;
      }

      parser->reportRemainingOptionsAsUnrecognized();
      if (parser->errors())
      {
         parser->writeErrorMessages(std::cerr);
         throw dtGame::GameApplicationConfigException(
            "Command Line Error.", __FILE__, __LINE__);
      }

      if (mMissingRequiredCommandLineOption)
      {
         parser->getApplicationUsage()->write(std::cerr);
         throw dtGame::GameApplicationConfigException(
            "Command Line Error.", __FILE__, __LINE__);
      }
   }


   void BaseGameEntryPoint::AssignProjectContext(dtGame::GameManager& gm)
   {
      dtUtil::FileUtils& fileUtils = dtUtil::FileUtils::GetInstance();

      if (mProjectPath.empty())
      {
         mProjectPath = gm.GetConfiguration().GetConfigPropertyValue(CONFIG_PROP_PROJECT_CONTEXT_PATH);
      }

      if (mProjectPath.empty())
      {
         if (fileUtils.FileExists(PROJECT_CONFIG_FILE))
         {
            mProjectPath = PROJECT_CONFIG_FILE;
         }
         else if (fileUtils.DirExists(PROJECT_CONTEXT_DIR))
         {
            mProjectPath = PROJECT_CONTEXT_DIR;
         }
         else if (fileUtils.DirExists(std::string("../") + PROJECT_CONTEXT_DIR))
         {
            mProjectPath = std::string("../") + PROJECT_CONTEXT_DIR;
         }
      }

      if(!mProjectPath.empty())
      {
         if (fileUtils.DirExists(mProjectPath))
         {
            LOG_INFO("The project context \"" + mProjectPath + "\" is a directory has been found. Attempting config project");
            dtDAL::Project::GetInstance().SetContext(mProjectPath);
         }
         else if (fileUtils.FileExists(mProjectPath))
         {
            LOG_INFO("The project context \"" + mProjectPath + "\" is a file.  Attempting to load as a project config.");
            dtDAL::Project::GetInstance().SetupFromProjectConfigFile(mProjectPath);
         }
         else
         {
            throw dtGame::GameApplicationConfigException(
                   "The path " + mProjectPath + " could not be located in the working directory or its parent directory. Aborting application. Make sure the config.xml is in the right directory and the ProjectPath is set correctly."
                   , __FILE__, __LINE__);
         }
      }
      else
      {
         throw dtGame::GameApplicationConfigException(
                  "The data directory " + PROJECT_CONTEXT_DIR +
                  " could not be located in the working directory or its parent directory. Aborting application. Make sure the config.xml is in the right directory and the ProjectPath is set correctly."
                  , __FILE__, __LINE__);
      }
   }

   void BaseGameEntryPoint::PreLoadMap()
   {
      if(!mIsUIRunning)
      {
         std::set<std::string> mapNames = dtDAL::Project::GetInstance().GetMapNames();
         bool containsMap = false;
         for(std::set<std::string>::iterator i = mapNames.begin(); i != mapNames.end(); ++i)
            if(*i == mMapName)
               containsMap = true;

         if(!containsMap)
         {
            std::ostringstream oss;
            oss << "A map named: " << mMapName << " could not be located in the project context: "
               << mProjectPath;
            throw dtGame::GameApplicationConfigException(
               oss.str(), __FILE__, __LINE__);
         }
      }
   }

   //////////////////////////////////////////////////////////////////////////
   void BaseGameEntryPoint::AssignAspectRatio(dtGame::GameApplication& app)
   {
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
   void BaseGameEntryPoint::OnStartup(dtGame::GameApplication& app)
   {

      AssignProjectContext(*app.GetGameManager());
      PreLoadMap();

      dtGame::GameManager& gameManager = *app.GetGameManager();

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
      AssignAspectRatio(app);

	   camera->AddChild(dtAudio::AudioManager::GetListener());

      gameManager.LoadActorRegistry(LIBRARY_NAME);

      RefPtr<dtGame::DeadReckoningComponent>   drComp            = new dtGame::DeadReckoningComponent;
      RefPtr<Components::ViewerNetworkPublishingComponent> rulesComp         = new Components::ViewerNetworkPublishingComponent;
      RefPtr<Components::TimedDeleterComponent>            mTimedDeleterComp = new Components::TimedDeleterComponent;
      RefPtr<Components::ParticleManagerComponent>         mParticleComp     = new Components::ParticleManagerComponent;
      RefPtr<Components::WeatherComponent>                 weatherComp       = new Components::WeatherComponent;
      RefPtr<Components::MunitionsComponent>               munitionsComp     = new Components::MunitionsComponent;
      RefPtr<Components::ViewerMaterialComponent> viewerMaterialComponent    = new Components::ViewerMaterialComponent;
      RefPtr<dtAnim::AnimationComponent>          animationComponent         = new dtAnim::AnimationComponent;

      gameManager.AddComponent(*weatherComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*drComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*rulesComp, dtGame::GameManager::ComponentPriority::HIGHER);
      gameManager.AddComponent(*mTimedDeleterComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*mParticleComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*viewerMaterialComponent, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*munitionsComp, dtGame::GameManager::ComponentPriority::NORMAL);
      gameManager.AddComponent(*animationComponent, dtGame::GameManager::ComponentPriority::NORMAL);

      std::string useGPUSkinning = gameManager.GetConfiguration().GetConfigPropertyValue(
               CONFIG_PROP_USE_GPU_CHARACTER_SKINNING, "1");


      dtAnim::AnimNodeBuilder& nodeBuilder = dtAnim::Cal3DDatabase::GetInstance().GetNodeBuilder();

      if (useGPUSkinning == "1" || useGPUSkinning == "true")
      {
         nodeBuilder.SetCreate(dtAnim::AnimNodeBuilder::CreateFunc(&nodeBuilder, &dtAnim::AnimNodeBuilder::CreateHardware));
         LOG_INFO("Using GPU Character Skinning");
      }
      else
      {
         nodeBuilder.SetCreate(dtAnim::AnimNodeBuilder::CreateFunc(&nodeBuilder, &dtAnim::AnimNodeBuilder::CreateSoftware));
         LOG_INFO("Using CPU Character Skinning");
      }

      std::string highResGroundClampingRange = gameManager.GetConfiguration().GetConfigPropertyValue(
         CONFIG_PROP_HIGH_RES_GROUND_CLAMP_RANGE, "200");

      // Setup the DR Component.
      dtCore::RefPtr<Components::MultiSurfaceClamper> clamper = new Components::MultiSurfaceClamper;
      
      clamper->SetHighResGroundClampingRange( dtUtil::ToFloat(highResGroundClampingRange) );
      drComp->SetGroundClamper( *clamper );

      // Setup the Weather Component.
      weatherComp->SetUpdatesEnabled(!mIsUIRunning);
      //WeatherComp->SetUseEphemeris(mIsUIRunning);
      weatherComp->SetNearFarClipPlanes( PLAYER_NEAR_CLIP_PLANE, PLAYER_FAR_CLIP_PLANE );
      //WeatherComp->SetUseEphemeris(true);
      weatherComp->UpdateFog();

      munitionsComp->SetMunitionConfigFileName(
         app.GetConfigPropertyValue(CONFIG_PROP_MUNITION_CONFIG_FILE, "Configs:MunitionsConfig.xml"));

      InitializeComponents(gameManager);

      // CURT - This conflicts with requirement request to make it a parameter but can't
      // support various munition types.
      //SimCore::Actors::DetonationActor::SetLingeringShotSecs(mLingeringShotEffectSecs);
   }

   DT_IMPLEMENT_ACCESSOR(BaseGameEntryPoint, std::string, MapName);
}
