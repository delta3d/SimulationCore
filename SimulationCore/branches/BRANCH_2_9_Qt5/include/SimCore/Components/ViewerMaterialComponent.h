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
 * @author Allen Danklefsen
 */
#ifndef _VIEWER_MATERIAL_COMPONENT_
#define _VIEWER_MATERIAL_COMPONENT_

// project includes needed
#include <SimCore/Export.h>

#include <dtGame/gmcomponent.h>

#include <SimCore/Actors/ViewerMaterialActor.h>

#include <vector>

namespace dtGame
{
   class GameManager;
   class TickMessage;
   class ActorUpdateMessage;
};

namespace SimCore
{
   namespace Components
   {
      class SIMCORE_EXPORT ViewerMaterialComponent : public dtGame::GMComponent
      {
         friend class SimCore::Actors::ViewerMaterialActor;
      public:
         typedef dtGame::GMComponent BaseClass;
         static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;
         static const dtUtil::RefString DEFAULT_NAME;

         /// Constructor
         ViewerMaterialComponent(dtCore::SystemComponentType& type = *TYPE);

         /**
          * Processes messages sent from the Game Manager
          * @param The message to process
          * @see dtGame::GameManager
          */
         virtual void ProcessMessage(const dtGame::Message &msg);

         //////////////////////////////////////////////////////////////////////////////////////
         const SimCore::Actors::ViewerMaterialActor& GetConstMaterialByName(const std::string& materialName);
         //////////////////////////////////////////////////////////////////////////////////////
         const SimCore::Actors::ViewerMaterialActor& GetConstMaterialByFID(const unsigned int fidIDToCheckWith);

         //////////////////////////////////////////////////////////////////////////////////////
         SimCore::Actors::ViewerMaterialActor& CreateOrChangeMaterialByName(const std::string& materialName);
         //////////////////////////////////////////////////////////////////////////////////////
         SimCore::Actors::ViewerMaterialActor& CreateOrChangeMaterialByFID(const unsigned int fidIDToMakeWith);

      protected:
         /// Destructor
         virtual ~ViewerMaterialComponent(void);

         /**
          * /brief   Purpose  : used for having the scene update all around
          *          Outs     : objects reaccting to physics
          * @param   msg : the message
          */
         virtual void ProcessTick(const dtGame::TickMessage &msg);

         // called only on map change if flag is set.
         void RemoveAllMaterials();

         // called internally
         const std::string FID_ID_ToString(const unsigned int nID);

         // called from an actor - ie from stage.
         void RegisterAMaterialWithComponent(SimCore::Actors::ViewerMaterialActor* material)
         {
            std::vector<dtCore::RefPtr<SimCore::Actors::ViewerMaterialActor> >::iterator iter =  mOurMaterials.begin();
            for(;iter != mOurMaterials.end(); ++iter)
            {
               if((*iter)->GetName() == material->GetName())
               {
                  return;
               }
            }
            mOurMaterials.push_back(material);
         }

      private:
         std::vector<dtCore::RefPtr<SimCore::Actors::ViewerMaterialActor> >  mOurMaterials;
         bool                                               mClearMaterialsOnMapChange;
      };
   }
}
#endif
