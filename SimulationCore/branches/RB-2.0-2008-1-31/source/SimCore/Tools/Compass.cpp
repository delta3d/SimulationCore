/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Eddie Johnson
 */
#include <prefix/SimCorePrefix-src.h>
#include <ostream>
#include <SimCore/Tools/Compass.h>
#include <SimCore/Actors/StealthActor.h>
#include <dtUtil/log.h>
#include <dtUtil/coordinates.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/globals.h>
#include <dtABC/application.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gameactor.h>
#include <dtGame/exceptionenum.h>

#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osg/Shader>
#include <osg/StateSet>
#include <osg/Uniform>
#include <osgDB/ReadFile>

#include <SimCore/Components/RenderingSupportComponent.h>

#ifdef None
#undef None
#endif

#include <CEGUI/CEGUI.h> 

namespace SimCore
{
   namespace Tools
   {
      //////////////////////////////////////////////////////////////////////////
      Compass::Compass(CEGUI::Window *mainWindow, bool useMagNorth, float aspectRatio) :
         Tool(mainWindow),
         mOverlay(NULL),
         mAzimuthText(NULL),
         mUseMagneticNorth(useMagNorth),
         mNeedleRotation(0.0f),
         mNeedlePosition(0.0f),
         mNeedleVelocity(0.0f),
         mNeedleAcceleration(0.0f),
         mNeedleTorque(2.0f),
         mNeedleDragCoef(0.9f)
      {
         try
         {
            CEGUI::WindowManager *wm = CEGUI::WindowManager::getSingletonPtr();
            mOverlay            = wm->createWindow("WindowsLook/StaticImage", "compass_overlay");
            mAzimuthText        = wm->createWindow("WindowsLook/StaticText", "azimuth_text");
            mAzimuthDegreesText = wm->createWindow("WindowsLook/StaticText", "azimuth_degrees_text");

            if(mainWindow != NULL)
               mainWindow->addChildWindow(mOverlay);
            mOverlay->setPosition(CEGUI::UVector2(cegui_reldim(0.0f), cegui_reldim(0.0f)));
            mOverlay->setSize(CEGUI::UVector2(cegui_reldim(1.0f), cegui_reldim(1.0f)));
            mOverlay->setProperty("BackgroundEnabled", "false");
            mOverlay->setProperty("FrameEnabled", "false");
            
            if(aspectRatio < 1.47)
            {
               mOverlay->setProperty("Image", "set:Compass4.3 image:Compass4.3");
               mAzimuthText->setPosition(CEGUI::UVector2(cegui_reldim(0.461f), cegui_reldim(0.47f)));
               mAzimuthDegreesText->setPosition(CEGUI::UVector2(cegui_reldim(0.461f), cegui_reldim(0.49f)));
            }
            else 
            {
               mOverlay->setProperty("Image", "set:Compass8.5 image:Compass8.5");
               mAzimuthText->setPosition(CEGUI::UVector2(cegui_reldim(0.461f), cegui_reldim(0.47f)));
               mAzimuthDegreesText->setPosition(CEGUI::UVector2(cegui_reldim(0.461f), cegui_reldim(0.49f)));
            }

            // Mils Display
            mOverlay->addChildWindow(mAzimuthText);
            mAzimuthText->setFont("DejaVuSans-10");
            mAzimuthText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(0, 0, 0)));
            mAzimuthText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mAzimuthText->setProperty("FrameEnabled", "false");
            mAzimuthText->setProperty("BackgroundEnabled", "false");
            mAzimuthText->setHorizontalAlignment(CEGUI::HA_LEFT);

