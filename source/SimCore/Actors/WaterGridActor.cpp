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
*
* @author Bradley Anderegg
* @author Curtiss Murphy
*/


#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/WaterGridActor.h>

#include <dtDAL/functor.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/actorproxyicon.h>
#include <dtDAL/project.h>

#include <dtGame/messagetype.h>
#include <dtGame/basemessages.h>
#include <dtGame/gameactor.h>
#include <dtGame/invokable.h>
#include <dtGame/actorupdatemessage.h>
#include <cmath>

#include <dtCore/shadermanager.h>
#include <dtCore/transform.h>
#include <dtCore/scene.h>
#include <dtCore/cloudplane.h>
#include <dtABC/application.h>

#include <dtUtil/mathdefines.h>
#include <dtUtil/noisetexture.h>
#include <dtUtil/matrixutil.h>

#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/PolygonMode>
#include <osg/Camera>
#include <osg/Geode>
#include <osg/Texture>
#include <osg/Depth>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/TexGen>
#include <osg/TexGenNode>
#include <osg/TexMat>
#include <osg/Math>
#include <osg/CullFace>
#include <osg/Projection>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgGA/GUIEventAdapter>

#include <osgViewer/GraphicsWindow>
#include <osgEphemeris/EphemerisModel.h>
#include <osgEphemeris/SkyDome.h>

#include <SimCore/Components/RenderingSupportComponent.h>
#include <SimCore/Components/WeatherComponent.h>
#include <SimCore/Actors/EphemerisEnvironmentActor.h>
#include <SimCore/Actors/OceanDataActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>

class UpdateReflectionCameraCallback : public osg::NodeCallback
{
public:

   UpdateReflectionCameraCallback(osg::Camera* trans, osg::Camera* camera)
   : mTarget(trans)
   , mCamera(camera)
   {
   }

   virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
   {
      // first update subgraph to make sure objects are all moved into postion
      traverse(node,nv);

      mCamera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
      mCamera->setProjectionMatrix(mTarget->getProjectionMatrix());;
      mCamera->setViewMatrix(mTarget->getViewMatrix());

   }

protected:

   virtual ~UpdateReflectionCameraCallback() {}

   dtCore::ObserverPtr<osg::Camera>                mTarget;
   dtCore::RefPtr<osg::Camera>                     mCamera;

};


osg::Node* CreateQuad( osg::Texture2D *tex, int renderBin )
{
   osg::Geometry* geo = new osg::Geometry;
   geo->setUseDisplayList( false );
   osg::Vec4Array* colors = new osg::Vec4Array;
   colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
   geo->setColorArray(colors);
   geo->setColorBinding(osg::Geometry::BIND_OVERALL);
   osg::Vec3Array *vx = new osg::Vec3Array;
   vx->push_back(osg::Vec3(-10, -10, 0));
   vx->push_back(osg::Vec3(10, -10, 0));
   vx->push_back(osg::Vec3(10, 10, 0 ));
   vx->push_back(osg::Vec3(-10, 10, 0));
   geo->setVertexArray(vx);
   osg::Vec3Array *nx = new osg::Vec3Array;
   nx->push_back(osg::Vec3(0, 0, 1));
   geo->setNormalArray(nx);
   if(tex != NULL)
   {
      osg::Vec2Array *tx = new osg::Vec2Array;
      tx->push_back(osg::Vec2(0, 0));
      tx->push_back(osg::Vec2(1, 0));
      tx->push_back(osg::Vec2(1, 1));
      tx->push_back(osg::Vec2(0, 1));
      geo->setTexCoordArray(0, tx);
      geo->getOrCreateStateSet()->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);
   }

   geo->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
   osg::Geode *geode = new osg::Geode;
   geode->addDrawable(geo);
   geode->setCullingActive(false);
   osg::StateSet* ss = geode->getOrCreateStateSet();
   ss->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
   ss->setMode( GL_DEPTH_TEST, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
   ss->setRenderBinDetails( renderBin, "RenderBin" );
   return geode;
}

osg::Texture2D* CreateTexture(int width, int height, bool mipmap)
{
   osg::Texture2D* tex = new osg::Texture2D();
   tex->setTextureSize(width, height);
   tex->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT);
   tex->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT);
   //tex->setSourceFormat( GL_RGBA );
   tex->setInternalFormat(GL_RGBA);
   if(mipmap)
   {
      tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR);
   }
   else
   {
      tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
   }

   tex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
   return tex;
}

namespace SimCore
{

   namespace Actors
   {

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //const int WaterGridActor::MAX_WAVES(8);
      const int WaterGridActor::MAX_TEXTURE_WAVES(32);
      const dtUtil::RefString WaterGridActor::UNIFORM_ELAPSED_TIME("elapsedTime");
      const dtUtil::RefString WaterGridActor::UNIFORM_MAX_COMPUTED_DISTANCE("maxComputedDistance");
#ifdef __APPLE__
      const dtUtil::RefString WaterGridActor::UNIFORM_WAVE_ARRAY("waveArray[0]");
#else
      const dtUtil::RefString WaterGridActor::UNIFORM_WAVE_ARRAY("waveArray");
#endif
      const dtUtil::RefString WaterGridActor::UNIFORM_TEXTURE_WAVE_ARRAY("TextureWaveArray");
      const dtUtil::RefString WaterGridActor::UNIFORM_REFLECTION_MAP("reflectionMap");
      const dtUtil::RefString WaterGridActor::UNIFORM_NOISE_TEXTURE("noiseTexture");
      const dtUtil::RefString WaterGridActor::UNIFORM_WAVE_TEXTURE("waveTexture");
      const dtUtil::RefString WaterGridActor::UNIFORM_SCREEN_WIDTH("ScreenWidth");
      const dtUtil::RefString WaterGridActor::UNIFORM_SCREEN_HEIGHT("ScreenHeight");
      const dtUtil::RefString WaterGridActor::UNIFORM_TEXTURE_WAVE_AMP("AmpOverLength");
      const dtUtil::RefString WaterGridActor::UNIFORM_WATER_HEIGHT("WaterHeight");
      const dtUtil::RefString WaterGridActor::UNIFORM_CENTER_OFFSET("cameraRecenter");
      const dtUtil::RefString WaterGridActor::UNIFORM_WAVE_DIRECTION("waveDirection");
      const dtUtil::RefString WaterGridActor::UNIFORM_WATER_COLOR("WaterColor");


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


      IMPLEMENT_ENUM(WaterGridActor::ChoppinessSettings);
      WaterGridActor::ChoppinessSettings WaterGridActor::ChoppinessSettings::
         CHOP_FLAT("CHOP_FLAT", 0.0f, 30.0f);
      WaterGridActor::ChoppinessSettings WaterGridActor::ChoppinessSettings::
         CHOP_MILD("CHOP_MILD", 0.615f, 45.0f);
      WaterGridActor::ChoppinessSettings WaterGridActor::ChoppinessSettings::
         CHOP_MED("CHOP_MED", 1.75f, 65.0f);
      WaterGridActor::ChoppinessSettings WaterGridActor::ChoppinessSettings::
         CHOP_ROUGH("CHOP_ROUGH", 3.5f, 90.0f);


      WaterGridActor::ChoppinessSettings::ChoppinessSettings(const std::string &name, float rotationSpread, float texMod)
         : dtUtil::Enumeration(name), mRotationSpread(rotationSpread), mTextureWaveModifier(texMod)
      {  
         AddInstance(this);
      }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(WaterGridActor::SeaState);///////////////////////////////// AMP  WaveLen  Speed
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_0("SeaState_0", 0.1, 0.1, 0.2);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_1("SeaState_1", 0.15, 0.15, 0.4);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_2("SeaState_2", 0.25, 0.25, 0.6);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_3("SeaState_3", 0.45, 0.45, 0.8);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_4("SeaState_4", 0.65, 0.65, 1.0);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_5("SeaState_5", 0.85, 0.85, 1.25);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_6("SeaState_6", 1.0, 1.0, 1.5);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_7("SeaState_7", 1.15, 1.15, 1.75);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_8("SeaState_8", 1.25, 1.25, 2.25);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_9("SeaState_9", 1.45, 1.45, 2.5);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_10("SeaState_10", 1.55, 1.55, 3.0);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_11("SeaState_11", 1.65, 1.65, 4.0);
      WaterGridActor::SeaState WaterGridActor::SeaState::SeaState_12("SeaState_12", 2.0, 2.0, 8.0);



