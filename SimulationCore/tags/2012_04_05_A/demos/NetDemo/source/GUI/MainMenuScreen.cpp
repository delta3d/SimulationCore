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
#include <osg/MatrixTransform>

#include <SimCore/Components/BaseHUDElements.h>
#include "GUI/MainMenuScreen.h"

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
      MainMenuScreen::MainMenuScreen()
         : BaseClass("MainMenuScreen","CEGUI/layouts/NetDemo/MainMenu.layout")
      {
      }

      //////////////////////////////////////////////////////////////////////////
      MainMenuScreen::~MainMenuScreen()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void MainMenuScreen::Reset()
      {
         BaseClass::Reset();

         // TODO:
         // Reset other variables.
      }

      //////////////////////////////////////////////////////////////////////////
      void MainMenuScreen::Setup(SimCore::Components::HUDGroup& root, osg::MatrixTransform& effectsLayer)
      {
         BaseClass::Setup(&root);

         //CEGUI::WindowManager& wm = *CEGUI::WindowManager::getSingletonPtr();

         // TODO:
      }

      //////////////////////////////////////////////////////////////////////////
      bool MainMenuScreen::Update(float simTimeDelta)
      {
         return BaseClass::Update(simTimeDelta);

         // TODO:
      }

   }
}
