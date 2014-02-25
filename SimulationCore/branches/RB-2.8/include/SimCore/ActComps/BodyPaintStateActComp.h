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

#ifndef BODY_PAINT_STATE_ACT_COMP_H_
#define BODY_PAINT_STATE_ACT_COMP_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Vec2>
#include <SimCore/ActComps/BodyPaintActComp.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BodyPaintStateActComp : public BodyPaintActComp
      {
         public:
            typedef BodyPaintActComp BaseClass;

            static const ActorComponent::ACType TYPE;

            // Property Names
            static const dtUtil::RefString PROPERTY_PAINT_STATE;
            static const dtUtil::RefString PROPERTY_DIFFUSE_FRAME_SCALE;
            static const dtUtil::RefString PROPERTY_OVERLAY_FRAME_SCALE;
            static const dtUtil::RefString PROPERTY_OVERLAY_TEXTURE;

            // Uniform Names
            static const dtUtil::RefString UNIFORM_FRAME_OFFSET_AND_SCALES;
            static const dtUtil::RefString UNIFORM_OVERLAY_TEXTURE;

            BodyPaintStateActComp();

            ////////////////////////////////////////////////////////////////////
            // PROPERTY DECLARATIONS - getter, setter and member variable.
            ////////////////////////////////////////////////////////////////////
            DT_DECLARE_ACCESSOR(float, PaintState);
            DT_DECLARE_ACCESSOR(float, DiffuseFrameScale);
            DT_DECLARE_ACCESSOR(float, OverlayFrameScale);
            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, OverlayTexture);

            ////////////////////////////////////////////////////////////////////
            // OVERRIDE METHODS - ActorComponent
            ////////////////////////////////////////////////////////////////////

            /**
            * Handles the setup and registration of its properties.
            */
            virtual void BuildPropertyMap();
         
         protected:
            BodyPaintStateActComp(const ActorComponent::ACType& actType); // for derived classes

            virtual ~BodyPaintStateActComp();

            /**
            * Sets default values for any constructor overloads.
            * This should be called from constructors only.
            */
            virtual void SetDefaults();
      };

   }
}

#endif
