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

#include "Components/GameLogicComponent.h"
#include "GUI/HUDScreen.h"

#ifdef None
#undef None
#endif
#include <CEGUI.h>



namespace NetDemo
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      HUDScreen::HUDScreen()
         : BaseClass("HUDScreen","CEGUI/layouts/NetDemo/HUD.layout")
         , mDamageMeter_Fort(NULL)
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

         mDamageMeter_Fort = static_cast<CEGUI::Window*>(wm.getWindow("HUD_DamageMeter_Fort"));
      }

      //////////////////////////////////////////////////////////////////////////
      void HUDScreen::SetFortDamageRatio(float damageRatio)
      {
         // Update fort damage meter.
         mDamageMeter_Fort->setProperty("MeterLevel",
            CEGUI::PropertyHelper::floatToString(damageRatio));
      }

      //////////////////////////////////////////////////////////////////////////
      bool HUDScreen::Update(float simTimeDelta)
      {
         return BaseClass::Update(simTimeDelta);

         // TODO:
      }

   }
}
