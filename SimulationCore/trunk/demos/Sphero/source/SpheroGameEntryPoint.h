/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008, Alion Science and Technology
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
 * David Guthrie
 */
#ifndef PRIMITIVE_EXAMPLE_ENTRY_POINT
#define PRIMITIVE_EXAMPLE_ENTRY_POINT

#include "export.h"
#include <SimCore/HLA/BaseHLAGameEntryPoint.h>

namespace Sphero
{
   class SPHERO_EXPORT SpheroGameEntryPoint : public SimCore::HLA::BaseHLAGameEntryPoint
   {
      public:
         typedef SimCore::HLA::BaseHLAGameEntryPoint BaseClass;
          
         SpheroGameEntryPoint();
         virtual ~SpheroGameEntryPoint();

         /**
          * Called to initialize the game application.
          * @param app the current application
          */
         virtual void Initialize(dtGame::GameApplication& app, int argc, char **argv);

         /**
          * Called after all startup related code is run.
          * @param app the current application
          */
         virtual void OnStartup(dtGame::GameApplication& app);

         /// May be overridden to allow subclassed to add components
         virtual void InitializeComponents(dtGame::GameManager &gm);

   };
}

#endif
