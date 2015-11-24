/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
 * @author Chris Rodgers
 */

#ifndef _BASE_HUD_ELEMENTS_H_
#define _BASE_HUD_ELEMENTS_H_

#include <SimCore/Export.h>
#include <dtCore/base.h>

#include <dtUtil/warningdisable.h>
DT_DISABLE_WARNING_ALL_START
#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>
#include <CEGUI/CEGUIVersion.h>
DT_DISABLE_WARNING_END

#include <dtGUI/gui.h>

#include <dtCore/deltawin.h>
#include <dtCore/deltadrawable.h>

#include <dtUtil/exception.h>
#include <dtUtil/enumeration.h>

#include <osg/ref_ptr>
#include <osg/Projection>
#include <osg/Group>

namespace SimCore
{
   namespace Components
   {
      class SIMCORE_EXPORT BaseHUDElementException : public dtUtil::Exception
      {
      public:
         BaseHUDElementException(const std::string& message, const std::string& filename, unsigned int linenum);
         virtual ~BaseHUDElementException();
      };

      class SIMCORE_EXPORT HUDAlignment : public dtUtil::Enumeration
      {
         DECLARE_ENUM(HUDAlignment);
         public:
            static HUDAlignment LEFT_TOP;
            static HUDAlignment LEFT_CENTER;
            static HUDAlignment LEFT_BOTTOM;
            static HUDAlignment CENTER_TOP;
            static HUDAlignment CENTER;
            static HUDAlignment CENTER_BOTTOM;
            static HUDAlignment RIGHT_TOP;
            static HUDAlignment RIGHT_CENTER;
            static HUDAlignment RIGHT_BOTTOM;

            static HUDAlignment& ClassifyAlignment( CEGUI::Window& window );

            CEGUI::HorizontalAlignment GetAlignH() const { return mAlignH; }
            CEGUI::VerticalAlignment GetAlignV() const { return mAlignV; }
            
         private:
            HUDAlignment(const std::string &name, 
               CEGUI::HorizontalAlignment alignH, CEGUI::VerticalAlignment alignV ) 
               : dtUtil::Enumeration(name),
               mAlignH(alignH),
               mAlignV(alignV)
            {
               AddInstance(this);
            }

