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

#ifndef DELTA_ENEMYAIHELPER_H
#define DELTA_ENEMYAIHELPER_H

#include <DemoExport.h>
#include <BaseAIHelper.h>

namespace dtCore
{
   class Transformable;
}

namespace NetDemo
{

   class AIEvent;
   class AIStateType;


   class NETDEMO_EXPORT EnemyAIHelper: public BaseAIHelper
   {
      public:
         typedef BaseAIHelper BaseClass;

         EnemyAIHelper();

         /*virtual*/ void OnInit(const EnemyDescriptionActor* desc);
         /*virtual*/ void Spawn();
         /*virtual*/ void Update(float dt);

         float GetDistance(const osg::Vec3& pos);
         void SetCurrentTarget(dtCore::Transformable& target);
         void ChangeSteeringBehavior(dtCore::RefPtr<SteeringBehaviorType> newBehavior);

      protected:
         EnemyAIHelper(const EnemyAIHelper&);  //not implemented by design
         EnemyAIHelper& operator=(const EnemyAIHelper&);  //not implemented by design
         ~EnemyAIHelper();

         /*virtual*/ void RegisterStates();
         /*virtual*/ void CreateStates();
         /*virtual*/ void SetupTransitions();
         /*virtual*/ void SetupFunctors();

         /*virtual*/ void SelectState(float dt);

         virtual void CalculateNextWaypoint();
         virtual void GoToWaypoint(float dt);
         virtual void DefaultStateUpdate(float dt);


      private:

   };

} //namespace NetDemo

#endif //DELTA_ENEMYAIHELPER_H
