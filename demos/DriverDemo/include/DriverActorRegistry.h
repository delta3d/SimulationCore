/* -*-c++-*-
* Driver Demo
* Copyright (C) 2008, Alion Science and Technology Corporation
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
*
* @author Curtiss Murphy
*/
#ifndef _DRIVER_ACTOR_REGISTRY_H_
#define _DRIVER_ACTOR_REGISTRY_H_

#include <dtDAL/actorpluginregistry.h>
#include <DriverExport.h>


namespace DriverDemo 
{
   /**
    * Class that exports the applicable actor proxies to a library
    */
   class DRIVER_DEMO_EXPORT DriverActorRegistry : public dtDAL::ActorPluginRegistry
   {
      public:

         //static dtCore::RefPtr<dtDAL::ActorType> TEST_ACTOR_TYPE;

#ifdef AGEIA_PHYSICS
         static dtCore::RefPtr<dtDAL::ActorType> HOVER_VEHICLE_ACTOR_TYPE;
         static dtCore::RefPtr<dtDAL::ActorType> HOVER_TARGET_ACTOR_TYPE;
#endif

         /// Constructor
         DriverActorRegistry();

         /// Registers all of the actor proxies to be exported
         void RegisterActorTypes();
   };	
}

#endif 
