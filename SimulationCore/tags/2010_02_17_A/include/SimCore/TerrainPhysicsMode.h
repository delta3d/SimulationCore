/* -*-c++-*-
 * SimulationCore
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
 * david
 */

#ifndef TERRAINPHYSICSMODE_H_
#define TERRAINPHYSICSMODE_H_

#include <dtUtil/enumeration.h>
#include <SimCore/Export.h>

namespace SimCore
{

   class SIMCORE_EXPORT TerrainPhysicsMode: public dtUtil::Enumeration
   {
      DECLARE_ENUM(TerrainPhysicsMode);
   public:

      /// Disable means that all terrain physics will be disabled.
      static TerrainPhysicsMode DISABLED;

      /// Used for static terrains where all the physics data can be loaded or generated immediately on creation
      static TerrainPhysicsMode IMMEDIATE;

      /**
       *  On any terrain where the physics is either very large or the terrain is paged dynamically, the
       *  physics generation will deferred until later where it can be determined based on the loaded/visible
       *  working set.
       */
      static TerrainPhysicsMode DEFERRED;

   protected:
      TerrainPhysicsMode(const std::string& name);
      virtual ~TerrainPhysicsMode();
   };

}

#endif /* TERRAINPHYSICSMODE_H_ */
