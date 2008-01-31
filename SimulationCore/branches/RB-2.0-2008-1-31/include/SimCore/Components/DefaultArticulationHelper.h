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

#ifndef _DEFAULT_ARTICULATION_HELPER_H_
#define _DEFAULT_ARTICULATION_HELPER_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtCore/refptr.h>
#include <SimCore/Components/ArticulationHelper.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class NodeCollector;
}

namespace dtDAL
{
   class NamedGroupParameter;
}



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // DEFAULT ARTICULATION HELPER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT DefaultArticulationHelper : public SimCore::Components::ArticulationHelper
      {
         public:
            DefaultArticulationHelper();

            virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> BuildGroupProperty();

            virtual void UpdateDOFReferences( dtCore::NodeCollector* nodeCollector );

            virtual bool HasDOF( osgSim::DOFTransform& dof ) const;

            virtual bool HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const;

         protected:
            virtual ~DefaultArticulationHelper();
      };
   }
}

#endif
