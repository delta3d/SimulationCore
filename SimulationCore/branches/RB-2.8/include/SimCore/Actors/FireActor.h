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
* @author Eddie Johnson 
*/
#ifndef _FIRE_ACTOR_H_
#define _FIRE_ACTOR_H_

#include <SimCore/Actors/LocalEffectActor.h>
#include <dtCore/particlesystem.h>
#include <dtDAL/actorproxyicon.h>

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT FireActor : public LocalEffectActor
      {
         public:

            /// Constructor
            FireActor(dtGame::GameActorProxy &proxy);

            /**
             * Sets the light range on this actor
             * @param range The new range
             */
            void SetLightRange(float range) { mLightRange = range; }

            /**
             * Gets the light range on this actor
             * @return mLightRange
             */
            float GetLightRange() const { return mLightRange; }

            /**
             * Loads the file this partile system will use
             * @param fileName The name of the file to use
             */
            void LoadFireFile(const std::string& fileName);

         protected:

            /// Destructor
            virtual ~FireActor();


         private:

            float mLightRange;
      };

      class SIMCORE_EXPORT FireActorProxy : public LocalEffectActorProxy
      {
         public:

            /// Constructor
            FireActorProxy();

            /// Adds the properties associated with this actor
            void BuildPropertyMap();

            /**
             * Gets the billboard used to represent particle systems.
             * @return
             */
            virtual dtDAL::ActorProxyIcon* GetBillBoardIcon();

            /// Creates the actor
            void CreateDrawable() { SetDrawable(*new FireActor(*this)); }

            /// Loads the file the particle system will use
            virtual void LoadFile(const std::string &fileName);

            /**
             * Gets the method by which a particle system is rendered.
             * @return dtDAL::BaseActorObject::RenderMode::DRAW_BILLBOARD_ICON.
             */
            virtual const dtDAL::BaseActorObject::RenderMode& GetRenderMode() 
            {
                return dtDAL::BaseActorObject::RenderMode::DRAW_BILLBOARD_ICON;
            }

         protected:

            /// Destructor
            virtual ~FireActorProxy();


         private:

            dtCore::RefPtr<dtDAL::ActorProxyIcon> mBillBoardIcon;

      };
   }
}

#endif