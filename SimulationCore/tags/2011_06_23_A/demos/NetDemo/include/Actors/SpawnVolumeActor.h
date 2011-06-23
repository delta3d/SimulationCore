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
#ifndef NETDEMO_SPAWNVOLUMEACTOR
#define NETDEMO_SPAWNVOLUMEACTOR

#include <DemoExport.h>
#include <dtGame/gameactor.h>
#include <dtDAL/propertycontainer.h>

#include <AIUtility.h>
#include <Actors/EnemyDescriptionActor.h>


namespace NetDemo
{
   class SpawnVolumeActorProxy;

   class NETDEMO_EXPORT SpawnVolumeActor : public dtGame::GameActor
   {
   public:

      typedef dtGame::GameActor BaseClass;

      /// Constructor
      SpawnVolumeActor (SpawnVolumeActorProxy &proxy);

      osg::BoundingBox GetBoundingBox();
      osg::Vec3 GetRandomPointInVolume();
      bool IsPointInVolume(osg::Vec3 pos);

      virtual void OnEnteredWorld();
      virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

   protected:
      /// Destructor
      virtual ~SpawnVolumeActor();


      // Private vars
   private:

  
   };


   class NETDEMO_EXPORT SpawnVolumeActorProxy : public dtGame::GameActorProxy
   {
   public:
      typedef dtGame::GameActorProxy BaseClass;
      typedef dtCore::UniqueId EnemyDescriptionId;
      typedef std::vector<EnemyDescriptionId> EnemyDescriptionArray;

      // public strings for the properties
      static const dtUtil::RefString PROP_ENEMY_ID;
      static const dtUtil::RefString PROP_ENEMY_ARRAY;

      SpawnVolumeActorProxy();
      /*virtual*/ void BuildPropertyMap();

      /*virtual*/ const dtDAL::BaseActorObject::RenderMode& GetRenderMode();

      EnemyDescriptionActor* FindEnemyById(const EnemyDescriptionId& id);
      EnemyDescriptionActor* FindEnemyByName(const std::string& name);
      EnemyDescriptionActor* LookupProxyById(const EnemyDescriptionId& id);

   protected:
      virtual ~SpawnVolumeActorProxy();
      void CreateActor();
      virtual void OnEnteredWorld();

      /**
      * Array actor property functors.
      */
      void SetEnemyGroupProperty(EnemyDescriptionId value);
      EnemyDescriptionId GetEnemyGroupProperty();

      void EnemyArraySetIndex(int index);
      EnemyDescriptionId EnemyArrayGetDefault();
      EnemyDescriptionArray EnemyArrayGetValue();
      void EnemyArraySetValue(const EnemyDescriptionArray& value);
      

   private:
      int mEnemyArrayIndex;
      EnemyDescriptionArray mEnemyArray;
   };
}

#endif //NETDEMO_SPAWNVOLUMEACTOR