      WaterGridActor::SeaState::SeaState(const std::string& name, float ampMod, float waveLenMod, float speedMod)
         : dtUtil::Enumeration(name), mAmplitudeModifier(ampMod) , mWaveLengthModifier(waveLenMod), mSpeedModifier(speedMod)
      {  
         AddInstance(this);
      }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //This should temporarily keep us from being culled out
      //TODO: calculate bounding box
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      class WaterGridComputeBound : public osg::Drawable::ComputeBoundingBoxCallback
      {
      public:
         WaterGridComputeBound()
         {
         }

         /*virtual*/ osg::BoundingBox computeBound(const osg::Drawable& drawable) const
         {
            //by default its min float to max float.. so that should do the trick ;)
            return osg::BoundingBox(-FLT_MAX,-FLT_MAX,-FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
         }

      };

      class WaterCullCallback: public osg::Drawable::CullCallback
      {
      public:
         WaterCullCallback()
         {}

         virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const
         {
            return false;
         }

      private:

      };

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //WATER GRID ACTOR
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      WaterGridActor::WaterGridActor(WaterGridActorProxy& proxy)
          : BaseClass(proxy)
          , mElapsedTime(0.0f)
          , mDeltaTime(0.0f)
          , mRenderWaveTexture(false)
          , mWireframe(false)
          , mDeveloperMode(false)
          , mComputedRadialDistance(0.0)
          , mTextureWaveAmpOverLength(1.0 / 64.0)
          , mModForWaveLength(1.0f)
          , mModForSpeed(1.0f)
          , mModForSteepness(1.0f)
          , mModForAmplitude(1.0f)
          , mModForDirectionInDegrees(0.0f)
          , mModForFOV(1.0f)
          , mCameraFoVScalar(1.0f)
          , mMaxWaveHeight(0.0f)
          , mWaterColor(0.117187, 0.3125, 0.58593, 1.0)
          , mChoppinessEnum(&ChoppinessSettings::CHOP_FLAT)
          , mSeaStateEnum(&SeaState::SeaState_4)
      {
         SetName("WaterGridActor"); // Set a default name
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      WaterGridActor::~WaterGridActor()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
      {
         Update(tickMessage.GetDeltaSimTime());
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::OnEnteredWorld()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetChoppiness(WaterGridActor::ChoppinessSettings &choppiness)
      {
         std::cout << "Setting Choppiness to ["<< choppiness.GetName() << "]." << std::endl;
         mChoppinessEnum = &choppiness;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      WaterGridActor::ChoppinessSettings& WaterGridActor::GetChoppiness() const
      {
         return *mChoppinessEnum;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetWaterColor( const osg::Vec4& color )
      {
         mWaterColor = color;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      osg::Vec4 WaterGridActor::GetWaterColor() const
      {
         return mWaterColor;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetMaxWaveHeight(float height)
      {
         mMaxWaveHeight = height;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetMaxWaveHeight() const
      {
         return mMaxWaveHeight;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::Init(const dtGame::Message&)
      {

         CreateGeometry();

         ///Added a callback to the camera this can set uniforms on each camera.
         dtCore::Camera::AddCameraSyncCallback(*this,
            dtCore::Camera::CameraSyncCallback(this, &WaterGridActor::UpdateWaveUniforms));

         std::string developerMode;
         developerMode = GetGameActorProxy().GetGameManager()->GetConfiguration().GetConfigPropertyValue("DeveloperMode");
         mDeveloperMode = (developerMode == "true" || developerMode == "1");


         const float kArray[] = {1.33, 1.76, 3.0, 2.246,
                                       1.0, 3.71, 1.0, 1.75,
                                       1.5, 1.0, 1.0, 2.0,
                                       2.2, 2.0, 1.113, 1.0,
                                       1.33, 1.76, 3.0, 2.246,
                                       1.0, 3.71, 1.0, 1.75,
                                       1.5, 1.0, 1.0, 2.0,
                                       2.2, 2.0, 1.113, 1.0};

         const float waveLengthArray[] = {0.1788, 0.0535, 0.12186, 0.24,
                                          0.14, 0.116844, 0.97437, 0.0805,
                                          0.067, 0.3565, 0.67135 , 0.191,
                                          0.155, 0.13917, 0.275, .448,
                                          0.1788, 0.0535, 0.12186, 0.24,
                                          0.14, 0.116844, 0.97437, 0.0805,
                                          0.067, 0.3565, 0.67135 , 0.191,
                                          0.155, 0.13917, 0.275, .448};

         const float waveSpeedArray[] = {0.0953, 0.03839, 0.0311, 0.04221,
                                         0.11497, 0.143213, 0.14571, 0.051181,

                                         0.01473, 0.1531, 0.2131, 0.0221,
                                         0.121497, 0.1213, 0.14571, 0.1181,
                                         0.0953, 0.03839, 0.0311, 0.04221,
                                         0.11497, 0.143213, 0.14571, 0.051181,

                                         0.01473, 0.1531, 0.2131, 0.0221,
                                         0.121497, 0.1213, 0.14571, 0.1181};

         for(int i = 0; i < MAX_TEXTURE_WAVES; ++i)
         {
            TextureWave tw;
            tw.mSteepness = kArray[i];
            tw.mSpeed = waveSpeedArray[i];
            tw.mWaveLength = waveLengthArray[i];

            AddTextureWave(tw);
         }

         SetSeaStateByNumber(4);
      }

      /////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::ClearWaves()
      {
         mWaves.clear();
      }


      /////////////////////////////////////////////////////////////////////////////
      bool WaterGridActor::GetHeightAndNormalAtPoint( const osg::Vec3& detectionPoint,
         float& outHeight, osg::Vec3& outNormal ) const
      {
         outHeight = GetWaterHeight();

         float distanceToCamera = (detectionPoint - mCurrentCameraPos).length();

         float xPos = detectionPoint[0] - mLastCameraOffsetPos[0];
         float yPos = detectionPoint[1] - mLastCameraOffsetPos[1];
         // There are 2 vec4's of data per wave, so the loop is MAX_WAVES * 2 but increments by 2's
         for(int i = 0; i < 16/*MAX_WAVES*/; i++)
         {
            // Order is: waveLength, speed, amp, freq, UNUSED, UNUSED, dirX, dirY
            float waveLen = mProcessedWaveData[i][0]; //waveArray[i].x
            float speed = mProcessedWaveData[i][1]; //waveArray[i].y;
            float freq = mProcessedWaveData[i][3]; //waveArray[i].w;
            float amp = mProcessedWaveData[i][2]; //waveArray[i].z;
            float waveDirX = mProcessedWaveData[i][6]; //waveArray[i + 1].zw;
            float waveDirY = mProcessedWaveData[i][7];
            float k = std::max(1.5f * mProcessedWaveData[i][4], 4.00001f); //(waveArray[i+1].x);

            // This math MUST match the calculations done in water_functions.vert AND water.vert
            float mPlusPhi = (freq * (speed * mElapsedTime + xPos * waveDirX + waveDirY * yPos));
            float sinDir = pow((std::sin(mPlusPhi) + 1.0f) / 2.0f, k);

            //using approximation here because the waves scale out with distance to avoid aliasing with the grid
            distanceToCamera /= 15.0f;
            dtUtil::Clamp(distanceToCamera, 0.0f, 1000.0f);
            float distBetweenVertsScalar = (2.5f + distanceToCamera) / waveLen;
            dtUtil::Clamp(distBetweenVertsScalar, 0.0f, 0.999f);
            amp *= 1.0f - distBetweenVertsScalar;

            outHeight += amp * sinDir;
         }

         outNormal.set(0.0f, 0.0f, 1.0f);

         return true;
      }

      /////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetWaveAmplitudeAtPoint( const Wave& wave,
         const osg::Vec3& worldPoint ) const
      {
         return 0.0;
      }

      /////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::Update(float dt)
      {
         mDeltaTime = dt;
         mElapsedTime += dt;

         dtCore::Keyboard* kb = GetGameActorProxy().GetGameManager()->GetApplication().GetKeyboard();

         static float keyTimeOut = 0.0f;
         keyTimeOut -= dt;

         if (kb != NULL && mDeveloperMode && keyTimeOut <= 0.0f)
         {

            //if (kb->GetKeyState('9'))
            //{
            //   mModForWaveLength *= 0.96f; // 10% less
            //   std::cout << "WaveLength mod changed to [" << mModForWaveLength << "]." << std::endl;
            //}
            //else if (kb->GetKeyState('0'))
            //{
            //   mModForWaveLength *= 1.04f; // 10% more
            //   std::cout << "WaveLength mod changed to [" << mModForWaveLength << "]." << std::endl;
            //}

            //if (kb->GetKeyState('7'))
            //{
            //   mModForSpeed *= 0.96f; // 10% less
            //   std::cout << "Speed mod changed to [" << mModForSpeed << "]." << std::endl;
            //}
            //else if (kb->GetKeyState('8'))
            //{
            //   mModForSpeed *= 1.04f; // 10% more
            //   std::cout << "Speed mod changed to [" << mModForSpeed << "]." << std::endl;
            //}

            //if (kb->GetKeyState('5'))
            //{
            //   mModForAmplitude *= 0.96f; // 10% less
            //   std::cout << "Amp mod changed to [" << mModForAmplitude << "]." << std::endl;
            //}
            //else if (kb->GetKeyState('6'))
            //{
            //   mModForAmplitude *= 1.04f; // 10% more
            //   std::cout << "Amp mod changed to [" << mModForAmplitude << "]." << std::endl;
            //}

            //if (kb->GetKeyState('3'))
            //{
            //   mModForDirectionInDegrees -= 2.00; // 10% less
            //   mModForDirectionInDegrees = (mModForDirectionInDegrees < 0.0f) ? (mModForDirectionInDegrees + 360.0f) : mModForDirectionInDegrees;
            //   std::cout << "Direction mod changed to [" << mModForDirectionInDegrees << "]." << std::endl;
            //}
            //else if (kb->GetKeyState('4'))
            //{
            //   mModForDirectionInDegrees += 2.00f; // 10% more
            //   mModForDirectionInDegrees = (mModForDirectionInDegrees > 360.0f) ? (mModForDirectionInDegrees - 360.0f) : mModForDirectionInDegrees;
            //   std::cout << "Direction mod changed to [" << mModForDirectionInDegrees << "]." << std::endl;
            //}

            //if (kb->GetKeyState('1'))
            //{
            //   mModForFOV *= 0.96f; // 10% less
            //   std::cout << "Mod For FOV changed to [" << mModForFOV << "]." << std::endl;
            //}
            //else if (kb->GetKeyState('2'))
            //{
            //   mModForFOV *= 1.04f; // 10% more
            //   std::cout << "Mod For FOV changed to [" << mModForFOV << "]." << std::endl;
            //}

            //if(kb->GetKeyState(osgGA::GUIEventAdapter::KEY_Return))
            //{
            //   mModForWaveLength = 1.0f; // 10% less
            //   mModForFOV = 1.0f; // 10% more
            //   mModForDirectionInDegrees = 0.0f;
            //   mModForAmplitude = 1.0f;
            //   mModForSpeed = 1.0f;
            //   std::cout << "Resetting ALL Dev mods to Default." << std::endl;

            //   mRenderWaveTexture = !mRenderWaveTexture;
            //   SetRenderWaveTexture(mRenderWaveTexture);
            //   keyTimeOut = 0.5;
            //}

            //if(kb->GetKeyState(osgGA::GUIEventAdapter::KEY_Space))
            //{
            //   mWireframe = !mWireframe;

            //   osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;

            //   if(mWireframe)
            //   {
            //      polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
            //   }
            //   else
            //   {
            //      polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::FILL);
            //   }

            //   mGeode->getOrCreateStateSet()->setAttributeAndModes(polymode.get(),osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
            //   keyTimeOut = 0.5;
            //}

            if(kb->GetKeyState(osgGA::GUIEventAdapter::KEY_Home))
            {
               static int testChoppiness = 0;
               testChoppiness++;
               testChoppiness %= 4;

               if (testChoppiness == 0)
                  SetChoppiness(ChoppinessSettings::CHOP_FLAT);
               else if (testChoppiness == 1)
                  SetChoppiness(ChoppinessSettings::CHOP_MILD);
               else if (testChoppiness == 2)
                  SetChoppiness(ChoppinessSettings::CHOP_MED);
               else if (testChoppiness == 3)
                  SetChoppiness(ChoppinessSettings::CHOP_ROUGH);

               keyTimeOut = 0.5;
            }

            //set to 4 because that is the default
            static int testSeaState = 4;
            if(kb->GetKeyState(osgGA::GUIEventAdapter::KEY_Page_Up))
            {
               if(testSeaState == 12) testSeaState = -1;
               
               testSeaState++;

               SetSeaStateByNumber(testSeaState);

               std::cout << "Setting Sea State to: " << testSeaState << std::endl;

               keyTimeOut = 0.5;
            }

            if(kb->GetKeyState(osgGA::GUIEventAdapter::KEY_Page_Down))
            {
               if(testSeaState == 0) testSeaState = 13;

               testSeaState--;

               SetSeaStateByNumber(testSeaState);

               std::cout << "Setting Sea State to: " << testSeaState << std::endl;

               keyTimeOut = 0.5;
            }

         }
      }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::CreateGeometry()
      {
         mGeode = new osg::Geode();
         mGeometry = BuildRadialGrid();
         mGeometry->setCullCallback(new WaterCullCallback());

         mGeode->addDrawable(mGeometry.get());

         osg::StateSet* ss = mGeode->getOrCreateStateSet();
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);
         osg::BlendFunc* bf = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         ss->setAttribute(bf);
         ss->setRenderBinDetails(-1, "RenderBin" );

         GetMatrixNode()->addChild(BuildSubmergedGeometry());
         GetMatrixNode()->addChild(mGeode.get());


         //for reflection texture
         //CreateReflectionTexture();
         
         CreateNoiseTexture();
         CreateWaveTexture();
         CreateReflectionCamera();

           
         osg::Texture2D* foamTexture2D = new osg::Texture2D();
         std::string foamTextureFile = dtDAL::Project::GetInstance().GetResourcePath(dtDAL::ResourceDescriptor("Textures:OceanFoam.tga"));
         osg::Image* newImage = osgDB::readImageFile(foamTextureFile);
         if (newImage == NULL)
         {
            LOG_ERROR("WaterGridActor failed to load Foam Texture file [" + foamTextureFile + "].");
         }

         foamTexture2D->setImage(newImage);
         foamTexture2D->dirtyTextureObject();
         foamTexture2D->setFilter( osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR_MIPMAP_LINEAR);
         foamTexture2D->setFilter( osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR );
         foamTexture2D->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
         foamTexture2D->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
         foamTexture2D->setUnRefImageDataAfterApply(true);
         osg::Uniform* foamTexUniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "foamTexture");
         foamTexUniform->set(2);
         ss->addUniform(foamTexUniform);
         ss->setTextureAttributeAndModes(2, foamTexture2D, osg::StateAttribute::ON);
         

         //add a custom compute bounding box callback
         mGeometry->setComputeBoundingBoxCallback(new WaterGridComputeBound());
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      osg::Node* WaterGridActor::BuildSubmergedGeometry()
      {
         osg::Node* quad = CreateQuad(NULL, -2);//SimCore::Components::RenderingSupportComponent::RENDER_BIN_WATER - 1);
         BindShader(quad, "UnderWater");

         osg::StateSet* ss = quad->getOrCreateStateSet();

         osg::Depth* depth = new osg::Depth;
         depth->setFunction(osg::Depth::ALWAYS);
         depth->setRange(1.0,1.0);
         ss->setAttributeAndModes(depth, osg::StateAttribute::ON );

         osg::BlendFunc* bf = new osg::BlendFunc();
         bf->setFunction( osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
         ss->setAttributeAndModes(bf);

         ss->setMode(GL_CULL_FACE, osg::StateAttribute::OFF );
         ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF );


         osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
         modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         modelview_abs->setMatrix(osg::Matrix::identity());
         modelview_abs->addChild(quad);

         osg::Projection* projection = new osg::Projection;
         projection->setMatrix(osg::Matrix::ortho2D(-1,1,-1,1));
         projection->addChild(modelview_abs);


         return projection;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::UpdateWaveUniforms(dtCore::Camera& pCamera)
      {

         osg::Camera* sceneCam = pCamera.GetOSGCamera();
         osg::StateSet* ss = sceneCam->getOrCreateStateSet();

         if(pCamera.GetOSGCamera()->getViewport() != NULL)
         {
            osg::Matrix matView, matProj, matViewProjScreenInverse, matScreen;

            matView.set(pCamera.GetOSGCamera()->getViewMatrix());
            matProj.set(pCamera.GetOSGCamera()->getProjectionMatrix());
            matScreen.set(pCamera.GetOSGCamera()->getViewport()->computeWindowMatrix());

            osg::Matrix mvps(matView * matProj * matScreen);
            matViewProjScreenInverse.invert(mvps);

            UpdateWaterPlaneFOV(pCamera, matViewProjScreenInverse);
            UpdateScreenSpaceWaterHeight(pCamera, matView * matProj);
         }

         osg::Uniform* screenWidth = ss->getOrCreateUniform(UNIFORM_SCREEN_WIDTH, osg::Uniform::FLOAT);
         osg::Uniform* screenHeight = ss->getOrCreateUniform(UNIFORM_SCREEN_HEIGHT, osg::Uniform::FLOAT);

         screenWidth->set(float(sceneCam->getViewport()->width()));
         screenHeight->set(float(sceneCam->getViewport()->height()));

         //set the elapsed time
         osg::Uniform* elapsedTime = ss->getOrCreateUniform(UNIFORM_ELAPSED_TIME, osg::Uniform::FLOAT);
         elapsedTime->set(mElapsedTime);

         //set the wave direction modifier
         osg::Uniform* waveDirModifier = ss->getOrCreateUniform(UNIFORM_WAVE_DIRECTION, osg::Uniform::FLOAT);
         waveDirModifier->set(mModForDirectionInDegrees);

         //set the max distance
         osg::Uniform* maxComputedDistance = ss->getOrCreateUniform(UNIFORM_MAX_COMPUTED_DISTANCE, osg::Uniform::FLOAT);
         maxComputedDistance->set(mComputedRadialDistance);

         // set the water height
         osg::Uniform* waterHeight = ss->getOrCreateUniform(UNIFORM_WATER_HEIGHT, osg::Uniform::FLOAT);
         waterHeight->set(GetWaterHeight());

         //update vertex wave uniforms
         osg::Uniform* waveArray = ss->getOrCreateUniform(UNIFORM_WAVE_ARRAY, osg::Uniform::FLOAT_VEC4, MAX_WAVES * 2);

         // Update the
         osg::Uniform* centerOffset = ss->getOrCreateUniform(UNIFORM_CENTER_OFFSET, osg::Uniform::FLOAT_VEC3);
         centerOffset->set(mLastCameraOffsetPos);

         osg::Uniform* waterColor = ss->getOrCreateUniform(UNIFORM_WATER_COLOR, osg::Uniform::FLOAT_VEC4);
         waterColor->set(mWaterColor);


         // Loop through the current list of waves and put them in the uniform
         DetermineCurrentWaveSet(pCamera);
         float maxWaveHeight = 0.0f;
         for(int count = 0; count < MAX_WAVES; count++)
         {
            // Order is: waveLength, speed, amp, freq, UNUSED, UNUSED, dirX, dirY
            waveArray->setElement(2 * count, osg::Vec4(mProcessedWaveData[count][0], mProcessedWaveData[count][1],
               mProcessedWaveData[count][2], mProcessedWaveData[count][3]));
            waveArray->setElement(2 * count + 1, osg::Vec4(mProcessedWaveData[count][4], mProcessedWaveData[count][5],
               mProcessedWaveData[count][6], mProcessedWaveData[count][7]));

            maxWaveHeight += mProcessedWaveData[2 * count][2];
         }

         SetMaxWaveHeight(maxWaveHeight);
         //std::cout << "Max Wave Height: " << maxWaveHeight << std::endl;

         //set the foam height
         osg::Uniform* foamMaxHeight = ss->getOrCreateUniform("foamMaxHeight", osg::Uniform::FLOAT);
         foamMaxHeight->set(GetWaterHeight() + GetMaxWaveHeight());

         //set the FOV Modifier - uses the value from DetermineCurrentWaveSet()
         osg::Uniform* fovModifier = ss->getOrCreateUniform("modForFOV", osg::Uniform::FLOAT);
         fovModifier->set(mCameraFoVScalar * mModForFOV);

         //set the TextureWaveChopModifier, changes the range of angles used to compute the wave directions
         osg::Uniform* twcModifier = ss->getOrCreateUniform("textureWaveChopModifier", osg::Uniform::FLOAT);
         twcModifier->set(mChoppinessEnum->mTextureWaveModifier);

         //update texture wave uniforms
         osg::Uniform* textureWaveAmp = ss->getOrCreateUniform(UNIFORM_TEXTURE_WAVE_AMP, osg::Uniform::FLOAT);
         textureWaveAmp->set(mTextureWaveAmpOverLength);

         osg::Uniform* textureWaveArray = ss->getOrCreateUniform(UNIFORM_TEXTURE_WAVE_ARRAY, osg::Uniform::FLOAT_VEC4, MAX_TEXTURE_WAVES);

         //int numWaves = float(mTextureWaves.size());
         TextureWaveArray::iterator tw_iter = mTextureWaves.begin();
         TextureWaveArray::iterator tw_endIter = mTextureWaves.end();

         for(int count = 0;count < MAX_TEXTURE_WAVES; ++count)
         {
            if(tw_iter != tw_endIter)
            {
               TextureWave& wave = (*tw_iter);

               float freq = (2.0f * osg::PI) / wave.mWaveLength;

               textureWaveArray->setElement(count, osg::Vec4(wave.mWaveLength, wave.mSpeed, wave.mSteepness, freq));

               ++tw_iter;
            }
            else
            {
               //else disable the wave by zero-ing it out
               textureWaveArray->setElement(count, osg::Vec4(0.0f, 0.0f, 0.0f, 0.0f));
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::DetermineCurrentWaveSet(dtCore::Camera& pCamera)
      {
         // Camera Height is used to skip past small waves when we are high up.
         dtCore::Camera *camera = &pCamera;
         dtCore::Transform trans;
         camera->GetTransform(trans);
         const osg::Vec3 translation = trans.GetTranslation();
         float cameraHeight = translation[2] - GetWaterHeight();
         cameraHeight = (cameraHeight > 1.0) ? cameraHeight : 1.0;

         // Reset the camera center so thatwe get a LOT less jitter due to near/far clipping
         mCurrentCameraPos = trans.GetTranslation();
         osg::Vec3 camTrans(mCurrentCameraPos);
         camTrans[2] = 0.0;
         float distance = (camTrans - mLastCameraOffsetPos).length();
         if (distance > 9999.0)
         {
            mLastCameraOffsetPos = camTrans;
         }

         // FOV calculation
         // ideal FoV is estimated to be 60 Hor & 90 Vert so ... 75 avgFoV.
         //float avgFoV = 0.5f * (camera->GetHorizontalFov() + camera->GetVerticalFov());
         //mCameraFoVScalar = (avgFoV / 75.0f);

         mCameraFoVScalar = camera->GetHorizontalFov() / 90.0f;

         int count = 0;
         float numWaves = float(mWaves.size());
         WaveArray::iterator iter = mWaves.begin();
         WaveArray::iterator endIter = mWaves.end();

         // Search for the next wave that is big enough to be worth showing.
         // Camera Cut Point is an estimated value for the cut point - scaled by all the FoV modifiers.
         bool quitLooking = false;
         int numIgnored = 0;
         float cameraCutPoint = 0.5 + cameraHeight / (24.0 * mModForWaveLength * mCameraFoVScalar * mModForFOV); // Used to pick waves
         while(iter != endIter && !quitLooking)
         {
            Wave &nextWave = (*iter);
            if (nextWave.mWaveLength > (cameraCutPoint) || (numWaves - numIgnored) <= MAX_WAVES)
            {
               quitLooking = true;
            }
            else
            {
               ++iter;
               numIgnored ++;
            }
         }


         // The choppiness rotation spreads out the 8 waves so that they come at wider angles
         // which causes then to be choppier.
         float choppinessRotationAmount = mChoppinessEnum->mRotationSpread * mModForAmplitude;
         float choppinessSign = 1.0;
         for(;count < MAX_WAVES * 2;)
         {
            if(iter != endIter)
            {
               Wave& wave = (*iter);
               // weaken the amp as it reaches the pop point to hide some of the popping
               //float fadeRatio = sqrt((wave.mWaveLength - cameraCutPoint) / cameraCutPoint);
               float amp = wave.mAmplitude * mModForAmplitude;// * dtUtil::Min(1.0f, dtUtil::Max(0.0f, fadeRatio));
               float waveLength = wave.mWaveLength * mModForWaveLength;
               float speed = wave.mSpeed * mModForSpeed;

               float freq = (2.0f * osg::PI) / waveLength;

               float steepness = 1.0 + 2.0 * wave.mSteepness;
               steepness = dtUtil::Max(steepness, 1.0f);

               //don't bind waves that have a zero amplitude
               if(amp > 0.001f)
               {
                  choppinessSign *= -1.0f;
                  float choppinessRotationOffset = choppinessSign * choppinessRotationAmount * count;
                  float curWaveDir = wave.mDirectionInDegrees + choppinessRotationOffset + choppinessRotationAmount * count/2.0f;
                  float dirX = sin(osg::DegreesToRadians(curWaveDir + mModForDirectionInDegrees));
                  float dirY = cos(osg::DegreesToRadians(curWaveDir + mModForDirectionInDegrees));

                  mProcessedWaveData[count/2][0] = waveLength * mSeaStateEnum->mWaveLengthModifier;
                  mProcessedWaveData[count/2][1] = speed * mSeaStateEnum->mSpeedModifier;
                  mProcessedWaveData[count/2][2] = amp * mSeaStateEnum->mAmplitudeModifier;
                  mProcessedWaveData[count/2][3] = freq;
                  mProcessedWaveData[count/2][4] = steepness;
                  mProcessedWaveData[count/2][5] = 1.0f;
                  mProcessedWaveData[count/2][6] = dirX;
                  mProcessedWaveData[count/2][7] = dirY; 

                  count += 2;
               }

               ++iter;
            }
            else
            {
               //else disable the wave by zero-ing it out
               mProcessedWaveData[count/2][0] = 0.0;
               mProcessedWaveData[count/2][1] = 0.0;
               mProcessedWaveData[count/2][2] = 0.0;
               mProcessedWaveData[count/2][3] = 0.0;
               mProcessedWaveData[count/2][4] = 1.0;
               mProcessedWaveData[count/2][5] = 1.0;
               mProcessedWaveData[count/2][6] = 0.0;
               mProcessedWaveData[count/2][7] = 0.0;
               count += 2;
            }
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::AddWave(Wave& pWave)
      {
         pWave.mDirection.set(sin(osg::DegreesToRadians(pWave.mDirectionInDegrees)),
                              cos(osg::DegreesToRadians(pWave.mDirectionInDegrees)));
         pWave.mDirection.normalize();
         mWaves.push_back(pWave);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::AddTextureWave( const TextureWave& pWave )
      {
         mTextureWaves.push_back(pWave);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      osg::Geometry* WaterGridActor::BuildRadialGrid()
      {
         osg::Geometry* geometry = new osg::Geometry();

         int N = 500; // rows from center outward
         int K = 360;//260; // columns around the circle

         //calculate num verts and num indices
         int numVerts = N * K;
         int numIndices = (N - 1) * (K - 1) * 6;

         //lets make the geometry
         dtCore::RefPtr<osg::Vec4Array> pVerts = new osg::Vec4Array(numVerts);
         dtCore::RefPtr<osg::IntArray> pIndices = new osg::IntArray(numIndices);

         //float a0 = 0.01f;
         //float a1 = 1.0f; // 5.0f;
         //float outerMostRingDistance = 5000.0; // the furthest rings get an extra reach.
         //float middleRingDistance = 20.0; // Middle rings get a minor boost too.
         //int numOuterRings = 15;
         //int numMiddleRings = 20;
         //float innerExpBase = 1.02f;
         //float middleExpBase = 1.2;
         //float outerExpBase = 1.19f;
         ////float exponent = 3;

         float a0 = 0.05f;
         float a1 = 1.205f; // 5.0f;
         float outerMostRingDistance = 1250.0; // the furthest rings get an extra reach.
         float middleRingDistance = 12.5; // Middle rings get a minor boost too.
         int numOuterRings = 10;
         int numMiddleRings = 50;
         float innerExpBase = 1.0205f;
         float middleExpBase = 1.05;
         float outerExpBase = 1.5f;
         //float exponent = 3;

         float r = a0;
         for(int i = 0; i < N; ++i)
         {
            float radiusIncrement = a0;
            // Radius extends with each row/ring.
            if (i != 0)
               radiusIncrement += a1 * powf(innerExpBase, i); //= a0 + a1 * powf(base, i);

            // Final rows/rings get an extra boost - solves some shrinkage & horizon problems.
            if ((i + numOuterRings) > N)
               radiusIncrement += outerMostRingDistance * powf(outerExpBase, (i + numOuterRings - N));
            // Middle rows/rings get a little boost - solves some shrinkage & horizon problems
            else if ((i + numOuterRings + numMiddleRings) > N)
               radiusIncrement += middleRingDistance * powf(middleExpBase, (i + numOuterRings + numMiddleRings - N));

            r += radiusIncrement;

            for(int j = 0; j < K; ++j)
            {
               //float PI2j = osg::PI * 2.0f * j;
               float x = 0.0;
               if(j == K - 1)
               {
                  //setting this to one makes the water mesh fully wrap around
                  x = 1;
               }
               else
               {
                  //we are setting our vertex position as a -1,1 quantized angle and a depth
                  x = float((2.0 * (double(j) / double(K))) - 1.0);
               }
               float y = r;
               float z = radiusIncrement; // We put the radius increment into the Z so we can use it in the shader
               float groupNum = 56.0f * (float(i) / float(N));
               (*pVerts)[(i * K) + j ].set(x, y, z, groupNum);
            }
         }
         mComputedRadialDistance = r;

         int counter = 0;

         for(int i = 0; i < N - 1; ++i)
         {
            for(int j = 0; j < K - 1; ++j)
            {
               int JPlusOne = (j + 1);// % K;

               (*pIndices)[counter] = (i * K) + j;
               (*pIndices)[counter + 1] = ((i + 1) * K) + j;
               (*pIndices)[counter + 2] = ((i + 1) * K) + (JPlusOne);

               (*pIndices)[counter + 3] = ((i + 1) * K) + (JPlusOne);
               (*pIndices)[counter + 4] = (i * K) + (JPlusOne);
               (*pIndices)[counter + 5] = (i * K) + j;

               counter += 6;
            }
         }


         geometry->setVertexArray(pVerts.get());
         geometry->setVertexIndices(pIndices.get());
         geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, numIndices));

         return geometry;
      }


      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::CreateNoiseTexture()
      {
         LOG_INFO("Creating noise texture.");
         dtUtil::NoiseTexture noise3d(6, 2, 0.7, 0.5, 128, 128, 32);
         dtCore::RefPtr<osg::Image> img = noise3d.MakeNoiseTexture(GL_ALPHA);
         LOG_INFO("Finished creating noise texture.");

         mNoiseTexture = new osg::Texture3D();
         mNoiseTexture->setImage(img.get());

         mNoiseTexture->setFilter( osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR );
         mNoiseTexture->setFilter( osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR );
         mNoiseTexture->setWrap( osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT );
         mNoiseTexture->setWrap( osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT );
         mNoiseTexture->setWrap( osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT );

         osg::Uniform* tex = new osg::Uniform(osg::Uniform::SAMPLER_3D, UNIFORM_NOISE_TEXTURE);
         tex->set(3);
         mGeometry->getOrCreateStateSet()->addUniform(tex);
         mGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(3, mNoiseTexture.get(), osg::StateAttribute::ON);

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::CreateWaveTexture()
      {
         SimCore::Components::RenderingSupportComponent* comp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, comp);

         if(comp != NULL)
         {
            if(!mWaveTexture.valid())
            {
               //TODO: GET DIMENSIONS OF SCREEN
               int width = 256;
               int height = 256;

               mWaveCamera = new osg::Camera();
               mWaveCamera->setRenderOrder(osg::Camera::PRE_RENDER, 1);
               mWaveCamera->setClearMask(GL_NONE);

               mWaveTexture = CreateTexture(width, height, false);
               InitAndBindToTarget(mWaveCamera.get(), mWaveTexture.get(), width, height);
               //mWaveCamera->setNodeMask(SimCore::Components::RenderingSupportComponent::MAIN_CAMERA_ONLY_FEATURE_NODE_MASK);
               AddOrthoQuad(mWaveCamera.get(), NULL, "TextureWave", "");

               comp->AddCamera(mWaveCamera.get());


               mWaveCameraScreen = new osg::Camera();
               mWaveCameraScreen->setRenderOrder(osg::Camera::POST_RENDER, 1);
               mWaveCameraScreen->setClearMask(GL_NONE);
               mWaveCameraScreen->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
               mWaveCameraScreen->setProjectionMatrixAsOrtho2D(-10.0, 10.0, -10.0, 10.0);
               mWaveCameraScreen->setViewport(0, 0, width, height);
               mWaveCameraScreen->setGraphicsContext(new osgViewer::GraphicsWindowEmbedded());
               AddOrthoQuad(mWaveCameraScreen.get(), mWaveTexture.get(), "WaveTest", "waveTexture");
               mWaveCameraScreen->setNodeMask(0x0);
               comp->AddCamera(mWaveCameraScreen.get());
            }

            osg::Uniform* tex = new osg::Uniform(osg::Uniform::SAMPLER_2D, UNIFORM_WAVE_TEXTURE);
            tex->set(0);
            mGeometry->getOrCreateStateSet()->addUniform(tex);
            mGeometry->getOrCreateStateSet()->setTextureAttributeAndModes(0, mWaveTexture.get(), osg::StateAttribute::ON);

         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::AddOrthoQuad(osg::Camera* cn, osg::Texture2D* tx, const std::string& shader, const std::string& texUniform)
      {
         osg::Node* quad = CreateQuad(tx, 50);
         cn->addChild(quad);
         BindShader(quad, shader);
         if(tx != NULL)
         {
            BindTextureUniformToNode(quad, tx, texUniform, 0);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::InitAndBindToTarget(osg::Camera* cn, osg::Texture2D* tx, int width, int height)
      {
         cn->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
         cn->setProjectionMatrixAsOrtho2D(-10.0, 10.0, -10.0, 10.0);
         cn->setViewport(0, 0, width, height);
         cn->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
         cn->detach( osg::Camera::COLOR_BUFFER );
         cn->attach( osg::Camera::COLOR_BUFFER, tx);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::BindTextureUniformToNode(osg::Node* node, osg::Texture2D* tex, const std::string& name, unsigned texUnit)
      {
         osg::StateSet* ss = node->getOrCreateStateSet();
         osg::Uniform* uniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, name);
         uniform->set(int(texUnit));
         ss->addUniform(uniform);
      }

      void WaterGridActor::BindShader(osg::Node* node, const std::string& shaderName)
      {
         const dtCore::ShaderGroup *shaderGroup = dtCore::ShaderManager::GetInstance().FindShaderGroupPrototype("WaterGroup");

         if (shaderGroup == NULL)
         {
            LOG_INFO("Could not find shader group: WaterGroup");
            return;
         }

         const dtCore::ShaderProgram *defaultShader = shaderGroup->FindShader(shaderName);

         try
         {
            if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *node);
            }
            else
            {
               LOG_WARNING("Could not find shader '" + shaderName);
               return;
            }
         }
         catch (const dtUtil::Exception &e)
         {
            LOG_WARNING("Caught Exception while assigning shader: " + e.ToString());
            return;
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetRenderWaveTexture( bool b )
      {
         if(b)
         {
            mWaveCameraScreen->setNodeMask(0xFFFFFFFF);
         }
         else
         {
            mWaveCameraScreen->setNodeMask(0x0);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::CreateReflectionCamera()
      {

         SimCore::Components::RenderingSupportComponent* comp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::RenderingSupportComponent::DEFAULT_NAME, comp);

         if(comp != NULL)
         {

            mReflectionCamera = new osg::Camera();
            //osg::Camera* sceneCam = GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->GetOSGCamera();

            int width = 512;//sceneCam->getViewport()->width();
            int height = 512;//sceneCam->getViewport()->height();


            mReflectionTexture = CreateTexture(width, height, false);
            InitAndBindToTarget(mReflectionCamera.get(), mReflectionTexture.get(), width, height);

            mReflectionCamera->setRenderOrder(osg::Camera::PRE_RENDER);
            mReflectionCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            mReflectionCamera->setUpdateCallback(new UpdateReflectionCameraCallback(GetGameActorProxy().GetGameManager()->GetApplication().GetCamera()->GetOSGCamera(), mReflectionCamera.get()));

            AddReflectionScene(mReflectionCamera.get());

            comp->AddCamera(mReflectionCamera.get());

            osg::Uniform* tex = new osg::Uniform(osg::Uniform::SAMPLER_2D, UNIFORM_REFLECTION_MAP);
            tex->set(1);
            osg::StateSet* ss = mGeometry->getOrCreateStateSet();
            ss->addUniform(tex);
            ss->setTextureAttributeAndModes(1, mReflectionTexture.get(), osg::StateAttribute::ON);

//            this is commented out because we are switching between using it for visualizing the reflection and visualizing the texture waves
            //mWaveCameraScreen = new osg::Camera();
            //mWaveCameraScreen->setRenderOrder(osg::Camera::POST_RENDER, 1);
            //mWaveCameraScreen->setClearMask(GL_NONE);
            //mWaveCameraScreen->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
            //mWaveCameraScreen->setProjectionMatrixAsOrtho2D(-10.0, 10.0, -10.0, 10.0);
            //mWaveCameraScreen->setViewport(0, 0, width, height);
            //mWaveCameraScreen->setGraphicsContext(new osgViewer::GraphicsWindowEmbedded());
            //AddOrthoQuad(mWaveCameraScreen.get(), mReflectionTexture.get(), "WaveTest", "waveTexture");
            //mWaveCameraScreen->setNodeMask(0x0);
            //comp->AddCamera(mWaveCameraScreen.get());
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetEnableWater( bool b )
      {
         if(b)
         {
            mGeode->setNodeMask(0xFFFFFFFF);
         }
         else
         {
            mGeode->setNodeMask(0x0);
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::AddReflectionScene( osg::Camera* cam )
      {
         SimCore::Components::WeatherComponent* comp = NULL;
         GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::WeatherComponent::DEFAULT_NAME, comp);

         osg::MatrixTransform* mat = new osg::MatrixTransform();
         mat->setMatrix(osg::Matrix::scale(osg::Vec3(1.0, 1.0, -1.0)));

         if(comp != NULL)
         {
            SimCore::Actors::EphemerisEnvironmentActor* env = dynamic_cast<SimCore::Actors::EphemerisEnvironmentActor*>(comp->GetEphemerisEnvironment());

            if(env != NULL)
            {
               osgEphemeris::EphemerisModel* ephem = env->GetEphemerisModel();
               if(ephem != NULL)
               {
                  //we have to reverse the cullface on the ephemeris or we wont see it
                  //this is necessary due to the reflection about the z axis
                  osg::Group* grp = new osg::Group();
                  grp->getOrCreateStateSet()->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                  grp->addChild(ephem);
                  mat->addChild(grp);

                  mat->addChild(env->GetFogSphere());
                  mat->addChild(env->GetCloudPlane()->GetOSGNode());
               }
            }
         }
         cam->addChild(mat);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetTextureWaveAmpOverLength( float ampOverLength )
      {
         mTextureWaveAmpOverLength = ampOverLength;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::ActorUpdate(const dtGame::Message& msg)
      {
        const dtGame::ActorUpdateMessage &updateMessage = static_cast<const dtGame::ActorUpdateMessage&>(msg);
        dtGame::GameActorProxy* proxy = GetGameActorProxy().GetGameManager()->FindGameActorById(updateMessage.GetAboutActorId());
        if (proxy == NULL) // Could be deleted or not fully created from partial
        {
           return;
        }

        OceanDataActor* oceanDataActor = NULL;
        proxy->GetActor(oceanDataActor);

        if(oceanDataActor != NULL)
        {
           int seaState = oceanDataActor->GetSeaState();
           float waveDirection = oceanDataActor->GetWaveDirectionPrimary();
           float waveHeight = oceanDataActor->GetWaveHeightSignificant();

           double lat = oceanDataActor->GetLatitude();
           double llong = oceanDataActor->GetLongitude();

           OceanDataUpdate(lat, llong, seaState, waveDirection, waveHeight);
        }

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::ActorCreated(const dtGame::Message& msg)
      {
         if(!mOceanDataProxy.valid())
         {
            dtGame::GameActorProxy* proxy = NULL;
            proxy = GetGameActorProxy().GetGameManager()->FindGameActorById(msg.GetAboutActorId());

            if(proxy != NULL && proxy->GetActorType() == *EntityActorRegistry::OCEAN_DATA_ACTOR_TYPE)
            {
               mOceanDataProxy = proxy;
               GetGameActorProxy().RegisterForMessagesAboutOtherActor(dtGame::MessageType::INFO_ACTOR_UPDATED, msg.GetAboutActorId(), WaterGridActorProxy::INVOKABLE_ACTOR_UPDATE);
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::OceanDataUpdate(double lat, double llong, int seaState, float waveDir, float waveHeight)
      {
         //mModForDirectionInDegrees = waveDir;

         unsigned seaStateNumber = 0;
         if(waveHeight <= 0.1f)
         {
            seaStateNumber = 0;
         }
         else if(waveHeight <= 0.2f)
         {
            seaStateNumber = 1;
         }
         else if(waveHeight <= 0.6f)
         {
            seaStateNumber = 2;
         }
         else if(waveHeight <= 1.0f)
         {
            seaStateNumber = 3;
         }
         else if(waveHeight <= 2.0f)
         {
            seaStateNumber = 4;
         }
         else if(waveHeight <= 3.0f)
         {
            seaStateNumber = 5;
         }
         else if(waveHeight <= 4.0f)
         {
            seaStateNumber = 6;
         }
         else if(waveHeight <= 5.5f)
         {
            seaStateNumber = 7;
         }
         else if(waveHeight <= 7.0f)
         {
            seaStateNumber = 8;
         }
         else if(waveHeight <= 9.0f)
         {
            seaStateNumber = 9;
         }
         else if(waveHeight <= 11.5f)
         {
            seaStateNumber = 10;
         }
         else if(waveHeight <= 14.0f)
         {
            seaStateNumber = 11;
         }
         else 
         {
            seaStateNumber = 12;
         }

         std::cout << "Ocean Update- Sea state: " << seaStateNumber << ", Wave Dir: " << waveDir << ", WaveHeight: " << waveHeight << ", Lat: " << lat << ", Long: " << llong << std::endl;

         SetSeaStateByNumber(seaStateNumber);
         SetPrimaryWaveDirection(180.0f + waveDir);
      }

      ///////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::UpdateWaterPlaneFOV(dtCore::Camera& pCamera, const osg::Matrix& inverseMVP)
      {
         osg::StateSet* ss = pCamera.GetOSGCamera()->getOrCreateStateSet();
         osg::Uniform* waterFOVUniform = ss->getOrCreateUniform("waterPlaneFOV", osg::Uniform::FLOAT);

         dtCore::Transform xform;
         osg::Vec3d waterCenter, screenPosOut, camPos;
         pCamera.GetTransform(xform);
         xform.GetTranslation(camPos);

         float waterHeight = GetWaterHeight();

         waterCenter.set(camPos.x(), camPos.y(), waterHeight);

         if(pCamera.ConvertWorldCoordinateToScreenCoordinate(waterCenter, screenPosOut))
         {
            waterFOVUniform->set(180.0f);
         }
         else
         {
            int width = int(pCamera.GetOSGCamera()->getViewport()->width());
            int height = int(pCamera.GetOSGCamera()->getViewport()->height());

            osg::Vec3 bottomLeft, bottomRight, topLeft, topRight;
            osg::Vec3 bottomLeftIntersect, bottomRightIntersect, topLeftIntersect, topRightIntersect;

            ComputeRay(0, 0, inverseMVP, bottomLeft);
            ComputeRay(width, 0, inverseMVP, bottomRight);
            ComputeRay(0, height, inverseMVP, topLeft);
            ComputeRay(width, height, inverseMVP, topRight);

            osg::Vec4 waterPlane(0.0, 0.0, 1.0, -waterHeight);

            bool bool_bottomLeftIntersect = IntersectRayPlane(waterPlane, camPos, bottomLeft, bottomLeftIntersect);
            bool bool_bottomRightIntersect = IntersectRayPlane(waterPlane, camPos, bottomRight, bottomRightIntersect);
            bool bool_topLeftIntersect = IntersectRayPlane(waterPlane, camPos, topLeft, topLeftIntersect);
            bool bool_topRightIntersect = IntersectRayPlane(waterPlane, camPos, topRight, topRightIntersect);

            if(bool_bottomLeftIntersect)
            {
               bottomLeft = bottomLeftIntersect;
               bottomLeft = bottomLeft - waterCenter;
               bottomLeft.normalize();
            }
            if(bool_bottomRightIntersect)
            {
               bottomRight = bottomRightIntersect;
               bottomRight = bottomRight - waterCenter;
               bottomRight.normalize();
            }
            if(bool_topLeftIntersect)
            {
               topLeft = topLeftIntersect;
               topLeft = topLeft - waterCenter;
               topLeft.normalize();
            }
            if(bool_topRightIntersect)
            {
               topRight = topRightIntersect;
               topRight = topRight - waterCenter;
               topRight.normalize();
            }

            float maxAngle1 = 0.0, maxAngle2 = 0.0, maxAngle3 = 0.0, maxAngle4 = 0.0, maxAngle5 = 0.0, maxAngle6 = 0.0;

            maxAngle1 = GetAngleBetweenVectors(bottomLeft, bottomRight);

            maxAngle2 = GetAngleBetweenVectors(bottomLeft, topLeft);

            maxAngle3 = GetAngleBetweenVectors(bottomRight, topRight);

            maxAngle4 = GetAngleBetweenVectors(topLeft, topRight);

            maxAngle5 = GetAngleBetweenVectors(bottomRight, topLeft);

            maxAngle6 = GetAngleBetweenVectors(bottomLeft, topRight);

            //take the max of the six angles
            float angle = dtUtil::Max(dtUtil::Max(maxAngle5, maxAngle6), dtUtil::Max(dtUtil::Max(maxAngle1, maxAngle2), dtUtil::Max(maxAngle3, maxAngle4)));
            angle = osg::RadiansToDegrees(angle);
            angle /= 2.0f;
            waterFOVUniform->set(angle);

            //std::cout << "Water Angle " << angle << std::endl;
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::ComputeRay(int x, int y, const osg::Matrix& inverseMVPS, osg::Vec3& rayToFill )
      {
         osg::Vec3 rayFrom, rayTo;

         rayFrom = osg::Vec3(x, y, 0.0f) * inverseMVPS;
         rayTo = osg::Vec3(x, y, 1.0f) * inverseMVPS;

         rayToFill = rayTo - rayFrom;
         rayToFill.normalize();
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      bool WaterGridActor::IntersectRayPlane( const osg::Vec4& plane, const osg::Vec3& rayOrigin, const osg::Vec3& rayDirection, osg::Vec3& intersectPoint )
      {
         osg::Vec3 norm(plane.x(), plane.y(), plane.z());
         float denominator = norm * rayDirection;

         //the normal is near parallel
         if(fabs(denominator) > FLT_EPSILON)
         {
            float t = -(norm * rayOrigin + plane.w());
            t /= denominator;
            intersectPoint = rayOrigin + (rayDirection * t);
            return t > 0;
         }

         //std::cout << "No Intersect" << std::endl;
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetAngleBetweenVectors( const osg::Vec3& v1, const osg::Vec3& v2 )
      {
         return std::acos(v1 * v2);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::UpdateScreenSpaceWaterHeight(dtCore::Camera& pCamera, const osg::Matrix& MVP)
      {
         osg::StateSet* ss = pCamera.GetOSGCamera()->getOrCreateStateSet();
         osg::Uniform* waterHeightScreenSpace = ss->getOrCreateUniform("waterHeightScreenSpace", osg::Uniform::FLOAT_VEC3);

         dtCore::Transform xform;
         osg::Vec3d waterCenter, screenPosOut, camPos;
         osg::Vec3 right, up, forward;
         pCamera.GetTransform(xform);
         xform.GetTranslation(camPos);
         xform.GetOrientation(right, up, forward);

         double vfov, aspect, nearClip, farClip;
         pCamera.GetPerspectiveParams(vfov, aspect, nearClip, farClip);

         osg::Vec3 posOnFarPlane = camPos + (forward * farClip);
         posOnFarPlane[2] = GetWaterHeight();

         posOnFarPlane = posOnFarPlane * MVP;

         waterHeightScreenSpace->set(posOnFarPlane);

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetWaveLengthMultiplier( float f )
      {
         mModForWaveLength = f;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetForWaveLengthMultiplier() const
      {
         return mModForWaveLength;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetSpeedMultiplier( float f )
      {
         mModForSpeed = f;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetSpeedMultiplier() const
      {
         return mModForSpeed;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetSteepnessMultiplier( float f )
      {
         mModForSteepness = f;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetSteepnessMultiplier() const
      {
         return mModForSteepness;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetAmplitudeMultiplier( float f )
      {
         mModForAmplitude = f;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetAmplitudeMultiplier() const
      {
         return mModForAmplitude;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetPrimaryWaveDirection( float degrees )
      {
         mModForDirectionInDegrees = degrees;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      float WaterGridActor::GetPrimaryWaveDirection() const
      {
         return mModForDirectionInDegrees;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetSeaStateByNumber(unsigned force)
      {
         int numWaves = 16;

         float waveLenMod = 2.0f;
         float ampMod = 0.25f;

         if(force == 0)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_0);
            //AddRandomizedWaves(0.667f, 0.12f, 0.5f, 1.0f, numWaves);
            AddRandomizedWaves(waveLenMod * 3.167f, ampMod * 00.16f, 1.0f, 2.5f, numWaves);
         }
         else if(force == 1)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_1);
            //AddRandomizedWaves(3.167f, 00.16f, 1.0f, 2.5f, numWaves);
            AddRandomizedWaves(waveLenMod * 8.667f, ampMod * 00.667f, 1.5f, 5.0f, numWaves);
         }
         else if(force == 2)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_2);
            //AddRandomizedWaves(8.667f, 00.667f, 1.5f, 5.0f, numWaves);
            AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 0.9667f, 1.75f, 6.0f, numWaves);
         }
         else if(force == 3)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_3);
            //AddRandomizedWaves(16.667f, 01.1667f, 2.0f, 6.5f, numWaves);
            AddRandomizedWaves(waveLenMod * 16.667f, ampMod * 1.1667f, 2.0f, 6.5f, numWaves);
         }
         else if(force == 4)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_4);
            //AddRandomizedWaves(26.66f, 2.0f, 2.5f, 8.5f, numWaves);
            AddRandomizedWaves(waveLenMod * 3.167f, ampMod * 00.16f, 1.0f, 2.5f, 4);
            AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 0.9667f, 1.75f, 6.0f, 4);
            AddRandomizedWaves(waveLenMod * 16.667f, ampMod * 1.1667f, 2.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 20.66f, ampMod * 1.5f, 4.5f, 8.5f, 4);
         }
         else if(force == 5)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_5);
            //AddRandomizedWaves(43.33, 02.667, 3.0f, 10.0f, numWaves);
            //AddRandomizedWaves(waveLenMod * 43.33, ampMod * 2.667, 6.0f, 10.0f, numWaves);
            
            AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 0.9667f, 1.75f, 6.0f, 4);
            AddRandomizedWaves(waveLenMod * 16.667f, ampMod * 1.1667f, 2.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 20.66f, ampMod * 1.5f, 4.5f, 8.5f, 4);
            AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 2.0f, 4.5f, 8.5f, 4);

         }
         else if(force == 6)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_6);
            //AddRandomizedWaves(73.33, 6.0f, 4.0f, 13.0f, numWaves);
            //AddRandomizedWaves(waveLenMod * 73.33, ampMod * 4.0f, 7.0f, 13.0f, numWaves);


            AddRandomizedWaves(waveLenMod * 11.667f, ampMod * 1.1471f, 2.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 2.14f, 4.5f, 8.5f, 4);
            AddRandomizedWaves(waveLenMod * 43.33, ampMod * 3.341f, 5.0f, 9.0f, 2);
            AddRandomizedWaves(waveLenMod * 65.33, ampMod * 5.4667, 6.0f, 8.0f, 2);
            AddRandomizedWaves(waveLenMod * 95.667, ampMod * 6.133f, 7.5f, 12.0f, 4);
         }
         else if(force == 7)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_7);
            //AddRandomizedWaves(133.33, 10.67f, 5.5f, 17.0f, numWaves);
            //AddRandomizedWaves(waveLenMod * 123.33, ampMod * 8.67f, 8.5f, 14.0f, numWaves);

            AddRandomizedWaves(waveLenMod * 9.667f, ampMod * 1.167f, 3.0f, 4.5f, 4);
            AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 3.14f, 4.5f, 8.5f, 4);
            AddRandomizedWaves(waveLenMod * 63.33, ampMod * 4.341f, 7.0f, 13.0f, 2);
            AddRandomizedWaves(waveLenMod * 85.33, ampMod * 6.667, 6.0f, 10.0f, 2);
            AddRandomizedWaves(waveLenMod * 109.667, ampMod * 8.33f, 10.5f, 20.0f, 4);
         }
         else if(force == 8)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_8);
            //AddRandomizedWaves(216.667, 17.33f, 7.5f, 23.0f, numWaves);
            //AddRandomizedWaves(waveLenMod * 166.667, ampMod * 10.633f, 9.5f, 16.5f, numWaves);

            //AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 1.1667f, 2.0f, 6.5f, 4);
            //AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 3.14f, 4.5f, 8.5f, 4);
            //AddRandomizedWaves(waveLenMod * 63.33, ampMod * 6.341f, 7.0f, 13.0f, 4);
            //AddRandomizedWaves(waveLenMod * 133.33, ampMod * 12.667, 6.0f, 10.0f, 2);
            //AddRandomizedWaves(waveLenMod * 149.667, ampMod * 14.33f, 10.5f, 20.0f, 2);

            AddRandomizedWaves(waveLenMod * 9.667f, ampMod * 1.167f, 3.0f, 4.5f, 2);
            AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 3.14f, 4.5f, 8.5f, 4);
            AddRandomizedWaves(waveLenMod * 63.33, ampMod * 5.341f, 7.0f, 13.0f, 2);
            AddRandomizedWaves(waveLenMod * 85.33, ampMod * 7.667, 6.0f, 10.0f, 4);
            AddRandomizedWaves(waveLenMod * 109.667, ampMod * 10.33f, 10.5f, 20.0f, 4);
         }
         else if(force == 9)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_9);
            //AddRandomizedWaves(waveLenMod * 178.667, ampMod * 11.33f, 10.5f, 18.0f, numWaves);
            AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 1.1667f, 3.0f, 6.5f, 2);
            AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 3.14f, 4.5f, 8.5f, 4);
            AddRandomizedWaves(waveLenMod * 63.33, ampMod * 5.341f, 7.0f, 13.0f, 2);
            AddRandomizedWaves(waveLenMod * 83.33, ampMod * 9.667, 6.0f, 10.0f, 4);
            AddRandomizedWaves(waveLenMod * 129.667, ampMod * 12.33f, 10.5f, 20.0f, 4);

            

         }
         else if(force == 10)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_10);
            //AddRandomizedWaves(waveLenMod * 199.667, ampMod * 12.33f, 10.5f, 20.0f, numWaves);

            AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 2.1667f, 4.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 26.66f, ampMod * 4.14f, 3.5f, 5.5f, 4);
            AddRandomizedWaves(waveLenMod * 73.33, ampMod * 8.341f, 5.0f, 11.0f, 4);
            AddRandomizedWaves(waveLenMod * 133.33, ampMod * 16.667, 8.0f, 12.0f, 2);
            AddRandomizedWaves(waveLenMod * 329.667, ampMod * 28.33f, 10.5f, 20.0f, 2);
         }
         else if(force == 11)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_11);

            AddRandomizedWaves(waveLenMod * 12.667f, ampMod * 1.1667f, 2.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 22.667f, ampMod * 4.1667f, 2.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 53.33, ampMod * 8.0f, 7.0f, 13.0f, 4);
            AddRandomizedWaves(waveLenMod * 143.33, ampMod * 16.667, 6.0f, 10.0f, 2);
            AddRandomizedWaves(waveLenMod * 399.667, ampMod * 64.33f, 10.5f, 20.0f, 2);

         }
         else if(force == 12)
         {
            ClearWaves();
            SetSeaState(SeaState::SeaState_12);            

            AddRandomizedWaves(waveLenMod * 38.667f, ampMod * 2.1667f, 2.0f, 6.5f, 3);
            AddRandomizedWaves(waveLenMod * 46.667f, ampMod * 4.1667f, 2.0f, 6.5f, 4);
            AddRandomizedWaves(waveLenMod * 93.33, ampMod * 6.667, 6.0f, 10.0f, 4);
            AddRandomizedWaves(waveLenMod * 123.33, ampMod * 12.67f, 8.5f, 14.0f, 2);
            AddRandomizedWaves(waveLenMod * 545.667, ampMod * 125.33f, 33.5f, 33.0f, 3);
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::AddRandomizedWaves(float meanWaveLength, float meanAmplitude, float minPeriod, float maxPeriod, unsigned numWaves)
      {
         float waveLenStart = meanWaveLength / 2.0f;
         float waveLenIncrement = meanWaveLength / numWaves;

         float ampStart = meanAmplitude / 2.0f;
         float ampIncrement = meanAmplitude / numWaves;

         for(unsigned i = 0; i < numWaves; ++i, waveLenStart += waveLenIncrement, ampStart += ampIncrement)
         {
            Wave w;
            w.mWaveLength = waveLenStart;
            w.mAmplitude = ampStart;
            w.mSpeed = dtUtil::RandFloat(minPeriod, maxPeriod);
            w.mSteepness = 1.0f;
            w.mDirectionInDegrees = dtUtil::RandFloat(-10.3333f, 10.3333f);
            AddWave(w); // 0
         }

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActor::SetSeaState(WaterGridActor::SeaState& seaState)
      {
         //mSeaStateEnum = &seaState;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      WaterGridActor::SeaState& WaterGridActor::GetSeaState() const
      {
         return *mSeaStateEnum;
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      //WATER GRID PROXY
      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
       const dtUtil::RefString WaterGridActorProxy::CLASSNAME("WaterGridActor");
       const dtUtil::RefString WaterGridActorProxy::PROPERTY_CHOPPINESS("Choppiness");
       const dtUtil::RefString WaterGridActorProxy::PROPERTY_WATER_COLOR("Water Color");
       const dtUtil::RefString WaterGridActorProxy::INVOKABLE_MAP_LOADED("Map Loaded");
       const dtUtil::RefString WaterGridActorProxy::INVOKABLE_ACTOR_CREATED("Actor Created");
       const dtUtil::RefString WaterGridActorProxy::INVOKABLE_ACTOR_UPDATE("Actor Updated");

      WaterGridActorProxy::WaterGridActorProxy()
      {
         SetClassName(WaterGridActorProxy::CLASSNAME);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      WaterGridActorProxy::~WaterGridActorProxy()
      {

      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActorProxy::CreateActor()
      {
         WaterGridActor* actor = new WaterGridActor(*this);
         SetActor(*actor);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActorProxy::BuildInvokables()
      {
         BaseClass::BuildInvokables();

         WaterGridActor* wga = NULL;
         GetActor(wga);

         AddInvokable(*new dtGame::Invokable(INVOKABLE_MAP_LOADED,
            dtUtil::MakeFunctor(&WaterGridActor::Init, *wga)));

         AddInvokable(*new dtGame::Invokable(INVOKABLE_ACTOR_UPDATE,
                  dtUtil::MakeFunctor(&WaterGridActor::ActorUpdate, *wga)));

         AddInvokable(*new dtGame::Invokable(INVOKABLE_ACTOR_CREATED,
                  dtUtil::MakeFunctor(&WaterGridActor::ActorCreated, *wga)));
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActorProxy::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         RegisterForMessages(dtGame::MessageType::TICK_LOCAL,
            dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);

         RegisterForMessages(dtGame::MessageType::INFO_MAP_LOADED,
            INVOKABLE_MAP_LOADED);

         RegisterForMessages(dtGame::MessageType::INFO_ACTOR_CREATED,
            INVOKABLE_ACTOR_CREATED);
      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      void WaterGridActorProxy::BuildPropertyMap()
      {
         const std::string GROUPNAME = "WaterGrid";

         BaseClass::BuildPropertyMap();

         WaterGridActor* actor = NULL;
         GetActor(actor);

         AddProperty(new dtDAL::ColorRgbaActorProperty(PROPERTY_WATER_COLOR, PROPERTY_WATER_COLOR,
            dtDAL::ColorRgbaActorProperty::SetFuncType(actor,&WaterGridActor::SetWaterColor),
            dtDAL::ColorRgbaActorProperty::GetFuncType(actor,&WaterGridActor::GetWaterColor),
            "Sets the color of the water.", GROUPNAME));

         AddProperty(new dtDAL::EnumActorProperty<WaterGridActor::ChoppinessSettings>(PROPERTY_CHOPPINESS, PROPERTY_CHOPPINESS,
            dtDAL::EnumActorProperty<WaterGridActor::ChoppinessSettings>::SetFuncType(actor, &WaterGridActor::SetChoppiness),
            dtDAL::EnumActorProperty<WaterGridActor::ChoppinessSettings>::GetFuncType(actor, &WaterGridActor::GetChoppiness),
            "Sets the choppiness for the water.", GROUPNAME));

         AddProperty(new dtDAL::EnumActorProperty<WaterGridActor::SeaState>("Sea State", "Sea State",
            dtDAL::EnumActorProperty<WaterGridActor::SeaState>::SetFuncType(actor, &WaterGridActor::SetSeaState),
            dtDAL::EnumActorProperty<WaterGridActor::SeaState>::GetFuncType(actor, &WaterGridActor::GetSeaState),
            "The Sea State number based on the Beaufort wind force scale.", GROUPNAME));

      }

      /////////////////////////////////////////////////////////////////////////////
      const dtDAL::ActorProxy::RenderMode& WaterGridActorProxy::GetRenderMode()
      {
         return dtDAL::ActorProxy::RenderMode::DRAW_BILLBOARD_ICON;
      }


      /////////////////////////////////////////////////////////////////////////////
      dtDAL::ActorProxyIcon* WaterGridActorProxy::GetBillBoardIcon()
      {
         if(!mBillBoardIcon.valid())
         {
            dtDAL::ActorProxyIcon::ActorProxyIconConfig config;
            config.mForwardVector = false;
            config.mUpVector = false;
            config.mScale = 1.0;

            mBillBoardIcon = new dtDAL::ActorProxyIcon(dtDAL::ActorProxyIcon::IMAGE_BILLBOARD_STATICMESH, config);
         }

         return mBillBoardIcon.get();
      }

   }//namespace Actors
}//namespace SimCore

