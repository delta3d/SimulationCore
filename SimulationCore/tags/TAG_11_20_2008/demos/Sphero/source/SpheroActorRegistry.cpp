#include "SpheroActorRegistry.h"

namespace Sphero
{
   ///////////////////////////////////////////////////////////////////////////
   extern "C" SPHERO_EXPORT dtDAL::ActorPluginRegistry* CreatePluginRegistry()
   {
       return new SpheroActorRegistry;
   }

   ///////////////////////////////////////////////////////////////////////////
   extern "C" SPHERO_EXPORT void DestroyPluginRegistry(dtDAL::ActorPluginRegistry *registry)
   {
       delete registry;
   }

   SpheroActorRegistry::SpheroActorRegistry():
      dtDAL::ActorPluginRegistry("Sphero", "Actors for the Sphero demo.")
   {
   }

   SpheroActorRegistry::~SpheroActorRegistry()
   {
   }

   void SpheroActorRegistry::RegisterActorTypes()
   {
      
   }

}
