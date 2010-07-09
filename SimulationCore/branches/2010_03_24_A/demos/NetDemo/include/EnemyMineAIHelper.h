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

#ifndef DELTA_ENEMYMINEAIHELPER_H
#define DELTA_ENEMYMINEAIHELPER_H

#include <DemoExport.h>
#include <EnemyAIHelper.h>

namespace dtCore
{
   class Transformable;
}

namespace NetDemo
{

   class AIEvent;
   class AIStateType;


   class NETDEMO_EXPORT EnemyMineAIHelper: public EnemyAIHelper
   {
      public:
         typedef EnemyAIHelper BaseClass;

         EnemyMineAIHelper();

         /*virtual*/ void OnInit(const EnemyDescriptionActor* desc);
         /*virtual*/ void Spawn();
         /*virtual*/ void Update(float dt);

      protected:
         EnemyMineAIHelper(const EnemyMineAIHelper&);  //not implemented by design
         EnemyMineAIHelper& operator=(const EnemyMineAIHelper&);  //not implemented by design
         ~EnemyMineAIHelper();

         /*virtual*/ void RegisterStates();
         /*virtual*/ void CreateStates();
         /*virtual*/ void SetupTransitions();
         /*virtual*/ void SetupFunctors();

         virtual void Attack(float dt);


      private:

         float mMaxVelocity;

   };

} //namespace NetDemo

#endif //DELTA_ENEMYMINEAIHELPER_H