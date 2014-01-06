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

#include <prefix/SimCorePrefix.h>

#include <SimCore/Components/StealthHUDElements.h>

#include <dtUtil/log.h>
#include <cmath>

namespace SimCore
{
   namespace Components
   {
      const std::string StealthToolbar::DEFAULT_TOOLBAR_IMAGE_SET("Toolbar");

      //////////////////////////////////////////////////////////////////////////
      // HUD Button Code
      //////////////////////////////////////////////////////////////////////////
      StealthButton::StealthButton( const std::string& name, const std::string& imageName,
         const std::string& keyLabel, const std::string& imageset, const std::string& type )
                                 : HUDButton( name, type )
      {
         std::string buttonStateName = imageName+"_ON";
         std::shared_ptr<HUDImage> image = new HUDImage( buttonStateName );
         image->SetImage(imageset,buttonStateName);
         SetActiveElement(image.get());

         buttonStateName = imageName+"_OFF";
         image = new HUDImage( buttonStateName );
         image->SetImage(imageset,buttonStateName);
         SetInactiveElement(image.get());

         buttonStateName = imageName+"_DISABLED";
         image = new HUDImage( buttonStateName );
         image->SetImage(imageset,buttonStateName);
         SetDisabledElement(image.get());

         SetKeyLabel( keyLabel );

         SetSize( 90.0f/1920, 99.0f/1200 );

         SetActive( false );
      }

