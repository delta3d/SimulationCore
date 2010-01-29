/* -*-c++-*-
* Using 'The MIT License'
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
* @author Bradley Anderegg
*/

#ifndef DELTA_TOWERAIHELPER_H
#define DELTA_TOWERAIHELPER_H

#include <DemoExport.h>
#include <BaseAIHelper.h>

#include <AIWeaponUtility.h>

namespace dtCore
{
   class Transformable;
}

namespace NetDemo
{

   class AIEvent;
   class AIStateType;


   class NETDEMO_EXPORT TowerAIHelper: public BaseAIHelper
   {
      public:
         typedef BaseAIHelper BaseClass;

         TowerAIHelper();

         /*virtual*/ void OnInit(const EnemyDescriptionActor* desc);
         /*virtual*/ void Spawn();

         /*virtual*/ void PreSync(const dtCore::Transform& trans);
         /*virtual*/ void Update(float dt);

         void SetCurrentTarget(const dtCore::Transformable& target);

         bool GetTriggerState() const;
         const osg::Vec2& GetWeaponAngle() const;
         void SetWeaponAngle(const osg::Vec2& angle);

      protected:
         TowerAIHelper(const TowerAIHelper&);  //not implemented by design
         TowerAIHelper& operator=(const TowerAIHelper&);  //not implemented by design
         ~TowerAIHelper();

         /*virtual*/ void RegisterStates();
         /*virtual*/ void CreateStates();
         /*virtual*/ void SetupTransitions();
         /*virtual*/ void SetupFunctors();

         void SelectState(float dt);

         virtual void Attack(float dt);
         float GetAngle(const osg::Vec3& pos);


      private:

         AITurret mTurretAI;
   };

} //namespace NetDemo

#endif //DELTA_TOWERAIHELPER_H
