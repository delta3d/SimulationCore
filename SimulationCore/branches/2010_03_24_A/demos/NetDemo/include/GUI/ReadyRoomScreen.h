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

#ifndef NETDEMO_READY_ROOM_SCREEN_H
#define NETDEMO_READY_ROOM_SCREEN_H

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
   class ItemListbox;
}

namespace dtGame
{
   class GameManager;
}

namespace SimCore
{
   namespace Components
   {
      class HUDGroup;
   }

   namespace GUI
   {
      class PositionController;
   }
}

namespace NetDemo
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      class NETDEMO_EXPORT ReadyRoomScreen : public SimCore::GUI::SimpleScreen
      {
         public:
            typedef SimCore::GUI::SimpleScreen BaseClass;

            ReadyRoomScreen();

            virtual void Setup(dtGame::GameManager& gm, SimCore::Components::HUDGroup* root = NULL);

            void UpdatePlayerList();

         protected:
            virtual ~ReadyRoomScreen();

         private:
            dtCore::RefPtr<dtGame::GameManager> mGM;

            // Animation Controllers
//            dtCore::RefPtr<SimCore::GUI::PositionController> mPlayerListPos;

            // Special Widgets
            CEGUI::ItemListbox* mListPlayers;
      };
   }
}

#endif
