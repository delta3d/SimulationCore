/*
* Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
* 
* See the .h file for complete licensing information.
* 
* Alion Science and Technology Corporation
* 5365 Robin Hood Road
* Norfolk, VA 23513
* (757) 857-5670, www.alionscience.com
* 
* @author Curtiss Murphy
*/
#include <prefix/SimCorePrefix-src.h>
#include <DriverActorRegistry.h>

#ifdef AGEIA_PHYSICS
   #include <HoverVehicleActor.h>
   #include <NxAgeiaWorldComponent.h>
#endif

#include <dtCore/shadermanager.h>
#include <dtCore/scene.h>

using dtCore::RefPtr;

namespace DriverDemo
{
   
#ifdef AGEIA_PHYSICS
   RefPtr<dtDAL::ActorType> DriverActorRegistry::HOVER_VEHICLE_ACTOR_TYPE(
      new dtDAL::ActorType("HoverActor", "DriverDemo", "A floaty drivable vehicle for Driver Demo"));
#endif
   
   ///////////////////////////////////////////////////////////////////////////
   extern "C" DRIVER_DEMO_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
   {
       return new DriverActorRegistry;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" DRIVER_DEMO_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
   {
       delete registry;
   }

   ///////////////////////////////////////////////////////////////////////////
   DriverActorRegistry::DriverActorRegistry() :
      dtDAL::ActorPluginRegistry("This library holds actors from the Driver Demo")
   {
      //dtCore::ShaderManager::GetInstance().LoadShaderDefinitions("Shaders/ShaderDefs.xml", true);
   }

   ///////////////////////////////////////////////////////////////////////////
   void DriverActorRegistry::RegisterActorTypes()
   {

#ifdef AGEIA_PHYSICS
      mActorFactory->RegisterType<HoverVehicleActorProxy>(HOVER_VEHICLE_ACTOR_TYPE.get());
#endif
   }
}
