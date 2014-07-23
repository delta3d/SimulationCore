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
      /**
       * The volume rendering component allows the user to create simple shapes (sphere, box, etc.) using the internal ShapeVolumeRecord class
       * which simulate volumes of fog or clouds (or by overriding the shader any sort of volume) by randomly placing particles within the volume
       * that are billboards rendered as soft particles.
       *
       * Here is an example of using this component
       *   
       *   SimCore::Components::VolumeRenderingComponent* vrc = NULL;
       *   GetGameActorProxy().GetGameManager()->GetComponentByName(SimCore::Components::VolumeRenderingComponent::DEFAULT_NAME, vrc); 
       *
       *  if(vrc != NULL)
       *  {
       *     SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord* svr = new SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord();
       *     svr->mPosition.set(0.0f, 0.0f, -18.0f);
       *     svr->mColor.set(1.0f, 1.0f, 1.0f, 1.0f);
       *     svr->mShapeType = SimCore::Components::VolumeRenderingComponent::CONE;
       *     svr->mRadius.set(10.0f, 20.0f, 0.0f);
       *     svr->mNumParticles = 50;
       *     svr->mParticleRadius = 15.0f;
       *     svr->mVelocity = 0.5f;
       *     svr->mDensity = 0.08f;
       *     svr->mTarget = GetDrawable();
       *     svr->mAutoDeleteOnTargetNull = true;
       *
       *     vrc->CreateShapeVolume(svr);
       *   }
       *
       */

      class SIMCORE_EXPORT VolumeRenderingComponent : public dtGame::GMComponent
      {
      public:

         typedef dtGame::GMComponent BaseClass;
         //the name of the component used to look up by name on the game manager
         static const std::string DEFAULT_NAME;

         //these are the shader uniforms used in the default volumetric shaders
         static const std::string VOLUME_PARTICLE_POS_UNIFORM;
         static const std::string VOLUME_PARTICLE_COLOR_UNIFORM;
         static const std::string VOLUME_PARTICLE_INTENSITY_UNIFORM; 
         static const std::string VOLUME_PARTICLE_DENSITY_UNIFORM; 
         static const std::string VOLUME_PARTICLE_VELOCITY_UNIFORM;
         static const std::string VOLUME_PARTICLE_RADIUS_UNIFORM;
         static const std::string VOLUME_PARTICLE_NOISE_SCALE_UNIFORM;
         //static const std::string CAMERA_LINEAR_DEPTH_UNIFORM;


         //a typedef for the id used by the shape volume record class
         typedef unsigned ShapeRecordId;

         //the shape type enumeration, set on the shape volume record class
         enum Shape{SPHERE, ELLIPSOID, BOX, CONE, CYLINDER};

         //the render mode is either SIMPLE_SHAPE_GEOMETRY or PARTICLE_VOLUME
         //the simple shape mode just uses an osg shape drawable and is mostly used for debugging
         //the particle volume randomly creates soft particles to fill the volume
         enum RenderMode{SIMPLE_SHAPE_GEOMETRY, PARTICLE_VOLUME};//, VOLUMETRIC_RAYCASTING};

         class ShapeVolumeRecord;
         /// a helper class to do our rendering for us
         class SIMCORE_EXPORT ParticleVolumeDrawable: public osg::Geometry
         {
         public:
            typedef osg::Geometry BaseClass;
            typedef std::vector<osg::Vec4> PointList; 

            META_Object(SimCore, ParticleVolumeDrawable);
            ParticleVolumeDrawable();
            ParticleVolumeDrawable(const ParticleVolumeDrawable& bd, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);         

            void Init(unsigned mNumParticles, ShapeVolumeRecord* svr);

            const osg::Vec4& GetPointLocation(unsigned i) const;
            void SetPointLocation(unsigned i, const osg::Vec4& newOffset);

            float GetParticleRadius() const;
            void SetParticleRadius(float f);

            unsigned GetNumParticles() const;
            
            //temporarily we will not support the primitive functor
            virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }
            //not sure if these will give desired results
            virtual bool supports(const osg::Drawable::AttributeFunctor&) const { return false; }
            virtual bool supports(const osg::Drawable::ConstAttributeFunctor&) const { return false; }
            virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return false; }


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

            //the shape type determines the shape and dictates how the radius params are interpreted
            //the shape types available are SPHERE, ELLIPSOID, BOX, CONE, and CYLINDER
            Shape mShapeType;
            
            //the render mode is either SIMPLE_SHAPE_GEOMETRY or PARTICLE_VOLUME
            //the simple shape mode just uses an osg shape drawable and is mostly used for debugging
            //the particle volume randomly creates soft particles to fill the volume
            RenderMode mRenderMode;

            //set the dirty flag to true if the color, radius, velocity, or intensity changes
            bool mDirtyParams;

            //this flag is used internally to delete the shape volume on the next tick but can be set externally as well
            bool mDeleteMe;

            //setting this flag to true will automatically remove the volume (after fade out time if it has one) if the 
            //mTarget is deleted and the observer pointer is set to null
            bool mAutoDeleteOnTargetNull;

            //using this flag will set the volume to be automatically removed after the number of seconds
            bool mAutoDeleteAfterMaxTime; 

            //use this value to set the max amount of time this shape volume will live 
            float mMaxTime;

            //setting this flag will fade out the voltume when the max time is reached or parent is NULL and the 
            //delete flag is set, the amount of time to fade out is determined by the parameter below
            bool mFadeOut;

            //see above- to use this fade out time you must set mFadeOut (above) to true, this param is used to
            //scale the intensity or alpha value down to zero over the amount of time
            float mFadeOutTime;

            //the intensity is simply a multiplier on the alpha value of the effect, it is used to allow the effect
            //to fade in or out as necessary
            float mIntensity;

            //the density of the effect is used to determine the opacity using an exponential fog equation
            float mDensity;

            //the velocity is a simple scalar value used to animate the noise, or turbulence of the effect
            //typically a value between 0.25 and 0.5 will be sufficient
            float mVelocity;

            //the noise scale value is used to scale the texture coordinates when looking up into a noise texture
            //a large number will make the effect more granular but patterns will be more apparent
            //a number between 0.5-1.5 is typical
            float mNoiseScale;

            //this parameter determines the number of particles to use for the effect, currently the max number is 150
            //but can easily be changed by editing the constant max number in the shader
            unsigned mNumParticles;

            //the particle radius is used to determine how big each particle should be, typically around 5-20
            //if you are unsure how large the particles should be to fill the volume you can call
            //ComputeParticleRadius() on the component and it will compute a particle radius provided that the num particles
            //and volume radius params are set, also make sure to add the volume before computing the radius otherwise the uniforms
            //will not be set
            float mParticleRadius;

            //the color of the particles, the 4th component is used as a multiplier for the final alpha
            //note that the alpha provided here, the intensity, and the density are all multiplied in to get the
            //final alpha, so an alpha value of 1 here is usually sufficient
            osg::Vec4 mColor;

            //this is the position of the center of the volume with respect to its parent transformable, if there is no transformable parent
            //then this position is in world space
            osg::Vec3 mPosition;
            
            //the radius is used to determine the dimension and is interpreted differently for each shape
            //SPHERE- mRadius[0] = outer radius, mRadius[1] = inner radius
            //ELLIPSOID- mRadius is equal to the extents in each direction currently the same as a BOX
            //BOX- mRadius is the extents in each direction
            //CONE- mRadius[0] = radius at end of cone, mRadius[1] = height
            //CYLINDER- mRadius[0] = radius, mRadius[1] = height
            osg::Vec3 mRadius;
            
            //this matrix is used internally and will be overwritten each frame, not intended for use
            //to apply a matrix use a transformable by setting the mTarget
            osg::Matrix mTransform;

            //the shader group and name refer to a specific shader in the ShaderManager shaderdef.xml file and
            //is only necessary to override the default shaders applied which simulate a cloud/fog effect
            std::string mShaderGroup;
            std::string mShaderName;

            //the mParentNode is used internally as a parent to the drawable, do not try to set this
            dtCore::ObserverPtr<osg::MatrixTransform> mParentNode; 

            //the mTarget is used like a parent to the effect, setting this will allow your volume to move with
            //another node, if mAutoDeleteOnTargetNull is true then this volume will be removed when the target is set to null
            dtCore::ObserverPtr<osg::Node> mTarget;

            //this is the osg shape and is not used unless the render mode is SIMPLE_SHAPE_GEOMETRY
            //this is created for you by the component
            dtCore::RefPtr<osg::Shape> mShape;

            //this variables is only for the particle volume types and holds onto the actual osg drawable 
            //used to render the volumes
            //this is created for you by the component
            dtCore::RefPtr<ParticleVolumeDrawable> mParticleDrawable;

            //use this to get the static counted id
            ShapeRecordId GetId() const;

            private:
               //this is the id used to look up the volumes, alternatively one can hold onto a ref or observer pointer
               ShapeRecordId mId;

               //this is a static counter used internally to give each volume an id
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

         //this function is used to add a shape volume to the component using a pre-created shape volume record class
         //see the shape record volume class above to figure out what params to set
         ShapeRecordId CreateShapeVolume(dtCore::RefPtr<ShapeVolumeRecord> svr);

         //just a helper class that will create a shape volume for you with minimal parameters
         ShapeRecordId CreateStaticShapeVolume(Shape s, const osg::Vec4& color, const osg::Vec3& center, const osg::Vec3& radius);

         //used to remove a shape volume from the system
         void RemoveShapeVolume(ShapeVolumeRecord* svr);

         //used to find a shape volume by Id
         ShapeVolumeRecord* FindShapeVolumeById(ShapeRecordId id);

         //a volume can be moved by setting the mPosition and updatingt the dirty flag, but this is a helper
         //function which will move the volume for you
         void MoveVolume(ShapeVolumeRecord& svr, const osg::Vec3& moveBy);

         //this function should be used to resize the radius of a specific volume, useful for a growing effect if called per frame
         //the simple moving shape actor uses this when dead reckoning the size
         void ExpandVolume(ShapeVolumeRecord& svr, const osg::Vec3& expandBy);

         //a helper function which uses the number of particles and total volume of shape to compute a particle radius
         //this function is useful if you are using the expand function above and need a computed particle radius
         void ComputeParticleRadius(ShapeVolumeRecord& svr);

         //a function used internally to create a linear pre depth pass
         void CreateDepthPrePass(unsigned width, unsigned height);

         //an internal function made public for unit testing
         void TimeoutAndDeleteVolumes(float dt);
      protected:

         /// Destructor
         virtual ~VolumeRenderingComponent();
         void CleanUp();
         
         void UpdateVolumes(float dt);
         ShapeVolumeRecord* FindVolume(ShapeRecordId id);

         void TransformAndSortVolumes();
         void SetPosition(ShapeVolumeRecord* dl);

         void CreateDrawable(ShapeVolumeRecord& newShape);
         void RemoveDrawable(ShapeVolumeRecord& svr);
         void CreateSimpleShape(ShapeVolumeRecord& newShape);
         void CreateParticleVolume(ShapeVolumeRecord& newShape);
         void CreateShape(ShapeVolumeRecord& newShape);

         void AssignParticleVolumeShader(ShapeVolumeRecord& svr, osg::Geode& g);
         void AssignParticleVolumeUniforms(ShapeVolumeRecord& newShape);
         void UpdateUniforms();
         void SetUniformData(ShapeVolumeRecord& s);
      private:

         void Tick(float dt);  
         void CreateNoiseTexture();

         bool mInitialized;
         unsigned mMaxParticlesPerDrawable;
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
