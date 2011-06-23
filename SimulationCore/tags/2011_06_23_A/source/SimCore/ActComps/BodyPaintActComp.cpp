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
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/propertymacros.h>
#include <dtDAL/project.h>
#include <dtGame/gameactor.h>
#include <dtUtil/boundingshapeutils.h>
#include <SimCore/ActComps/BodyPaintActComp.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CONSTANTS
      //////////////////////////////////////////////////////////////////////////
      const dtGame::ActorComponent::ACType BodyPaintActComp::TYPE("BodyPaintActComp");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PAINT_COLOR_1("Paint Color 1");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PAINT_COLOR_2("Paint Color 2");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PAINT_COLOR_3("Paint Color 3");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PAINT_COLOR_4("Paint Color 4");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PATTERN_SCALE("Pattern Scale");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PROJECTION_DIRECTION("Projection Direction");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_REPLACEMENT_DIFFUSE_MASK_TEXTURE("Replacement Diffuse Mask Texture");
      const dtUtil::RefString BodyPaintActComp::PROPERTY_PATTERN_TEXTURE("Pattern Texture"); 

      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PAINT_COLOR_1("PaintColor1");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PAINT_COLOR_2("PaintColor2");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PAINT_COLOR_3("PaintColor3");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PAINT_COLOR_4("PaintColor4");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PATTERN_SCALE("PatternScale");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PROJECTION_DIRECTION("ProjectionDir");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_REPLACEMENT_DIFFUSE_MASK_TEXTURE("diffuseTexture");
      const dtUtil::RefString BodyPaintActComp::UNIFORM_PATTERN_TEXTURE("PatternTexture");



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      BodyPaintActComp::BodyPaintActComp()
         : BaseClass(TYPE)
      {
         SetDefaults();
      }

      //////////////////////////////////////////////////////////////////////////
      BodyPaintActComp::BodyPaintActComp(const ActorComponent::ACType& actType) // for derived classes to call
         : BaseClass(actType)
      {
         SetDefaults();
      }

      //////////////////////////////////////////////////////////////////////////
      BodyPaintActComp::~BodyPaintActComp()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetDefaults()
      {
         // Set model dimensions to a non-zero value.
         mPatternScale.set(1.0f,1.0f,1.0f,1.0f);

         // Set model dimensions to a non-zero value.
         // The alpha component determines if the effect is
         // fully applied (1) or not (0).
         mProjectionDirection.set(1.0f,1.0f,1.0f,1.0f);
      }



      //////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter and Setter method body for each property
      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, osg::Vec4, PaintColor1);
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, osg::Vec4, PaintColor2);
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, osg::Vec4, PaintColor3);
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, osg::Vec4, PaintColor4);
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, osg::Vec4, PatternScale);
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, osg::Vec4, ProjectionDirection);
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, dtDAL::ResourceDescriptor, ReplacementDiffuseMaskTexture); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintActComp, dtDAL::ResourceDescriptor, PatternTexture); // Setter is implemented below

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetPaintColor1(const osg::Vec4& color)
      {
         SetProperty(mPaintColor1, color, GetStateSet(), UNIFORM_PAINT_COLOR_1);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetPaintColor2(const osg::Vec4& color)
      {
         SetProperty(mPaintColor2, color, GetStateSet(), UNIFORM_PAINT_COLOR_2);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetPaintColor3(const osg::Vec4& color)
      {
         SetProperty(mPaintColor3, color, GetStateSet(), UNIFORM_PAINT_COLOR_3);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetPaintColor4(const osg::Vec4& color)
      {
         SetProperty(mPaintColor4, color, GetStateSet(), UNIFORM_PAINT_COLOR_4);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetPatternScale(const osg::Vec4& dims)
      {
         SetProperty(mPatternScale, dims, GetStateSet(), UNIFORM_PATTERN_SCALE);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetProjectionDirection(const osg::Vec4& direction)
      {
         SetProperty(mProjectionDirection, direction, GetStateSet(),
            UNIFORM_PROJECTION_DIRECTION);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetReplacementDiffuseMaskTexture(const dtDAL::ResourceDescriptor& file)
      {
         SetProperty(mReplacementDiffuseMaskTexture, file, GetStateSet(),
            UNIFORM_REPLACEMENT_DIFFUSE_MASK_TEXTURE, 0);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetPatternTexture(const dtDAL::ResourceDescriptor& file)
      {
         SetProperty(mPatternTexture, file, GetStateSet(), UNIFORM_PATTERN_TEXTURE, 1);
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Node* BodyPaintActComp::GetOwnerNode()
      {
         dtGame::GameActor* actor = NULL;
         GetOwner(actor);
         return actor != NULL ? actor->GetOSGNode() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Node* BodyPaintActComp::GetOwnerNode() const
      {
         const dtGame::GameActor* actor = NULL;
         GetOwner(actor);
         return actor != NULL ? actor->GetOSGNode() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      osg::StateSet* BodyPaintActComp::GetStateSet()
      {
         osg::Node* node = GetOwnerNode();
         return node != NULL ? node->getOrCreateStateSet() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::StateSet* BodyPaintActComp::GetStateSet() const
      {
         const osg::Node* node = GetOwnerNode();
         return node != NULL ? node->getStateSet() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetEffectEnabledOnNode(osg::Node& node, bool enabled)
      {
         osg::StateSet* ss = node.getOrCreateStateSet();
         osg::Vec4 value(mProjectionDirection);
         value.w() = enabled ? 1.0f : 0.0f;
         SetUniform(ss, UNIFORM_PROJECTION_DIRECTION, value);
      }

      //////////////////////////////////////////////////////////////////////////
      bool BodyPaintActComp::IsEffectEnabledOnNode(osg::Node& node) const
      {
         osg::Vec4 value;
         const osg::StateSet* ss = node.getStateSet();
         if(ss != NULL)
         {
            const osg::Uniform* uniform = ss->getUniform(UNIFORM_PROJECTION_DIRECTION);
            if(uniform != NULL)
            {
               uniform->get(value);
            }
         }
         return value.w() > 0.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Vec4 BodyPaintActComp::GetDimensions(osg::Node& node)
      {
         dtUtil::BoundingBoxVisitor visitor;
         visitor.apply(node);

         const osg::BoundingBox& bb = visitor.mBoundingBox;
         osg::Vec4 dims(
            bb.xMax() - bb.xMin(),
            bb.yMax() - bb.yMin(),
            bb.zMax() - bb.zMin(), 1.0f);

         return dims;
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetUniform(osg::StateSet* ss, const std::string& uniformName, const osg::Vec4& value)
      {
         if(ss != NULL)
         {
            ss->getOrCreateUniform(uniformName, osg::Uniform::FLOAT_VEC4)->set(value);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetUniform(osg::StateSet* ss, const std::string& uniformName, const dtDAL::ResourceDescriptor& value, int texUnit)
      {
         if(ss != NULL && ! value.IsEmpty())
         {
            try
            {
               osg::Uniform* uniform = ss->getOrCreateUniform(uniformName, osg::Uniform::SAMPLER_2D);
               uniform->set(texUnit);

               dtCore::RefPtr<osg::Texture2D> tex = new osg::Texture2D();
               std::string textureFile = dtDAL::Project::GetInstance().GetResourcePath(value);
               
               dtCore::RefPtr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
               options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_ALL);

               dtCore::RefPtr<osg::Image> newImage = osgDB::readImageFile(textureFile, options.get());
               if(newImage == NULL)
               {
                  LOG_ERROR("BodyPaintActComp failed to load texture file [" + textureFile + "].");
               }
               else
               {
                  tex->setImage(newImage.get());
                  tex->dirtyTextureObject();
                  tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR_MIPMAP_LINEAR);
                  tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
                  tex->setWrap(osg::Texture::WRAP_S, osg::Texture::REPEAT);
                  tex->setWrap(osg::Texture::WRAP_T, osg::Texture::REPEAT);
                  tex->setUnRefImageDataAfterApply(true);
                  ss->addUniform(uniform);
                  ss->setTextureAttributeAndModes(texUnit, tex, osg::StateAttribute::ON);
               }
            }
            catch (const dtUtil::Exception& ex)
            {
               // Do not crash. Just do not make the texture.
               LOGN_ERROR("BodyPaintActComp.cpp", ex.ToString());
            }
            catch (...)
            {
               // Do not crash. Just do not make the texture.
               LOGN_ERROR("BodyPaintActComp.cpp", "Unknown exception trying to assign the stateset for texture \""
                        + value.GetResourceIdentifier() + "\" for body paint.");
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetProperty(osg::Vec4& propertyToSet, const osg::Vec4& value,
         osg::StateSet* stateSetToUpdate, const std::string& shaderParamName)
      {
         propertyToSet = value;
         SetUniform(stateSetToUpdate, shaderParamName, value);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::SetProperty(dtDAL::ResourceDescriptor& propertyToSet, const dtDAL::ResourceDescriptor& value,
         osg::StateSet* stateSetToUpdate, const std::string& shaderParamName, int texUnit)
      {
         propertyToSet = value;
         SetUniform(stateSetToUpdate, shaderParamName, value, texUnit);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();

         // Set the dimensions for the model if they were not specified.
         if(mPatternScale.length() == 0.0f)
         {
            osg::Vec4 scale(1.0f, 1.0f, 1.0f, 1.0f);
            SetPatternScale(scale);
         }

         SetProjectionDirection(mProjectionDirection);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintActComp::BuildPropertyMap()
      {
         typedef dtDAL::PropertyRegHelper<BodyPaintActComp&, BodyPaintActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Body Paint");

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            PaintColor1,
            PROPERTY_PAINT_COLOR_1,
            PROPERTY_PAINT_COLOR_1,
            "Top most color.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            PaintColor2,
            PROPERTY_PAINT_COLOR_2,
            PROPERTY_PAINT_COLOR_2,
            "Second top most color.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            PaintColor3,
            PROPERTY_PAINT_COLOR_3,
            PROPERTY_PAINT_COLOR_3,
            "Third top most color.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            PaintColor4,
            PROPERTY_PAINT_COLOR_4,
            PROPERTY_PAINT_COLOR_4,
            "Base/bottom most color to be used if no other colors have been used.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            PatternScale,
            PROPERTY_PATTERN_SCALE,
            PROPERTY_PATTERN_SCALE,
            "Optional 3D vector defining the scale of the pattern effect. This stretches the pattern in X & Y relative to projection space. One tile of the pattern is equal to one unit of world space.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            ProjectionDirection,
            PROPERTY_PROJECTION_DIRECTION,
            PROPERTY_PROJECTION_DIRECTION,
            "Optional 3D vector defining the direction in which to orthogonally project the pattern texture onto the model.",
            PropRegType, propRegHelper);

         // FILE PROPERTIES
         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(
            dtDAL::DataType::TEXTURE,
            ReplacementDiffuseMaskTexture,
            PROPERTY_REPLACEMENT_DIFFUSE_MASK_TEXTURE,
            PROPERTY_REPLACEMENT_DIFFUSE_MASK_TEXTURE,
            "Texture used as the replacement diffuse texture, which also has an alpha channel for masking off the entire paint effect from the rest of the model.",
            PropRegType, propRegHelper);

         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(
            dtDAL::DataType::TEXTURE,
            PatternTexture,
            PROPERTY_PATTERN_TEXTURE,
            PROPERTY_PATTERN_TEXTURE,
            "Texture used as the pattern for body painting.",
            PropRegType, propRegHelper);
      }

   } // ActComps namespace
} // SimCore namespace