            // Degrees Display
            mOverlay->addChildWindow(mAzimuthDegreesText);
            mAzimuthDegreesText->setFont("DejaVuSans-10");
            mAzimuthDegreesText->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(1, 0, 0)));
            mAzimuthDegreesText->setSize(CEGUI::UVector2(cegui_reldim(0.8f), cegui_reldim(0.25f)));
            mAzimuthDegreesText->setProperty("FrameEnabled", "false");
            mAzimuthDegreesText->setProperty("BackgroundEnabled", "false");
            mAzimuthDegreesText->setHorizontalAlignment(CEGUI::HA_LEFT);
         }
         catch(CEGUI::Exception &e)
         {
            std::ostringstream oss;
            oss << "CEGUI exception caught: " << e.getMessage().c_str();
            throw dtUtil::Exception(dtGame::ExceptionEnum::GAME_APPLICATION_CONFIG_ERROR,
               oss.str(), __FILE__, __LINE__);
         }
         Enable(false);
      }

      //////////////////////////////////////////////////////////////////////////
      Compass::~Compass()
      {
         mOverlay->removeChildWindow(mAzimuthText);
         mAzimuthText->destroy();
         mAzimuthDegreesText->destroy();
         if(mMainWindow != NULL)
            mMainWindow->removeChildWindow(mOverlay);
         mOverlay->destroy();
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass::Enable(bool enable)
      {
         Tool::Enable(enable);

         IsEnabled() ? mOverlay->show() : mOverlay->hide();

         if( mLensOverlay.valid() )
         {
            mLensOverlay->setNodeMask(enable?0xFFFFFFFF:0);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass::Update( float timeDelta )
      {
         if(IsEnabled() && GetPlayerActor() != NULL)
         {
            dtCore::Transform xform;
            SimCore::Actors::StealthActor& player = *GetPlayerActor();
            player.GetTransform(xform);
            float h, p, r;
            xform.GetRotation(h, p, r);
            if(mUseMagneticNorth)
               h += GetCoordinateConverter().GetMagneticNorthOffset();
            unsigned int mils = dtUtil::Coordinates::DegreesToMils(h);

            h = float(UpdateNeedle(timeDelta,h+180.0f))-180.0f;

            // --- LOCKHEED CODE --- START --- //
            if( mDisk.valid() )
            { 
               osg::Matrix rotation;

               //just to test
               rotation.makeRotate(osg::DegreesToRadians(-h), osg::Vec3f(0,0,1.0));
               //int foo = m_spCompassDisk->getNumChildren();
               mDisk->setMatrix(rotation);
               //move the focus of the lens as we move the dial.
               float xVal = cos(osg::DegreesToRadians(h+90))*.375f+.5f;
               float yVal = sin(osg::DegreesToRadians(-h+90))*.375f+.5f;
               osg::Vec2f foc (xVal, yVal);
               mLensFocus->set(foc);
            }
            // --- LOCKHEED CODE --- END --- //
         }
      }

      //////////////////////////////////////////////////////////////////////////
      float Compass::UpdateNeedle( float deltaTime, float heading )
      {
         if (heading < 0.0f)
            heading += 360.0f;
         if (heading > 360.0f)
            heading -= 360.0f;

         if (mNeedlePosition < 0.0f)
            mNeedlePosition += 360.0f;
         if (mNeedlePosition > 360.0f)
            mNeedlePosition -= 360.0f;

         float diff = heading - mNeedlePosition;

         //figure out the direction of acceleration
         static const float deg360 = 360.0f;//(osg::PI + osg::PI);
         static const float deg90 = 90.0f;//(osg::PI_2);
         static const float deg270 = 270.0f;//(osg::PI + osg::PI_2);
         if (heading > deg270 && mNeedlePosition < deg90)
            diff = (heading - deg360) + (0.0f - mNeedlePosition);
         else if (heading <  deg90 && mNeedlePosition > deg270)
            diff = (heading - 0.0f) + (deg360 - mNeedlePosition);
         else if (diff > 0.0f)
         {
            float ccwDiff = -mNeedlePosition + (heading - deg360);
            if (fabsf(ccwDiff) < diff)
               diff = ccwDiff;
         }
         else if (diff < 0.0f)
         {
            float cwDiff = (deg360 - mNeedlePosition) + heading;
            if (cwDiff < fabs(diff))
               diff = cwDiff;
         }

         mNeedleAcceleration = (mNeedleTorque * diff) -
            (mNeedleDragCoef * mNeedleVelocity);

         mNeedleVelocity = mNeedleVelocity + mNeedleAcceleration*deltaTime;

         float newPosition = mNeedlePosition + mNeedleVelocity*deltaTime;

         static const float zero = 0.0001f;
         if (mNeedleVelocity < zero && mNeedleVelocity > -zero)
            newPosition = mNeedlePosition;

         mNeedlePosition = newPosition;
         return mNeedlePosition;
      }

      //////////////////////////////////////////////////////////////////////////
      //stolen directly from ModuleLAV25. This probably should be more generic
      class FindNamedNodeVisitor : public osg::NodeVisitor
      {
      public:
         FindNamedNodeVisitor(const std::string& name):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
               _name(name) {}

               virtual void apply(osg::Node& node)
               {
                  if (node.getName()==_name)
                  {
                     _foundNodes.push_back(&node);
                  }
                  traverse(node);
               }

               virtual void apply(osg::Geode& node)
               {
                  if (node.getName()==_name)
                  {
                     _foundNodes.push_back(&node);
                  }
                  traverse(node);
               }

               typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;

               std::string _name;
               NodeList _foundNodes;
      };

      //////////////////////////////////////////////////////////////////////////
      void Compass::InitLens( dtCore::DeltaDrawable& hudLayer )
      {
         dtCore::DeltaWin::Resolution res = dtCore::DeltaWin::GetCurrentResolution();
         double windowWidth = res.width;//1920.0; // TODO: access the window for screen width
         double windowHeight = res.height;//1200.0; // TODO: access the window for screen height

         std::string lensaticNode("Textures/hud/compass/lensatic.osg");
         lensaticNode = dtCore::FindFileInPathList(lensaticNode);
         
         osg::ref_ptr<osg::Node> fileNode; 
         
         if (!lensaticNode.empty())
            fileNode = osgDB::readNodeFile(lensaticNode);
         
         if( fileNode != NULL )
         {
            mLensOverlay = dynamic_cast< osg::MatrixTransform* >(fileNode.get());
            if( ! mLensOverlay.valid() )
            {
               LOG_ERROR("Compass::InitLens: unable to convert file node to an osg::MatrixTransform node.");
            }
         }
         else
         {
            std::ostringstream ss;
            ss << "Compass::InitLens: unable to load node file \"" << lensaticNode << "\"." << std::endl;
            LOG_ERROR(ss.str());
         }

         if( ! mLensOverlay.valid() )
         {
            LOG_ERROR("Compass::InitLens: lens overlay node was unable to be created.");
            return;
         }


         osg::Matrix trans;
         FindNamedNodeVisitor dialNV("dial");

         mLensOverlay->accept(dialNV);
         if(!dialNV._foundNodes.empty())
         {
            mDisk = dynamic_cast< osg::MatrixTransform* >(
               dialNV._foundNodes.front().get());
         }

         double dAspect = windowWidth/windowHeight;
         double dScale = (1.0/(120.0))*.8;//*1.43; // CR: I am not sure what the numbers stand for (1.43) was added to the equation.
         //1.0/(m_fHFOV*M_PI/180.0*1000.0); //360.0 * 120.0/(2.0*M_PI*1000.0); // FOV = 120m @ 1km ~= 6.875deg
         trans.makeScale(dScale,dScale,dScale);
         trans.setTrans(0.5*dAspect, 0.27/dAspect, 0.0);

         mLensOverlay->setMatrix(trans);
         mLensOverlay->setNodeMask(0);

         mLensFocus = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "lensFocus");
         std::string fragFileName = dtCore::FindFileInPathList("Shaders/Base/fisheye.frag");
         std::string vertFileName = dtCore::FindFileInPathList("Shaders/Base/fisheye.vert");

         if( fragFileName.length() != 0 && vertFileName.length() != 0)
         {
            osg::ref_ptr<osg::Program> fishEyeProg = new osg::Program;
            osg::ref_ptr<osg::Shader>    vertShader = new osg::Shader( osg::Shader::VERTEX );
            osg::ref_ptr<osg::Shader>    fragShader = new osg::Shader( osg::Shader::FRAGMENT );

            vertShader->loadShaderSourceFromFile(vertFileName);
            fishEyeProg->addShader( vertShader.get() );
            fragShader->loadShaderSourceFromFile(fragFileName);
            fishEyeProg->addShader( fragShader.get() );

            osg::StateSet *states = mDisk->getOrCreateStateSet();
            states->setAttributeAndModes(fishEyeProg.get(),osg::StateAttribute::ON);
            // The shader will access the 'baseTexture' loaded with your model - in slot 0
            osg::Uniform* pUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D,"baseTexture");
            pUniform->set(0);
            states->addUniform(pUniform);

            //move the focus as we move the dial
            osg::Vec2f focus(0.5f,0.875f);
            mLensFocus->set(focus);
            states->addUniform(mLensFocus.get());
         }

         // Create the projection
         osg::Projection *projection = new osg::Projection();
         projection->setMatrix(
            osg::Matrix::ortho2D(
            0,
            windowWidth/windowHeight,
            0,
            1.0));

         // Create the view
         osg::MatrixTransform *viewABS = new osg::MatrixTransform;
         viewABS->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         viewABS->setMatrix(osg::Matrix::identity());
         viewABS->getOrCreateStateSet()->setRenderBinDetails(SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD, "RenderBin");

         // Setup hierarchy
         // root <- proj <- view <- lens
         viewABS->addChild(mLensOverlay.get());
         projection->addChild(viewABS);
         hudLayer.GetOSGNode()->asGroup()->addChild(projection);

         // Ensure all node levels are renderable.
         mLensOverlay->setNodeMask(IsEnabled()?0xFFFFFFFF:0);
         viewABS->setNodeMask(0xFFFFFFFF);
         projection->setNodeMask(0xFFFFFFFF);
      }
   }
}
