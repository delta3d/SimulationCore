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

#include "SpheroGameEntryPoint.h"
#include "SpheroInputComponent.h"

#include <dtGame/gamemanager.h>
#include <dtGame/gameapplication.h>
#include <dtGame/defaultmessageprocessor.h>

#include <dtPhysics/physicscomponent.h>

namespace Sphero
{
   ///////////////////////////////////////////////////////////////////////////
   extern "C" SPHERO_EXPORT dtGame::GameEntryPoint* CreateGameEntryPoint()
   {
      return new SpheroGameEntryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" SPHERO_EXPORT void DestroyGameEntryPoint(dtGame::GameEntryPoint* entryPoint)
   {
      delete entryPoint;
   }

   ///////////////////////////////////////////////////////////////////////////
   SpheroGameEntryPoint::SpheroGameEntryPoint(): BaseClass()
   {
      
   }

   ///////////////////////////////////////////////////////////////////////////
   SpheroGameEntryPoint::~SpheroGameEntryPoint()
   {
      
   }

   ///////////////////////////////////////////////////////////////////////////
   void SpheroGameEntryPoint::Initialize(dtGame::GameApplication& app, int argc, char **argv)
   {
      BaseClass::Initialize(app, argc, argv);
   }

   ///////////////////////////////////////////////////////////////////////////
   void SpheroGameEntryPoint::InitializeComponents(dtGame::GameManager &gm)
   {
      BaseClass::InitializeComponents(gm);

      gm.AddComponent(*new SpheroInputComponent(), dtGame::GameManager::ComponentPriority::NORMAL);
   }

   ///////////////////////////////////////////////////////////////////////////
   void SpheroGameEntryPoint::OnStartup(dtGame::GameApplication& app)
   {
      BaseClass::OnStartup(app);

      dtGame::GameManager& gm = *app.GetGameManager();
      gm.SetProjectContext("demos/Sphero/ProjectAssets");
      gm.ChangeMap("Primitives");
   }

}
