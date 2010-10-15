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


#include <SimCore/Components/VolumeRenderingComponent.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <dtCore/transform.h>
#include <dtCore/scene.h>

#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/actorupdatemessage.h>

#include <SimCore/Actors/SimpleMovingShapeActor.h>

#include <SimCore/Actors/EntityActorRegistry.h>

#include <osg/Group>
#include <osg/Geode>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/RenderInfo>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>

namespace SimCore
{
   namespace Components
   {

   const std::string VolumeRenderingComponent::DEFAULT_NAME = "VolumeRenderingComponent";

   /////////////////////////////////////////////////////////////
   //useful functors
   /////////////////////////////////////////////////////////////
   struct findVolumeById
   {
      findVolumeById(VolumeRenderingComponent::ShapeRecordID id): mId(id){}

      template<class T>
      bool operator()(T ptr)
      {
         return ptr->mId == mId;
      }
   private:

      VolumeRenderingComponent::ShapeRecordID mId;
   };

   struct findVolumeByUniqueId
   {
      findVolumeByUniqueId(const dtCore::UniqueId& id): mId(id){}

      template<class T>
      bool operator()(T ptr)
      {
         bool result = false; 

         if(ptr->mTarget.valid())
         {
            result = (ptr->mTarget->GetUniqueId() == mId);
         }
         
         return result;
      }
   private:

      dtCore::UniqueId mId;
   };

   struct removeVolumesFunc
   {
      template<class T>
      bool operator()(T ptr)
      {
         return ptr->mDeleteMe;
      }
   };

   /////////////////////////////////////////////////////////////
   //ShapeVolumeRecord
   /////////////////////////////////////////////////////////////
   OpenThreads::Atomic VolumeRenderingComponent::ShapeVolumeRecord::mCounter = 0;
   
   //////////////////////////////////////////////////////////////////////////
   VolumeRenderingComponent::ShapeVolumeRecord::ShapeVolumeRecord()
      : mId(mCounter)
      , mShape(VolumeRenderingComponent::SPHERE)
      , mDeleteMe(false)
      , mAutoDeleteAfterMaxTime(false)
      , mMaxTime(0.0f)
      , mFadeOut(false)
      , mFadeOutTime(0.0f)
      , mIntensity(1.0f)
      , mColor(0.25f, 0.25f, 0.8f, 0.5f)
      , mPosition(0.0f, 0.0f, 0.0f)
      , mRadius(0.0f, 0.0f, 0.0f)
      , mVelocity(0.0f, 0.0f, 0.0f)
      , mTransform()
      , mTarget(NULL)
   {
      ++mCounter;
      mTransform.makeIdentity();

   }

   /////////////////////////////////////////////////////////////
   VolumeRenderingComponent::VolumeRenderingComponent(const std::string& name)
   : BaseClass(name) 
   , mRenderMode(SIMPLE_SHAPE_GEOMETRY)
   , mMaxRenderedVolumes(500)
   , mRootNode(new osg::Group())
   , mVolumes()
   {

   }

