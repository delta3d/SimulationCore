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
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <SimCore/Utilities.h>

#include <SimCore/IGExceptionEnum.h>
#include <dtABC/application.h>
#include <dtDAL/project.h>
#include <dtUtil/stringutils.h>
#include <sstream>


namespace SimCore
{
namespace Utils
{

   const std::string CONFIG_PROP_ADDITIONAL_MAP("AdditionalMap");

   ///////////////////////////////////////////////////////////////////////
   void GetAdditionalMaps(dtGame::GameManager& gm, std::vector<std::string>& toFill)
   {
      std::ostringstream oss;
      std::string addMap;
      unsigned i = 1;
      do
      {
         if (i > 1)
            toFill.push_back(addMap);

         oss << CONFIG_PROP_ADDITIONAL_MAP << i;
         addMap = gm.GetConfiguration().GetConfigPropertyValue(oss.str());
         oss.str("");
         ++i;
      }
      while (!addMap.empty());
   }


   ///////////////////////////////////////////////////////////////////////
   void LoadMaps(dtGame::GameManager& gm, std::string baseMapName)
   {
      // BaseMapName is not required, but if provided, it must exist
      if (!baseMapName.empty())
      {
         // Determine if the specified map is valid.
         typedef std::set<std::string> MapNameList;
         const MapNameList& names = dtDAL::Project::GetInstance().GetMapNames();
         if( names.find( baseMapName ) == names.end() )
         {
            std::ostringstream oss;
            oss << "Cannot connect because \"" << baseMapName << "\" is not a valid map name." << std::endl;
            throw dtUtil::Exception( oss.str(), __FUNCTION__, __LINE__ );
         }
      }


      // Get the other map names used in loading prototypes
      // and other application data.
      std::vector<std::string> mapNames;

      GetAdditionalMaps(gm, mapNames);
      if (!baseMapName.empty())
         mapNames.push_back(baseMapName);

      try
      {
         gm.ChangeMapSet(mapNames, false);
      }
      catch(const dtUtil::Exception& e)
      {
         e.LogException(dtUtil::Log::LOG_ERROR);
         throw;
      }
   }
}
}
