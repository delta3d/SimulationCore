/*
 * DVTE Stealth Viewer
 * Copyright (C) 2006, Alion Science and Technology.
 *
 * @author Eddie Johnson
 */
#ifndef _ENTITY_SEARCH_H_
#define _ENTITY_SEARCH_H_

#include <dtGame/gameactor.h>
#include <string>

namespace StealthQt
{
   class EntitySearch
   {
      public:

         /**
          * Searches for entities based on the specified parameters
          * @param toFill The vector to fill
          * @param gm The game manager to search
          * @param callSign The call sign to look for
          * @param force The force to look for
          * @param damageState The damagestate to look for
          */
         static void FindEntities(std::vector<dtCore::ObserverPtr<dtGame::GameActorProxy> > &toFill, 
            dtGame::GameManager &gm,
            const std::string &callSign, 
            const std::string &force, 
            const std::string &damageState);

         /**
          * Returns an entity's last update time
          * @param proxy, the proxy to check
          * @return The last update time of the rotation, or translation. 
          * Whichever is greater
          */
         static double GetLastUpdateTime(const dtGame::GameActorProxy &proxy);
   };
}

#endif
