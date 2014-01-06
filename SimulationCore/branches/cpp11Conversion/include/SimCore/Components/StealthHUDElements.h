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

#ifndef _STEALTH_HUD_ELEMENTS_H_
#define _STEALTH_HUD_ELEMENTS_H_

#include <SimCore/Export.h>
#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>
#include <SimCore/Components/BaseHUDElements.h>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // HUD Button
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthButton : public HUDButton
      {
         public:
            StealthButton( const std::string& name, const std::string& imageName,
               const std::string& keyLabel, const std::string& imageset="Toolbar", 
               const std::string& type = DEFAULT_IMAGE_TYPE );

            bool SetKeyLabel( const std::string& keyLabel );
            const HUDImage* GetKeyLabel() const { return mKeyLabel.get(); }

         protected:
            virtual ~StealthButton();

         private:
            std::shared_ptr<HUDImage> mKeyLabel;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Toolbar
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthToolbar : public HUDToolbar
      {
         public:
            static const std::string DEFAULT_TOOLBAR_IMAGE_SET;

            StealthToolbar( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            StealthButton* GetButton( const std::string& buttonName );

            void GetButtonNames( std::vector<std::string>& outButtonNames ) const;

            bool AddButton( const std::string& buttonName, const std::string& key, bool updateElementSizes = true );

            bool AddButton( const std::string& buttonName, const std::string& key,
               const std::string& imageSet, bool updateElementSizes = true );

            bool AddButton( std::shared_ptr<StealthButton>& newButton, bool updateElementSizes = true );

            // @param oldButtonName The name of the button to be replaced
            // @param newButton The button to take the place of the old button.
            //        This parameter is a ref pointer to ensure local scope
            //        buttons are not inserted into this toolbar.
            //        The new button is assumed to have the same dimensions as the
            //        old button that is to be replaced.
            // @param outOldButton A pointer to a ref pointer to capture the old button
            // @return TRUE if the operation was successful; FALSE if the old button
            //         is not found or the new button is nullptr.
            bool ReplaceButton( const std::string& oldButtonName, 
               std::shared_ptr<StealthButton>& newButton, std::shared_ptr<StealthButton>* outOldButton = nullptr );

            bool RemoveButton( const std::string& buttonName );

            void SetButtonsActive( bool active );

            bool SetButtonActive( const std::string& buttonName, bool active );

            unsigned int GetButtonCount() const;

            void ClearButtons();

         protected:
            virtual ~StealthToolbar();

            virtual bool ClearElements();

            void UpdateElementSizes();

         private:
            std::map< const std::string, std::shared_ptr<StealthButton> > mNamedButtonMap;

      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthMeter : public HUDImage
      {
         public:
            StealthMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            virtual void Initialize();

            // This function is being overridden so that it can affect
            // the meter mask length.
            virtual void SetValue( float current, float max = 1.0f, float min = 0.0f );

            float GetValue() const;

            HUDMeter& GetMeterElement();
            HUDText& GetTextElement();

            const HUDMeter& GetMeterElement() const;
            const HUDText& GetTextElement() const;

         protected:
            virtual ~StealthMeter();

            // This function is called by the constructor.
            // Override this function to change the meter element this object uses.
            // By default, this function creates a HUDBarMeter and assigns it to
            // outMeterOfThis
            // @param outMeterOfThis The reference this object contains that will receive the new meter element
            // @param meterName The default name to be given to the new meter element
            virtual void CreateMeterElement( std::shared_ptr<HUDMeter>& outMeterOfThis,
               const std::string& meterName );

         private:
            // Value text will display the numeric
            // value of this meter.
            std::shared_ptr<HUDText> mValueText;

            std::shared_ptr<HUDMeter> mMeter;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Health Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthHealthMeter : public StealthMeter
      {
         public:
            StealthHealthMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            virtual void Initialize();

         protected:
            virtual ~StealthHealthMeter();

         private:
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Ammo Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthAmmoMeter : public StealthMeter
      {
         public:
            StealthAmmoMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            virtual void Initialize();

         protected:
            virtual ~StealthAmmoMeter();

         private:
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Compass Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthCompassMeter : public StealthMeter
      {
         public:
            StealthCompassMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            virtual void Initialize();

            /** This function is being overridden so that it can prevent
             * sliding the compass strip too far off either ends.
             */
            virtual void SetValue( float current, float max = 1.0f, float min = 0.0f );

            /// Set this if the numeric compass should round to whole numbers or not.
            void SetShowWholeNumbersOnly(bool whole);
            /// @return if the numeric compass should round to whole numbers or not.
            bool GetShowWholeNumbersOnly() const;

         protected:
            virtual ~StealthCompassMeter();

            virtual void CreateMeterElement( std::shared_ptr<HUDMeter>& outMeterOfThis,
               const std::string& meterName );

         private:
            bool mShowWholeNumbersOnly;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD GPS Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthGPSMeter : public HUDImage
      {
         public:
            StealthGPSMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            void SetLatitude( float latitude );
            void SetLongitude( float longitude );

            void SetLatLong( float latitude, float longitude );

            void SetText1( const std::string& text );
            void SetText2( const std::string& text );

            HUDText& GetText1();
            const HUDText& GetText1() const;

            HUDText& GetText2();
            const HUDText& GetText2() const;

         protected:
            virtual ~StealthGPSMeter();

         private:
            std::shared_ptr<HUDText> mLat;
            std::shared_ptr<HUDText> mLong;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD GMRS Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthMGRSMeter : public HUDImage
      {
         public:
            StealthMGRSMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            void SetText( const std::string& text );

            HUDText& GetText();
            const HUDText& GetText() const;

         protected:
            virtual ~StealthMGRSMeter();

         private:
            std::shared_ptr<HUDText> mText;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Cartesian Meter
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthCartesianMeter : public HUDImage
      {
         public:
            StealthCartesianMeter( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            void SetX(float x, const std::string& unit);
            void SetY(float y, const std::string& unit);
            void SetZ(float z, const std::string& unit);

            HUDText& GetX();
            const HUDText& GetX() const;

            HUDText& GetY();
            const HUDText& GetY() const;

            HUDText& GetZ();
            const HUDText& GetZ() const;

         protected:
            virtual ~StealthCartesianMeter();

            static void SetFloat(float value, const std::string& unit, HUDText& text);
         private:
            std::shared_ptr<HUDText> mX;
            std::shared_ptr<HUDText> mY;
            std::shared_ptr<HUDText> mZ;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Call Sign
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT StealthCallSign : public HUDImage
      {
         public:
            StealthCallSign( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            void SetCallSign( const std::string& callSign );

            const HUDText* GetTextElement() const { return mCallSign.get(); }

         protected:
            virtual ~StealthCallSign();

         private:
            std::shared_ptr<HUDText> mCallSign;
      };



      //////////////////////////////////////////////////////////////////////////
      // HUD Call Sign
      //////////////////////////////////////////////////////////////////////////
#if CEGUI_VERSION_MAJOR == 0 && CEGUI_VERSION_MINOR < 7
      typedef dtCore::DeltaDrawable GuiDrawable;
#else
      typedef dtGUI::GUI GuiDrawable;
#endif
      class SIMCORE_EXPORT StealthSpeedometer : public HUDImage
      {
         public:
            StealthSpeedometer( const std::string& name, const std::string& type = DEFAULT_IMAGE_TYPE );

            bool RegisterNeedleWithGUI( GuiDrawable* gui );
            bool UnregisterNeedleWithGUI();

            // This function is being overridden so that it can 
            // control the rotation of the speedometer needle and text.
            virtual void SetValue( float current, float max = 1.0f, float min = 0.0f );

            virtual void SetVisible( bool visible );

            // The following accessors are primarily for testing purposes.
            const HUDQuadElement* GetNeedle() const { return mNeedle.get(); }
            const HUDText* GetTextElement() const { return mText.get(); }

         protected:
            virtual ~StealthSpeedometer();

            void UpdateNeedlePosition();

         private:
            float mPreviousSpeed;
            float mLastReadout;
            std::shared_ptr<HUDText> mText;
            std::shared_ptr<HUDQuadElement> mNeedle;
            std::weak_ptr<GuiDrawable> mGUI;
            osg::Vec2 mPivot; // offset from the bottom left of the meter
      };

   }
}

#endif
