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

            virtual void UpdateDOFReferences( dtUtil::NodeCollector* nodeCollector );

            virtual bool HasDOF( osgSim::DOFTransform& dof ) const;

            virtual bool HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const;

         protected:
            virtual ~DefaultArticulationHelper();
      };
   }
}

#endif
