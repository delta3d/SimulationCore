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


#include <dtUtil/mswin.h>
#include <Actors/EnemyDescriptionActor.h>

#include <dtDAL/enginepropertytypes.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <dtGame/actorupdatemessage.h>
#include <dtUtil/mathdefines.h>


namespace NetDemo
{
   ///////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(EnemyDescriptionActor::EnemyType);
   EnemyDescriptionActor::EnemyType EnemyDescriptionActor::EnemyType::ENEMY_DEFAULT("ENEMY_DEFAULT");
   EnemyDescriptionActor::EnemyType EnemyDescriptionActor::EnemyType::ENEMY_MINE("ENEMY_MINE");
   EnemyDescriptionActor::EnemyType EnemyDescriptionActor::EnemyType::ENEMY_HELIX("ENEMY_HELIX");

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemyType::EnemyType(const std::string &name)
      : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemyDescriptionActor(EnemyDescriptionActorProxy &proxy)
      : BaseClass(proxy)
      , mType(&EnemyDescriptionActor::EnemyType::ENEMY_DEFAULT)
   {
      SetName("EnemyDescriptionActor");
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::~EnemyDescriptionActor()
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyDescriptionActor::SetEnemyType( EnemyDescriptionActor::EnemyType& newValue )
   {
      mType = &newValue;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemyType& EnemyDescriptionActor::GetEnemyType() const
   {
      return *mType;
   }

   //////////////////////////////////////////////////////////////////////
   // PROXY
   //////////////////////////////////////////////////////////////////////
   EnemyDescriptionActorProxy::EnemyDescriptionActorProxy()
   {
      SetClassName("EnemyDescriptionActor");
   }

   const dtUtil::RefString EnemyDescriptionActorProxy::PROP_ENEMY_TYPE("Enemy Type");


   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyDescriptionActorProxy::BuildPropertyMap()
   {
      const std::string& GROUP = "Enemy Description";

      BaseClass::BuildPropertyMap();

      EnemyDescriptionActor &actor = static_cast<EnemyDescriptionActor &>(GetGameActor());

      static const dtUtil::RefString PROP_ENEMY_TYPE_DESC("Indicates the enemy type.");
      AddProperty(new dtDAL::EnumActorProperty<EnemyDescriptionActor::EnemyType>(PROP_ENEMY_TYPE, PROP_ENEMY_TYPE,
         dtDAL::MakeFunctor(actor, &EnemyDescriptionActor::SetEnemyType),
         dtDAL::MakeFunctorRet(actor, &EnemyDescriptionActor::GetEnemyType),
         PROP_ENEMY_TYPE_DESC, GROUP));

   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActorProxy::~EnemyDescriptionActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyDescriptionActorProxy::CreateActor()
   {
      EnemyDescriptionActor* newActor = new EnemyDescriptionActor(*this);
      SetActor(*newActor);
   }


} // namespace
//#endif
