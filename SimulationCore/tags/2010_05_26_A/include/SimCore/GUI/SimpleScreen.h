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

#ifndef SIMCORE_SIMPLE_SCREEN_H
#define SIMCORE_SIMPLE_SCREEN_H



////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtCore/refptr.h>
#include <SimCore/Export.h>
#include <SimCore/GUI/Screen.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Components
   {
      class HUDElement;
      class HUDGroup;
   }
}

namespace SimCore
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // SIMPLE SCREEN CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT SimpleScreen : public Screen
      {
         public:
            typedef Screen BaseClass;

            static const std::string DEFAULT_NAME;

            SimpleScreen( const std::string& name, const std::string& layoutFile );

            SimCore::Components::HUDElement* GetRoot();
            const SimCore::Components::HUDElement* GetRoot() const;

            const std::string& GetLayoutFileName() const;

            /**
            * Set the visibility of the screen.
            * @param visible TRUE if the screen should be made visible.
            */
            virtual void SetVisible( bool visible );
            virtual bool IsVisible() const;

            /**
             * Perform the initial setup of the object.
             * @param root Delta object that acts as the root attach point
             *        for the screen drawables. It is up to the implementation
             *        to cast the root and handle attaching elements to it.
             */
            virtual void Setup( SimCore::Components::HUDGroup* root = NULL );

         protected:
            virtual ~SimpleScreen();

         private:
            std::string mLayoutFile;

            // GUI Elements
            dtCore::RefPtr<SimCore::Components::HUDElement> mRoot;
      };
   }
}

#endif
