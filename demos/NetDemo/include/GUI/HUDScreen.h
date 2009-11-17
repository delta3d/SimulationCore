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

#ifndef NETDEMO_HUD_SCREEN_H
#define NETDEMO_HUD_SCREEN_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/GUI/SimpleScreen.h>
#include "DemoExport.h"



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace CEGUI
{
   class Window;
}

namespace SimCore
{
   namespace Components
   {
      class HUDGroup;
   }
}

namespace NetDemo
{
   class GameLogicComponent;

   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      class NETDEMO_EXPORT HUDScreen : public SimCore::GUI::SimpleScreen
      {
         public:
            typedef SimCore::GUI::SimpleScreen BaseClass;

            HUDScreen();

            virtual void Reset();

            virtual void Setup(GameLogicComponent& logicComp,
               SimCore::Components::HUDGroup* root = NULL);

            void SetFortDamageRatio(float damageRatio);

            virtual bool Update(float simTimeDelta);

         protected:
            virtual ~HUDScreen();

         private:
            dtCore::RefPtr<GameLogicComponent> mLogicComp;

            // Animation Controllers

            // Special Widgets
            CEGUI::Window* mDamageMeter_Fort;
      };
   }
}

#endif
