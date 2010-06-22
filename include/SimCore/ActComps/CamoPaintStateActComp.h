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
#include <dtCore/observerptr.h>
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
            DECLARE_PROPERTY(int, CamoId);
            DECLARE_PROPERTY(bool, ConcealedState);
            DECLARE_PROPERTY(osg::Vec4, ConcealMeshDims);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, ConcealMesh);
            DECLARE_PROPERTY(std::string, ConcealShaderGroup);

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
            dtCore::RefPtr<osg::Node> mConcealMeshNode;
            dtCore::RefPtr<osg::MatrixTransform> mOffsetNode;
            dtCore::ObserverPtr<osg::Node> mHiderNode;
            dtCore::ObserverPtr<osg::Group> mParentNode;
      };

   }
}

#endif
