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
#ifndef _TEXTURE_PROJECTOR_COMPONENT_H_
#define _TEXTURE_PROJECTOR_COMPONENT_H_

#include <dtGame/gmcomponent.h>
#include <SimCore/Export.h>

namespace dtGame
{
   class Message;
   class TickMessage;
}

namespace SimCore
{
   namespace Actors
   {
      class TextureProjectorActor;
   }

   namespace Components
   {
      class SIMCORE_EXPORT TextureProjectorComponent : public dtGame::GMComponent
      {
      public:
         static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;
         static const dtUtil::RefString DEFAULT_NAME;
         TextureProjectorComponent(dtCore::SystemComponentType& type = *TYPE);

      protected:

         /// Destructor
         virtual ~TextureProjectorComponent();

      public:
         void AddTextureProjectorActorToComponent(Actors::TextureProjectorActor &toAdd);
         void RemoveTextureProjectorActorFromComponent(Actors::TextureProjectorActor &toRemove);

         void ProcessTick(const dtGame::TickMessage &msg);
         void ProcessMessage(const dtGame::Message &msg);

      private:
         unsigned int mMaxNumberOfProjectedTextures;
         std::list<dtCore::RefPtr<Actors::TextureProjectorActor> > mActorList;
      };
   }
}

#endif

