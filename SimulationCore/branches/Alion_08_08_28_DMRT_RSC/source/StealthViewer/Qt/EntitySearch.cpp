/* -*-c++-*-
* Stealth Viewer
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
 * @author Curtiss Murphy
 */
#include <StealthViewer/Qt/EntitySearch.h>
#include <SimCore/Actors/BaseEntity.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtCore/system.h>

namespace StealthQt
{
   void EntitySearch::FindEntities(std::vector<dtCore::ObserverPtr<dtGame::GameActorProxy> > &toFill, 
                                   dtGame::GameManager &gm,
                                   const std::string &callSign, 
                                   const std::string &force, 
                                   const std::string &damageState)
   {
      toFill.clear();

      std::vector<dtGame::GameActorProxy*> allProxies;
      gm.GetAllGameActors(allProxies);

      for(size_t i = 0; i < allProxies.size(); i++)
      {
         SimCore::Actors::BaseEntityActorProxy *eap = 
            dynamic_cast<SimCore::Actors::BaseEntityActorProxy*>(allProxies[i]);

         // Could be the environment actor proxy or something. Skip it
         if(eap == NULL)
            continue;

         SimCore::Actors::BaseEntity &entity = static_cast<SimCore::Actors::BaseEntity&>(eap->GetGameActor());

         if(!callSign.empty())
         {
            // Skip it
            std::string entityName = entity.GetName();

            int index = entityName.find(callSign);
            if(index != 0)
               continue;
         }

         if(force != "Any")
         {
            const std::string &value = entity.GetForceAffiliation().GetName();
            
            // Force search string is not empty, and this doesn't match. Skip it. 
            if(value != force)
               continue;
         }

         if(damageState != "Any")
         {
            const std::string &value = entity.GetDamageState().GetName();
            
            // Skip it.
            if(value != damageState)
               continue;
         }

         // The name, force, and damage state matches, add it
         toFill.push_back(&entity.GetGameActorProxy());
      }
   }

   double EntitySearch::GetLastUpdateTime(const dtGame::GameActorProxy &proxy)
   {
      const SimCore::Actors::BaseEntity *entity = 
         dynamic_cast<const SimCore::Actors::BaseEntity*>(&proxy.GetGameActor());

      if(entity == NULL)
         return 0.0;

      const dtGame::DeadReckoningHelper& drhelp = entity->GetDeadReckoningHelper();

      double lastTransUpdate = drhelp.GetLastTranslationUpdatedTime();
      double lastRotUpdate   = drhelp.GetLastRotationUpdatedTime();
      double mostRecentUpdate = (lastTransUpdate > lastRotUpdate) ? lastTransUpdate : lastRotUpdate;
      double timePassedSinceUpdate = dtCore::System::GetInstance().GetSimulationTime() - mostRecentUpdate;
      timePassedSinceUpdate = (timePassedSinceUpdate < 0.0) ? 0.0 : timePassedSinceUpdate; // neg could happen and looks wierd.

      return timePassedSinceUpdate;
   }
}
