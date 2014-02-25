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
#include <dtCore/enginepropertytypes.h>
#include <dtCore/propertymacros.h>
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
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_PAINT_STATE("Paint State");
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_DIFFUSE_FRAME_SCALE("Diffuse Frame Scale");
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_OVERLAY_FRAME_SCALE("Overlay Frame Scale");
      const dtUtil::RefString BodyPaintStateActComp::PROPERTY_OVERLAY_TEXTURE("Overlay Texture");

      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString BodyPaintStateActComp::UNIFORM_FRAME_OFFSET_AND_SCALES("FrameOffsetAndScales");
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
      BodyPaintStateActComp::BodyPaintStateActComp(const ActorComponent::ACType& actType)
         : BaseClass(actType)
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

         mPaintState = 0.0f;
         mDiffuseFrameScale = 1.0f;
         mOverlayFrameScale = 1.0f;
      }



      //////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter method body for each property
      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintStateActComp, float, PaintState); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintStateActComp, float, DiffuseFrameScale); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintStateActComp, float, OverlayFrameScale); // Setter is implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(BodyPaintStateActComp, dtCore::ResourceDescriptor, OverlayTexture); // Setter is implemented below

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetPaintState(float state)
      {
         mPaintState = state;

         osg::Vec4 offsetAndScale(
            state,
            mDiffuseFrameScale,
            mOverlayFrameScale,
            0.0f);
         SetUniform(GetStateSet(), UNIFORM_FRAME_OFFSET_AND_SCALES, offsetAndScale);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetDiffuseFrameScale(float scale)
      {
         mDiffuseFrameScale = scale;

         osg::Vec4 offsetAndScale(
            mPaintState,
            mDiffuseFrameScale,
            mOverlayFrameScale,
            0.0f);
         SetUniform(GetStateSet(), UNIFORM_FRAME_OFFSET_AND_SCALES, offsetAndScale);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetOverlayFrameScale(float scale)
      {
         mOverlayFrameScale = scale;

         osg::Vec4 offsetAndScale(
            mPaintState,
            mDiffuseFrameScale,
            mOverlayFrameScale,
            0.0f);
         SetUniform(GetStateSet(), UNIFORM_FRAME_OFFSET_AND_SCALES, offsetAndScale);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::SetOverlayTexture(const dtCore::ResourceDescriptor& file)
      {
         SetProperty(mOverlayTexture, file, GetStateSet(), UNIFORM_OVERLAY_TEXTURE, 2);
      }

      //////////////////////////////////////////////////////////////////////////
      void BodyPaintStateActComp::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         typedef dtCore::PropertyRegHelper<BodyPaintStateActComp&, BodyPaintStateActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Body Paint (States)");

         // VEC PROPERTIES
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            PaintState,
            PROPERTY_PAINT_STATE,
            PROPERTY_PAINT_STATE,
            "The whole number frame offset into the diffuse and overlay textures, to shift their UV coordinates to a certain image/frame, as if the texture were a series of images. The frame offset multiplies with each frame scale to determine the appropriate linear offset for UVs.",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            DiffuseFrameScale,
            PROPERTY_DIFFUSE_FRAME_SCALE,
            PROPERTY_DIFFUSE_FRAME_SCALE,
            "The scale factor of imagery in the diffuse texture, as if the texture were a series of images. The scale is relative to the direction images are lain out (horizontal or vertical) and is equal to 1/number of frames",
            PropRegType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            OverlayFrameScale,
            PROPERTY_OVERLAY_FRAME_SCALE,
            PROPERTY_OVERLAY_FRAME_SCALE,
            "The scale factor of imagery in the overlay texture, as if the texture were a series of images. The scale is relative to the direction images are lain out (horizontal or vertical) and is equal to 1/number of frames",
            PropRegType, propRegHelper);

         // FILE PROPERTIES
         DT_REGISTER_RESOURCE_PROPERTY_WITH_NAME(
            dtCore::DataType::TEXTURE,
            OverlayTexture,
            PROPERTY_OVERLAY_TEXTURE,
            PROPERTY_OVERLAY_TEXTURE,
            "Texture used as an overlay effect, such as for mud, decals, damage, etc.",
            PropRegType, propRegHelper);
      }

   } // ActComps namespace
} // SimCore namespace
