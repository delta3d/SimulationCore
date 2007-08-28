/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#include <StealthViewer/Qt/EntitySearch.h>
#include <SimCore/Actors/BaseEntity.h>
#include <dtGame/deadreckoninghelper.h>

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

      return (lastTransUpdate > lastRotUpdate) ? lastTransUpdate : lastRotUpdate;
   }
}
