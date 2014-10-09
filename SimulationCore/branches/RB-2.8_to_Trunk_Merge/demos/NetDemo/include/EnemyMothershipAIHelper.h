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

#ifndef DELTA_AIEnemyMothership_H
#define DELTA_AIEnemyMothership_H

#include <AIUtility.h>
#include <DemoExport.h>
#include <EnemyAIHelper.h>
 
#include <dtCore/propertymacros.h>
#include <dtAI/controllable.h>
#include <dtAI/steeringbehavior.h>
#include <dtAI/steeringpipeline.h>
#include <osg/Matrix>
#include <osg/Vec3>

#include <stack>

namespace NetDemo
{
   class NETDEMO_EXPORT EnemyMothershipAIHelper: public EnemyAIHelper
   {
   public:
      typedef EnemyAIHelper BaseClass;
      EnemyMothershipAIHelper();

      /*virtual*/ void OnInit(const EnemyDescriptionActor* desc);
      /*virtual*/ void Spawn();
      /*virtual*/ void Update(float dt);

      /*virtual*/ void PreSync(const dtCore::Transform& trans);
      /*virtual*/ void PostSync(dtCore::Transform& trans) const;

      void SetCurrentTarget(dtCore::Transformable& target);
      
   protected:
      EnemyMothershipAIHelper(const EnemyMothershipAIHelper&);  //not implemented by design
      EnemyMothershipAIHelper& operator=(const EnemyMothershipAIHelper&);  //not implemented by design
      ~EnemyMothershipAIHelper();

      /*virtual*/ void RegisterStates();
      /*virtual*/ void CreateStates();
      /*virtual*/ void SetupTransitions();
      /*virtual*/ void SetupFunctors();

      /*virtual*/ void SelectState(float dt);

      virtual void Attack(float dt);

      void ComputeTargetOffset();

   private:

      osg::Vec3 mTargetOffset;
      BaseAISteeringBehavior* mDefaultBehavior;

   };


}//namespace NetDemo

#endif // DELTA_AIEnemyMothership_H
