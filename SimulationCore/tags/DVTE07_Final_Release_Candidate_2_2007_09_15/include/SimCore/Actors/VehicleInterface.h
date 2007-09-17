/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

#ifndef _VEHICLE_INTERFACE_H_
#define _VEHICLE_INTERFACE_H_

#include <SimCore/Export.h>

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT VehicleInterface
      {
         public:
            virtual float GetMPH() = 0;
            virtual void SetHasDriver( bool hasDriver ) {}
            virtual bool GetHasDriver() { return false; }
            
         protected:
            //added to get rid of warning
            virtual ~VehicleInterface() {}
      };
   }
}

#endif
