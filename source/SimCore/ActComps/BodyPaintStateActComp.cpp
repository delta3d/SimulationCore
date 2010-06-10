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
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/propertymacros.h>
#include <dtGame/gameactor.h>
#include <SimCore/ActComps/BodyPaintStateActComp.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CONSTANTS
      //////////////////////////////////////////////////////////////////////////
      const dtGame::ActorComponent::ACType BodyPaintStateActComp::TYPE("BodyPaintStateActComp");
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_DIFFUSE_FRAME_OFFSET("Diffuse Frame Offset");
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_DIFFUSE_FRAME_SCALE("Diffuse Frame Scale");
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_OVERLAY_TEXTURE("Overlay Texture");

      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString BodyPaintStateActComp::UNIFORM_DIFFUSE_FRAME_OFFSET_AND_SCALE("DiffuseOffsetAndScale");
      const dtUtil::RefString BodyPaintStateActComp::UNIFORM_OVERLAY_TEXTURE("OverlayTexture");



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      BodyPaintStateActComp::BodyPaintStateActComp()
         : BaseClass(TYPE)
      {
         SetDefaults();
      }

      //////////////////////////////////////////////////////////////////////////
      BodyPaintStateActComp::~BodyPaintStateActComp()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetDefaults()
      {
         BaseClass::SetDefaults();

         mDiffuseFrameOffset.set(0.0f, 0.0f);
         mDiffuseFrameScale.set(1.0f, 1.0f);
      }



      //////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter method body for each property
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_PROPERTY_GETTER(BodyPaintStateActComp, osg::Vec2, DiffuseFrameOffset); // Setter is implemented below
      IMPLEMENT_PROPERTY_GETTER(BodyPaintStateActComp, osg::Vec2, DiffuseFrameScale); // Setter is implemented below
      IMPLEMENT_PROPERTY_GETTER(BodyPaintStateActComp, dtDAL::ResourceDescriptor, OverlayTexture); // Setter is implemented below

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetDiffuseFrameOffset(const osg::Vec2& offset)
      {
         mDiffuseFrameOffset.set(offset.x(), offset.y());

         osg::Vec4 offsetAndScale(
            offset.x(), offset.y(),
            mDiffuseFrameScale.x(),
            mDiffuseFrameScale.y());
         SetUniform(GetStateSet(), UNIFORM_DIFFUSE_FRAME_OFFSET_AND_SCALE, offsetAndScale);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetDiffuseFrameScale(const osg::Vec2& scale)
      {
         mDiffuseFrameScale.set(scale.x(), scale.y());

         osg::Vec4 offsetAndScale(
            mDiffuseFrameOffset.x(),
            mDiffuseFrameOffset.y(),
            scale.x(), scale.y());
         SetUniform(GetStateSet(), UNIFORM_DIFFUSE_FRAME_OFFSET_AND_SCALE, offsetAndScale);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetOverlayTexture(const dtDAL::ResourceDescriptor& file)
      {
         SetProperty(mOverlayTexture, file, GetStateSet(), UNIFORM_OVERLAY_TEXTURE, 2);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         typedef dtDAL::PropertyRegHelper<BodyPaintStateActComp&, BodyPaintStateActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Body Paint (States)");

         // VEC PROPERTIES
         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            DiffuseFrameOffset,
            PROPERTY_DIFFUSE_FRAME_OFFSET,
            PROPERTY_DIFFUSE_FRAME_OFFSET,
            "The offset into the diffuse texture to shift the UV coordinates to a certain image, as if the texture were a series of images.",
            PropRegType, propRegHelper);

         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            DiffuseFrameScale,
            PROPERTY_DIFFUSE_FRAME_SCALE,
            PROPERTY_DIFFUSE_FRAME_SCALE,
            "The scale factor of imagery in the diffuse texture, as if the texture were a series of images.",
            PropRegType, propRegHelper);

         // FILE PROPERTIES
         REGISTER_RESOURCE_PROPERTY_WITH_NAME(
            dtDAL::DataType::TEXTURE,
            OverlayTexture,
            PROPERTY_OVERLAY_TEXTURE,
            PROPERTY_OVERLAY_TEXTURE,
            "Texture used as an overlay effect, such as for mud, decals, damage, etc.",
            PropRegType, propRegHelper);
      }

   } // ActComps namespace
} // SimCore namespace
