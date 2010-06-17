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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osgDB/ReadFile>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/propertymacros.h>
#include <dtDAL/project.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
#include <dtUtil/boundingshapeutils.h>
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <SimCore/Actors/CamoConfigActor.h>
#include <SimCore/Actors/IGActor.h> // For SetNodeVisible function.
#include <SimCore/Actors/EntityActorRegistry.h>

// DEBUG:
#include <iostream>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CONSTANTS
      //////////////////////////////////////////////////////////////////////////
      const dtGame::ActorComponent::ACType CamoPaintStateActComp::TYPE("CamoPaintStateActComp");
      
      // Property Names
      const dtUtil::RefString CamoPaintStateActComp::PROPERTY_CAMO_ID("Camo Id");
      
      // Uniform Names
      const dtUtil::RefString CamoPaintStateActComp::UNIFORM_CONCEAL_MESH_DIMS("ConcealDims");


      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      CamoPaintStateActComp::CamoPaintStateActComp()
         : BaseClass(TYPE)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      CamoPaintStateActComp::~CamoPaintStateActComp()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetDefaults()
      {
         BaseClass::SetDefaults();

         // Ensure the concealed mesh has a unit scale as default,
         // whenever it is attached later without setting the dimensions.
         mConcealMeshDims.set(1.0f,1.0f,1.0f,1.0f);
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetParentNode(osg::Group* node)
      {
         mParentNode = node;
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Group* CamoPaintStateActComp::GetParentNode()
      {
         return mParentNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Group* CamoPaintStateActComp::GetParentNode() const
      {
         return mParentNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetHiderNode(osg::Node* node)
      {
         mHiderNode = node;
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Node* CamoPaintStateActComp::GetHiderNode()
      {
         return mHiderNode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Node* CamoPaintStateActComp::GetHiderNode() const
      {
         return mHiderNode.get();
      }



      //////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter method body for each property
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_PROPERTY_GETTER(CamoPaintStateActComp, int, CamoId); // Setter is implemented below
      IMPLEMENT_PROPERTY_GETTER(CamoPaintStateActComp, osg::Vec4, ConcealMeshDims); // Setter is implemented below
      IMPLEMENT_PROPERTY_GETTER(CamoPaintStateActComp, dtDAL::ResourceDescriptor, ConcealMesh); // Setter is implemented below
      IMPLEMENT_PROPERTY(CamoPaintStateActComp, std::string, ConcealShaderGroup);

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetCamoId(int camoId)
      {
         const SimCore::Actors::CamoParams* camo = GetCamoParameters(camoId);
         if(camo != NULL)
         {
            SetPaintColor1(camo->GetColor1());
            SetPaintColor2(camo->GetColor2());
            SetPaintColor3(camo->GetColor3());
            SetPaintColor4(camo->GetColor4());
            SetPatternTexture(camo->GetPatternTexture());
            SetConcealMesh(camo->GetConcealMesh());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::UpdateConcealMeshDimsToFit()
      {
         // Determine some node to get the dimensions from.
         // If the hider node is not specified, get the parent.
         osg::Node* node = NULL;
         if(mHiderNode.valid())
         {
            node = mHiderNode.get();
         }
         else if(mParentNode.valid())
         {
            node = mParentNode.get();
         }
         else // Get the root node...it may or may not be the immediate parent node.
         {
            node = GetOwnerNode();
         }

         // Set the dimensions of the conceal mesh.
         if(node != NULL)
         {
            dtUtil::BoundingBoxVisitor visitor;
            visitor.apply(*node);

            const osg::BoundingBox& bb = visitor.mBoundingBox;
            osg::Vec4 dims(
               bb.xMax() - bb.xMin(),
               bb.yMax() - bb.yMin(),
               bb.zMax() - bb.zMin(), 1.0f);
            SetConcealMeshDims(dims);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetConcealMeshDims(const osg::Vec4& dims)
      {
         mConcealMeshDims = dims;

         // Conceal dimensions should only apply to the conceal net sub mesh.
         if(mConcealMeshNode.valid())
         {
            osg::StateSet* ss = mConcealMeshNode->getOrCreateStateSet();
            SetUniform(ss, UNIFORM_CONCEAL_MESH_DIMS, dims);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetConcealMesh(const dtDAL::ResourceDescriptor& file)
      {
         // DEBUG:
         std::cout << "\nGot file: " << file.GetResourceIdentifier() << "\n";

         bool changed = mConcealMesh != file;
         bool fileValid = ! changed;
         if(changed || ( ! mConcealMeshNode.valid() && ! file.IsEmpty()))
         {
            mConcealMesh = file;

            // DEBUG:
            std::cout << "\tLoading file: " << file.GetResourceIdentifier() << "\n";

            // Changing file. Cannot assume it is valid yet.
            fileValid = false;

            if( ! file.IsEmpty())
            {
               try
               {
                  if(mConcealMeshNode.valid())
                  {
                     // DEBUG:
                     std::cout << "\t\tDETACH\n\n";

                     DetachNode(*mConcealMeshNode);
                  }

                  std::string meshFile = dtDAL::Project::GetInstance().GetResourcePath(file);
                  mConcealMeshNode = osgDB::readNodeFile(meshFile);

                  if(mConcealMeshNode.valid())
                  {
                     // A valid file was loaded.
                     fileValid = true;

                     // DEBUG:
                     std::cout << "\t\tATTACH\n\n";

                     AttachNode(*mConcealMeshNode);

                     // Ensure the new mesh fits the owner.
                     UpdateConcealMeshDimsToFit();
                  }
               }
               catch(dtUtil::Exception& e)
               {
                  LOG_WARNING(e.ToString());
               }
            }
         }
         
         // Mesh file was not valid. Remove the current mesh if it exists.
         if( ! fileValid && mConcealMeshNode.valid())
         {
            // DEBUG:
            std::cout << "\t\tDETACH - Default\n\n";

            DetachNode(*mConcealMeshNode);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::CamoParams* CamoPaintStateActComp::GetCamoParameters(int camoId)
      {
         using namespace SimCore::Actors;

         const CamoParams* camo = NULL;

         dtGame::GameActor* actor = NULL;
         GetOwner(actor);
         if(actor != NULL)
         {
            dtGame::GameManager* gm = actor->GetGameActorProxy().GetGameManager();
            if(gm != NULL)
            {
               dtDAL::ActorProxy* proxy = NULL;
               gm->FindActorByType(*EntityActorRegistry::CAMO_CONFIG_ACTOR_TYPE, proxy);

               CamoConfigActor* actor = NULL;
               if(proxy != NULL)
               {
                  proxy->GetActor(actor);
               }

               if(actor != NULL)
               {
                  camo = actor->GetCamoParamsByCamoId(camoId);
               }
            }
         }

         return camo;
      }

      //////////////////////////////////////////////////////////////////////////
      bool CamoPaintStateActComp::AttachNode(osg::Node& node)
      {
         using namespace dtCore;

         bool success = false;

         if( ! mParentNode.valid())
         {
            mParentNode = dynamic_cast<osg::Group*>(GetOwnerNode());
         }

         if(mParentNode.valid())
         {
            success = mParentNode->addChild(&node);

            if(success)
            {
               // HACK:
               mConcealShaderGroup = "ConcealCamoPaintGroup";

               std::string shaderGroupName(mConcealShaderGroup);

               if(shaderGroupName.empty())
               {
                  // Find the shader group name applied to the owner.
                  dtDAL::ActorProperty* shaderProp = NULL;
                  dtGame::GameActor* gameActor = dynamic_cast<dtGame::GameActor*>(GetOwner());
                  if(gameActor != NULL)
                  {
                     shaderProp = gameActor->GetGameActorProxy().GetProperty("ShaderGroup");
                     if(shaderProp != NULL)
                     {
                        shaderGroupName = (shaderProp->ToString());
                     }
                  }
               }

               // Find the shader prototype that the owner used.
               ShaderManager& sm = ShaderManager::GetInstance();
               ShaderGroup* shaderGroup = sm.FindShaderGroupPrototype(shaderGroupName);
               ShaderProgram* shader = shaderGroup == NULL ? NULL : shaderGroup->GetDefaultShader();
               if(shader != NULL)
               {
                  // Assign a copy of the shader.
                  sm.AssignShaderFromPrototype(*shader, node);
               }
            }
         }

         if(success && mHiderNode.valid())
         {
            // Hide the current node.
            SimCore::Actors::IGActor::SetNodeVisible(false, *mHiderNode);
         }
         
         return success;
      }
      
      //////////////////////////////////////////////////////////////////////////
      bool CamoPaintStateActComp::DetachNode(osg::Node& node)
      {
         bool success = false;
         
         if(mParentNode.valid())
         {
            // Remove the shader instance from the node.
            dtCore::ShaderManager::GetInstance().UnassignShaderFromNode(node);

            // Remove the node from the owner.
            success = mParentNode->removeChild(&node);
         }

         if(mHiderNode.valid())
         {
            // Hide the current node.
            SimCore::Actors::IGActor::SetNodeVisible(true, *mHiderNode);
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         // Ensure the root entity is not distorted by the conceal scale.
         osg::Vec4 unitScale(1.0f, 1.0f, 1.0f, 1.0f);
         SetUniform(GetStateSet(), UNIFORM_CONCEAL_MESH_DIMS, unitScale);

         UpdateConcealMeshDimsToFit();
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         typedef dtDAL::PropertyRegHelper<CamoPaintStateActComp&, CamoPaintStateActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Camo Paint Id");

         // INT PROPERTIES
         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            CamoId,
            PROPERTY_CAMO_ID,
            PROPERTY_CAMO_ID,
            "Id of the camo object (from the Camo Config Actor) that specifies the color parameters and pattern texture.",
            PropRegType, propRegHelper);
      }

   } // ActComps namespace
} // SimCore namespace
