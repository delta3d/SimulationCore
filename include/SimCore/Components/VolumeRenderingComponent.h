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
#ifndef DELTA_VOLUMERENDERINGCOMPONENT
#define DELTA_VOLUMERENDERINGCOMPONENT

#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>
#include <dtCore/observerptr.h>
#include <SimCore/Actors/SimpleMovingShapeActor.h>
//#include <SimCore/MultiPassNode.h>

#include <dtCore/camera.h>

#include <osg/Geometry>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/Texture3D>

#include <dtCore/view.h>

#include <vector>

namespace osg
{
   class RenderInfo;
}

namespace SimCore
{
   namespace Components
   {

      class SIMCORE_EXPORT VolumeRenderingComponent : public dtGame::GMComponent
      {
      public:

         typedef dtGame::GMComponent BaseClass;
         static const std::string DEFAULT_NAME;
         static const std::string VOLUME_PARTICLE_POS_UNIFORM;
         static const std::string VOLUME_PARTICLE_COLOR_UNIFORM;
         static const std::string VOLUME_PARTICLE_INTENSITY_UNIFORM; 
         static const std::string VOLUME_PARTICLE_VELOCITY_UNIFORM;
         static const std::string VOLUME_PARTICLE_RADIUS_UNIFORM;
         //static const std::string CAMERA_LINEAR_DEPTH_UNIFORM;


         typedef unsigned ShapeRecordID;

         enum Shape{SPHERE, ELLIPSOID, BOX, CAPSULE, CONE, CYLINDER};

         enum RenderMode{SIMPLE_SHAPE_GEOMETRY, PARTICLE_VOLUME};//, VOLUMETRIC_RAYCASTING};

         class ShapeVolumeRecord;
         /// a helper class to do our rendering for us
         class SIMCORE_EXPORT ParticleVolumeDrawable: public osg::Geometry
         {
         public:
            typedef osg::Geometry BaseClass;
            typedef std::vector<osg::Vec3> PointList; 

            META_Object(SimCore, ParticleVolumeDrawable);
            ParticleVolumeDrawable();
            ParticleVolumeDrawable(const ParticleVolumeDrawable& bd, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);         

            void Init(unsigned mNumParticles, ShapeVolumeRecord* svr);

            const osg::Vec3& GetPointLocation(unsigned i) const;

            unsigned GetNumParticles() const;

            /*virtual*/ osg::BoundingBox computeBound() const;

            /*virtual*/ void drawImplementation(osg::RenderInfo& renderInfo) const;

         private:
            void CreateBillboards(unsigned numParticles, float width, float height);
            void CreateRandomPointsInVolume(Shape s, float numPoints, const osg::Vec3& center, const osg::Vec3& radius, PointList& pointArrayToFill);

            float mParticleRadius;
            unsigned mNumParticles;
            PointList mPoints;
         };


         class SIMCORE_EXPORT ShapeVolumeRecord: public osg::Referenced
         {
           public:
            ShapeVolumeRecord();

            ShapeRecordID mId;
            Shape mShapeType;
            RenderMode mRenderMode;

            //set the dirty flag to true if the color, radius, velocity, or intensity changes
            bool mDirtyParams;
            bool mDeleteMe;
            bool mAutoDeleteAfterMaxTime; //using this flag will set the volume to be automatically removed after the number of seconds
            float mMaxTime;
            bool mFadeOut;      
            float mFadeOutTime; 
            float mIntensity;

            //these variables are only for the particle volume types
            unsigned mNumParticles;
            float mParticleRadius;

            osg::Vec4 mColor;
            osg::Vec3 mPosition;
            osg::Vec3 mRadius;
            osg::Vec3 mVelocity;
            osg::Matrix mTransform;

            bool mAutoDeleteOnTargetNull;
            dtCore::ObserverPtr<osg::MatrixTransform> mParentNode; 
            dtCore::ObserverPtr<dtCore::Transformable> mTarget;
            dtCore::RefPtr<osg::Shape> mShape;

            //this variables is only for the particle volume types
            dtCore::RefPtr<ParticleVolumeDrawable> mParticleDrawable;

            static OpenThreads::Atomic mCounter;
         };

         //////////////////////////////////////////////////////////////////////////
         //ShapeVolumeArray
         typedef std::vector<dtCore::RefPtr<ShapeVolumeRecord> > ShapeVolumeArray;

         public:

         /// Constructor
         VolumeRenderingComponent(const std::string& name = DEFAULT_NAME);

         void Init();

         /**
         * Handles incoming messages
         */
         virtual void ProcessMessage(const dtGame::Message& message);

         /**
         * Called when this component is added to the game manager
         */
         virtual void OnAddedToGM();

         virtual void OnRemovedFromGM();

         ShapeRecordID CreateShapeVolume(dtCore::RefPtr<ShapeVolumeRecord> svr);
         ShapeRecordID CreateStaticShapeVolume(Shape s, const osg::Vec4& color, const osg::Vec3& center, const osg::Vec3& radius);

         void RemoveShapeVolume(ShapeRecordID id);

         void RegisterActor(Actors::SimpleMovingShapeActorProxy& newActor);
         void UnregisterActor(const dtCore::UniqueId& actorID);

         ShapeVolumeRecord* FindShapeVolumeById(ShapeRecordID id);
         ShapeVolumeRecord* FindShapeVolumeForActor(const dtCore::UniqueId& actorID);
         //void FindAllShapeVolumesForActor(const dtCore::UniqueId& actorID, std::vector<ShapeVolumeRecord*> pContainerToFill);

         void CreateDepthPrePass(const std::string& textureName, unsigned width, unsigned height);

      protected:

         /// Destructor
         virtual ~VolumeRenderingComponent();
         void CleanUp();
         
         void UpdateVolumes(float dt);
         void RemoveVolume(ShapeVolumeArray::iterator iter);
         ShapeVolumeRecord* FindVolume(ShapeRecordID id);

         void TimeoutAndDeleteVolumes(float dt);
         void TransformAndSortVolumes();
         void SetPosition(ShapeVolumeRecord* dl);

         void CreateDrawable(ShapeVolumeRecord& newShape);
         void CreateSimpleShape(ShapeVolumeRecord& newShape);
         void CreateParticleVolume(ShapeVolumeRecord& newShape);
         void CreateShape(ShapeVolumeRecord& newShape);

         void AssignParticleVolumeShader(ParticleVolumeDrawable& pvd, osg::Geode& g);
         void AssignParticleVolumeUniforms(ShapeVolumeRecord& newShape);
         void UpdateUniforms();
         void SetUniformData(ShapeVolumeRecord& s);

      private:

         void Tick(float dt);  
         void CreateNoiseTexture();

         dtCore::RefPtr<osg::Group> mRootNode;

         dtCore::RefPtr<dtCore::Camera> mDepthCamera;
         dtCore::RefPtr<osg::Camera> mDebugCamera;
         dtCore::RefPtr<dtCore::View> mDepthView;

         dtCore::RefPtr<osg::Texture2D> mDepthTexture;
         dtCore::RefPtr<osg::Uniform> mDepthTextureUniform;

         dtCore::RefPtr<osg::Texture3D> mNoiseTexture;
         dtCore::RefPtr<osg::Uniform> mNoiseTextureUniform;

         ShapeVolumeArray mVolumes;

      };
   }// namespace Components
}//namespace SimCore

#endif //DELTA_VOLUMERENDERINGCOMPONENT
