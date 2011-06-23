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
#include <dtGame/gmcomponent.h>

#include <osg/Texture2D>
#include <osg/Drawable>
#include <osg/MatrixTransform>

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

            class SIMCORE_EXPORT LensFlareOSGDrawable: public osg::Drawable
            {
               public:
                  META_Object(osg::Drawable, LensFlareOSGDrawable);
                  LensFlareOSGDrawable(const LensFlareOSGDrawable& bd, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY)
                  {
                     
                  }

                  LensFlareOSGDrawable();

                  void SetLightPos(const osg::Vec3d& pos){mLightPos = pos;}

                  /*virtual*/ void drawImplementation(osg::RenderInfo& renderInfo) const;


                   void LoadTextures();
                   float CalculateEffectScale(osg::Camera* cam, bool visible) const;
                   void EnableTextureState(osg::RenderInfo& renderInfo) const;
                   void InitTexture(const std::string& filename, osg::Texture2D* ptr);
                   void RenderQuad(const osg::Vec4& rgba, const osg::Vec2& point, float scale) const;


                   bool mUseRayCast;
                   osg::Vec3d mLightPos;

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
                      {
                      }

                      bool mVisible, mRayCastVisible;
                      int mFadeDirection;
                      float mFadeRate, mLastTickTime, mFadeCurrent;
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

            void Init();
            void SetUseRayCast(bool b);
            bool GetUseRayCast() const;

            void Update(const osg::Vec3& lightPos);

            void OnMessage(dtCore::Base::MessageData* data);
            
            void UpdateView(dtCore::Camera& pCamera);

            /*virtual*/ osg::Node* GetOSGNode();
            /*virtual*/ const osg::Node* GetOSGNode() const;

         protected:

            /// Destructor
            virtual ~LensFlareDrawable();

         private:           

            dtCore::RefPtr<osg::MatrixTransform> mNode;
            dtCore::RefPtr<LensFlareOSGDrawable> mLensFlareDrawable;

            typedef std::pair<dtPhysics::RayCast, dtCore::ObserverPtr<osg::Camera> > RayCastCameraPair;
            typedef std::vector<RayCastCameraPair> RayCastArray;
            RayCastArray mRayCastArray;
      };
   }
}

#endif //DELTA_LENS_FLARE_DRAWABLE
