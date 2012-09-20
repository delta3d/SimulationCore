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
#include <ostream>
#include <SimCore/Tools/Compass.h>
#include <SimCore/Actors/StealthActor.h>
#include <dtUtil/datapathutils.h>
#include <dtUtil/log.h>
#include <dtCore/camera.h>
#include <dtCore/deltawin.h>
#include <dtCore/transform.h>

#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osg/Projection>
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
      Compass::Compass(CEGUI::Window *mainWindow, dtCore::Camera& camera, bool useMagNorth, float aspectRatio) :
         Tool(mainWindow),
         mUseMagneticNorth(useMagNorth),
         mNeedleRotation(0.0f),
         mNeedlePosition(0.0f),
         mNeedleVelocity(0.0f),
         mNeedleAcceleration(0.0f),
         mNeedleTorque(2.0f),
         mNeedleDragCoef(0.9f),
         mCamera(&camera)
      {
         Enable(false);
      }

      //////////////////////////////////////////////////////////////////////////
      Compass::~Compass()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void Compass::Enable(bool enable)
      {
         Tool::Enable(enable);

         if( mCompassOverlay.valid() )
         {
            mCompassOverlay->setNodeMask(enable?0xFFFFFFFF:0);
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

            h = float(UpdateNeedle(timeDelta,h+180.0f))-180.0f;

            // --- LOCKHEED CODE --- START --- //
            if( mDisk.valid() )
            {
               //printf("Disk is valid\n");
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
            if (std::abs(ccwDiff) < diff)
               diff = ccwDiff;
         }
         else if (diff < 0.0f)
         {
            float cwDiff = (deg360 - mNeedlePosition) + heading;
            if (cwDiff < std::abs(diff))
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
      void Compass::InitLens(osg::Group& hudLayer)
      {
         dtCore::DeltaWin::Resolution res = dtCore::DeltaWin::GetCurrentResolution();
         double windowWidth = res.width;
         double windowHeight = res.height;

         std::string lensaticNode("Textures/hud/compass/lensatic.osg");
         lensaticNode = dtUtil::FindFileInPathList(lensaticNode);

         osg::ref_ptr<osg::Node> fileNode;

         if (!lensaticNode.empty())
            fileNode = osgDB::readNodeFile(lensaticNode);

         if( fileNode != NULL )
         {
            mCompassOverlay = dynamic_cast< osg::MatrixTransform* >(fileNode.get());
            if( ! mCompassOverlay.valid() )
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

         if( ! mCompassOverlay.valid() )
         {
            LOG_ERROR("Compass::InitLens: lens overlay node was unable to be created.");
            return;
         }


         osg::Matrix trans;
         FindNamedNodeVisitor dialNV("dial");

         mCompassOverlay->accept(dialNV);
         if(!dialNV._foundNodes.empty())
         {
            mDisk = dynamic_cast< osg::MatrixTransform* >(
               dialNV._foundNodes.front().get());
         }

         float horizontalFOV = 1.0f;
         if(mCamera != NULL)
            horizontalFOV = mCamera->GetHorizontalFov();

         double dAspect = windowWidth/windowHeight;
         double dScale = 1.0/(horizontalFOV*M_PI/180.0*60.0);
         trans.makeScale(dScale,dScale,dScale);
         trans.setTrans(0.5*dAspect, 0.17/dAspect, -1.0);

         mCompassOverlay->setMatrix(trans);
         mCompassOverlay->setNodeMask(0);

         mLensFocus = new osg::Uniform(osg::Uniform::FLOAT_VEC2, "lensFocus");
         std::string fragFileName = dtUtil::FindFileInPathList("Shaders/Base/fisheye.frag");
         std::string vertFileName = dtUtil::FindFileInPathList("Shaders/Base/fisheye.vert");

         if( fragFileName.length() != 0 && vertFileName.length() != 0)
         {
            osg::ref_ptr<osg::Program> fishEyeProg = new osg::Program;
            osg::ref_ptr<osg::Shader>    vertShader = new osg::Shader( osg::Shader::VERTEX );
            osg::ref_ptr<osg::Shader>    fragShader = new osg::Shader( osg::Shader::FRAGMENT );

            bool resultValue = vertShader->loadShaderSourceFromFile(vertFileName);
            resultValue = fishEyeProg->addShader( vertShader.get() );
            resultValue = fragShader->loadShaderSourceFromFile(fragFileName);
            resultValue = fishEyeProg->addShader( fragShader.get() );

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
            dAspect,
            0,
            1.0));

         // Create the view
         osg::MatrixTransform *viewABS = new osg::MatrixTransform;
         viewABS->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         viewABS->setMatrix(osg::Matrix::identity());
         viewABS->getOrCreateStateSet()->setRenderBinDetails(
            SimCore::Components::RenderingSupportComponent::RENDER_BIN_HUD-1, "RenderBin");

         // Setup hierarchy
         // root <- proj <- view <- lens
         viewABS->addChild(mCompassOverlay.get());
         projection->addChild(viewABS);
         hudLayer.addChild(projection);

         // Ensure all node levels are renderable.
         mCompassOverlay->setNodeMask(IsEnabled()?0xFFFFFFFF:0);
         viewABS->setNodeMask(0xFFFFFFFF);
         projection->setNodeMask(0xFFFFFFFF);
      }
   }
}
