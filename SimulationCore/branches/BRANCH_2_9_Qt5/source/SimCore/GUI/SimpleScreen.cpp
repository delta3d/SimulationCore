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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <SimCore/GUI/SimpleScreen.h>

#ifdef None
#undef None
#endif
#include <CEGUI/CEGUI.h>


namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // SIMPLE SCREEN CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string SimpleScreen::DEFAULT_NAME("SimCore::GUI::SimpleScreen");

      //////////////////////////////////////////////////////////////////////////
      SimpleScreen::SimpleScreen( const std::string& name, const std::string& layoutFile )
         : BaseClass(name)
         , mLayoutFile(layoutFile)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SimpleScreen::~SimpleScreen()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      SimCore::Components::HUDElement* SimpleScreen::GetRoot()
      {
         return mRoot.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Components::HUDElement* SimpleScreen::GetRoot() const
      {
         return mRoot.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& SimpleScreen::GetLayoutFileName() const
      {
         return mLayoutFile;
      }

      //////////////////////////////////////////////////////////////////////////
      void SimpleScreen::SetVisible( bool visible )
      {
         if( mRoot.valid() )
         {
            mRoot->SetVisible( visible );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool SimpleScreen::IsVisible() const
      {
         return mRoot.valid() && mRoot->IsVisible();
      }

      //////////////////////////////////////////////////////////////////////////
      void SimpleScreen::Setup( SimCore::Components::HUDGroup* root )
      {
         CEGUI::WindowManager* wm = CEGUI::WindowManager::getSingletonPtr();

         CEGUI::Window* ceguiWindow = wm->loadWindowLayout( mLayoutFile );
         mRoot = new SimCore::Components::HUDElement( *ceguiWindow );
         root->Add( mRoot.get() );
      }

   }
}
