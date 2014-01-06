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
         static void FindEntities(std::vector<std::weak_ptr<dtGame::GameActorProxy> > &toFill, 
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
