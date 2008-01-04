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