   /////////////////////////////////////////////////////////////
   VolumeRenderingComponent::~VolumeRenderingComponent()
   {

   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::CleanUp()
   {
      mVolumes.clear();
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::OnAddedToGM()
   {
      BaseClass::OnAddedToGM();

      GetGameManager()->GetScene().GetSceneNode()->addChild(mRootNode.get());
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::OnRemovedFromGM()
   {
      CleanUp();
   }

   /////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::Init(RenderMode r, unsigned maxRenderedVolumes)
   {
      mRenderMode = r;
      mMaxRenderedVolumes = maxRenderedVolumes;
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RegisterActor(Actors::SimpleMovingShapeActorProxy& newActor)
   {
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::UnregisterActor(const dtCore::UniqueId& actorID)
   {
      
   }
   
   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RemoveShapeVolume(ShapeRecordID id)
   {
      mVolumes.erase(std::remove_if(mVolumes.begin(), mVolumes.end(), findVolumeById(id)), mVolumes.end());
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeRecordID VolumeRenderingComponent::CreateShapeVolume(dtCore::RefPtr<ShapeVolumeRecord> svr)
   {
      if (svr != NULL)
      {
         mVolumes.push_back(svr);
         CreateDrawable(*svr);
         return svr->mId;
      }
      else
      {
         LOG_ERROR("Attempting to add a ShapeVolumeRecord that is NULL to the Volume Rendering Component");
         return 0;
      }
   }

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeRecordID VolumeRenderingComponent::CreateStaticShapeVolume(Shape s, const osg::Vec4& color, const osg::Vec3& center, const osg::Vec3& radius)
   {
      dtCore::RefPtr<ShapeVolumeRecord> svr = new ShapeVolumeRecord();
      svr->mShape = s;
      svr->mColor = color;
      svr->mPosition = center;
      svr->mRadius = radius;
      return CreateShapeVolume(svr);
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindShapeVolumeForActor(const dtCore::UniqueId& actorID)
   {
      ShapeVolumeArray::iterator iter = std::find_if(mVolumes.begin(), mVolumes.end(), findVolumeByUniqueId(actorID));
      if(iter != mVolumes.end())
      {
         return (*iter).get();
      }

      return NULL;
   }

   ////////////////////////////////////////////////////////////////////////// 
   /*void VolumeRenderingComponent::FindAllShapeVolumesForActor( const dtCore::UniqueId& actorID, std::vector<ShapeVolumeRecord*> pContainerToFill )
   {

   }*/

   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::ProcessMessage(const dtGame::Message& message)
   {
      
      if(message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
      {
         float dt = float(static_cast<const dtGame::TickMessage&>(message).GetDeltaSimTime());
         Tick(dt);
      }
      else if (dtGame::MessageType::INFO_ACTOR_UPDATED == message.GetMessageType())
      {
         const dtGame::ActorUpdateMessage& updateMessage = static_cast<const dtGame::ActorUpdateMessage&> (message);
         
         dtDAL::ActorProxy* actor = GetGameManager()->FindActorById(updateMessage.GetAboutActorId());
         if (actor == NULL)
         {
            return;
         }

         const dtDAL::ActorType& type = actor->GetActorType();

         if (type == *SimCore::Actors::EntityActorRegistry::ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE)
         {
            Actors::SimpleMovingShapeActorProxy* proxy =
               static_cast<Actors::SimpleMovingShapeActorProxy*>(actor);
            RegisterActor(*proxy);
         }

      }
      else if (message.GetMessageType() == dtGame::MessageType::INFO_ACTOR_DELETED)
      {
         // TODO Write unit tests for the delete message.
         UnregisterActor(message.GetAboutActorId());
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_LOADED)
      {
      }
      else if(message.GetMessageType() == dtGame::MessageType::INFO_MAP_UNLOADED)
      {
         CleanUp();
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::Tick(float dt)
   {
      UpdateVolumes(dt);
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::UpdateVolumes(float dt)
   {
      TimeoutAndDeleteVolumes(dt);
      TransformAndSortVolumes();
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::TimeoutAndDeleteVolumes(float dt)
   {
      ShapeVolumeArray::iterator iter = mVolumes.begin();
      ShapeVolumeArray::iterator endIter = mVolumes.end();

      for(;iter != endIter; ++iter)
      {
         ShapeVolumeRecord* svr = (*iter).get();

         if((svr->mAutoDeleteOnTargetNull && !svr->mTarget.valid()) || (!svr->mAutoDeleteAfterMaxTime && svr->mAutoDeleteOnTargetNull))
         {
            if(!svr->mTarget.valid())
            {
               if(svr->mFadeOut)
               {
                  //by setting this to false we will continue into a fade out
                  svr->mAutoDeleteOnTargetNull = false;
                  //by setting this one false we ensure we will begin fading out next frame
                  svr->mAutoDeleteAfterMaxTime = false;
               }
               else
               {
                  svr->mDeleteMe = true;
                  //std::cout << "Auto delete on NULL Ptr" << std::ensvr;
                  continue;
               }
            }
         }
         else if(svr->mAutoDeleteAfterMaxTime)
         {
            svr->mMaxTime -= dt;

            if(svr->mMaxTime <= 0.0f)
            {
               if(svr->mFadeOut)
               {
                  //by setting this to false we will continue into a fade out
                  svr->mAutoDeleteAfterMaxTime = false;
                  //by setting this one false we ensure we will begin fading out next frame
                  svr->mAutoDeleteOnTargetNull = false;
               }
               else
               {
                  svr->mDeleteMe = true;
                  //std::cout << "Auto delete on Max Time" << std::ensvr;
                  continue;
               }
            }
         }
         else if(svr->mFadeOut)
         {
            svr->mIntensity -= (dt / svr->mFadeOutTime);
            if(svr->mIntensity <= 0.0f)
            {
               svr->mDeleteMe = true;
               //std::cout << "Auto delete on fade out" << std::ensvr;
               continue;
            }
         }
      }

      //now remove all flagged Volumes, note this is actually faster because we only have a single deallocation for N Volumes
      mVolumes.erase(std::remove_if(mVolumes.begin(), mVolumes.end(), removeVolumesFunc()), mVolumes.end());

   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::TransformAndSortVolumes()
   {
      ShapeVolumeArray::iterator iter = mVolumes.begin();
      ShapeVolumeArray::iterator endIter = mVolumes.end();

      for(;iter != endIter; ++iter)
      {
         ShapeVolumeRecord* svr = (*iter).get();
         if(svr->mTarget.valid())
         {
            //update the Volume's position
            SetPosition(svr);
         }
      }

      ////update uniforms by finding the closest Volumes to the camera
      //dtCore::Transform trans;
      //GetGameManager()->GetApplication().GetCamera()->GetTransform(trans);
      //osg::Vec3 pos;
      //trans.GetTranslation(pos);
      ////sort the Volumes, though a heap may be more efficient here, we will sort so that we can combine Volumes later
      //std::sort(mVolumes.begin(), mVolumes.end(), funcCompareVolumes(pos));
   }

   ///////////////////////////////////////////////////////////////////////////////////////////////////
   void VolumeRenderingComponent::SetPosition(ShapeVolumeRecord* svr)
   {
      if(svr != NULL && svr->mTarget.valid())
      {
         dtCore::Transform xform;
         svr->mTarget->GetTransform(xform);
         xform.Move(svr->mPosition);

         osg::Matrix mat;
         xform.Get(mat);
         svr->mParentNode->setMatrix(mat);
      }
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::RemoveVolume(ShapeVolumeArray::iterator iter)
   {
      mVolumes.erase(iter);
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindVolume(ShapeRecordID id)
   {
      ShapeVolumeArray::iterator iter = std::find_if(mVolumes.begin(), mVolumes.end(), findVolumeById(id));
      if(iter != mVolumes.end())
      {
         return (*iter).get();
      }

      return NULL;
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::ShapeVolumeRecord* VolumeRenderingComponent::FindShapeVolumeById(ShapeRecordID id)
   {
      return FindVolume(id);
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::CreateDrawable(ShapeVolumeRecord& newShape)
   {
      switch(mRenderMode)
      {
      case SIMPLE_SHAPE_GEOMETRY:
         {
            dtCore::RefPtr<osg::Geode> g = new osg::Geode();
            dtCore::RefPtr<osg::MatrixTransform> matTrans = new osg::MatrixTransform();
            newShape.mParentNode = matTrans.get();
            newShape.mParentNode->addChild(g.get());
            //dtCore::RefPtr<osg::Material> material = new osg::Material;
            //material->setTransparency(osg::Material::FRONT, newShape.mColor[3]);
            //material->setAlpha(osg::Material::FRONT, newShape.mColor[3]);

            osg::StateSet* ss = g->getOrCreateStateSet();
            ss->setMode(GL_BLEND, osg::StateAttribute::ON);


            osg::BlendFunc* blendFunc = new osg::BlendFunc();
            blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
            ss->setAttributeAndModes(blendFunc);
            ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);


            switch(newShape.mShape)
            {
               case SPHERE:
                  {
                     dtCore::RefPtr<osg::Sphere> s = new osg::Sphere(newShape.mPosition, newShape.mRadius[0]);
                     dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(s);
                     shapeDrawable->setColor(newShape.mColor);
                     g->addDrawable(shapeDrawable);
                  }
                  break;
               case BOX:
                  {
                     dtCore::RefPtr<osg::Box> s = new osg::Box(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1], newShape.mRadius[2]);
                     dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(s);
                     shapeDrawable->setColor(newShape.mColor);
                     g->addDrawable(shapeDrawable);
                  }
                  break;
               case CAPSULE:
                  {
                     dtCore::RefPtr<osg::Capsule> s = new osg::Capsule(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1]);
                     dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(s);
                     shapeDrawable->setColor(newShape.mColor);
                     g->addDrawable(shapeDrawable);
                  }
                  break;
               case CYLINDER:
                  {
                     dtCore::RefPtr<osg::Cylinder> s = new osg::Cylinder(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1]);
                     dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(s);
                     shapeDrawable->setColor(newShape.mColor);
                     g->addDrawable(shapeDrawable);
                  }
                  break;
               case CONE:
                  {
                     dtCore::RefPtr<osg::Cone> s = new osg::Cone(newShape.mPosition, newShape.mRadius[0], newShape.mRadius[1]);
                     dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(s);
                     shapeDrawable->setColor(newShape.mColor);
                     g->addDrawable(shapeDrawable);
                  }
                  break;
               default:
                  break;
            }


            mRootNode->addChild(newShape.mParentNode.get());
         }
         break;

      default:
         break;
      }
   }


   ////////////////////////////////////////////////////////////////////////// 
   //VolumeRenderingDrawable
   VolumeRenderingComponent::VolumeRenderingDrawable::VolumeRenderingDrawable()
   {
      setUseDisplayList(false);
   }

   ////////////////////////////////////////////////////////////////////////// 
   VolumeRenderingComponent::VolumeRenderingDrawable::VolumeRenderingDrawable(const VolumeRenderingDrawable& bd, const osg::CopyOp& copyop)
   {
   }

   ////////////////////////////////////////////////////////////////////////// 
   void VolumeRenderingComponent::VolumeRenderingDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
   {

   }

   ////////////////////////////////////////////////////////////////////////// 
   osg::BoundingBox VolumeRenderingComponent::VolumeRenderingDrawable::computeBound() const
   {
      return BaseClass::computeBound();
   }


   }//namespace Components
}//namespace SimCore

