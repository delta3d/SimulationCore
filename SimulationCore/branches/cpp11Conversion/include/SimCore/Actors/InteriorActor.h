/* -*-c++-*-
* Simulation Core
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

            bool HasLoadedDamageFiles() const {return mLoadedDamageFiles;}
            void SetHasLoadedDamageFiles(bool value) {mLoadedDamageFiles = value;}

         protected:

            /// Destructor
            virtual ~InteriorActor();

         private:
            bool mVisible;
            bool mLoadedDamageFiles;
      };

      class SIMCORE_EXPORT InteriorActorProxy : public PlatformActorProxy
      {
         public:

            /// Constructor
            InteriorActorProxy();

            /// Creates the actor
            virtual void CreateDrawable();

         protected:

            /// Destructor
            virtual ~InteriorActorProxy();

         private:

      };
   }
}

#endif
