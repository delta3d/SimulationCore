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
 * @author David Guthrie
 */
#ifndef BASEHLAGAMEENTRYPOINT_H_
#define BASEHLAGAMEENTRYPOINT_H_

#include <SimCore/HLA/Export.h>
#include <SimCore/BaseGameEntryPoint.h>

namespace dtHLAGM
{
   class HLAComponent;
}

namespace SimCore
{
   namespace HLA
   {
      class SIMCORE_HLA_EXPORT BaseHLAGameEntryPoint : public SimCore::BaseGameEntryPoint
      {
         public:
            typedef SimCore::BaseGameEntryPoint BaseClass;

            BaseHLAGameEntryPoint();
            virtual ~BaseHLAGameEntryPoint();

            /// called virtual for loading specific maps
            virtual void HLAConnectionComponentSetup(dtGame::GameManager &gm);

            /// creates and configures the HLA Component.
            virtual dtCore::RefPtr<dtHLAGM::HLAComponent> CreateAndSetupHLAComponent(dtGame::GameManager &gm);

            virtual void Initialize(dtGame::GameApplication& app, int argc, char **argv);

            /// Overridden to add HLA components
            virtual void InitializeComponents(dtGame::GameManager &gm);

            virtual void OnShutdown(dtGame::GameApplication& app);

         protected:
            std::string mFederationExecutionName;
            std::string mFederateName;
            std::string mFedFileResource;
            std::string mFedMappingResource;
      };

   }

}

#endif /*BASEHLAGAMEENTRYPOINT_H_*/
