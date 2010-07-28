/* -*-c++-*-
* Simulation Core
* Copyright 2009, Alion Science and Technology
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
* @author Curtiss Murphy
* 
*/

#ifndef _SIMCORE_UTILITIES_H_
#define _SIMCORE_UTILITIES_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>

#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>


namespace SimCore
{
namespace Utils
{

   extern SIMCORE_EXPORT const std::string CONFIG_PROP_ADDITIONAL_MAP;


   ////////////////////////////////////////////////////////////////////////////////
   // This file provides general utility methods that are useful in various places 
   // but don't have a particular home.
   ////////////////////////////////////////////////////////////////////////////////


   // Looks up the additional maps from the config file. 
   //    Ex: <Property Name="AdditionalMap1">DriverPrototypes</Property> 
   void SIMCORE_EXPORT GetAdditionalMaps(dtGame::GameManager& gm, std::vector<std::string>& toFill);

   // Attempts to load the maps - using passed in map PLUS any additional maps in config.
   void SIMCORE_EXPORT LoadMaps(dtGame::GameManager& gm, std::string baseMapName);

   /// A simple utility to check the DeveloperMode config parameter.
   bool SIMCORE_EXPORT IsDevModeOn(dtGame::GameManager& gm); 

   /// Calls the GM CreateActorFromPrototype() method, but throws an exception if there's an error. 
   template <typename T>
   void CreateActorFromPrototypeWithException(dtGame::GameManager& gm, const std::string& prototypeName, 
      dtCore::RefPtr<T>& proxy, const std::string& errorMsg = "")
   {
      gm.CreateActorFromPrototype(prototypeName, proxy);
      if (proxy == NULL)
      {
         std::string errorText = "Failed to create actor from prototype named [" + prototypeName + "]. " + errorMsg;
         throw dtGame::InvalidParameterException(errorText, __FILE__, __LINE__);
      }
   }

}
}

#endif
