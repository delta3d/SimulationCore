/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
 * @author Chris Rodgers
 */

#ifndef PLAYER_MOTION_MODEL_H
#define PLAYER_MOTION_MODEL_H

#include <SimCore/Export.h>
#include <dtCore/fpsmotionmodel.h>
#include <dtCore/refptr.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   //////////////////////////////////////////////////////////////////////////
   // PLAYER MOTION MODEL
   //////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT PlayerMotionModel : public dtCore::FPSMotionModel
   {
      DECLARE_MANAGEMENT_LAYER(PlayerMotionModel)

      public:

         // Constructor.
         // @param keyboard the keyboard instance, or NULL to avoid creating default input mappings
         // @param mouse the mouse instance, or NULL to avoid creating default input mappings
         PlayerMotionModel(dtCore::Keyboard* keyboard = NULL,
                           dtCore::Mouse* mouse = NULL);

         // Destructor
         virtual ~PlayerMotionModel();

         // Message handler callback.
         // @param data the message data
         virtual void OnSystem(const dtUtil::RefString& phase, double deltaSim, double deltaReal)
;

      private:
   };
}

#endif
