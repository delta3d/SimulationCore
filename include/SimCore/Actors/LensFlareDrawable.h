/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine 
 * Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
 * Bradley Anderegg 
 */
#ifndef DELTA_LENS_FLARE_DRAWABLE
#define DELTA_LENS_FLARE_DRAWABLE

#include <SimCore/Export.h>

#include <dtUtil/refstring.h>
#include <dtCore/observerptr.h>
#include <dtCore/deltadrawable.h>
#include <dtCore/cameradrawcallback.h>
#include <dtGame/gmcomponent.h>
#include <dtGame/gamemanager.h>
#include <dtCore/batchisector.h>

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Uniform>

#include <vector>
#include <map>

namespace osg
{
   class Camera;
}

namespace dtPhysics
{
   class RayCast;
}

namespace dtCore
{
   class Camera;
}

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT LensFlareDrawable : public dtCore::DeltaDrawable
      {
         public:
            
            class LensFlareOSGDrawable;
            class SIMCORE_EXPORT LensFlareUpdateCallback: public dtCore::CameraDrawCallback
            {
            public:
               LensFlareUpdateCallback(LensFlareOSGDrawable* lensFlareReference, const dtCore::Camera& camera);
               void operator () (const dtCore::Camera& camera, osg::RenderInfo& renderInfo) const;
            
               bool mAttach;
               dtCore::ObserverPtr<LensFlareOSGDrawable> mLensFlare;
            };


            class SIMCORE_EXPORT LensFlareOSGDrawable: public osg::Geometry
            {
               public:
                  META_Object(osg::Geometry, LensFlareOSGDrawable);
                  LensFlareOSGDrawable(const LensFlareOSGDrawable& bd, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);                  

                  LensFlareOSGDrawable();

                  void SetLightPos(const osg::Vec3d& pos);

                  /*virtual*/ void drawImplementation(osg::RenderInfo& renderInfo) const;

                   void Init();
                   void LoadTextures();
                   float CalculateEffectScale(osg::Camera* cam, bool visible) const;
                   void EnableTextureState(osg::RenderInfo& renderInfo) const;
                   void InitTexture(const std::string& filename, osg::Texture2D* ptr, const std::string& uniformName, unsigned index);


                   bool mUseRayCast;
                   bool mUsePhysicsRaycast;
                   osg::Vec3d mLightPos;

                   dtCore::RefPtr<osg::Uniform> mLightPosUniform;
                   dtCore::RefPtr<osg::Uniform> mEffectRadiusUniform;

                   dtCore::RefPtr<osg::Texture2D> mSoftGlow;
                   dtCore::RefPtr<osg::Texture2D> mHardGlow;
                   dtCore::RefPtr<osg::Texture2D> mStreaks;

                   struct FadeParams
                   {
                      FadeParams()
                         : mVisible(false)
                         , mRayCastVisible(false)
                         , mFadeDirection(0)
                         , mFadeRate(1.0f)
                         , mLastTickTime(0.0)
                         , mFadeCurrent(0.0f)
                         , mLastDepthTest(false) 
                         , mDepthTestPos()
                         , mDepthTexCoords()
                         , mDepthTexCoordUniform()
                      {
                      }

                                            
                      bool mVisible, mRayCastVisible;
                      int mFadeDirection;
                      float mFadeRate, mLastTickTime, mFadeCurrent;
                      
                      bool mLastDepthTest;
                      osg::Vec4d mDepthTestPos;
                      osg::Vec2 mDepthTexCoords;
                      dtCore::RefPtr<osg::Uniform> mDepthTexCoordUniform;
                      dtCore::RefPtr<osg::Texture2D> mDepthTexture;
                      dtCore::RefPtr<osg::Texture2D> mColorTexture;
                   };

                   //this requires a map so the fading can work with each camera
                   //if there are multiple
                   typedef std::map<osg::Camera*, FadeParams> CameraFadeMap;
                   mutable CameraFadeMap mFadeMap;
               };  


            static const dtUtil::RefString TEXTURE_LENSFLARE_SOFT_GLOW;
            static const dtUtil::RefString TEXTURE_LENSFLARE_HARD_GLOW;
            static const dtUtil::RefString TEXTURE_LENSFLARE_STREAKS;
            
            /// Constructor
            LensFlareDrawable();

            void Init(dtGame::GameManager& gm);

            //raycast technique is currently commented out
            //has been replaced by depth test technique 

            //void SetUseRayCast(bool useRayCast, bool usePhysicsRaycast);
            //void GetUseRayCast(bool& useRayCast, bool& usePhysicsRaycast) const;

            void Update(const osg::Vec3& lightPos);

            void OnSystem(const dtUtil::RefString& phase, double deltaSim, double deltaReal)
;
            
            void UpdateView(dtCore::Camera& pCamera);

            /*virtual*/ osg::Node* GetOSGNode();
            /*virtual*/ const osg::Node* GetOSGNode() const;

            /**
            *  Setting the terrain node is used in the raycast mode and allows a faster intersection test
            *     raycast technique is currently commented out, has been replaced by depth test technique 
            */
            //void SetTerrainNode(dtCore::DeltaDrawable* terrainRoot);
            //dtCore::DeltaDrawable* GetTerrainNode();

            /*
            * This is an OSG traversal mask and only used for the non physics raycast
            *    raycast technique is currently commented out, has been replaced by depth test technique 
            */
            //void SetRayCastTraversalMask(int mask);
            //int GetRayCastTraversalMask() const;

         protected:

            /// Destructor
            virtual ~LensFlareDrawable();

         private:           

            bool HasCamera(dtCore::Camera* cam) const;
            
            //int mTraversalMask;
            dtCore::RefPtr<dtCore::BatchIsector> mIsector; 
            dtCore::RefPtr<osg::Group> mNode;
            dtCore::RefPtr<LensFlareOSGDrawable> mLensFlareDrawable;
            dtCore::ObserverPtr<dtCore::DeltaDrawable> mTerrainNode;

            typedef std::pair<dtPhysics::RayCast, dtCore::ObserverPtr<osg::Camera> > RayCastCameraPair;
            typedef std::vector<RayCastCameraPair> RayCastArray;
            RayCastArray mRayCastArray;

            typedef std::pair<dtCore::RefPtr<LensFlareUpdateCallback>, dtCore::ObserverPtr<dtCore::Camera> > UpdateCallbackCameraPair;
            typedef std::vector<UpdateCallbackCameraPair> CameraArray;
            CameraArray mCurrentCameras;
      };
   }
}

#endif //DELTA_LENS_FLARE_DRAWABLE
