/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*/
#ifndef _TEXTURE_PROJECTOR_ACTOR_
#define _TEXTURE_PROJECTOR_ACTOR_

#include <SimCore/Export.h>

#include <dtGame/gameactor.h>

#include <SimCore/Projector.h>

#include <osg/observer_ptr>

namespace dtGame
{
   class Message;
}

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT TextureProjectorActor: public dtGame::GameActor
      {
         public:
            /// constructor for NxAgeiaBaseActor
            TextureProjectorActor(dtGame::GameActorProxy& proxy);

            /**
            * This method is an invokable called when an object is local and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void OnTickLocal(const dtGame::TickMessage& tickMessage);

            // SUPER NEEDS TO BE CALLED AFTER YOUR OTHER CODE, MODEL MUST BE LOADED IN!
            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

         protected:
            /// destructor
            virtual ~TextureProjectorActor(void);

         public:

            // gets -- Can't use 'GetCurrentTime' - reserved in WinBase.h
            float GetCurrTime() {return mCurrentTime;}
            float GetMaxTime()     {return mMaxTime;}
            float GetCurrentAlpha(){return mCurrentAlpha;}
            std::string GetImageProjectorFile() {return mImageProjectorFile;}

            // sets
            void SetCurrTime(float value) {mCurrentTime = value;}
            void SetMaxTime(float value)     {mMaxTime = value;}
            void SetCurrentAlpha(float value){mCurrentAlpha = value;}
            void SetImageProjectorFile(const std::string& value) {mImageProjectorFile = value;}
            void SetAttachementEntity(osg::Node* nodeToAttachTo) { mEntityToAttachTo = nodeToAttachTo;}

         private:
            float                      mCurrentTime;
            float                      mMaxTime;
            float                      mCurrentAlpha;
            std::string                mImageProjectorFile;
            dtCore::RefPtr<Projector>     mProjector;
            osg::observer_ptr<osg::Node>  mEntityToAttachTo;
      };

      ////////////////////////////////////////////////////////
      // PROXY
      ////////////////////////////////////////////////////////
      class SIMCORE_EXPORT TextureProjectorActorProxy : public dtGame::GameActorProxy
      {
         public:
            TextureProjectorActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~TextureProjectorActorProxy();
            void CreateDrawable();
            virtual void OnEnteredWorld();
      };
   }
}
#endif