            CEGUI::HorizontalAlignment mAlignH;
            CEGUI::VerticalAlignment mAlignV;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Element:
      // Base class for all HUD elements
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDElement : public dtCore::Base
      {
         public:
            static const dtUtil::RefString DEFAULT_BLANK_TYPE;
            static const dtUtil::RefString DEFAULT_IMAGE_TYPE;
            static const dtUtil::RefString DEFAULT_TEXT_TYPE;
            static const dtUtil::RefString PROPERTY_IMAGE;
            static const dtUtil::RefString PROPERTY_FRAME_ENABLED;
            static const dtUtil::RefString PROPERTY_BACKGROUND_ENABLED;

            // @param type The defined CEGUI window type
            // @name The name of this element
            HUDElement( const std::string& name, const std::string& type );

            HUDElement( CEGUI::Window& window );

            CEGUI::Window* GetCEGUIWindow() { return mWindow; }
            const CEGUI::Window* GetCEGUIWindow() const { return mWindow; }

            void SetPositionByVec( const osg::Vec2& position );
            void SetPosition( float x, float y );
            void SetPosition( float x, float y, bool absoluteCoords );
            void SetPosition( float x, float y, const HUDAlignment& align );
            void SetPosition( float x, float y, const HUDAlignment& align, bool absoluteCoords );
            void GetPosition( osg::Vec2& outPos ) const;

            void SetSizeByVec( const osg::Vec2& dimensions );
            virtual void SetSize( float width, float height, bool absoluteCoords = false );
            void GetSize( osg::Vec2& outSize ) const;

            void SetBoundsByVec( const osg::Vec4& bounds );
            void SetBounds( float x, float y, float width, float height, 
               const HUDAlignment& align, bool absolutePos = false, bool absoluteSize = false );
            void SetBounds( float x, float y, float width, float height, 
               bool absolutePos = false, bool absoluteSize = false );
            void GetBounds( osg::Vec4& outBounds ) const;

            void SetAlignment( const HUDAlignment& align );
            const HUDAlignment& GetAlignment() const;
            
            void SetProperty( const std::string& propName, const std::string& value );
            void SetProperty( const std::string& propName, const char* value );
            void SetProperty( const std::string& propName, bool value);
            std::string GetProperty( const std::string& propName ) const;

            virtual void SetVisible( bool visible );
            bool IsVisible() const;

            void SetAlpha(float alpha);
            float GetAlpha() const;

            void SetDeleteWindowOnDestruct(bool enable);
            bool GetDeleteWindowOnDestruct() const;

            virtual void Hide();
            virtual void Show();

            bool IsAbsolutePosition() const { return mAbsPos; }
            bool IsAbsoluteSize() const { return mAbsSize; }

            // Override this if the element is complex and
            // needs to update child elements
            virtual void UpdateLayout() {}

            // Utility function
            void SetCEGUIImage( CEGUI::Window& window, 
               const std::string& imagesetName,
               const std::string& imageName,
               const std::string& imagePropertyName = HUDElement::PROPERTY_IMAGE );

         protected:
            virtual ~HUDElement();

            // The window object that is used to draw the element
            const HUDAlignment* mAlign;
            CEGUI::Window* mWindow;
            bool mAbsPos;
            bool mAbsSize;

         private:
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Text
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDText : public HUDElement
      {
         public:
            HUDText( const std::string& name, const std::string& type = DEFAULT_TEXT_TYPE );
            
            void SetText( const std::string& text, float x, float y );
            void SetText( const std::string& text );
            const std::string& GetText() const;

            void SetFont( const std::string& font );
            void SetFontAndText( const std::string& font, const std::string& text );
            void SetFontAndText( const std::string& font, const std::string& text, float x, float y );

            void SetColor( float r, float g, float b, float a = 1.0f );

         protected:
            virtual ~HUDText();
         private:
            std::string mText;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Text
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDImage : public HUDElement
      {
      public:
         HUDImage( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

         void SetImage( const std::string& imageSet, const std::string& imageName );

      protected:
         virtual ~HUDImage();
      private:
      };

      //////////////////////////////////////////////////////////////////////////
      // HUDGroup:
      // Used for grouping elements together for easier
      // element toggling and/or access.
      // Example: show/hide elements based on HUD states.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDGroup : public HUDElement
      {
         public:
            HUDGroup( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            HUDGroup( CEGUI::Window& window );

            bool Add( HUDElement* child );

            bool Remove( HUDElement* child );

            bool Has( const HUDElement& child ) const;

            bool Has( const std::string& childName ) const;

            unsigned int GetTotalElements() const;

            CEGUI::Window* GetCEGUIChild( const std::string& childName, bool deepSearch = false );
            const CEGUI::Window* GetCEGUIChild( const std::string& childName, bool deepSearch = false ) const;

         protected:
            virtual ~HUDGroup();

         private:

            std::map<dtCore::UniqueId, dtCore::RefPtr<HUDElement> > mChildRefs;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Button
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDButton : public HUDElement
      {
         public:
            HUDButton( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            void SetActiveElement( HUDElement* activeElement );
            HUDElement* GetActiveElement();
            const HUDElement* GetActiveElement() const;

            void SetInactiveElement( HUDElement* activeElement );
            HUDElement* GetInactiveElement();
            const HUDElement* GetInactiveElement() const;

            void SetDisabledElement( HUDElement* disabledElement );
            HUDElement* GetDisabledElement();
            const HUDElement* GetDisabledElement() const;

            void SetActive( bool active );
            bool IsActive() const;

            void SetDisabled( bool active );
            bool IsDisabled() const;

         protected:
            virtual ~HUDButton();

         private:
            bool mActive;
            bool mDisabled;
            dtCore::RefPtr<HUDElement> mActiveElement;
            dtCore::RefPtr<HUDElement> mInactiveElement;
            dtCore::RefPtr<HUDElement> mDisabledElement;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Toolbar
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDToolbar : public HUDElement
      {
      public:
         HUDToolbar( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );
         HUDToolbar( CEGUI::Window& window );

         void SetHorizontal( bool horizontal ) { mHorizontal = horizontal; }
         bool IsHorizontal() const { return mHorizontal; }

         void SetVertical( bool vertical ) { mHorizontal = !vertical; }
         bool IsVertical() const { return !mHorizontal; }

         void SetStartSpace( float space ) { mStartSpace = space; }
         float GetStartSpace() const { return mStartSpace; }

         void SetMidSpace( float space ) { mMidSpace = space; }
         float GetMidSpace() const { return mMidSpace; }

         void SetEndSpace( float space ) { mEndSpace = space; }
         float GetEndSpace() const { return mEndSpace; }

         void SetStartElement( HUDElement* element );
         HUDElement* GetStartElement();
         const HUDElement* GetStartElement() const;

         void SetEndElement( HUDElement* element );
         HUDElement* GetEndElement();
         const HUDElement* GetEndElement() const;

         unsigned int GetTotalElements() const;

         HUDElement* GetElement( unsigned int index );
         const HUDElement* GetElement( unsigned int index ) const;

         // @param element The element whose index is being queried.
         // @return The index of the element in this toolbar's element list; 
         // -1 is returned if element is not found
         int GetElementIndex( const HUDElement& element ) const;

         // Add a HUD element to the toolbar.
         // @param element The element (usually a button) to be added to this toolbar
         // @param index The index where the element is inserted. -1 means end of list
         // @return TRUE if element was successfully inserted
         //
         // NOTE: Relative sized elements will have to have their
         // size ratios set relative to this toolbar so that they
         // will render at the proper size; the toolbar should be
         // set to the size of the largest element.
         virtual bool InsertElement( HUDElement* element, int index = -1 );

         virtual bool RemoveElement( const HUDElement* element );

         virtual bool HasElement( const HUDElement& element ) const;

         virtual bool ClearElements();

         // Updates the toolbar dimensions and its button positions.
         // This should be called after adding/removing an element
         // that affects the overall toolbar dimensions
         virtual void UpdateLayout();

      protected:
         virtual ~HUDToolbar();

         // Utility function for advancing an iterator to a specific index
         // @return TRUE if the outIter is not at the end of mElements.
         bool GetItorAtIndex( unsigned int index, 
            std::vector< dtCore::RefPtr<HUDElement> >::iterator& outIter );
         bool GetItorAtIndex( unsigned int index, 
            std::vector< dtCore::RefPtr<HUDElement> >::const_iterator& outIter ) const;

         // Utility function for advancing an iterator to a specific index
         // @param outIndex Index at which the element was found; -1 if not found
         // @return TRUE if the outIter is not at the end of mElements.
         bool GetItorAtElement( const HUDElement& element, 
            std::vector< dtCore::RefPtr<HUDElement> >::iterator& outIter,
            int* outIndex = NULL );
         bool GetItorAtElement( const HUDElement& element, 
            std::vector< dtCore::RefPtr<HUDElement> >::const_iterator& outIter,
            int* outIndex = NULL ) const;

      private:
         // Primitive Types
         bool mHorizontal;
         float mStartSpace;
         float mMidSpace;
         float mEndSpace;

         // Complex Types
         std::vector< dtCore::RefPtr<HUDElement> > mElements;
         dtCore::RefPtr<HUDElement> mStartElement;
         dtCore::RefPtr<HUDElement> mEndElement;

      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Meter:
      // Base meter element used to indicate a specific
      // value within a range of numeric values.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDMeter : public HUDElement
      {
         public:
            HUDMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            void SetHorizontal( bool horizontal ) { mHorizontal = horizontal; }
            bool IsHorizontal() const { return mHorizontal; }

            void SetVertical( bool vertical ) { mHorizontal = !vertical; }
            bool IsVertical() const { return !mHorizontal; }

            // Override this function if the value is to affect other
            // visual traits of sub-classed meters. Overrides must call
            // this version so that the private value can be changed.
            virtual void SetValue( float current, float maxValue = 1.0f, float minValue = 0.0f );
            float GetValue() { return mValue; }

            virtual void SetScale( float scale );
            float GetScale() const { return mScale; }

            virtual void SetImage( HUDImage* image );
            HUDImage* GetImage();
            const HUDImage* GetImage() const;

            void SetUnitCount( float numUnits ) { mUnits = numUnits; }
            float GetUnitCount() const { return mUnits; }

         protected:
            virtual ~HUDMeter();

         private:
            float mValue;
            float mScale;
            float mUnits;
            bool mHorizontal;
            dtCore::RefPtr<HUDImage> mImage;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Bar Meter:
      // Meter element that has its values displayed
      // in a straight bar form and clips its contained image.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDBarMeter : public HUDMeter
      {
         public:
            HUDBarMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            virtual void SetImage( HUDImage* image );

            virtual void SetSize( float width, float height, bool absoluteSize = false );

            virtual void SetScale( float scale );

            void GetOriginalSize( osg::Vec2& outSize ) const;
            void GetOriginalImageSize( osg::Vec2& outSize ) const;

            const osg::Vec2& GetOriginalSize() const { return mOriginalSize; }
            const osg::Vec2& GetOriginalImageSize() const { return mOriginalImageSize; }

         protected:
            virtual ~HUDBarMeter();

         private:
            osg::Vec2 mOriginalSize;
            osg::Vec2 mOriginalImageSize;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Slide Bar Meter:
      // Meter element that has its values displayed
      // in a straight bar form and slides its contained image.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDSlideBarMeter : public HUDMeter
      {
      public:
         HUDSlideBarMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

         // This determines if the slide motion goes
         // opposite to the normally expected motion
         // Horizontal slide direction is usually Left
         // Vertical slide direction is usually Down
         void SetSlideReversed( bool reverse ) { mSlideReversed = true; }
         bool IsSlideReversed() const { return mSlideReversed; }

         // This determines where the 0-point is on the meter element.
         // Example: if the 0-point is to be in the middle, this offset should be 0.5.
         // Example: if the 0-point is to be at the end, this offset should be 1.0.
         virtual void SetImageOffset( float offset );
         float GetImageOffset() const { return mImageOffset; }

         // This sets the total percentage of the range represented
         // by the contained image. Any percentage over a hundred percent (1.0)
         // declares that the image has repetition in some of its range.
         // Repetition on the ends of range image help to hide the absolute
         // range ends and make the range look continuous, like rotating a cylinder.
         void SetImageRangeScale( float imageRangeScale ) { mImageRangeScale = imageRangeScale; }
         float GetImageRangeScale() const { return mImageRangeScale; }

         // This override of SetScale will "slide" the contained 
         // image, relative to its width/height (which is 1.0)
         virtual void SetScale( float scale );

      protected:
         virtual ~HUDSlideBarMeter();

      private:
         float mImageOffset;
         float mImageRangeScale;
         bool mSlideReversed;
      };

      //////////////////////////////////////////////////////////////////////////
      // HUD Quad Element:
      // HUD element that extends the DeltaDrawable and allows
      // for effects such as rotation, scale, and hue/alpha.
      // This uses osg for all drawing.
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDQuadElement : public dtCore::DeltaDrawable
      {
      public:
         static const unsigned int BIN_NUMBER = 21;
         static const std::string BIN_NAME;
         static const float WIN_HEIGHT_RATIO;

         HUDQuadElement(  const std::string& name, const std::string& imageFileName = "" );

         virtual osg::Node* GetOSGNode();
         virtual const osg::Node* GetOSGNode() const;

         void SetSize( float width, float height );
         void GetSize( osg::Vec2& outSize ) const;

         void SetPosition( float x, float y, float z = 0.0f );
         void GetPosition( osg::Vec3& outPos ) const;
         void GetPosition( osg::Vec2& outPos ) const;

         void SetOffset( float x, float y );
         void GetOffset( osg::Vec2& outOffset ) const;

         void SetRotation( float degrees );
         float GetRotation() const { return mRotation; }

         void SetColor( float r, float g, float b );
         void SetColor( float r, float g, float b, float a );
         void GetColor( osg::Vec4& outColor ) const;

         void SetAlpha( float alpha );
         float GetAlpha() const;

         virtual void SetVisible( bool visible );
         bool IsVisible() const { return mVisible; }
         virtual void Show();
         virtual void Hide();

         osg::Geode* GetGeode();
         const osg::Geode* GetGeode() const;

         bool Has( HUDQuadElement& element ) const;
         bool Add( HUDQuadElement& element, int index = -1 );
         bool Remove( HUDQuadElement& element );

         unsigned GetTotalChildren() const;

      protected:
         virtual ~HUDQuadElement();

      private:
         bool mVisible;
         float mAlpha;
         float mWidth;
         float mHeight;
         float mRotation;
         osg::ref_ptr<osg::Vec3Array> mVerts;
         osg::ref_ptr<osg::Vec2Array> mUvs;
         osg::ref_ptr<osg::Vec4Array> mColor;
         osg::ref_ptr<osg::MatrixTransform> mTrans;
         osg::ref_ptr<osg::Group> mRoot;
         osg::ref_ptr<osg::Geode> mGeode;

         osg::Vec3 mPos;
      };

   }
}

#endif
