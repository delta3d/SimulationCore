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
#include <dtGame/gamemanager.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/GUI/SimpleScreen.h>
#include <SimCore/GUI/DefaultAnimationControllers.h>

#include "Actors/PlayerStatusActor.h"
#include "ActorRegistry.h"
#include "GUI/CustomCeguiWidgets.h"
#include "GUI/ReadyRoomScreen.h"

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
      ReadyRoomScreen::ReadyRoomScreen()
         : BaseClass("ReadRoomScreen","CEGUI/layouts/NetDemo/ReadyRoom.layout")
         , mListPlayers(nullptr)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      ReadyRoomScreen::~ReadyRoomScreen()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ReadyRoomScreen::Setup(dtGame::GameManager& gm, SimCore::Components::HUDGroup* root)
      {
         BaseClass::Setup(root);

         mGM = &gm;

         CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();

         mListPlayers = static_cast<CEGUI::ItemListbox*>(wm.getWindow("ReadyRoom_PlayerList"));
      }

      /////////////////////////////////////////////////////////////////////////////
      void ReadyRoomScreen::UpdatePlayerList()
      {
         CEGUI::WindowManager& wm = CEGUI::WindowManager::getSingleton();

         // Clear the old list to be updated with new entries.
         mListPlayers->resetList();

         // Capture all the player status objects.
         typedef std::vector<dtDAL::ActorProxy*> ProxyArray;
         ProxyArray proxies;
         mGM->FindActorsByType(*NetDemo::NetDemoActorRegistry::PLAYER_STATUS_ACTOR_TYPE, proxies);

         // Create list items for each of the player status objects.
         CEGUI::String checkboxSuffix("_ReadyBox");
         PlayerStatusActor* curPlayerStats = nullptr;
         dtDAL::ActorProxy* curProxy = nullptr;
         ProxyArray::iterator proxyIter = proxies.begin();
         ProxyArray::iterator endProxyArray = proxies.end();
         for( ; proxyIter != endProxyArray; ++proxyIter)
         {
            curProxy = *proxyIter;
            if(curProxy != nullptr)
            {
               curProxy->GetActor(curPlayerStats);
               if(curPlayerStats != nullptr)
               {
                  // Create the new list item.
                  CEGUI::String itemName(curPlayerStats->GetUniqueId().ToString().c_str());
                  CEGUI::CustomWidgets::ListItem* item = dynamic_cast<CEGUI::CustomWidgets::ListItem*>
                     (wm.createWindow(CEGUI::CustomWidgets::ListItem::WidgetTypeName, itemName));

                  // Format the item.
                  CEGUI::String itemText(curPlayerStats->GetName().c_str());
                  item->setText(itemText);

                  // Set ready indicator...
                  CEGUI::String checkboxName(itemName + checkboxSuffix);
                  CEGUI::Checkbox* readyBox = static_cast<CEGUI::Checkbox*>
                     (wm.createWindow("WindowsLook/Checkbox", checkboxName));
                  CEGUI::UVector2 dims(CEGUI::UDim(0.0f,40),CEGUI::UDim(0.0f,40));
                  readyBox->setSize(dims);
                  readyBox->setSelected(curPlayerStats->IsReady());
                  readyBox->setEnabled(false);
                  readyBox->setHorizontalAlignment(CEGUI::HA_RIGHT);
                  item->addChildWindow(readyBox);

                  // Add the new list item to the list box.
                  mListPlayers->addItem(item);
               }
            }
         }
      }


   }
}
