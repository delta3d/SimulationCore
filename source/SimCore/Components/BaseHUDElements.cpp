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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/BaseHUDElements.h>

#include <dtCore/project.h>
#include <dtUtil/exception.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/nodemask.h>
#include <osgDB/FileUtils>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

#include <cmath>



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Base HUD Element Exceptions Code
      //////////////////////////////////////////////////////////////////////////
      BaseHUDElementException::BaseHUDElementException(const std::string& message, const std::string& filename, unsigned int linenum)
      : dtUtil::Exception(message, filename, linenum)
      {}

      BaseHUDElementException::~BaseHUDElementException() {}

      //////////////////////////////////////////////////////////////////////////
      // HUD Alignment Enumeration Code
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(HUDAlignment);
      HUDAlignment HUDAlignment::LEFT_TOP("ALIGN LEFT TOP",
         CEGUI::HA_LEFT, CEGUI::VA_TOP);
      HUDAlignment HUDAlignment::LEFT_CENTER("ALIGN LEFT CENTER",
         CEGUI::HA_LEFT, CEGUI::VA_CENTRE);
      HUDAlignment HUDAlignment::LEFT_BOTTOM("ALIGN LEFT BOTTOM",
         CEGUI::HA_LEFT, CEGUI::VA_BOTTOM);
      HUDAlignment HUDAlignment::CENTER_TOP("ALIGN CENTER TOP",
         CEGUI::HA_CENTRE, CEGUI::VA_TOP);
      HUDAlignment HUDAlignment::CENTER("ALIGN CENTER",
         CEGUI::HA_CENTRE, CEGUI::VA_CENTRE);
      HUDAlignment HUDAlignment::CENTER_BOTTOM("ALIGN CENTER BOTTOM",
         CEGUI::HA_CENTRE, CEGUI::VA_BOTTOM);
      HUDAlignment HUDAlignment::RIGHT_TOP("ALIGN RIGHT TOP",
         CEGUI::HA_RIGHT, CEGUI::VA_TOP);
      HUDAlignment HUDAlignment::RIGHT_CENTER("ALIGN RIGHT CENTER",
         CEGUI::HA_RIGHT, CEGUI::VA_CENTRE);
      HUDAlignment HUDAlignment::RIGHT_BOTTOM("ALIGN RIGHT BOTTOM",
         CEGUI::HA_RIGHT, CEGUI::VA_BOTTOM);//*/

      //////////////////////////////////////////////////////////////////////////
      HUDAlignment& HUDAlignment::ClassifyAlignment(CEGUI::Window& window)
      {
         CEGUI::VerticalAlignment alignV = window.getVerticalAlignment();
         switch(window.getHorizontalAlignment())
         {
            case CEGUI::HA_RIGHT:
            {
               switch(alignV)
               {
                  case CEGUI::VA_BOTTOM: return HUDAlignment::RIGHT_BOTTOM;
                  case CEGUI::VA_CENTRE: return HUDAlignment::RIGHT_CENTER;
                  default:               return HUDAlignment::RIGHT_TOP;
               }
            }

            case CEGUI::HA_CENTRE:
            {
               switch(alignV)
               {
                  case CEGUI::VA_BOTTOM: return HUDAlignment::CENTER_BOTTOM;
                  case CEGUI::VA_CENTRE: return HUDAlignment::CENTER;
                  default:               return HUDAlignment::CENTER_TOP;
               }
            }

            default: // LEFT
            {
               switch(alignV)
               {
                  case CEGUI::VA_BOTTOM: return HUDAlignment::LEFT_BOTTOM;
                  case CEGUI::VA_CENTRE: return HUDAlignment::LEFT_CENTER;
                  default: break; // LEFT TOP. Break to allow a function-scope return statement.
               }
            }

            // Default return. Avoids compile warnings.
            return HUDAlignment::LEFT_TOP;
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Element Code
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString HUDElement::DEFAULT_BLANK_TYPE("DefaultWindow");
      const dtUtil::RefString HUDElement::DEFAULT_IMAGE_TYPE("WindowsLook/StaticImage");
      const dtUtil::RefString HUDElement::DEFAULT_TEXT_TYPE("WindowsLook/StaticText");
      const dtUtil::RefString HUDElement::PROPERTY_IMAGE("Image");
      const dtUtil::RefString HUDElement::PROPERTY_FRAME_ENABLED("FrameEnabled");
      const dtUtil::RefString HUDElement::PROPERTY_BACKGROUND_ENABLED("BackgroundEnabled");

      //////////////////////////////////////////////////////////////////////////
      HUDElement::HUDElement(const std::string& name, const std::string& type)
         : dtCore::Base(name)
         , mAlign(&HUDAlignment::LEFT_TOP)
         , mAbsPos(false)
         , mAbsSize(false)
      {
         CEGUI::WindowManager* wm = CEGUI::WindowManager::getSingletonPtr();
         try
         {
            mWindow = wm->createWindow(type, name);
         }
         catch (CEGUI::Exception& e)
         {
            std::ostringstream oss;
            oss << "CEGUI while setting up BaseHUD: " << e.getMessage().c_str();
            throw BaseHUDElementException(oss.str(), __FILE__, __LINE__);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement::HUDElement(CEGUI::Window& window)
         : dtCore::Base(window.getName().c_str())
         , mAlign(&HUDAlignment::ClassifyAlignment(window))
         , mAbsPos(false)
         , mAbsSize(false)
      {
         mWindow = &window;
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement::~HUDElement()
      {
         if (mWindow != NULL && (!mWindow->isDestroyedByParent() || mWindow->getParent() == NULL))
         {
            mWindow->destroy();
         }
         mWindow = NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetPositionByVec( const osg::Vec2& position )
      {
         SetPosition( position.x(), position.y() );
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetPosition(float x, float y)
      {
         if (mAbsPos)
         {
            mWindow->setXPosition(cegui_absdim(x));
            mWindow->setYPosition(cegui_absdim(y));
         }
         else
         {
            mWindow->setXPosition(cegui_reldim(x));
            mWindow->setYPosition(cegui_reldim(y));
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetPosition(float x, float y, bool absoluteCoords)
      {
         mAbsPos = absoluteCoords;
         SetPosition(x, y);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetPosition(float x, float y, const HUDAlignment& align)
      {
         SetPosition(x, y);
         SetAlignment(align);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetPosition(float x, float y, const HUDAlignment& align, bool absoluteCoords)
      {
         SetPosition(x, y, absoluteCoords);
         SetAlignment(align);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::GetPosition(osg::Vec2& outPos) const
      {
         const CEGUI::UVector2& ceguiPos = mWindow->getPosition();
         if (mAbsPos)
         {
            outPos.set(ceguiPos.d_x.d_offset, ceguiPos.d_y.d_offset);
         }
         else
         {
            outPos.set(ceguiPos.d_x.d_scale, ceguiPos.d_y.d_scale);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetSizeByVec( const osg::Vec2& dimensions )
      {
         SetSize( dimensions.x(), dimensions.y(), mAbsSize );
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetSize(float width, float height, bool absoluteCoords)
      {
         mAbsSize = absoluteCoords;
         if (mAbsSize)
         {
            mWindow->setWidth(cegui_absdim(width));
            mWindow->setHeight(cegui_absdim(height));
         }
         else
         {
            mWindow->setWidth(cegui_reldim(width));
            mWindow->setHeight(cegui_reldim(height));
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::GetSize(osg::Vec2& outSize) const
      {
         const CEGUI::UVector2& ceguiSize = mWindow->getSize();
         if (mAbsSize)
         {
            outSize.set(ceguiSize.d_x.d_offset, ceguiSize.d_y.d_offset);
         }
         else
         {
            outSize.set(ceguiSize.d_x.d_scale, ceguiSize.d_y.d_scale);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetBoundsByVec( const osg::Vec4& bounds )
      {
         SetPosition( bounds.x(), bounds.y(), mAbsPos );
         SetSize( bounds.z(), bounds.w(), mAbsSize );
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetBounds(float x, float y, float width, float height, const HUDAlignment& align, bool absolutePos, bool absoluteSize)
      {
         SetPosition(x, y, absolutePos);
         SetSize(width, height, absoluteSize);
         SetAlignment(align);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetBounds(float x, float y, float width, float height, bool absolutePos, bool absoluteSize)
      {
         SetPosition(x, y, absolutePos);
         SetSize(width, height, absoluteSize);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::GetBounds(osg::Vec4& outBounds) const
      {
         outBounds.set(
            mAbsPos ? mWindow->getXPosition().d_offset : mWindow->getXPosition().d_scale,
            mAbsPos ? mWindow->getYPosition().d_offset : mWindow->getYPosition().d_scale,
            mAbsSize ? mWindow->getWidth().d_offset : mWindow->getWidth().d_scale,
            mAbsSize ? mWindow->getHeight().d_offset : mWindow->getHeight().d_scale);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetAlignment(const HUDAlignment& align)
      {
         mAlign = &align;
         mWindow->setHorizontalAlignment(align.GetAlignH());
         mWindow->setVerticalAlignment(align.GetAlignV());
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDAlignment& HUDElement::GetAlignment() const
      {
         return *mAlign;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetProperty(const std::string& propName, const std::string& value)
      {
         try
         {
            if(mWindow->isPropertyPresent(propName))
            {
               mWindow->setProperty(propName, value);
            }
         }
         catch(CEGUI::Exception& e)
         {
            std::ostringstream oss;
            oss << "FAILURE: "<<GetName().c_str()<<".SetProperty("
               << propName.c_str() << "\", \""<< value.c_str() << "\")" << std::endl
                << e.getMessage().c_str();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetProperty( const std::string& propName, const char* value )
      {
         // This method overload prevents string literals from being treated
         // as booleans.
         std::string str(value);
         SetProperty(propName, str);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetProperty(const std::string& propName, bool value)
      {
         static const dtUtil::RefString STR_TRUE("true");
         static const dtUtil::RefString STR_FALSE("false");
         SetProperty(propName, value?STR_TRUE.c_str():STR_FALSE.c_str());
      }

      //////////////////////////////////////////////////////////////////////////
      std::string HUDElement::GetProperty(const std::string& propName) const
      {
         try
         {
            return std::string(mWindow->getProperty(CEGUI::String(propName)).c_str());
         }
         catch(CEGUI::Exception& ex)
         {
            std::ostringstream oss;
            oss << "FAILURE: " << GetName() << ".GetProperty("
               << propName << "\")" << std::endl
               << ex.getMessage().c_str();
            LOG_ERROR(oss.str())
            return "";
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetVisible(bool visible)
      {
         if (! visible)
         {
            mWindow->hide();
         }
         else
         {
            mWindow->show();
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDElement::IsVisible() const
      {
         return mWindow->isVisible();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetAlpha(float alpha)
      {
         mWindow->setAlpha(alpha);
      }

      //////////////////////////////////////////////////////////////////////////
      float HUDElement::GetAlpha() const
      {
         return mWindow->getAlpha();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetDeleteWindowOnDestruct(bool enable)
      {
         if (mWindow != NULL)
         {
            mWindow->setDestroyedByParent(!enable);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDElement::GetDeleteWindowOnDestruct() const
      {
         if (mWindow != NULL)
         {
            return !mWindow->isDestroyedByParent();
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::Hide()
      {
         SetVisible(false);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::Show()
      {
         SetVisible(true);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDElement::SetCEGUIImage(CEGUI::Window& window,
         const std::string& imagesetName, const std::string& imageName,
         const std::string& imagePropertyName)
      {
         std::stringstream ss;
         ss << "set:" << imagesetName << " image:" << imageName;
         window.setProperty(imagePropertyName, ss.str());
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Text Code
      //////////////////////////////////////////////////////////////////////////
      HUDText::HUDText(const std::string& name, const std::string& type)
         : HUDElement(name, type)
      {
         SetAlignment(HUDAlignment::LEFT_TOP);
         SetProperty(PROPERTY_FRAME_ENABLED, false);
         SetProperty(PROPERTY_BACKGROUND_ENABLED, false);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText::~HUDText()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void HUDText::SetText(const std::string& text, float x, float y)
      {
         SetText(text);
         SetPosition(x, y);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDText::SetText(const std::string& text)
      {
         mText = text;
         mWindow->setText(text);
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& HUDText::GetText() const
      {
         return mText;//std::string(mWindow->getText().c_str());
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDText::SetFont(const std::string& font)
      {
         mWindow->setFont(font);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDText::SetFontAndText(const std::string& font, const std::string& text)
      {
         SetFont(font);
         SetText(text);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDText::SetFontAndText(const std::string& font, const std::string& text, float x, float y)
      {
         SetFont(font);
         SetText(text);
         SetPosition(x, y);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDText::SetColor(float r, float g, float b, float a)
      {
         if (r >= 0.0f && g >= 0.0f && b >= 0.0f)
            mWindow->setProperty("TextColours", CEGUI::PropertyHelper::colourToString(CEGUI::colour(r, g, b, a)));
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Image Code
      //////////////////////////////////////////////////////////////////////////
      HUDImage::HUDImage(const std::string& name, const std::string& type)
         : HUDElement(name, type)
      {
         SetProperty(PROPERTY_FRAME_ENABLED, false);
         SetProperty(PROPERTY_BACKGROUND_ENABLED, false);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDImage::~HUDImage()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void HUDImage::SetImage(const std::string& imageSet, const std::string& imageName)
      {
         SetProperty("Image", "set:"+imageSet+" image:"+imageName);
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Group Code
      //////////////////////////////////////////////////////////////////////////
      HUDGroup::HUDGroup(const std::string& name, const std::string& type)
         : HUDElement(name, type)
      {
         SetProperty(PROPERTY_FRAME_ENABLED, false);
         SetProperty(PROPERTY_BACKGROUND_ENABLED, false);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDGroup::HUDGroup(CEGUI::Window& window)
         : HUDElement(window)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      HUDGroup::~HUDGroup()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDGroup::Add(HUDElement* child)
      {
         if (child == NULL) { return false; }

         // On successful insert...
         if (mChildRefs.insert(std::make_pair(child->GetUniqueId(), child)).second)
         {
            // ...add the child CEGUI window pointer to this object's window
            mWindow->addChildWindow(child->GetCEGUIWindow());
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDGroup::Remove(HUDElement* child)
      {
         if (child == NULL) { return false; }

         std::map<dtCore::UniqueId, dtCore::RefPtr<HUDElement> >::iterator i =
            mChildRefs.find(child->GetUniqueId());

         if (i != mChildRefs.end())
         {
            mWindow->removeChildWindow(child->GetCEGUIWindow());
            mChildRefs.erase(i);
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDGroup::Has(const HUDElement& child) const
      {
         return mWindow->isChild(child.GetCEGUIWindow());
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDGroup::Has(const std::string& childName) const
      {
         return mWindow->isChild(childName);
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int HUDGroup::GetTotalElements() const
      {
         return (unsigned int) mChildRefs.size();
      }

      //////////////////////////////////////////////////////////////////////////
      CEGUI::Window* HUDGroup::GetCEGUIChild(const std::string& childName, bool deepSearch)
      {
         if (mWindow->isChild(childName))
         {
            return mWindow->getChild(childName);
         }

         if (deepSearch)
         {
            // TODO:
         }
         return NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const CEGUI::Window* HUDGroup::GetCEGUIChild(const std::string& childName, bool deepSearch) const
      {
         if (mWindow->isChild(childName))
         {
            return mWindow->getChild(childName);
         }

         if (deepSearch)
         {
            // TODO:
         }
         return NULL;
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Button Code
      //////////////////////////////////////////////////////////////////////////
      HUDButton::HUDButton(const std::string& name, const std::string& type)
         : HUDElement(name, type),
         mActive(true),
         mDisabled(false)
      {
         SetProperty(PROPERTY_FRAME_ENABLED, false);
         SetProperty(PROPERTY_BACKGROUND_ENABLED, false);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDButton::~HUDButton()
      {
         if (mActiveElement.valid())
         {
            mWindow->removeChildWindow(mActiveElement->GetCEGUIWindow());
            mActiveElement = NULL;
         }
         if (mInactiveElement.valid())
         {
            mWindow->removeChildWindow(mInactiveElement->GetCEGUIWindow());
            mInactiveElement = NULL;
         }
         if (mDisabledElement.valid())
         {
            mWindow->removeChildWindow(mDisabledElement->GetCEGUIWindow());
            mDisabledElement = NULL;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDButton::SetActiveElement(HUDElement* activeElement)
      {
         if (mActiveElement.valid())
         {
            mWindow->removeChildWindow(mActiveElement->GetCEGUIWindow());
         }
         mActiveElement = activeElement;
         if (activeElement != NULL)
         {
            mWindow->addChildWindow(activeElement->GetCEGUIWindow());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement* HUDButton::GetActiveElement()
      {
         return mActiveElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDElement* HUDButton::GetActiveElement() const
      {
         return mActiveElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDButton::SetInactiveElement(HUDElement* inactiveElement)
      {
         if (mInactiveElement.valid())
         {
            mWindow->removeChildWindow(mInactiveElement->GetCEGUIWindow());
         }
         mInactiveElement = inactiveElement;
         if (inactiveElement != NULL)
         {
            mWindow->addChildWindow(inactiveElement->GetCEGUIWindow());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement* HUDButton::GetInactiveElement()
      {
         return mInactiveElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDElement* HUDButton::GetInactiveElement() const
      {
         return mInactiveElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDButton::SetDisabledElement(HUDElement* disabledElement)
      {
         if (mDisabledElement.valid())
         {
            mWindow->removeChildWindow(mDisabledElement->GetCEGUIWindow());
         }
         mDisabledElement = disabledElement;
         if (mDisabledElement.valid())
         {
            mWindow->addChildWindow(mDisabledElement->GetCEGUIWindow());
            mDisabledElement->SetVisible(mDisabled);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement* HUDButton::GetDisabledElement()
      {
         return mDisabledElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDElement* HUDButton::GetDisabledElement() const
      {
         return mDisabledElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDButton::SetActive(bool active)
      {
         if (! mDisabled)
         {
            mActive = active;
            if (mActiveElement.valid())
            {
               mActiveElement->SetVisible(active);
            }
            if (mInactiveElement.valid())
            {
               mInactiveElement->SetVisible(!active);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDButton::IsActive() const
      {
         return mActive;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDButton::SetDisabled(bool disabled)
      {
         if (mDisabled != disabled)
         {
            mDisabled = disabled;
            if (mDisabledElement.valid())
            {
               mDisabledElement->SetVisible(mDisabled);
            }

            if (mDisabled)
            {
               if (mActiveElement.valid())
               {
                  mActiveElement->SetVisible(false);
               }
               if (mInactiveElement.valid())
               {
                  mInactiveElement->SetVisible(false);
               }
            }
            else
            {
               // Default to inactive but enabled state
               SetActive(false);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDButton::IsDisabled() const
      {
         return mDisabled;
      }


      //////////////////////////////////////////////////////////////////////////
      // HUD Meter Code
      //////////////////////////////////////////////////////////////////////////
      HUDMeter::HUDMeter(const std::string& name, const std::string& type)
         : HUDElement(name, type),
         mValue(0.0f),
         mScale(0.0f),
         mUnits(0.0f),
         mHorizontal(true)
      {
         SetProperty(PROPERTY_FRAME_ENABLED, false);
         SetProperty(PROPERTY_BACKGROUND_ENABLED, false);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDMeter::~HUDMeter()
      {
         if (mImage.valid())
         {
            mWindow->removeChildWindow(mImage->GetCEGUIWindow());
            mImage = NULL;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDMeter::SetScale(float scale)
      {
         mScale = scale;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDMeter::SetValue(float current, float maxValue, float minValue)
      {
         mValue = current > maxValue ? maxValue : current < minValue ? minValue : current;
         maxValue = std::abs(maxValue-minValue);
         if (mUnits > 0.0f)
         {
            float unitSize = maxValue/mUnits;
            mValue = unitSize != 0.0f
               ? (int)(mValue/unitSize + (mValue/maxValue > 0.0f && mValue < maxValue ? 1.0 : 0.0f))*unitSize
               : mValue;
         }
         current = mValue-minValue;
         SetScale(maxValue != 0.0f ? current/maxValue : 0.0f);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDMeter::SetImage(HUDImage* image)
      {
         if (mImage.valid())
         {
            mWindow->removeChildWindow(mImage->GetCEGUIWindow());
         }
         mImage = image;
         if (mImage.valid())
         {
            mWindow->addChildWindow(mImage->GetCEGUIWindow());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDImage* HUDMeter::GetImage()
      {
         return mImage.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDImage* HUDMeter::GetImage() const
      {
         return mImage.get();
      }


      //////////////////////////////////////////////////////////////////////////
      // HUD Bar Meter Code
      //////////////////////////////////////////////////////////////////////////
      HUDBarMeter::HUDBarMeter(const std::string& name, const std::string& type)
         : HUDMeter(name, type)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      HUDBarMeter::~HUDBarMeter()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDBarMeter::SetImage(HUDImage* image)
      {
         HUDMeter::SetImage(image);

         if (GetImage() != NULL)
         {
            GetImage()->GetSize(mOriginalImageSize);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDBarMeter::SetSize(float width, float height, bool absoluteSize)
      {
         HUDMeter::SetSize(width, height, absoluteSize);
         GetSize(mOriginalSize);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDBarMeter::SetScale(float scale)
      {
         if (scale == HUDMeter::GetScale()) { return; }

         HUDMeter::SetScale(scale);

         // Scale this window up or down so that it will
         // clip the image it contains when rendering.
         osg::Vec2 size;
         GetOriginalSize(size);
         HUDImage* image = GetImage();

         if (IsHorizontal())
         {
            GetOriginalSize(size);

            // Scale up/down this frame
            // NOTE: Use parent version of SetSize since the sub-class version will
            // modify mOriginalSize
            HUDMeter::SetSize(size[0]*scale, size[1], IsAbsoluteSize());

            // Scale up the contained image so that it will be clipped
            // and look like it was not scaled (only if this container
            // is in relative size mode).
            if (!IsAbsoluteSize() && image != NULL
               && scale != 0.0f && mOriginalSize[0] != 0.0f)
            {
               GetOriginalImageSize(size);
               image->SetSize(size[0]/scale, size[1], image->IsAbsoluteSize());
            }
         }
         else
         {
            GetOriginalSize(size);

            // Scale up/down this frame
            // NOTE: Use parent version of SetSize since the sub-class version will
            // modify mOriginalSize
            HUDMeter::SetSize(size[0], size[1]*scale, IsAbsoluteSize());

            // Scale up the contained image so that it will be clipped
            // and look like it was not scaled (only if this container
            // is in relative size mode).
            if (!IsAbsoluteSize() && image != NULL
               && scale != 0.0f && mOriginalSize[1] != 0.0f)
            {
               GetOriginalImageSize(size);
               image->SetSize(size[0], size[1]/scale, image->IsAbsoluteSize());
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDBarMeter::GetOriginalSize(osg::Vec2& outSize) const
      {
         outSize = mOriginalSize;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDBarMeter::GetOriginalImageSize(osg::Vec2& outSize) const
      {
         outSize = mOriginalImageSize;
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Slide Bar Meter Code
      //////////////////////////////////////////////////////////////////////////
      HUDSlideBarMeter::HUDSlideBarMeter(const std::string& name, const std::string& type)
         : HUDMeter(name),
         mImageOffset(0.0f),
         mImageRangeScale(1.0f),
         mSlideReversed(false)
      {

      }

      //////////////////////////////////////////////////////////////////////////
      HUDSlideBarMeter::~HUDSlideBarMeter()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void HUDSlideBarMeter::SetImageOffset(float offset)
      {
         mImageOffset = offset;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDSlideBarMeter::SetScale(float scale)
      {
         HUDMeter::SetScale(scale);

         HUDImage* image = GetImage();

         if (image == NULL) { return; }

         osg::Vec2 size;
         image->GetSize(size);

         osg::Vec2 imgPos;
         image->GetPosition(imgPos);

         if (mImageRangeScale != 0.0f)
         {
            scale /= mImageRangeScale;
         }

         if (IsHorizontal())
         {
            if (size[0] != 0.0f)
            {
               image->SetPosition((mSlideReversed?scale:-scale)*size[0]+mImageOffset, imgPos[1]);
            }
         }
         else
         {
            if (size[1] != 0.0f)
            {
               image->SetPosition( imgPos[0], (mSlideReversed?scale:-scale)*size[1]+mImageOffset);
            }
         }
      }


      //////////////////////////////////////////////////////////////////////////
      // HUD Toolbar Code
      //////////////////////////////////////////////////////////////////////////
      HUDToolbar::HUDToolbar(const std::string& name, const std::string& type)
         : HUDElement(name, type),
         mHorizontal(true),
         mStartSpace(0.0f),
         mMidSpace(0.0f),
         mEndSpace(0.0f)
      {
         SetProperty(PROPERTY_FRAME_ENABLED, false);
         SetProperty(PROPERTY_BACKGROUND_ENABLED, false);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDToolbar::HUDToolbar(CEGUI::Window& window)
         : HUDElement(window),
         mHorizontal(true),
         mStartSpace(0.0f),
         mMidSpace(0.0f),
         mEndSpace(0.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      HUDToolbar::~HUDToolbar()
      {
         ClearElements();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDToolbar::SetStartElement(HUDElement* element)
      {
         if (mStartElement.valid())
         {
            mWindow->removeChildWindow(mStartElement->GetCEGUIWindow());
         }
         mStartElement = element;
         if (mStartElement.valid())
         {
            mWindow->addChildWindow(mStartElement->GetCEGUIWindow());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement* HUDToolbar::GetStartElement()
      {
         return mStartElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDElement* HUDToolbar::GetStartElement() const
      {
         return mStartElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDToolbar::SetEndElement(HUDElement* element)
      {
         if (mEndElement.valid())
         {
            mWindow->removeChildWindow(mEndElement->GetCEGUIWindow());
         }
         mEndElement = element;
         if (mEndElement.valid())
         {
            mWindow->addChildWindow(mEndElement->GetCEGUIWindow());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement* HUDToolbar::GetEndElement()
      {
         return mEndElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDElement* HUDToolbar::GetEndElement() const
      {
         return mEndElement.get();
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::GetItorAtIndex(unsigned int index,
         std::vector< dtCore::RefPtr<HUDElement> >::iterator& outIter)
      {
         // Step the iterator forward to index
         for (unsigned int curIndex = 0;
            curIndex < index && outIter != mElements.end();
            ++outIter, ++curIndex)
         {
            // Do nothing
         }

         return outIter != mElements.end();
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::GetItorAtIndex(unsigned int index,
         std::vector< dtCore::RefPtr<HUDElement> >::const_iterator& outIter) const
      {
         // Step the iterator forward to index
         for (unsigned int curIndex = 0;
            curIndex < index && outIter != mElements.end();
            ++outIter, ++curIndex)
         {
            // Do nothing
         }

         return outIter != mElements.end();
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::GetItorAtElement(const HUDElement& element,
         std::vector< dtCore::RefPtr<HUDElement> >::iterator& outIter,
         int* outIndex)
      {
         if (outIndex != NULL) { *outIndex = 0; }

         for (; outIter != mElements.end(); ++outIter)
         {
            // Is this the element to be removed?
            if (outIter->get() == &element)
            {
               return true;
            }
            if (outIndex != NULL) { (*outIndex)++; }
         }

         if (outIndex != NULL) { *outIndex = -1; }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::GetItorAtElement(const HUDElement& element,
         std::vector< dtCore::RefPtr<HUDElement> >::const_iterator& outIter,
         int* outIndex) const
      {
         if (outIndex != NULL) { *outIndex = 0; }

         for (; outIter != mElements.end(); ++outIter)
         {
            // Is this the element to be removed?
            if (outIter->get() == &element)
            {
               return true;
            }
            if (outIndex != NULL) { (*outIndex)++; }
         }

         if (outIndex != NULL) { *outIndex = -1; }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int HUDToolbar::GetTotalElements() const
      {
         return mElements.size();
      }

      //////////////////////////////////////////////////////////////////////////
      HUDElement* HUDToolbar::GetElement(unsigned int index)
      {
         std::vector< dtCore::RefPtr<HUDElement> >::iterator iter = mElements.begin();
         return GetItorAtIndex(index, iter) ? iter->get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDElement* HUDToolbar::GetElement(unsigned int index) const
      {
         std::vector< dtCore::RefPtr<HUDElement> >::const_iterator iter = mElements.begin();
         return GetItorAtIndex(index, iter) ? iter->get() : NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      int HUDToolbar::GetElementIndex(const HUDElement& element) const
      {
         std::vector< dtCore::RefPtr<HUDElement> >::const_iterator iter = mElements.begin();
         int index = -1;
         GetItorAtElement(element, iter, &index);
         return index;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::InsertElement(HUDElement* element, int index)
      {
         if (element == NULL) { return false; }

         if (index < 0 || (unsigned int)index > mElements.size()) { index = mElements.size(); }

         // Step the iterator forward to index
         std::vector< dtCore::RefPtr<HUDElement> >::iterator iter = mElements.begin();
         GetItorAtIndex(index, iter);

         try{
            // Attach and register the HUD element
            mWindow->addChildWindow(element->GetCEGUIWindow());
            mElements.insert(iter, element);
            return true;
         }
         catch(CEGUI::Exception& e)
         {
            std::ostringstream oss;
            oss << "FAILURE: "<<GetName().c_str()<<".InsertElement(HUDElement.\""
               << element->GetName().c_str() << "\", "<< index <<")"
               << std::endl
               << e.getMessage().c_str();
         }

         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::RemoveElement(const HUDElement* element)
      {
         if (element == NULL) { return false; }

         if (mElements.empty()) { return false; }

         std::vector< dtCore::RefPtr<HUDElement> >::iterator iter = mElements.begin();
         if (GetItorAtElement(*element, iter))
         {
            try{
               mWindow->removeChildWindow((*iter)->GetCEGUIWindow());
               mElements.erase(iter);
               return true;
            }
            catch(CEGUI::Exception& e)
            {
               std::ostringstream oss;
               oss << "FAILURE: "<<GetName().c_str()<<".ClearElements() "
                  << "removing element \"" << ((*iter).valid()?(*iter)->GetName():"NULL") << "\""
                  << std::endl
                  << e.getMessage().c_str();
            }
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::ClearElements()
      {
         if (mElements.empty()) { return false; }

         std::vector< dtCore::RefPtr<HUDElement> >::iterator iter = mElements.begin();
         for (; iter != mElements.end(); ++iter)
         {
            try{
               mWindow->removeChildWindow((*iter)->GetCEGUIWindow());
            }
            catch(CEGUI::Exception& e)
            {
               std::ostringstream oss;
               oss << "FAILURE: "<<GetName().c_str()<<".ClearElements() "
                  << "removing element \"" << ((*iter).valid()?(*iter)->GetName():"NULL") << "\""
                  << std::endl
                  << e.getMessage().c_str();
            }
         }

         mElements.clear();
         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDToolbar::HasElement(const HUDElement& element) const
      {
         return GetElementIndex(element) >= 0;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDToolbar::UpdateLayout()
      {
         // Calculate the size
         osg::Vec2 curSize;
         osg::Vec2 curPos;

         // Get size of start element and advance current position
         if (mStartElement.valid())
         {
            mStartElement->GetSize(curSize);
            if (mHorizontal)
            {
               curPos[0] += curSize[0] + mStartSpace;
            }
            else
            {
               curPos[1] += curSize[1] + mStartSpace;
            }
         }

         // Loop through and distribute the elements between
         // the start and end elements.
         std::vector< dtCore::RefPtr<HUDElement> >::iterator iter, iterEnd;
         iter = mElements.begin();
         iterEnd = mElements.end();
         for (; iter != iterEnd; ++iter)
         {
            (*iter)->SetPosition(curPos[0], curPos[1]);
            (*iter)->GetSize(curSize);
            if (mHorizontal)
            {
               curPos[0] += curSize[0] + mMidSpace;
            }
            else
            {
               curPos[1] += curSize[1] + mMidSpace;
            }
         }

         // Advance current position and set the position of the end element
         if (mEndElement.valid())
         {
            if (mHorizontal)
            {
               curPos[0] += mEndSpace;
            }
            else
            {
               curPos[1] += mEndSpace;
            }
            mEndElement->SetPosition(curPos[0], curPos[1]);
            mEndElement->GetSize(curSize);
         }

      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Quad Element
      //////////////////////////////////////////////////////////////////////////
      const std::string HUDQuadElement::BIN_NAME = "RenderBin";
      const float HUDQuadElement::WIN_HEIGHT_RATIO = 1200.0f/1920.0f;

      //////////////////////////////////////////////////////////////////////////
      HUDQuadElement::HUDQuadElement(const std::string& name, const std::string& imageFileName)
         : dtCore::DeltaDrawable(name),
         mVisible(true),
         mAlpha(1.0f),
         mWidth(10.0f),
         mHeight(10.0f),
         mRotation(0.0f)
      {
         mRoot = new osg::Group();
         mGeode = new osg::Geode();

         osg::Projection* projection = new osg::Projection;
         projection->setMatrix(osg::Matrix::ortho2D(0,1.0f,0,WIN_HEIGHT_RATIO));

         mTrans = new osg::MatrixTransform;
         mTrans->setMatrix(osg::Matrix::identity());
         mTrans->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

         // Setup the node tree
         mRoot->addChild(projection);
         projection->addChild(mTrans.get());
         mTrans->addChild(mGeode.get());

         // VERTICES
         mVerts = new osg::Vec3Array;
         mVerts->push_back(osg::Vec3(0,    0,-1));
         mVerts->push_back(osg::Vec3(1.0,  0,-1));
         mVerts->push_back(osg::Vec3(1.0,1.0,-1));
         mVerts->push_back(osg::Vec3(  0,1.0,-1));

         // INDICES
         osg::DrawElementsUInt* indices =
            new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
         indices->push_back(0);
         indices->push_back(1);
         indices->push_back(2);
         indices->push_back(3);

         // COLOR
         mColor = new osg::Vec4Array;
         mColor->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

         // UVS
         mUvs = new osg::Vec2Array(4);
         (*mUvs)[0].set(0.0f,0.0f);
         (*mUvs)[1].set(1.0f,0.0f);
         (*mUvs)[2].set(1.0f,1.0f);
         (*mUvs)[3].set(0.0f,1.0f);

         // NORMALS
         osg::Vec3Array* norms = new osg::Vec3Array;
         norms->push_back(osg::Vec3(0.0f,0.0f,1.0f));

         // STATES
         osg::StateSet* states = new osg::StateSet();
         states->setMode(GL_BLEND,osg::StateAttribute::ON);
         states->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
         states->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
         states->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
         states->setRenderBinDetails(BIN_NUMBER, BIN_NAME);

         // Get the current texture directory
         if (! imageFileName.empty())
         {
            // Set the texture
            dtCore::RefPtr<osg::Texture2D> texture = new osg::Texture2D;
            texture->setDataVariance(osg::Object::DYNAMIC);
            osg::Image* image;
            std::string filePath("Textures:");
            filePath += imageFileName;
            try{
               filePath = dtCore::Project::GetInstance().GetResourcePath(dtCore::ResourceDescriptor(filePath));
               image = osgDB::readImageFile(filePath);
               texture->setImage(image);
               states->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
            }
            catch(dtUtil::Exception& e)
            {
               std::stringstream ss;
               ss << "HUDQuadElement could not load texture \"" << filePath << "\" because:\n"
                  << e.ToString() << std::endl;
               LOG_ERROR(ss.str());
            }
         }

         // Setup the geometry
         osg::Geometry* geometry = new osg::Geometry();
         geometry->setNormalArray(norms);
         geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
         geometry->addPrimitiveSet(indices);
         geometry->setTexCoordArray(0,mUvs.get());
         geometry->setVertexArray(mVerts.get());
         geometry->setColorArray(mColor.get());
         geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

         mGeode->setStateSet(states);
         mGeode->addDrawable(geometry);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDQuadElement::~HUDQuadElement()
      {
         if (mRoot.valid() && mRoot->getNumParents() > 0)
         {
            osg::Group* parent = dynamic_cast<osg::Group*> (mRoot->getParent(0));
            if (parent != NULL)
            {
               parent->removeChild(mRoot.get());
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDQuadElement::Has(HUDQuadElement& element) const
      {
         return mRoot->getNumChildren() != mRoot->getChildIndex(element.mRoot.get());
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDQuadElement::Add(HUDQuadElement& element, int index)
      {
         unsigned numChildren = mRoot->getNumChildren();
         unsigned safeIndex = mRoot->getChildIndex(element.mRoot.get());

         // Avoid adding new child if it already exists
         if (safeIndex != numChildren || &element == this)
         {
            return false;
         }

         // Modify the safe index
         if (index < 0 || index >= int(numChildren))
         {
            // Index was out of range and unsafe.
            safeIndex = numChildren;
         }
         else
         {
            // User supplied index is safe.
            safeIndex = unsigned(index);
         }

         return mRoot->insertChild(safeIndex, element.mRoot.get());
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDQuadElement::Remove(HUDQuadElement& element)
      {
         return mRoot->removeChild(element.mRoot.get());
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned HUDQuadElement::GetTotalChildren() const
      {
         return mRoot->getNumChildren() - 1; // -1 for the root's first main child
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Node* HUDQuadElement::GetOSGNode()
      {
         return mRoot.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Node* HUDQuadElement::GetOSGNode() const
      {
         return mRoot.get();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetSize(float width, float height)
      {
         mWidth = width;
         mHeight = height;
         (*mVerts)[0].set(0,0,0);
         (*mVerts)[1].set(mWidth,0,0);
         (*mVerts)[2].set(mWidth,mHeight,0);
         (*mVerts)[3].set(0,mHeight,0);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::GetSize(osg::Vec2& outSize) const
      {
         outSize.set(mWidth, mHeight);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetPosition(float x, float y, float z)
      {
         // Modify Y based on window height ratio
         y *= WIN_HEIGHT_RATIO;

         mPos.set(x, y, z);
         osg::Matrix mtx = mTrans->getMatrix();
         mtx.setTrans(mPos);
         mTrans->setMatrix(mtx);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::GetPosition(osg::Vec3& outPos) const
      {
         // Y component: undo the ratio adjustment set in SetPosition
         outPos.set(mPos[0], mPos[1]/WIN_HEIGHT_RATIO, mPos[2]);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::GetPosition(osg::Vec2& outPos) const
      {
         // Y component: undo the ratio adjustment set in SetPosition
         outPos.set(mPos[0], mPos[1]/WIN_HEIGHT_RATIO);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetOffset(float x, float y)
      {
         (*mVerts)[0].set(x,y,0);
         (*mVerts)[1].set(x+mWidth,y,0);
         (*mVerts)[2].set(x+mWidth,y+mHeight,0);
         (*mVerts)[3].set(x,y+mHeight,0);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::GetOffset(osg::Vec2& outOffset) const
      {
         outOffset.set((*mVerts)[0][0], (*mVerts)[0][1]);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetRotation(float degrees)
      {
         osg::Matrix mtx;
         mtx.makeRotate(degrees, osg::Vec3(0.0,0.0,1.0));
         mtx.setTrans(mPos);
         mTrans->setMatrix(mtx);
         mRotation = degrees;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetColor(float r, float g, float b)
      {
         (*mColor)[0].set(r, g, b, mAlpha);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetColor(float r, float g, float b, float a)
      {
         mAlpha = a;
         (*mColor)[0].set(r, g, b, a);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::GetColor(osg::Vec4& outColor) const
      {
         outColor = (*mColor)[0];
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetAlpha(float alpha)
      {
         mAlpha = alpha;
         (*mColor)[0][3] = alpha;
      }

      //////////////////////////////////////////////////////////////////////////
      float HUDQuadElement::GetAlpha() const
      {
         return mAlpha;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::SetVisible(bool visible)
      {
         mVisible = visible;
         if (mVisible)
         {
            GetOSGNode()->setNodeMask(dtUtil::NodeMask::BACKGROUND);
         }
         else
         {
            GetOSGNode()->setNodeMask(dtUtil::NodeMask::NOTHING);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::Show()
      {
         SetVisible(true);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDQuadElement::Hide()
      {
         SetVisible(false);
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Geode* HUDQuadElement::GetGeode()
      {
         return mGeode.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Geode* HUDQuadElement::GetGeode() const
      {
         return mGeode.get();
      }

   }
}
