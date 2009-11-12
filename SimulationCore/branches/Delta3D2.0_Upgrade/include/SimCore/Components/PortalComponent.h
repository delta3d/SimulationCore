/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/

#ifndef _PORTAL_COMPONENT_
#define _PORTAL_COMPONENT_

#include <list>

// project includes needed
#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>
#include <SimCore/Actors/PortalActor.h>

namespace dtGame
{
   class GameManager;
   class TickMessage;
   class ActorPublishedMessage;
   class ActorDeletedMessage;
   class ActorUpdateMessage;
};

namespace SimCore
{
   namespace Components
   {
      ///////////////////////////////////////////////////////
      //    The Component
      ///////////////////////////////////////////////////////
      class SIMCORE_EXPORT PortalComponent : public dtGame::GMComponent
      {
         public:
            static const std::string DEFAULT_NAME;
            /// Constructor
            PortalComponent(const std::string &name = DEFAULT_NAME);
            
            /**
            * Processes messages sent from the Game Manager
            * @param The message to process
            * @see dtGame::GameManager
            */
            virtual void ProcessMessage(const dtGame::Message &msg);

            //////////////////////////////////////////
            void RegisterPortal(SimCore::Actors::Portal* portal);
            //////////////////////////////////////////
            void UnRegisterPortal(SimCore::Actors::Portal* portal);
            //////////////////////////////////////////
            int GetNumberOfPortals() {return mOurPortals.size();}
            //////////////////////////////////////////
            void FindPortalsInRange(const osg::Vec3& position, float radius, std::vector<dtGame::GameActorProxy*> &toFillIn);

         protected:
            /// Destructor
            virtual ~PortalComponent(void);

            /**
            * /brief   Purpose  : used for having the scene update all around
            *          Outs     : objects reaccting to physics 
            * @param   msg : the message
            */
            virtual void ProcessTick(const dtGame::TickMessage &msg);

         private:
            std::vector<dtCore::RefPtr<SimCore::Actors::Portal> >   mOurPortals;
      };
   }
}
#endif