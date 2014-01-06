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
#include <osg/MatrixTransform>
#include <osg/Vec2>
#include <dtUtil/refcountedbase.h>
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
            static const dtUtil::RefString PROPERTY_CONCEALED_STATE;
            static const dtUtil::RefString PROPERTY_CONCEAL_SHADER_GROUP;
            static const dtUtil::RefString PROPERTY_CONCEAL_MESH_DIMENSIONS;

            // Uniform Names
            static const dtUtil::RefString UNIFORM_CONCEAL_MESH_DIMS;

            CamoPaintStateActComp();

            void SetParentNode(osg::Group* node);
            osg::Group* GetParentNode();
            const osg::Group* GetParentNode() const;
            
            void SetHiderNode(osg::Node* node);
            osg::Node* GetHiderNode();
            const osg::Node* GetHiderNode() const;

            void UpdateConcealMeshDimsToFit();

            ////////////////////////////////////////////////////////////////////
            // PROPERTY DECLARATIONS - getter, setter and member variable.
            ////////////////////////////////////////////////////////////////////
            DT_DECLARE_ACCESSOR(int, CamoId);
            DT_DECLARE_ACCESSOR(bool, ConcealedState);
            DT_DECLARE_ACCESSOR(osg::Vec4, ConcealMeshDims);
            DT_DECLARE_ACCESSOR(dtDAL::ResourceDescriptor, ConcealMesh);
            DT_DECLARE_ACCESSOR(std::string, ConcealShaderGroup);

            ////////////////////////////////////////////////////////////////////
            // OVERRIDE METHODS - ActorComponent
            ////////////////////////////////////////////////////////////////////

            /**
            * Handles final setup of the owner before entering the seen.
            */
            virtual void OnEnteredWorld();

            /**
            * Handles the setup and registration of its properties.
            */
            virtual void BuildPropertyMap();

         protected:
            virtual ~CamoPaintStateActComp();

            virtual void SetDefaults();

            const SimCore::Actors::CamoParams* GetCamoParameters(int camoId);

            bool AttachNode(osg::Node& node);
            bool DetachNode(osg::Node& node);

         private:
            bool mEnteredWorld;
            osg::Vec4 mOriginalConcealMeshDims; // Captures originally set values before entering the sim world.
            osg::ref_ptr<osg::Node> mConcealMeshNode;
            osg::ref_ptr<osg::MatrixTransform> mOffsetNode;
            osg::observer_ptr<osg::Node> mHiderNode;
            osg::observer_ptr<osg::Group> mParentNode;
      };

   }
}

#endif
