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
