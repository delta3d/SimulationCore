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
#include <StealthGM/ControlsCameraConfigObject.h>

namespace StealthGM
{
   ControlsCameraConfigObject::ControlsCameraConfigObject()
   {

   }

   ControlsCameraConfigObject::~ControlsCameraConfigObject()
   {

   }

   void ControlsCameraConfigObject::ApplyChanges(dtGame::GameManager &gameManager)
   {
      if(!IsUpdated())
         return;

      SetIsUpdated(false);
   }
}