      //////////////////////////////////////////////////////////////////////////
      StealthButton::~StealthButton()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthButton::SetKeyLabel( const std::string& keyLabel )
      {
         // Remove the old label
         if( mKeyLabel.valid() )
         {
            mWindow->removeChildWindow( mKeyLabel->GetCEGUIWindow() );
         }

         try
         {
            // Add the new label
            mKeyLabel = new HUDImage( GetName()+"_KeyLabel"+keyLabel );
            mKeyLabel->SetImage("KeyLabels",keyLabel);
            mKeyLabel->SetSize( 32.0f/90.0f, 16.0f/99.0f );
            mKeyLabel->SetPosition( 0.0f, -1.0f/99.0f, HUDAlignment::CENTER_BOTTOM );
            mKeyLabel->GetCEGUIWindow()->setAlwaysOnTop(true);
            mWindow->addChildWindow( mKeyLabel->GetCEGUIWindow() );
            return true;
         }
         catch( CEGUI::Exception& )
         {
            std::stringstream ss;
            ss << "\nStealthButton \"" << GetName() << "\" was unable to change its key label to \"" 
               << keyLabel << "\".\n" << std::endl;
            LOG_ERROR( ss.str() );
            return false;
         }
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Toolbar Code
      //////////////////////////////////////////////////////////////////////////
      StealthToolbar::StealthToolbar( const std::string& name, const std::string& type )
      : HUDToolbar( name, type )
      {
         std::shared_ptr<HUDImage> image = new HUDImage( "LeftBracket" );
         image->SetImage(DEFAULT_TOOLBAR_IMAGE_SET,"Bracket_Left");
         SetStartElement( image.get() );

         image = new HUDImage( "RightBracket" );
         image->SetImage(DEFAULT_TOOLBAR_IMAGE_SET,"Bracket_Right");
         SetEndElement( image.get() );

         UpdateElementSizes();
      }

      //////////////////////////////////////////////////////////////////////////
      StealthToolbar::~StealthToolbar()
      {
         ClearButtons();
      }

      //////////////////////////////////////////////////////////////////////////
      StealthButton* StealthToolbar::GetButton( const std::string& buttonName )
      {
         std::map< const std::string, std::shared_ptr<StealthButton> >::iterator iter = 
            mNamedButtonMap.find( buttonName );

         return iter != mNamedButtonMap.end() ? iter->second.get() : nullptr;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthToolbar::GetButtonNames( std::vector<std::string>& outButtonNames ) const
      {
         std::map< const std::string, std::shared_ptr<StealthButton> >::const_iterator iter = 
            mNamedButtonMap.begin();

         for( ; iter != mNamedButtonMap.end(); ++iter )
         {
            outButtonNames.push_back( iter->second->GetName() );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::AddButton( const std::string& buttonName, const std::string& key, 
         bool updateElementSizes )
      {
         return AddButton( buttonName, key, DEFAULT_TOOLBAR_IMAGE_SET, updateElementSizes );
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::AddButton( const std::string& buttonName, const std::string& key, 
         const std::string& imageset, bool updateElementSizes )
      {
         if( GetButton( buttonName ) != nullptr ) { return false; }

         std::shared_ptr<StealthButton> button = new StealthButton( buttonName, buttonName, key, imageset );
         bool success = mNamedButtonMap.insert(std::make_pair(buttonName, button.get() )).second;

         if( success )
         {
            success = InsertElement( button.get() );
            if( updateElementSizes )
            {
               UpdateElementSizes();
            }
         }
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::AddButton( std::shared_ptr<StealthButton>& newButton, bool updateElementSizes )
      {
         if( ! newButton.valid() || GetButton( newButton->GetName() ) != nullptr ) { return false; }

         bool success = mNamedButtonMap.insert(std::make_pair(newButton->GetName(), newButton.get() )).second;

         if( success )
         {
            success = InsertElement( newButton.get() );
            if( updateElementSizes )
            {
               UpdateElementSizes();
            }
         }
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::ReplaceButton( const std::string& oldButtonName, 
         std::shared_ptr<StealthButton>& newButton, std::shared_ptr<StealthButton>* outOldButton )
      {
         std::map< const std::string, std::shared_ptr<StealthButton> >::iterator iter = 
            mNamedButtonMap.find( oldButtonName );

         if( iter == mNamedButtonMap.end() || ! newButton.valid() ) { return false; }

         StealthButton* old = iter->second.get();
         int index = GetElementIndex( *old );
         RemoveButton( oldButtonName );

         if( outOldButton != nullptr ) { *outOldButton = old; }

         bool success = mNamedButtonMap.insert(std::make_pair(newButton->GetName(), newButton.get() )).second;
         InsertElement(newButton.get(),index);
         UpdateElementSizes();

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::RemoveButton( const std::string& buttonName )
      {
         std::map< const std::string, std::shared_ptr<StealthButton> >::iterator iter = 
            mNamedButtonMap.find( buttonName );

         if( iter == mNamedButtonMap.end() ) { return false; }

         bool success = RemoveElement( iter->second.get() );
         UpdateElementSizes();
         mNamedButtonMap.erase(iter);
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthToolbar::UpdateElementSizes()
      {
         unsigned int numButtons = GetTotalElements();

         float totalWidth = 26.0f * 2.0f + 90.0f * numButtons;
         SetSize( totalWidth/1920.0f, 99.0f/1200.0f );

         float endRatio = 26.0f/totalWidth;
         float buttonRatio = 90.0f/totalWidth;

         GetStartElement()->SetSize( endRatio, 1.0f );
         GetEndElement()->SetSize( endRatio, 1.0f );

         std::map< const std::string, std::shared_ptr<StealthButton> >::iterator iter = 
            mNamedButtonMap.begin();
         for( ; iter != mNamedButtonMap.end(); ++iter )
         {
            if( iter->second.valid() )
            {
               iter->second->SetSize( buttonRatio, 1.0f );
            }
         }

         UpdateLayout();
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthToolbar::SetButtonsActive( bool active )
      {
         std::map< const std::string, std::shared_ptr<StealthButton> >::iterator iter = 
            mNamedButtonMap.begin();
         for( ; iter != mNamedButtonMap.end(); ++iter )
         {
            if( iter->second.valid() )
            {
               iter->second->SetActive( active );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::SetButtonActive( const std::string& buttonName, bool active )
      {
         StealthButton* button = GetButton( buttonName );

         if( button == nullptr ) { return false; }

         button->SetActive( active );
         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int StealthToolbar::GetButtonCount() const
      {
         return mNamedButtonMap.size();
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthToolbar::ClearElements()
      {
         bool success = HUDToolbar::ClearElements();
         if( ! mNamedButtonMap.empty() ) { mNamedButtonMap.clear(); }
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthToolbar::ClearButtons()
      {
         ClearElements();
      }


      //////////////////////////////////////////////////////////////////////////
      // HUD Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthMeter::StealthMeter( const std::string& name, const std::string& type )
      : HUDImage( name, type )
      {
      }

      //////////////////////////////////////////////////////////////////////////
      StealthMeter::~StealthMeter()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void StealthMeter::Initialize()
      {
         SetSize( 256.0f/1920.0f, 64.0f/1200.0f );

         // -183, -15, 64, 21 (x,y,w,h)
         mValueText = new HUDText(GetName()+".Value");
         mValueText->SetFontAndText("Arial-Bold-16","0",-183.0f/256.0f,-15.0f/64.0f);
         mValueText->SetAlignment(HUDAlignment::RIGHT_BOTTOM);
         mValueText->SetColor( 86.0f/256.0f, 0.0f, 0.0f );
         mValueText->SetSize( 64.0f/256.0f*0.75f, 21.0f/64.0f);
         mWindow->addChildWindow(mValueText->GetCEGUIWindow());

         // 84, 11, 150, 41 (x,y,w,h)
         CreateMeterElement( mMeter, GetName()+".MeterBar" );
         mMeter->SetPosition( 85.0f/256.0f, 11.0/64.0f);
         mMeter->SetSize( 150.0f/256.0f, 41.0f/64.0f );
         std::shared_ptr<HUDImage> meterImage = new HUDImage(GetName()+".MeterBarImage");
         mMeter->SetImage( meterImage.get() );
         mWindow->addChildWindow(mMeter->GetCEGUIWindow());
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthMeter::CreateMeterElement( std::shared_ptr<HUDMeter>& outMeterOfThis, const std::string& meterName )
      {
         outMeterOfThis = new HUDBarMeter( meterName );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthMeter::SetValue( float current, float max, float min )
      {
         std::stringstream text;
         text << (int)current; // convert number to a string

         mValueText->SetText( text.str() );
         mMeter->SetValue( current, max, min );
      }

      //////////////////////////////////////////////////////////////////////////
      float StealthMeter::GetValue() const
      {
         return mMeter->GetValue();
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDMeter& StealthMeter::GetMeterElement() const
      {
         return *mMeter;
      }

      //////////////////////////////////////////////////////////////////////////
      HUDMeter& StealthMeter::GetMeterElement()
      {
         return *mMeter;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthMeter::GetTextElement() const
      {
         return *mValueText;
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthMeter::GetTextElement()
      {
         return *mValueText;
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Health Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthHealthMeter::StealthHealthMeter( const std::string& name, const std::string& type )
      : StealthMeter( name, type )
      {
      }

      //////////////////////////////////////////////////////////////////////////
      StealthHealthMeter::~StealthHealthMeter()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void StealthHealthMeter::Initialize()
      {
         StealthMeter::Initialize();

         // Set the frame image
         SetImage( "MeterBars", "MeterFrame_Health" );

         HUDText& text = GetTextElement();
         text.SetColor( 86.0f/256.0f, 0.0f, 0.0f );

         HUDImage* meterImage = GetMeterElement().GetImage();
         meterImage->SetImage( "MeterBars", "MeterBar_Health" );

         SetValue( 100.0f, 100.0f, 0.0f );
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Ammo Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthAmmoMeter::StealthAmmoMeter( const std::string& name, const std::string& type )
      : StealthMeter( name, type )
      {
      }

      //////////////////////////////////////////////////////////////////////////
      StealthAmmoMeter::~StealthAmmoMeter()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void StealthAmmoMeter::Initialize()
      {
         StealthMeter::Initialize();

         // Set the frame image
         SetImage( "MeterBars", "MeterFrame_Ammo" );

         HUDText& text = GetTextElement();
         text.SetColor( 86.0f/256.0f, 0.0f, 0.0f );

         HUDImage* meterImage = GetMeterElement().GetImage();
         meterImage->SetImage( "MeterBars", "MeterBar_Ammo" );
         GetMeterElement().SetUnitCount( 15.0f );

         SetValue( 100.0f, 100.0f, 0.0f );
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Compass Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthCompassMeter::StealthCompassMeter( const std::string& name, const std::string& type )
      : StealthMeter( name, type )
      , mShowWholeNumbersOnly(true)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      StealthCompassMeter::~StealthCompassMeter()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCompassMeter::Initialize()
      {
         StealthMeter::Initialize();

         // Set the frame image
         SetImage( "MeterBars", "MeterFrame_Compass" );

         HUDText& text = GetTextElement();
         text.SetColor( 86.0f/256.0f, 0.0f, 0.0f );
         osg::Vec2 tmp;
         text.GetPosition( tmp );
         text.SetPosition( tmp[0]-4.0f/256.0f, tmp[1] );
         text.GetSize( tmp );
         text.SetSize( tmp[0]+4.0f/256.0f, tmp[1] );


         HUDSlideBarMeter& meter = static_cast<HUDSlideBarMeter&> (GetMeterElement());
         // The compass strip repeats the North units so that
         // the image starts with North at the center and so that
         // the image does not appear to end at North on both ends.
         // 150 is the width of the meter element
         meter.SetImageRangeScale(512.0f/(512.0f-150.0f));

         HUDImage* meterImage = meter.GetImage();
         meterImage->SetImage( "MeterBars", "MeterBar_Compass" );
         meterImage->SetPosition( 0.0f, 13.0f/41.0f );
         meterImage->SetSize( 512.0f/150.0f, 16.0f/41.0f );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCompassMeter::SetValue(float current, float max, float min)
      {
         std::stringstream text;

         float range = max - min;
         float intpart = 0.0; // ignored

         while (current < min)
         {
            current += range;
         }

         while (current > max)
         {
            current -= range;
         }

         float value = (std::modf((current - min) / range, &intpart) * range) + min;
         if (GetShowWholeNumbersOnly())
         {
            text << std::floor(value + 0.5f); // convert number to a string
         }
         else
         {
            text.precision(3);
            text << value;
         }

         GetTextElement().SetText(text.str());

         // We just want 0 - range in the end for the image, so if min isn't 0, we correct for it now.
         // rather than making the modf code more complex.
         GetMeterElement().SetValue(value - min,
                  max - min, min - min);
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCompassMeter::SetShowWholeNumbersOnly(bool whole)
      {
         mShowWholeNumbersOnly = whole;
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthCompassMeter::GetShowWholeNumbersOnly() const
      {
         return mShowWholeNumbersOnly;
      }


      //////////////////////////////////////////////////////////////////////////
      void StealthCompassMeter::CreateMeterElement( std::shared_ptr<HUDMeter>& outMeterOfThis,
                                                   const std::string& meterName )
      {
         outMeterOfThis = new HUDSlideBarMeter( meterName );
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD GPS Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthGPSMeter::StealthGPSMeter( const std::string& name, const std::string& type )
      : HUDImage( name, type )
      {
         SetImage( "MeterBars", "MeterFrame_GPS" );
         SetSize( 256.0f/1920.0f, 64.0f/1200.0f );

         mLat = new HUDText( name+".Latitude" );
         mLat->SetSize(100.0f/256.0f,40.0f/64.0f);
         mLat->SetAlignment(HUDAlignment::LEFT_CENTER);
         mLat->SetFontAndText("Arial-Bold-16","0.0",30.0f/256.0f,0.0f);
         mWindow->addChildWindow( mLat->GetCEGUIWindow() );

         mLong = new HUDText( name+".Longitude" );
         mLong->SetSize(100.0f/256.0f,40.0f/64.0f);
         mLong->SetAlignment(HUDAlignment::LEFT_CENTER);
         mLong->SetFontAndText("Arial-Bold-16","0.0",140.0f/256.0f,0.0f);
         mWindow->addChildWindow( mLong->GetCEGUIWindow() );
      }

      //////////////////////////////////////////////////////////////////////////
      StealthGPSMeter::~StealthGPSMeter()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthGPSMeter::SetLatitude( float latitude )
      {
         std::stringstream ss;
         ss << latitude;
         mLat->SetText( ss.str() );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthGPSMeter::SetLongitude( float longitude )
      {
         std::stringstream ss;
         ss << longitude;
         mLong->SetText( ss.str() );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthGPSMeter::SetLatLong( float latitude, float longitude )
      {
         SetLatitude( latitude );
         SetLongitude( longitude );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthGPSMeter::SetText1( const std::string& text )
      {
         mLat->SetText( text );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthGPSMeter::SetText2( const std::string& text )
      {
         mLong->SetText( text );
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthGPSMeter::GetText1()
      {
         return *mLat;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthGPSMeter::GetText1() const
      {
         return *mLat;
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthGPSMeter::GetText2()
      {
         return *mLong;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthGPSMeter::GetText2() const
      {
         return *mLong;
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD GMRS Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthMGRSMeter::StealthMGRSMeter( const std::string& name, const std::string& type )
      : HUDImage( name, type )
      {
         SetImage( "MeterBars", "MeterFrame_GMRS" );
         SetSize( 256.0f/1920.0f, 64.0f/1200.0f );

         mText = new HUDText( name+".Text" );
         mText->SetSize(210.0f/256.0f,40.0f/64.0f);
         mText->SetAlignment(HUDAlignment::LEFT_CENTER);
         mText->SetFontAndText("Arial-Bold-16","0.0",30.0f/256.0f,0.0f);
         mWindow->addChildWindow( mText->GetCEGUIWindow() );
      }

      //////////////////////////////////////////////////////////////////////////
      StealthMGRSMeter::~StealthMGRSMeter()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthMGRSMeter::SetText( const std::string& text )
      {
         mText->SetText( text );
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthMGRSMeter::GetText()
      {
         return *mText;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthMGRSMeter::GetText() const
      {
         return *mText;
      }



      //////////////////////////////////////////////////////////////////////////
      // HUD Cartesian Meter Code
      //////////////////////////////////////////////////////////////////////////
      StealthCartesianMeter::StealthCartesianMeter( const std::string& name, const std::string& type )
      : HUDImage( name, type )
      {
         SetImage( "MeterBars", "MeterFrame_Cartesian" );
         SetSize( 600.0f/1920.0f, 64.0f/1200.0f );

         mX = new HUDText( name+".X" );
         mX->SetSize(100.0f/384.0f,40.0f/64.0f);
         mX->SetAlignment(HUDAlignment::LEFT_CENTER);
         mX->SetFontAndText("Arial-Bold-16","0.0",27.0f/384.0f,0.0f);
         mWindow->addChildWindow( mX->GetCEGUIWindow() );

         mY = new HUDText( name+".Y" );
         mY->SetSize(100.0f/384.0f,40.0f/64.0f);
         mY->SetAlignment(HUDAlignment::LEFT_CENTER);
         mY->SetFontAndText("Arial-Bold-16","0.0",137.0f/384.0f,0.0f);
         mWindow->addChildWindow( mY->GetCEGUIWindow() );

         mZ = new HUDText( name+".Z" );
         mZ->SetSize(95.0f/384.0f,40.0f/64.0f);
         mZ->SetAlignment(HUDAlignment::LEFT_CENTER);
         mZ->SetFontAndText("Arial-Bold-16","0.0",252.0f/384.0f,0.0f);
         mWindow->addChildWindow( mZ->GetCEGUIWindow() );
      }

      //////////////////////////////////////////////////////////////////////////
      StealthCartesianMeter::~StealthCartesianMeter()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCartesianMeter::SetX(float x, const std::string& unit)
      {
         SetFloat(x, unit, *mX);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthCartesianMeter::GetX()
      {
         return *mX;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthCartesianMeter::GetX() const
      {
         return *mX;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCartesianMeter::SetY(float y, const std::string& unit)
      {
         SetFloat(y, unit, *mY);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthCartesianMeter::GetY()
      {
         return *mY;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthCartesianMeter::GetY() const
      {
         return *mY;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCartesianMeter::SetZ(float z, const std::string& unit)
      {
         SetFloat(z, unit, *mZ);
      }

      //////////////////////////////////////////////////////////////////////////
      HUDText& StealthCartesianMeter::GetZ()
      {
         return *mZ;
      }

      //////////////////////////////////////////////////////////////////////////
      const HUDText& StealthCartesianMeter::GetZ() const
      {
         return *mZ;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCartesianMeter::SetFloat(float value, const std::string& unit, HUDText& text)
      {
         std::stringstream ss;
         ss << value;
         ss << " " << unit;
         text.SetText(ss.str());
      }


      //////////////////////////////////////////////////////////////////////////
      // HUD Call Sign Code
      //////////////////////////////////////////////////////////////////////////
      StealthCallSign::StealthCallSign( const std::string& name, const std::string& type )
      : HUDImage( name, type )
      {
         SetImage( "CallSign", "CallSign_HummerDriver" );
         SetSize( 256.0f/1920.0f, 256.0f/1200.0f );

         mCallSign = new HUDText( name+".Text" );
         mCallSign->SetAlignment( HUDAlignment::LEFT_TOP );
         mCallSign->SetSize( 212.0f/256.0f, 44.0f/256.0f*0.66f ); // 0.66 scales the box down by 1/3
         mCallSign->SetFontAndText( "Arial-Bold-16", "", 30.0f/256.0f, 210.0f/256.0f );
         mWindow->addChildWindow( mCallSign->GetCEGUIWindow() );
      }

      //////////////////////////////////////////////////////////////////////////
      StealthCallSign::~StealthCallSign()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void StealthCallSign::SetCallSign( const std::string& callSign )
      {
         mCallSign->SetText( callSign );
      }

      //////////////////////////////////////////////////////////////////////////
      // HUD Speedometer Code
      //////////////////////////////////////////////////////////////////////////
      StealthSpeedometer::StealthSpeedometer( const std::string& name, const std::string& type )
         : HUDImage( name, type ),
         mPreviousSpeed(0.0f),
         mLastReadout(0.0f)
      {
         SetImage( "Speedometer", "Speedometer" );
         SetSize( 256.0f/1920.0f, 256.0f/1200.0f );

         mText = new HUDText( name+".Text" );
         mText->SetAlignment( HUDAlignment::LEFT_TOP );
         mText->SetSize( 212.0f/256.0f, 44.0f/256.0f*0.66f ); // 0.66 scales the box down by 1/3
         mText->SetFontAndText( "Arial-Bold-16", "0 mph", 50.0f/256.0f, 210.0f/256.0f );
         mWindow->addChildWindow( mText->GetCEGUIWindow() );

         mNeedle = new HUDQuadElement(name+".Needle","needle.png");
         mNeedle->SetSize( 96.0f/1920.0f, 8.0f/1200.0f );
         mNeedle->SetOffset( -4.0f/1920.0f, -4.0f/1200.0f );
         mPivot.set(128.0f/1920.0f,67.0f/1200.0f);
         UpdateNeedlePosition();

         SetValue( 0.0f );
      }

      //////////////////////////////////////////////////////////////////////////
      StealthSpeedometer::~StealthSpeedometer()
      {
         UnregisterNeedleWithGUI();
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthSpeedometer::RegisterNeedleWithGUI( GuiDrawable* gui )
      {
         bool success = false;
         if( gui != nullptr && mNeedle.valid() )
         {
            mGUI = gui;
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
            mGUI->AddChild( mNeedle.get() );
#else
            mGUI->GetRootNode().addChild(mNeedle->GetOSGNode());
#endif
            success = true;
         }
         UpdateNeedlePosition();
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool StealthSpeedometer::UnregisterNeedleWithGUI()
      {
         if( mGUI.valid() && mNeedle.valid() )
         {
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
            mGUI->RemoveChild( mNeedle.get() );
#else
            mGUI->GetRootNode().removeChild(mNeedle->GetOSGNode());
#endif
            return true;
         }
         return false;
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthSpeedometer::UpdateNeedlePosition()
      {
         osg::Vec2 offset;
         GetPosition( offset );

         const SimCore::Components::HUDAlignment& align = GetAlignment();
         if( align == SimCore::Components::HUDAlignment::RIGHT_BOTTOM )
         {
            offset[0] += -mPivot[0] + 1.0f;
            offset[1] += mPivot[1];
         }
         else
         {
            offset += mPivot; // already left bottom based
         }

         mNeedle->SetPosition( offset[0], offset[1] );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthSpeedometer::SetValue( float current, float max, float min )
      {
         mPreviousSpeed = (current + mPreviousSpeed) * 0.5f;

         if( (int)mLastReadout != (int)mPreviousSpeed 
            && std::abs(mPreviousSpeed-mLastReadout) > 0.5f )
         {
            mLastReadout = mPreviousSpeed;
         }

         std::stringstream ss;
         ss << (int)(mLastReadout) << " mph";
         mText->SetText( ss.str() );

         current = current > max ? max : current < min ? min : current;
         max = std::abs(max-min);
         current = max != 0.0f ? current/max : 0.0f;
         // This meter uses left handed rotation
         mNeedle->SetRotation( (1.0f-current)*3.141593f );
      }

      //////////////////////////////////////////////////////////////////////////
      void StealthSpeedometer::SetVisible( bool visible )
      {
         HUDImage::SetVisible( visible );
         if( mNeedle.valid() )
         {
            mNeedle->SetVisible( visible );
         }
      }

   }
}
