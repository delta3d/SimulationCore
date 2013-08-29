/* Net Demo - EnemyDescriptionActor (.cpp & .h) - Using 'The MIT License'
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
#ifndef NETDEMO_ENEMYDESCRIPTIONACTOR
#define NETDEMO_ENEMYDESCRIPTIONACTOR

#include <DemoExport.h>
#include <dtGame/gameactor.h>
#include <dtDAL/propertycontainer.h>
#include <dtDAL/containeractorproperty.h>

#include <AIUtility.h>

namespace NetDemo
{
   class EnemyDescriptionActorProxy;

   class NETDEMO_EXPORT EnemyDescriptionActor : public dtGame::GameActor
   {
   public:

      class NETDEMO_EXPORT EnemyType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(EnemyType);

         public:
            static EnemyType ENEMY_DEFAULT;
            static EnemyType ENEMY_MINE;
            static EnemyType ENEMY_HELIX;

         private:
            EnemyType();
            EnemyType(const std::string &name);
      };



      
      struct NETDEMO_EXPORT EnemySpawnInfo
      {
         //this is used in RegisterProperties()
         typedef EnemySpawnInfo value_type;

         EnemySpawnInfo();
         ~EnemySpawnInfo();

         DT_DECLARE_ACCESSOR_INLINE(float, LastSpawnTime);
         DT_DECLARE_ACCESSOR_INLINE(float, NumSpawnPerMinute);
         DT_DECLARE_ACCESSOR_INLINE(int, WaveDenominator);

         DT_DECLARE_ACCESSOR_INLINE(std::string, EnemyPrototypeName);
         DT_DECLARE_ACCESSOR_INLINE(int, MaxVelocity);

         void RegisterProperties(dtDAL::PropertyContainer& pc, const std::string& group);
         void RegisterProperties(dtDAL::ContainerActorProperty& pc, const std::string& group);
      };


      typedef dtGame::GameActor BaseClass;

   public:
      /// Constructor
      EnemyDescriptionActor (EnemyDescriptionActorProxy& proxy);

      void SetEnemyType(EnemyDescriptionActor::EnemyType& newValue);
      EnemyDescriptionActor::EnemyType& GetEnemyType() const;

      EnemySpawnInfo& GetSpawnInfo(){ return mSpawnInfo; }
      const EnemySpawnInfo& GetSpawnInfo() const { return mSpawnInfo; }

   protected:
      /// Destructor
      virtual ~EnemyDescriptionActor();

   private:

      EnemyType* mType;
      
      EnemySpawnInfo mSpawnInfo;
  
   };


   class NETDEMO_EXPORT EnemyDescriptionActorProxy : public dtGame::GameActorProxy
   {
   public:
      typedef dtGame::GameActorProxy BaseClass;

      // public strings for the properties
      static const dtUtil::RefString PROP_ENEMY_TYPE;

      EnemyDescriptionActorProxy();
      /*virtual*/ void BuildPropertyMap();

      /*virtual*/ bool IsPlaceable(){ return false; };

   protected:
      virtual ~EnemyDescriptionActorProxy();
      void CreateDrawable();

   private:
   };
}

#endif //NETDEMO_ENEMYDESCRIPTIONACTOR
