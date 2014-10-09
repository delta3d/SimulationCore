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
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <dtCore/shaderprogram.h>
#include <dtCore/shadermanager.h>
#include <dtCore/enginepropertytypes.h>
#include <dtCore/propertymacros.h>
#include <dtCore/project.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
#include <osg/ComputeBoundsVisitor>
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <SimCore/Actors/CamoConfigActor.h>
#include <SimCore/Actors/IGActor.h> // For SetNodeVisible function.
#include <SimCore/Actors/EntityActorRegistry.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CONSTANTS
      //////////////////////////////////////////////////////////////////////////
      const dtGame::ActorComponent::ACType CamoPaintStateActComp::TYPE(new dtCore::ActorType("CamoPaintStateActComp", "ActorComponents",
            "Camoflage painting actor component.", SimCore::ActComps::BodyPaintActComp::TYPE));
      
      // Property Names
      const dtUtil::RefString CamoPaintStateActComp::PROPERTY_CAMO_ID("Camo Id");
      const dtUtil::RefString CamoPaintStateActComp::PROPERTY_CONCEALED_STATE("Concealed State");
      const dtUtil::RefString CamoPaintStateActComp::PROPERTY_CONCEAL_SHADER_GROUP("Conceal Shader Group");
      const dtUtil::RefString CamoPaintStateActComp::PROPERTY_CONCEAL_MESH_DIMENSIONS("Conceal Mesh Dimensions");
      
      // Uniform Names
      const dtUtil::RefString CamoPaintStateActComp::UNIFORM_CONCEAL_MESH_DIMS("ConcealDims");


      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      CamoPaintStateActComp::CamoPaintStateActComp()
         : BaseClass(TYPE)
      {
         SetDefaults();
      }

      //////////////////////////////////////////////////////////////////////////
      CamoPaintStateActComp::~CamoPaintStateActComp()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetDefaults()
      {
         BaseClass::SetDefaults();

         mEnteredWorld = false;
         mCamoId = 0;
         mOffsetNode = new osg::MatrixTransform();
         mConcealedState = false;
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
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoPaintStateActComp, int, CamoId); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoPaintStateActComp, bool, ConcealedState); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoPaintStateActComp, osg::Vec4, ConcealMeshDims); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoPaintStateActComp, dtCore::ResourceDescriptor, ConcealMesh); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR(CamoPaintStateActComp, std::string, ConcealShaderGroup);

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetCamoId(int camoId)
      {
         mCamoId = camoId;

         const SimCore::Actors::CamoParams* camo = GetCamoParameters(camoId);
         if(camo != NULL)
         {
            SetPaintColor1(camo->GetColor1());
            SetPaintColor2(camo->GetColor2());
            SetPaintColor3(camo->GetColor3());
            SetPaintColor4(camo->GetColor4());
            SetPatternTexture(camo->GetPatternTexture());
            SetConcealMesh(camo->GetConcealMesh());

            // Set the master scale component on the pattern scale property.
            // This uniformly scales the whole effect.
            osg::Vec4 scale(GetPatternScale());
            scale.w() = camo->GetPatternScale();
            SetPatternScale(scale);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::UpdateConcealMeshDimsToFit()
      {
         if(mConcealMeshNode.valid())
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
               osg::ComputeBoundsVisitor visitor;
               visitor.apply(*node);

               const osg::BoundingBox& bb = visitor.getBoundingBox();
               if(bb.valid())
               {
                  osg::Vec4 dims = mOriginalConcealMeshDims;
                  if(dims.x() <= 0.0f)
                     dims.x() = bb.xMax() - bb.xMin();
                  if(dims.y() <= 0.0f)
                     dims.y() = bb.yMax() - bb.yMin();
                  if(dims.z() <= 0.0f)
                     dims.z() = bb.zMax() - bb.zMin();
                  dims.w() = 1.0f;
                  SetConcealMeshDims(dims);

                  // Offset the mesh to be centered over the 
                  osg::Matrix mtx = mOffsetNode->getMatrix();
                  osg::Vec3 offset(bb.center());
                  offset.z() = 0.0f;
                  mtx = osg::Matrix::identity();
                  osg::Vec3 scaleVec(dims.x(), dims.y(), dims.z());
                  mtx.makeScale(scaleVec);
                  mtx.setTrans(offset);
                  mOffsetNode->setMatrix(mtx);
               }
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetConcealMeshDims(const osg::Vec4& dims)
      {
         mConcealMeshDims = dims;

         // Set the original conceal dims, only if this is being set
         // before entering the sim world.
         if( ! mEnteredWorld)
         {
            mOriginalConcealMeshDims = dims;
         }

         // Conceal dimensions should only apply to the conceal net sub mesh.
         if(mConcealMeshNode.valid())
         {
            // Error check the dimensions before setting them in the shader.
            osg::Vec4 val(dims);
            if(val.x() == 0.0f)
               val.x() = 1.0f;
            if(val.y() == 0.0f)
               val.y() = 1.0f;
            if(val.z() == 0.0f)
               val.z() = 1.0f;
            if(val.w() == 0.0f)
               val.w() = 1.0f;

            // Set the value for the shader.
            osg::StateSet* ss = mConcealMeshNode->getOrCreateStateSet();
            SetUniform(ss, UNIFORM_CONCEAL_MESH_DIMS, dims);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetConcealMesh(const dtCore::ResourceDescriptor& file)
      {
         bool changed = mConcealMesh != file;
         if(changed || ( ! mConcealMeshNode.valid() && ! file.IsEmpty()))
         {
            mConcealMesh = file;

            // Update the concealment mesh if it is currently enabled.
            if(GetConcealedState())
            {
               SetConcealedState(true);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetConcealedState(bool enabled)
      {
         // Changing file. Cannot assume it is valid yet.
         mConcealedState = enabled;

         if(enabled && ! mConcealMesh.IsEmpty() && mEnteredWorld)
         {
            try
            {
               if(mConcealMeshNode.valid())
               {
                  DetachNode(*mOffsetNode);
                  mOffsetNode->removeChildren(0,mOffsetNode->getNumChildren());
               }

               std::string meshFile = dtCore::Project::GetInstance().GetResourcePath(mConcealMesh);
               mConcealMeshNode = osgDB::readNodeFile(meshFile);

               if(mConcealMeshNode.valid())
               {
                  // Add an offset transform node. This will be used
                  // to center the conceal mesh over the entity bounding box.
                  mOffsetNode->addChild(mConcealMeshNode.get());

                  AttachNode(*mOffsetNode);

                  // Ensure the new mesh fits the owner.
                  UpdateConcealMeshDimsToFit();
               }
            }
            catch(dtUtil::Exception& e)
            {
               LOG_WARNING(e.ToString());
            }
         }
         // Mesh file was not valid. Remove the current mesh if it exists.
         else
         {
            DetachNode(*mOffsetNode);
            mOffsetNode->removeChildren(0,mOffsetNode->getNumChildren());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::CamoParams* CamoPaintStateActComp::GetCamoParameters(int camoId)
      {
         using namespace SimCore::Actors;

         const CamoParams* camo = NULL;

         dtGame::GameActorProxy* actor = NULL;
         GetOwner(actor);
         if(actor != NULL)
         {
            dtGame::GameManager* gm = actor->GetGameManager();
            if(gm != NULL)
            {
               dtCore::BaseActorObject* configActor = NULL;
               gm->FindActorByType(*EntityActorRegistry::CAMO_CONFIG_ACTOR_TYPE, configActor);

               CamoConfigActor* configDraw = NULL;
               if(configActor != NULL)
               {
                  configActor->GetDrawable(configDraw);
               }

               if(configDraw != NULL)
               {
                  camo = configDraw->GetCamoParamsByCamoId(camoId);
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
               std::string shaderGroupName(mConcealShaderGroup);

               if(shaderGroupName.empty())
               {
                  // Find the shader group name applied to the owner.
                  dtCore::ActorProperty* shaderProp = NULL;
                  dtGame::GameActorProxy* actor = NULL;
                  GetOwner(actor);
                  if(actor != NULL)
                  {
                     shaderProp = actor->GetProperty("ShaderGroup");
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

         // Let this component know that the actor has now entered the world,
         // and thus handle changing values accordingly.
         mEnteredWorld = true;

         // Ensure the root entity is not distorted by the conceal scale.
         osg::Vec4 unitScale(1.0f, 1.0f, 1.0f, 1.0f);
         SetUniform(GetStateSet(), UNIFORM_CONCEAL_MESH_DIMS, unitScale);

         // Ensure the proper camo/conceal mesh is loaded.
         // This subsequently adds a conceal mesh if the actor is
         // flagged as concealed and if the mesh has been specified
         // for the camo type.
         SetCamoId(mCamoId);
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         typedef dtCore::PropertyRegHelper<CamoPaintStateActComp&, CamoPaintStateActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Camo Paint Id");

         // INT PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            CamoId,
            PROPERTY_CAMO_ID,
            PROPERTY_CAMO_ID,
            "Id of the camo object (from the Camo Config Actor) that specifies the color parameters and pattern texture.",
            PropRegType, propRegHelper);

         // BOOL PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            ConcealedState,
            PROPERTY_CONCEALED_STATE,
            PROPERTY_CONCEALED_STATE,
            "Flag that determines if a concealed mesh should be visble or not, if the mesh is specified and attached.",
            PropRegType, propRegHelper);

         // STRING PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            ConcealShaderGroup,
            PROPERTY_CONCEAL_SHADER_GROUP,
            PROPERTY_CONCEAL_SHADER_GROUP,
            "Shader group to be applied to the conceal mesh.",
            PropRegType, propRegHelper);

         // VEC4 PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            ConcealMeshDims,
            PROPERTY_CONCEAL_MESH_DIMENSIONS,
            PROPERTY_CONCEAL_MESH_DIMENSIONS,
            "Dimensions to use for the concealment mesh when the actor is concealed. Using a value of 0 or less will cause the dimensions to be automatically calculated based on the actor's geometry.",
            PropRegType, propRegHelper);
      }

   } // ActComps namespace
} // SimCore namespace
