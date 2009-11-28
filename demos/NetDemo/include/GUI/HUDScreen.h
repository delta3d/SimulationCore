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
   class PlayerStatusActor;

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

            void UpdatePlayerInfo(PlayerStatusActor& player);

            virtual bool Update(float simTimeDelta);

            // DEBUG:
            void SetHelpTextLine(int index, const std::string& text,
               const osg::Vec4 color = osg::Vec4(1.0,1.0,1.0,1.0));

         protected:
            virtual ~HUDScreen();

         private:
            dtCore::RefPtr<GameLogicComponent> mLogicComp;

            // Animation Controllers

            // Special Widgets
            CEGUI::Window* mDamageMeter_Fort;
            CEGUI::Window* mScore;
      };
   }
}

#endif
