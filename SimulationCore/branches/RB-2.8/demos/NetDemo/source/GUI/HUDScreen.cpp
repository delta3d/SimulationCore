/*
 * Copyright, 2007, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/GUI/CeguiUtils.h>

#include "Actors/PlayerStatusActor.h"
#include "Components/GameLogicComponent.h"
#include "GUI/HUDScreen.h"

#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>

// DEBUG:
#include <sstream>



namespace NetDemo
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      static const float FLASH_CYCLE_TIME_MAX = 0.75f;
      static const float FLASH_CYCLE_TIME_MIN = 0.25f;
      static const osg::Vec4 FLASH_COLOR_DEFAULT(0.5f, 0.5f, 1.0f, 1.0f);
      static const osg::Vec4 FLASH_COLOR_CRITICAL(1.0f, 0.0f, 0.0f, 1.0f);

      //////////////////////////////////////////////////////////////////////////
      HUDScreen::HUDScreen()
         : BaseClass("HUDScreen","CEGUI/layouts/NetDemo/HUD.layout")
         , mHelpEnabled(false)
         , mDamageMeterTimer(0.0f)
         , mDamageMeterLevel(0.0f)
         , mFortPoints(NULL)
         , mDamageMeter_Fort(NULL)
         , mScore(NULL)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      HUDScreen::~HUDScreen()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::Reset()
      {
         BaseClass::Reset();

         // TODO:
         // Reset other variables.
         mHelpEnabled = false;
         mControlHelpPos->SetToStart();
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::Setup(GameLogicComponent& logicComp, SimCore::Components::HUDGroup* root)
      {
         BaseClass::Setup(root);

         mLogicComp = &logicComp;

         CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();

         mFortPoints = wm.getWindow("HUD_FortPoints");
         mDamageMeter_Fort = wm.getWindow("HUD_DamageMeter_Fort");
         mScore = wm.getWindow("HUD_Score");

         // Help Prompt
         CEGUI::Window* window = wm.loadWindowLayout("CEGUI/layouts/NetDemo/Help.layout");
         mHelp = new SimCore::Components::HUDElement(*window);
         GetRoot()->GetCEGUIWindow()->addChildWindow(window);
         
         mControlHelpPos = static_cast<SimCore::GUI::PositionController*>
            (AddAnimationControl(SimCore::GUI::Screen::ANIM_TYPE_MOTION, *mHelp));
         osg::Vec4 bounds = SimCore::GUI::CeguiUtils::GetNormalizedScreenBounds(*window);
         bounds.y() = window->getPosition().d_y.d_scale;
         osg::Vec2 startPos(-(bounds.x() + bounds.z()), bounds.y());
         mControlHelpPos->SetStartTarget(startPos);
         mControlHelpPos->SetToStart();

         // Debug Info Window
         window = wm.loadWindowLayout("CEGUI/layouts/NetDemo/DebugInfo.layout");
         mDebugInfo = new SimCore::Components::HUDElement(*window);
         GetRoot()->GetCEGUIWindow()->addChildWindow(window);

         // Debug Info - Animation Controller
         mControlDebugInfoPos = static_cast<SimCore::GUI::PositionController*>
            (AddAnimationControl(SimCore::GUI::Screen::ANIM_TYPE_MOTION, *mDebugInfo));
         osg::Vec4 bounds2 = SimCore::GUI::CeguiUtils::GetNormalizedScreenBounds(*window);
         bounds2.y() = window->getPosition().d_y.d_scale;
         osg::Vec2 startPos2(-(bounds2.x() + bounds2.z()), bounds2.y());
         mControlDebugInfoPos->SetStartTarget(startPos2);
         mControlDebugInfoPos->SetToStart();

      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::SetFortDamageRatio(float damageRatio)
      {
         mDamageMeterLevel = damageRatio;

         mFortPoints->setText(CEGUI::PropertyHelper::intToString(int(1000.0f * damageRatio)));

         // Update fort damage meter.
         mDamageMeter_Fort->setProperty("MeterLevel",
            CEGUI::PropertyHelper::floatToString(damageRatio));
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::UpdatePlayerInfo(PlayerStatusActor& player)
      {
         CEGUI::String scoreText("Score: ");
         scoreText += CEGUI::PropertyHelper::intToString(player.GetScore());
         mScore->setText(scoreText);
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDScreen::Update(float simTimeDelta)
      {
         if(BaseClass::Update(simTimeDelta))
         {
            if(mDamageMeterLevel <= 0.5f)
            {
               mDamageMeterTimer -= simTimeDelta;
            }

            if(mDamageMeterTimer <= 0.0f)
            {
               if(mDamageMeterColor == FLASH_COLOR_DEFAULT)
               {
                  mDamageMeterColor = FLASH_COLOR_CRITICAL;
               }
               else
               {
                  mDamageMeterColor = FLASH_COLOR_DEFAULT;
               }

               CEGUI::colour ceguiColor(mDamageMeterColor.x(), mDamageMeterColor.y(), mDamageMeterColor.z());
               mDamageMeter_Fort->setProperty("MeterColor", CEGUI::PropertyHelper::colourToString(ceguiColor));
               mDamageMeterTimer = FLASH_CYCLE_TIME_MAX * (mDamageMeterLevel * 2.0f);
               if(mDamageMeterTimer < FLASH_CYCLE_TIME_MIN)
               {
                  mDamageMeterTimer = FLASH_CYCLE_TIME_MIN;
               }
            }
         }

         return true;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::SetHelpEnabled(bool enabled)
      {
         if(mHelpEnabled != enabled)
         {
            mHelpEnabled = enabled;

            if(enabled)
            {
               mControlHelpPos->Execute(0.4f, 0.0f, false); // Slide In
            }
            else
            {
               mControlHelpPos->Execute(0.4f, 0.0f, true); // Slide Out
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDScreen::IsHelpEnabled() const
      {
         return mHelpEnabled;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::ToggleHelp()
      {
         SetHelpEnabled( ! mHelpEnabled);
      }

      //////////////////////////////////////////////////////////////////////////
      // DEBUG:
      void HUDScreen::SetHelpTextLine(int index, const std::string& text, const osg::Vec4 color)
      {
         if(index >= 0 && index < 9)
         {
            CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();

            try
            {
               std::stringstream ss;
               ss << "Help_" << (index+1);
               CEGUI::Window* textLine = wm.getWindow(ss.str());

               if(textLine != NULL)
               {
                  textLine->setText(text.c_str());

                  CEGUI::ColourRect colorRect;
                  CEGUI::colour cornerColor(color.x(),color.y(),color.z(),color.w());
                  colorRect.d_bottom_left = cornerColor;
                  colorRect.d_bottom_right = cornerColor;
                  colorRect.d_top_left = cornerColor;
                  colorRect.d_top_right = cornerColor;
                  textLine->setProperty("TextColours", CEGUI::PropertyHelper::colourRectToString(colorRect));
               }
            }
            catch(...)
            {
               std::stringstream ss;
               ss << "\n\tHUDScreen could not find help text line \"Help_" << (index+1) << "\"\n";
               printf("%s\n", ss.str().c_str());
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::HandleDebugInfoUpdated()
      {
         if (mLogicComp.valid())
         {
            // std::string mDRGhostMode;

            SetDebugInfoEnabled(mLogicComp->GetDebugInfo().mShowDebugWindow);

            if (mLogicComp->GetDebugInfo().mShowDebugWindow)
            {
               //CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();
               std::string tempValue;
               osg::Vec4 highlightColor(0.2, 1.0, 0.2, 1.0);
               osg::Vec4 normalColor(1.0, 1.0, 1.0, 1.0);

               // Current Debug Variable
               tempValue = "Cur Var: " + mLogicComp->GetDebugInfo().mCurDebugVar;
               SetDebugInfoTextLine(2, tempValue, highlightColor);

               // DR Algorithm
               tempValue = "DR Algorithm: " + mLogicComp->GetDebugInfo().mDRAlgorithm;
               SetDebugInfoTextLine(3, tempValue, 
                  (mLogicComp->GetDebugInfo().mCurDebugVar == "DR Algorithm" 
                     ? highlightColor : normalColor));

               // Ground Clamp
               tempValue = "DR Ground Clamp: " + mLogicComp->GetDebugInfo().mDRGroundClampStatus;
               SetDebugInfoTextLine(4, tempValue, 
                  (mLogicComp->GetDebugInfo().mCurDebugVar == "DR Ground Clamp" 
                     ? highlightColor : normalColor));

               // Ground Clamp
               tempValue = "DR Blending Type: " + (mLogicComp->GetDebugInfo().mDRUseSplines 
                  ? std::string("Splines") : std::string("Projective"));
               SetDebugInfoTextLine(5, tempValue,
                  (mLogicComp->GetDebugInfo().mCurDebugVar == "DR Blending Type" 
                     ? highlightColor : normalColor));

               // DR Ghost Mode
               tempValue = "DR Ghost Mode: " + mLogicComp->GetDebugInfo().mDRGhostMode;
               SetDebugInfoTextLine(6, tempValue, normalColor);

               // Publish Rate
               tempValue = "Publish Rate: " + dtUtil::ToString(mLogicComp->GetDebugInfo().mDRPublishRate) + "/sec";
               SetDebugInfoTextLine(7, tempValue, normalColor);

               // DR Fixed Blend
               tempValue = "DR Fixed Blend: " + (mLogicComp->GetDebugInfo().mDRUseFixedBlend
                  ? std::string("TRUE") : std::string("FALSE"));
               SetDebugInfoTextLine(8, tempValue, 
                  (mLogicComp->GetDebugInfo().mCurDebugVar == "DR Fixed Blend" 
                     ? highlightColor : normalColor));

               // DR Publish Ang Vel
               tempValue = "DR Publish Ang Vel: " + (mLogicComp->GetDebugInfo().mDRPublishAngularVel
                  ? std::string("TRUE") : std::string("FALSE"));
               SetDebugInfoTextLine(9, tempValue, 
                  (mLogicComp->GetDebugInfo().mCurDebugVar == "DR Publish Ang Vel" 
                  ? highlightColor : normalColor));

               // Avg DR error and cur speed
               tempValue = "Spd: " + dtUtil::ToString(mLogicComp->GetDebugInfo().mDRAvgSpeed) + 
                  "  DRErr: " + dtUtil::ToString(mLogicComp->GetDebugInfo().mDRAvgError);
               SetDebugInfoTextLine(10, tempValue, normalColor);
               
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::SetDebugInfoEnabled(bool enabled)
      {
         if(mDebugInfoEnabled != enabled)
         {
            mDebugInfoEnabled = enabled;

            if(enabled)
            {
               mControlDebugInfoPos->Execute(0.4f, 0.0f, false); // Slide In
            }
            else
            {
               mControlDebugInfoPos->Execute(0.4f, 0.0f, true); // Slide Out
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDScreen::IsDebugInfoEnabled() const
      {
         return mDebugInfoEnabled;
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::ToggleDebugInfo()
      {
         SetDebugInfoEnabled( !mDebugInfoEnabled );
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::SetDebugInfoTextLine(int index, const std::string& text, const osg::Vec4 color)
      {
         //if(index >= 0 && index < 6)
         //{
 	         CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();
 
            try
            {
               std::stringstream ss;
               ss << "DebugInfo_" << (index+1);
               CEGUI::Window* textLine = wm.getWindow(ss.str());

               if(textLine != NULL)
               {
                  textLine->setText(text.c_str());

                  CEGUI::ColourRect colorRect;
                  CEGUI::colour cornerColor(color.x(),color.y(),color.z(),color.w());
                  colorRect.d_bottom_left = cornerColor;
                  colorRect.d_bottom_right = cornerColor;
                  colorRect.d_top_left = cornerColor;
                  colorRect.d_top_right = cornerColor;
                  textLine->setProperty("TextColours", CEGUI::PropertyHelper::colourRectToString(colorRect));
               }
            }
            catch(...)
            {
               std::stringstream ss;
               ss << "\n\tHUDScreen could not find DebugInfo line \"DebugInfo_" << (index+1) << "\"\n\n";
               printf("%s", ss.str().c_str());
            }
         //}
      }
   }
}
