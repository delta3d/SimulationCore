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

#ifndef NETDEMO_MAIN_MENU_SCREEN_H
#define NETDEMO_MAIN_MENU_SCREEN_H

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

namespace osg
{
   class MatrixTransform;
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

   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // CODE
      //////////////////////////////////////////////////////////////////////////
      class NETDEMO_EXPORT MainMenuScreen : public SimCore::GUI::SimpleScreen
      {
         public:
            typedef SimCore::GUI::SimpleScreen BaseClass;

            MainMenuScreen();

            virtual void Reset();

            virtual void Setup(SimCore::Components::HUDGroup& root, osg::MatrixTransform& effectsLayer);

            virtual bool Update(float simTimeDelta);

         protected:
            virtual ~MainMenuScreen();

         private:
            // Animation Controllers

            // Special Widgets
      };
   }
}

#endif
