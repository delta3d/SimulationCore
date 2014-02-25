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

#include <dtCore/enginepropertytypes.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#include <dtGame/actorupdatemessage.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/propertymacros.h>

namespace NetDemo
{
   ///////////////////////////////////////////////////////////////////////////////////
   //EnemyDescriptionActor::EnemyType
   ///////////////////////////////////////////////////////////////////////////////////
   IMPLEMENT_ENUM(EnemyDescriptionActor::EnemyType);
   EnemyDescriptionActor::EnemyType EnemyDescriptionActor::EnemyType::ENEMY_DEFAULT("ENEMY_DEFAULT");
   EnemyDescriptionActor::EnemyType EnemyDescriptionActor::EnemyType::ENEMY_MINE("ENEMY_MINE");
   EnemyDescriptionActor::EnemyType EnemyDescriptionActor::EnemyType::ENEMY_HELIX("ENEMY_HELIX");

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemyType::EnemyType()
      : dtUtil::Enumeration("ENEMY_DEFAULT")
   {
      AddInstance(this);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemyType::EnemyType(const std::string& name)
      : dtUtil::Enumeration(name)
   {
      AddInstance(this);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   //EnemyDescriptionActor::EnemySpawnInfo
   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemySpawnInfo::EnemySpawnInfo()
      : mLastSpawnTime(0)
      , mNumSpawnPerMinute(10)
      , mWaveDenominator(1)
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActor::EnemySpawnInfo::~EnemySpawnInfo()
   {
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyDescriptionActor::EnemySpawnInfo::RegisterProperties(dtCore::PropertyContainer& pc, const std::string& group)
   {
      typedef dtCore::PropertyRegHelper<dtCore::PropertyContainer&, value_type> RegHelperType;
      RegHelperType propReg(pc, this, group);

      DT_REGISTER_PROPERTY(LastSpawnTime, "The total amount SimTime that has past since the last spawn.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(NumSpawnPerMinute, "The rate at which this enemy spawns per minute.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(WaveDenominator, "This enemy spawns when the wave number is a multiple of this number.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxVelocity, "This represents the maximum velocity for the enemy", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(EnemyPrototypeName, "This is the name of the enemy prototype to spawn", RegHelperType, propReg);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyDescriptionActor::EnemySpawnInfo::RegisterProperties(dtCore::ContainerActorProperty& pc, const std::string& group)
   {
      typedef dtCore::PropertyRegHelper<dtCore::ContainerActorProperty&, value_type> RegHelperType;
      RegHelperType propReg(pc, this, group);

      DT_REGISTER_PROPERTY(LastSpawnTime, "The total amount SimTime that has past since the last spawn.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(NumSpawnPerMinute, "The rate at which this enemy spawns per minute.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(WaveDenominator, "This enemy spawns when the wave number is a multiple of this number.", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(MaxVelocity, "This represents the maximum velocity for the enemy", RegHelperType, propReg);
      DT_REGISTER_PROPERTY(EnemyPrototypeName, "This is the name of the enemy prototype to spawn", RegHelperType, propReg);

   }

   ///////////////////////////////////////////////////////////////////////////////////
   //EnemyDescriptionActor
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

   /////////////////////////////////////////////////////////////////////////////////
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
      static const dtUtil::RefString GROUP = "Enemy Description";

      BaseClass::BuildPropertyMap();

      EnemyDescriptionActor* actor = NULL;
      GetActor(actor);

      static const dtUtil::RefString PROP_ENEMY_TYPE_DESC("Indicates the enemy type.");
      AddProperty(new dtCore::EnumActorProperty<EnemyDescriptionActor::EnemyType>(PROP_ENEMY_TYPE, PROP_ENEMY_TYPE,
               dtCore::EnumActorProperty<EnemyDescriptionActor::EnemyType>::SetFuncType(actor, &EnemyDescriptionActor::SetEnemyType),
               dtCore::EnumActorProperty<EnemyDescriptionActor::EnemyType>::GetFuncType(actor, &EnemyDescriptionActor::GetEnemyType),
               PROP_ENEMY_TYPE_DESC, GROUP));

      //do this to register properties from EnemySpawnInfo
      actor->GetSpawnInfo().RegisterProperties(*this, GROUP);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   EnemyDescriptionActorProxy::~EnemyDescriptionActorProxy(){}

   ///////////////////////////////////////////////////////////////////////////////////
   void EnemyDescriptionActorProxy::CreateDrawable()
   {
      EnemyDescriptionActor* newDraw = new EnemyDescriptionActor(*this);
      SetDrawable(*newDraw);
   }


} // namespace
//#endif
