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

#include <osg/Drawable>

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

         typedef unsigned ShapeRecordID;

         enum Shape{SPHERE, BOX, CAPSULE, CONE, CYLINDER};

         enum RenderMode{SIMPLE_SHAPE_GEOMETRY, PARTICLE_VOLUMES, VOLUMETRIC_RAYCASTING};

         struct SIMCORE_EXPORT ShapeVolumeRecord: public osg::Referenced
         {
            ShapeVolumeRecord();

            ShapeRecordID mId;
            Shape mShape;
            RenderMode mRenderMode;

            bool mDeleteMe;
            bool mAutoDeleteAfterMaxTime; //using this flag will set the volume to be automatically removed after the number of seconds
            float mMaxTime;
            bool mFadeOut;      
            float mFadeOutTime; 
            float mIntensity;

            osg::Vec4 mColor;
            osg::Vec3 mPosition;
            osg::Vec3 mRadius;
            osg::Vec3 mVelocity;
            osg::Matrix mTransform;

            bool mAutoDeleteOnTargetNull;
            dtCore::ObserverPtr<osg::MatrixTransform> mParentNode; 
            dtCore::ObserverPtr<dtCore::Transformable> mTarget; 

            static OpenThreads::Atomic mCounter;
         };

         //////////////////////////////////////////////////////////////////////////
         //ShapeVolumeArray
         typedef std::vector<dtCore::RefPtr<ShapeVolumeRecord> > ShapeVolumeArray;

         /// a helper class to do our rendering for us
         class SIMCORE_EXPORT VolumeRenderingDrawable: public osg::Drawable
         {
         public:
            typedef osg::Drawable BaseClass;

            META_Object(dtAI, VolumeRenderingDrawable);
            VolumeRenderingDrawable();
            VolumeRenderingDrawable(const VolumeRenderingDrawable& bd, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);         

            /*virtual*/ osg::BoundingBox computeBound() const;

            /*virtual*/ void drawImplementation(osg::RenderInfo& renderInfo) const;

         };

         public:

         /// Constructor
         VolumeRenderingComponent(const std::string& name = DEFAULT_NAME);

         void Init(unsigned maxRenderedVolumes);

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

      private:

         void Tick(float dt);  

         unsigned mMaxRenderedVolumes;
    
         dtCore::RefPtr<osg::Group> mRootNode;
         
         ShapeVolumeArray mVolumes;


      };
   }// namespace Components
}//namespace SimCore

#endif //DELTA_VOLUMERENDERINGCOMPONENT
