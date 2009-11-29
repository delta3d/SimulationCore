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

#include "Actors/PlayerStatusActor.h"
#include "Components/GameLogicComponent.h"
#include "GUI/HUDScreen.h"

#ifdef None
#undef None
#endif
#include <CEGUI.h>

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
      static const osg::Vec4 FLASH_COLOR_DEFAULT(1.0f, 0.0f, 0.0f, 1.0f);
      static const osg::Vec4 FLASH_COLOR_CRITICAL(1.0f, 1.0f, 0.0f, 1.0f);

      //////////////////////////////////////////////////////////////////////////
      HUDScreen::HUDScreen()
         : BaseClass("HUDScreen","CEGUI/layouts/NetDemo/HUD.layout")
         , mDamageMeterTimer(0.0f)
         , mDamageMeterLevel(0.0f)
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
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::Setup(GameLogicComponent& logicComp, SimCore::Components::HUDGroup* root)
      {
         BaseClass::Setup(root);

         mLogicComp = &logicComp;

         CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();

         mDamageMeter_Fort = wm.getWindow("HUD_DamageMeter_Fort");
         mScore = wm.getWindow("HUD_Score");

         // Help Prompt
         CEGUI::Window* helpLayout = wm.loadWindowLayout("CEGUI/layouts/NetDemo/Help.layout");
         GetRoot()->GetCEGUIWindow()->addChildWindow(helpLayout);
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::SetFortDamageRatio(float damageRatio)
      {
         mDamageMeterLevel = damageRatio;

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
               ss << "\n\tHUDScreen could not find help text line \"Help_" << (index+1) << "\"\n\n";
               printf(ss.str().c_str());
            }
         }
      }

   }
}
