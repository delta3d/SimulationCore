#ifndef SPHEROACTORREGISTRY_H_
#define SPHEROACTORREGISTRY_H_

#include "SpheroExport.h"
#include <dtDAL/actorpluginregistry.h>
#include <dtCore/refptr.h>

namespace Sphero
{

   class SpheroActorRegistry : public dtDAL::ActorPluginRegistry
   {
      public:
         SpheroActorRegistry();
         virtual ~SpheroActorRegistry();

         /// Registers all of the actor proxies to be exported
         virtual void RegisterActorTypes();
   };

}

#endif /*SPHEROACTORREGISTRY_H_*/
