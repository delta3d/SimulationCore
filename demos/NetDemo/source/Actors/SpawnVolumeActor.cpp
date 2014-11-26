/* Net Demo - SpawnVolumeActor (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* Bradley Anderegg
*/


#include <dtUtil/mswin.h>
#include <Actors/SpawnVolumeActor.h>

#include <dtUtil/functor.h>
#include <dtCore/arrayactorproperty.h>
#include <dtCore/enginepropertytypes.h>
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/gamemanager.h>
#include <dtGame/actorupdatemessage.h>
#include <dtUtil/mathdefines.h>


namespace NetDemo
{

   ///////////////////////////////////////////////////////////////////////////////////
   SpawnVolumeActor::SpawnVolumeActor(SpawnVolumeActorProxy &proxy)
      : BaseClass(proxy)
   {
      SetName("SpawnVolumeActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   SpawnVolumeActor::~SpawnVolumeActor()
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActor::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      // LOCAL - means server, means we have control
      if(!IsRemote())
      {
      }
   }

   //////////////////////////////////////////////////////////////////////
   void SpawnVolumeActor::OnTickLocal(const dtGame::TickMessage& tickMessage)
   {
      BaseClass::OnTickLocal(tickMessage);
      //tickMessage.GetDeltaSimTime()
   }

   //////////////////////////////////////////////////////////////////////////////
   osg::BoundingBox SpawnVolumeActor::GetBoundingBox()
   {
      osg::BoundingBox bb;

      osg::Vec3 dimensions(10.0f, 10.0f, 10.0f);

      float halfWidth  = dimensions[0] * 0.5f;
      float halfLength = dimensions[1] * 0.5f;
      float halfHeight = dimensions[2] * 0.5f;

      bb._min.set(-halfWidth, -halfLength, -halfHeight);
      bb._max.set(halfWidth, halfLength, halfHeight);

      return bb;
   }

   //////////////////////////////////////////////////////////////////////////////
   osg::Vec3 SpawnVolumeActor::GetRandomPointInVolume()
   {
      osg::Vec3 pos;

      dtCore::Transform trans;
      GetTransform(trans);
      trans.GetTranslation(pos);
      osg::BoundingBox bb = GetBoundingBox();
      pos[0] += dtUtil::RandFloat(bb._min[0], bb._max[0]);
      pos[1] += dtUtil::RandFloat(bb._min[1], bb._max[1]);
      pos[2] += dtUtil::RandFloat(bb._min[2], bb._max[2]);

      return pos;
   }

   //////////////////////////////////////////////////////////////////////////////
   bool SpawnVolumeActor::IsPointInVolume(osg::Vec3 pos)
   {
      dtCore::Transform transform;
      GetTransform(transform);

      osg::Vec3 translation;
      transform.GetTranslation(translation);

      osg::Matrix rotation;
      transform.GetRotation(rotation);

      rotation.invert(rotation);

      pos -= translation;
      pos = pos * rotation;

      osg::BoundingBox bb = GetBoundingBox();

      if (pos.x() > bb._min.x() && pos.x() < bb._max.x() &&
         pos.y() > bb._min.y() && pos.y() < bb._max.y() &&
         pos.z() > bb._min.z() && pos.z() < bb._max.z())
      {
         return true;
      }
      return false;
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   SpawnVolumeActorProxy::SpawnVolumeActorProxy()
   {
      SetClassName("SpawnVolumeActor");
   }

   const dtUtil::RefString SpawnVolumeActorProxy::PROP_ENEMY_ID("EnemyID");
   const dtUtil::RefString SpawnVolumeActorProxy::PROP_ENEMY_ARRAY("EnemyArray");


   ///////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActorProxy::BuildPropertyMap()
   {
      const std::string& GROUP = "Spawn Volume";

      BaseClass::BuildPropertyMap();

      dtCore::ActorIDActorProperty* actorProp = new dtCore::ActorIDActorProperty(
         *this, "Enemy", "Enemy",
         dtCore::ActorIDActorProperty::SetFuncType(this, &SpawnVolumeActorProxy::SetEnemyGroupProperty),
         dtCore::ActorIDActorProperty::GetFuncType(this, &SpawnVolumeActorProxy::GetEnemyGroupProperty),
         PROP_ENEMY_ID, "A UniqueId to a EnemyDescriptionActor", GROUP);


      AddProperty(new dtCore::ArrayActorProperty<EnemyDescriptionId>(
         PROP_ENEMY_ARRAY, "List of enemies to spawn.", "List of enemies to spawn.",
         dtCore::ArrayActorProperty<EnemyDescriptionId>::SetIndexFuncType(this, &SpawnVolumeActorProxy::EnemyArraySetIndex),
         dtCore::ArrayActorProperty<EnemyDescriptionId>::GetDefaultFuncType(this, &SpawnVolumeActorProxy::EnemyArrayGetDefault),
         dtCore::ArrayActorProperty<EnemyDescriptionId>::GetArrayFuncType(this, &SpawnVolumeActorProxy::EnemyArrayGetValue),
         dtCore::ArrayActorProperty<EnemyDescriptionId>::SetArrayFuncType(this, &SpawnVolumeActorProxy::EnemyArraySetValue),
         actorProp, GROUP));

   }

   ///////////////////////////////////////////////////////////////////////////////////
   SpawnVolumeActorProxy::~SpawnVolumeActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActorProxy::CreateDrawable()
   {
      SpawnVolumeActor* newActor = new SpawnVolumeActor(*this);
      SetDrawable(*newActor);
   }


   ////////////////////////////////////////////////////////////////////////////////
   const dtCore::ActorProxy::RenderMode& SpawnVolumeActorProxy::GetRenderMode()
   {
      return dtCore::ActorProxy::RenderMode::DRAW_ACTOR_AND_BILLBOARD_ICON;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActorProxy::OnEnteredWorld()
   {
      BaseClass::OnEnteredWorld();

      if (!IsRemote())
      {
         RegisterForMessages(dtGame::MessageType::TICK_LOCAL, dtGame::GameActorProxy::TICK_LOCAL_INVOKABLE);
      }
   }

   //////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActorProxy::SetEnemyGroupProperty(EnemyDescriptionId value)
   {
      if (mEnemyArrayIndex < (int)mEnemyArray.size())
      {
         mEnemyArray[mEnemyArrayIndex] = value;
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   SpawnVolumeActorProxy::EnemyDescriptionId SpawnVolumeActorProxy::GetEnemyGroupProperty()
   {
      if (mEnemyArrayIndex < (int)mEnemyArray.size())
      {
         return mEnemyArray[mEnemyArrayIndex];
      }
      return dtCore::UniqueId("");
   }


   ////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActorProxy::EnemyArraySetIndex(int index)
   {
      mEnemyArrayIndex = index;
   }

   ////////////////////////////////////////////////////////////////////////////////
   SpawnVolumeActorProxy::EnemyDescriptionId SpawnVolumeActorProxy::EnemyArrayGetDefault()
   {
      return dtCore::UniqueId("");
   }

   ////////////////////////////////////////////////////////////////////////////////
   SpawnVolumeActorProxy::EnemyDescriptionArray SpawnVolumeActorProxy::EnemyArrayGetValue()
   {
      return mEnemyArray;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void SpawnVolumeActorProxy::EnemyArraySetValue(const EnemyDescriptionArray& value)
   {
      //just remove them and re-add them-.
      mEnemyArray.clear();

      // Now add all the tasks.
      for (int index = 0; index < (int)value.size(); index++)
      {
         mEnemyArray.push_back(value[index]);
      }
   }

   //////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor *SpawnVolumeActorProxy::FindEnemyByName(const std::string& name)
   {
      EnemyDescriptionArray::iterator iter = mEnemyArray.begin();
      EnemyDescriptionArray::iterator iterEnd = mEnemyArray.end();

      for (; iter != iterEnd; ++iter)
      {
         EnemyDescriptionActor* proxy = LookupProxyById((*iter));
         if(proxy->GetName() == name)
            return proxy;
      }
      return NULL;
   }

   //////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor* SpawnVolumeActorProxy::FindEnemyById(const EnemyDescriptionId& id)
   {
      EnemyDescriptionArray::iterator iter = mEnemyArray.begin();
      EnemyDescriptionArray::iterator iterEnd = mEnemyArray.end();

      for (; iter != iterEnd; ++iter)
      {
         if ((*iter) == id)
         {
            return LookupProxyById((*iter));
         }
      }
      return NULL;
   }

   ////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor* SpawnVolumeActorProxy::LookupProxyById(const EnemyDescriptionId& id)
   {
      EnemyDescriptionActor* proxy = NULL;
      if (IsInGM())
      {
         GetGameManager()->FindGameActorById(id, proxy);
      }
      return proxy;
   }


} // namespace
//#endif
