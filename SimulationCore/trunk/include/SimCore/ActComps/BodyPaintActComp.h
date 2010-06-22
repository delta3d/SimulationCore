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

#ifndef BODY_PAINT_ACT_COMP_H_
#define BODY_PAINT_ACT_COMP_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Vec4>
#include <dtDAL/resourcedescriptor.h>
#include <dtUtil/getsetmacros.h>
#include <dtGame/actorcomponentbase.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Node;
   class StateSet;
}



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT BodyPaintActComp : public dtGame::ActorComponent
      {
         public:
            typedef dtGame::ActorComponent BaseClass;

            static const ActorComponent::ACType TYPE;

            // Property Names
            static const dtUtil::RefString PROPERTY_PAINT_COLOR_1;
            static const dtUtil::RefString PROPERTY_PAINT_COLOR_2;
            static const dtUtil::RefString PROPERTY_PAINT_COLOR_3;
            static const dtUtil::RefString PROPERTY_PAINT_COLOR_4;
            static const dtUtil::RefString PROPERTY_MODEL_DIMENSIONS;
            static const dtUtil::RefString PROPERTY_PROJECTION_DIRECTION;
            static const dtUtil::RefString PROPERTY_REPLACEMENT_DIFFUSE_MASK_TEXTURE;
            static const dtUtil::RefString PROPERTY_PATTERN_TEXTURE;

            // Uniform Names
            static const dtUtil::RefString UNIFORM_PAINT_COLOR_1;
            static const dtUtil::RefString UNIFORM_PAINT_COLOR_2;
            static const dtUtil::RefString UNIFORM_PAINT_COLOR_3;
            static const dtUtil::RefString UNIFORM_PAINT_COLOR_4;
            static const dtUtil::RefString UNIFORM_MODEL_DIMENSIONS;
            static const dtUtil::RefString UNIFORM_PROJECTION_DIRECTION;
            static const dtUtil::RefString UNIFORM_REPLACEMENT_DIFFUSE_MASK_TEXTURE;
            static const dtUtil::RefString UNIFORM_PATTERN_TEXTURE;

            BodyPaintActComp();

            ////////////////////////////////////////////////////////////////////
            // PROPERTY DECLARATIONS - getter, setter and member variable.
            ////////////////////////////////////////////////////////////////////
            DECLARE_PROPERTY(osg::Vec4, PaintColor1);
            DECLARE_PROPERTY(osg::Vec4, PaintColor2);
            DECLARE_PROPERTY(osg::Vec4, PaintColor3);
            DECLARE_PROPERTY(osg::Vec4, PaintColor4);
            DECLARE_PROPERTY(osg::Vec4, ModelDimensions);
            DECLARE_PROPERTY(osg::Vec4, ProjectionDirection);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, ReplacementDiffuseMaskTexture);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, PatternTexture);

            ////////////////////////////////////////////////////////////////////
            // OVERRIDE METHODS - ActorComponent
            ////////////////////////////////////////////////////////////////////

            virtual void OnEnteredWorld();

            /**
            * Handles the setup and registration of its properties.
            */
            virtual void BuildPropertyMap();

            ////////////////////////////////////////////////////////////////////
            // CONVENIENCE METHODS
            ////////////////////////////////////////////////////////////////////
            osg::Node* GetOwnerNode();
            const osg::Node* GetOwnerNode() const;

            osg::StateSet* GetStateSet();
            const osg::StateSet* GetStateSet() const;

            void SetEffectEnabledOnNode(osg::Node& node, bool enabled);
            bool IsEffectEnabledOnNode(osg::Node& node) const;

            /**
             * Convenience function for calculating the dimensions of a model's bounding box.
             * @param node Node that has geometry under it that should be measured.
             * @return 3D dimensions of the node's bounding box, measured in meters.
             *         Vec4 is returned instead of Vec3 for the sake of code that could
             *         be setting Vec4 uniforms on shaders.
             */
            static osg::Vec4 GetDimensions(osg::Node& node);
         
         protected:
            BodyPaintActComp(const ActorComponent::ACType& actType); // for derived classes

            virtual ~BodyPaintActComp();

            /**
            * Sets default values for any constructor overloads.
            * This should be called from constructors only.
            */
            virtual void SetDefaults();

            void SetUniform(osg::StateSet* ss, const std::string& uniformName, const osg::Vec4& value);

            void SetUniform(osg::StateSet* ss, const std::string& uniformName, const dtDAL::ResourceDescriptor& value, int texUnit);

            void SetProperty(osg::Vec4& propertyToSet, const osg::Vec4& value,
               osg::StateSet* stateSetToUpdate, const std::string& shaderParamName);

            void SetProperty(dtDAL::ResourceDescriptor& propertyToSet, const dtDAL::ResourceDescriptor& value,
               osg::StateSet* stateSetToUpdate, const std::string& shaderParamName, int texUnit);
      };

   }
}

#endif
