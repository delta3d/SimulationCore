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
#include "GUI/CustomCeguiWidgets.h"



namespace CEGUI
{
   namespace CustomWidgets
   {
      //////////////////////////////////////////////////////////////////////////
      // CUSTOM WINDOW FACTORIES
      //////////////////////////////////////////////////////////////////////////
      CEGUI_DEFINE_WINDOW_FACTORY(ListItem)



      //////////////////////////////////////////////////////////////////////////
      // CUSTOM WIDGET FUNCTIONS
      //////////////////////////////////////////////////////////////////////////
      void bindCEGUIWindowFactories()
      {
         // Register special window factories.
         CEGUI::WindowFactoryManager& wfm = CEGUI::WindowFactoryManager::getSingleton();
         wfm.addFactory( &CEGUI_WINDOW_FACTORY(ListItem) );
         /*wfm.addFalagardWindowMapping(
            ListItem::WidgetTypeName, "CEGUI/ItemEntry",
            ListItem::WidgetTypeName, "Falagard/ItemEntry" );*/
      }



      //////////////////////////////////////////////////////////////////////////
      // LIST ITEM CODE
      //////////////////////////////////////////////////////////////////////////
      const CEGUI::String ListItem::WidgetTypeName("NetDemo/ListBoxItem");

      const dtUtil::RefString ListItem::WIDGET_TYPE(ListItem::WidgetTypeName.c_str());
      const dtUtil::RefString ListItem::PROPERTY_ACTION("Action");

      //////////////////////////////////////////////////////////////////////////
      ListItem::ListItem(CEGUI::String type, CEGUI::String name)
         : CEGUI::ItemEntry(type,name)
      {
         rename(name);
      }

      //////////////////////////////////////////////////////////////////////////
      ListItem::~ListItem()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      Size ListItem::getItemPixelSize(void) const
      {
         return CEGUI::Size( 10.0f, 10.0f );
      }

   }
}
