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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/dvteprefix-src.h>
#include <osgSim/DOFTransform>
#include <dtCore/nodecollector.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtDAL/namedparameter.h>
#include <SimCore/Components/DefaultArticulationHelper.h>



namespace SimCore
{
   namespace Components
   {
      ////////////////////////////////////////////////////////////////////////////////
      // ARTICULATION HELPER CODE
      ////////////////////////////////////////////////////////////////////////////////
      DefaultArticulationHelper::DefaultArticulationHelper()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////
      DefaultArticulationHelper::~DefaultArticulationHelper()
      {
      }

      ////////////////////////////////////////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::NamedGroupParameter> DefaultArticulationHelper::BuildGroupProperty()
      {
         dtCore::RefPtr<dtDAL::NamedGroupParameter> articArrayProp 
            = new dtDAL::NamedGroupParameter( GetArticulationArrayPropertyName() );
         return articArrayProp;
      }

      ////////////////////////////////////////////////////////////////////////////////
      void DefaultArticulationHelper::UpdateDOFReferences( dtCore::NodeCollector* nodeCollector )
      {
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool DefaultArticulationHelper::HasDOF( osgSim::DOFTransform& dof ) const
      {
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////
      bool DefaultArticulationHelper::HasDOFMetric( 
         const osgSim::DOFTransform& dof, const std::string& metricName ) const
      {
         return false;
      }
   }
}
