/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */
#ifndef _INTERIOR_ACTOR_H_
#define _INTERIOR_ACTOR_H_

#include <SimCore/Actors/Platform.h>

namespace osgSim
{
   class DOFTransform;
}

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT InteriorActor : public Platform
      {
         public:

            /// Constructor
            InteriorActor(dtGame::GameActorProxy &proxy);

            void SetVisible( bool visible );
            bool IsVisible() const;

            osgSim::DOFTransform* GetSteeringWheelDOF(const std::string& dofName);

         protected:

            /// Destructor
            virtual ~InteriorActor();

         private:
            bool mVisible;

      };

      class SIMCORE_EXPORT InteriorActorProxy : public PlatformActorProxy
      {
         public:

            /// Constructor
            InteriorActorProxy();

            /// Creates the actor
            virtual void CreateActor();

         protected:

            /// Destructor
            virtual ~InteriorActorProxy();

         private:

      };
   }
}

#endif
