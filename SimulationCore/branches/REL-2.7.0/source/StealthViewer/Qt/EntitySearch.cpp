/* -*-c++-*-
 * Stealth Viewer - EntitySearch (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2007-2008, Alion Science and Technology Corporation
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
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Eddie Johnson
 * @author Curtiss Murphy
 */
#include <prefix/StealthQtPrefix.h>
#include <StealthViewer/Qt/EntitySearch.h>
#include <SimCore/Actors/BaseEntity.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtGame/gamemanager.h>
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

         SimCore::Actors::BaseEntity& entity = static_cast<SimCore::Actors::BaseEntity&>(eap->GetGameActor());

         if (!entity.IsVisible())
            continue;

         if (!callSign.empty())
         {
            // Skip it
            std::string entityName = entity.GetName();

            int index = entityName.find(callSign);
            if(index != 0)
               continue;
         }

         if (force != "Any")
         {
            const std::string &value = entity.GetForceAffiliation().GetName();

            // Force search string is not empty, and this doesn't match. Skip it.
            if(value != force)
               continue;
         }

         if (damageState != "Any")
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

         const dtGame::DeadReckoningHelper* drhelp = NULL;
         entity->GetComponent(drhelp);

         double lastTransUpdate = drhelp->GetLastTranslationUpdatedTime();
         double lastRotUpdate   = drhelp->GetLastRotationUpdatedTime();
         double mostRecentUpdate = (lastTransUpdate > lastRotUpdate) ? lastTransUpdate : lastRotUpdate;
         double timePassedSinceUpdate = dtCore::System::GetInstance().GetSimulationTime() - mostRecentUpdate;
         timePassedSinceUpdate = (timePassedSinceUpdate < 0.0) ? 0.0 : timePassedSinceUpdate; // neg could happen and looks wierd.

         return timePassedSinceUpdate;
   }
}
