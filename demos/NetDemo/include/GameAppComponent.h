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
* @author David Guthrie
*/
#ifndef RES_GAMEAPPCOMPONENT_H_
#define RES_GAMEAPPCOMPONENT_H_

#include <SimCore/Components/BaseGameAppComponent.h>
#include <SimCore/Actors/StealthActor.h>

namespace NetDemo
{
   class GameAppComponent : public SimCore::Components::BaseGameAppComponent
   {
      public:
         /// Constructor
         GameAppComponent(const std::string& name = BaseGameAppComponent::DEFAULT_NAME);

         virtual void ProcessMessage(const dtGame::Message& msg);

         void InitializePlayer();

         //SimCore::Actors::BasePhysicsVehicleActor* CreateNewVehicle();
      private:
         dtCore::RefPtr<SimCore::Actors::StealthActor> mStealth;
   };

}
#endif /* RES_GAMEAPPCOMPONENT_H_ */
