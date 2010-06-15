/* -*-c++-*-
* Simulation Core
* Copyright 2010, Alion Science and Technology
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
*
* @author Chris Rodgers
*/

#ifndef CAMO_PAINT_STATE_ACT_COMP_H_
#define CAMO_PAINT_STATE_ACT_COMP_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Vec2>
#include <SimCore/ActComps/BodyPaintStateActComp.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Actors
   {
      class CamoParams;
   }
}



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT CamoPaintStateActComp : public BodyPaintStateActComp
      {
         public:
            typedef BodyPaintStateActComp BaseClass;

            static const ActorComponent::ACType TYPE;

            // Property Names
            static const dtUtil::RefString PROPERTY_CAMO_ID;

            CamoPaintStateActComp();

            ////////////////////////////////////////////////////////////////////
            // PROPERTY DECLARATIONS - getter, setter and member variable.
            ////////////////////////////////////////////////////////////////////
            DECLARE_PROPERTY(int, CamoId);

            ////////////////////////////////////////////////////////////////////
            // OVERRIDE METHODS - ActorComponent
            ////////////////////////////////////////////////////////////////////

            /**
            * Handles the setup and registration of its properties.
            */
            virtual void BuildPropertyMap();
         
         protected:
            virtual ~CamoPaintStateActComp();

            const SimCore::Actors::CamoParams* GetCamoParameters(int camoId);
      };

   }
}

#endif
