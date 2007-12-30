/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
 */
#ifndef _CONTROLS_CAMERA_CONFIG_OBJECT_H_
#define _CONTROLS_CAMERA_CONFIG_OBJECT_H_

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

namespace StealthGM
{
   class STEALTH_GAME_EXPORT ControlsCameraConfigObject : public ConfigurationObjectInterface
   {
      public:

         /// Constructor
         ControlsCameraConfigObject();

         /**
          * Applys the changes into the game manager
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

      protected:

         /// Destructor
         virtual ~ControlsCameraConfigObject();
   };
}

#endif
