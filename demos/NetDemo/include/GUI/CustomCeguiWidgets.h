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

#ifndef NETDEMO_CUSTOM_CEGUI_WIDGETS
#define NETDEMO_CUSTOM_CEGUI_WIDGETS

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <DemoExport.h>
#include <dtUtil/refstring.h>

#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>



namespace CEGUI
{
   /////////////////////////////////////////////////////////////////////////////
   // CUSTOM WIDGET FUNCTIONS
   /////////////////////////////////////////////////////////////////////////////
   namespace CustomWidgets
   {
      void bindCEGUIWindowFactories();



      //////////////////////////////////////////////////////////////////////////
      // LIST ITEM CODE
      //////////////////////////////////////////////////////////////////////////
      class NETDEMO_EXPORT ListItem : public CEGUI::ItemEntry
      {
         public:
            static const CEGUI::String WidgetTypeName;

            static const dtUtil::RefString WIDGET_TYPE;
            static const dtUtil::RefString PROPERTY_ACTION;

            ListItem(CEGUI::String type, CEGUI::String name);

            virtual ~ListItem();

         protected:
            virtual Size getItemPixelSize(void) const;
      };



      //////////////////////////////////////////////////////////////////////////
      // CUSTOM WINDOW FACTORY DECLARATIONS
      //////////////////////////////////////////////////////////////////////////
      CEGUI_DECLARE_WINDOW_FACTORY(ListItem)

   }
}

#endif
