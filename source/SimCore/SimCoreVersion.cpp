/* -*-c++-*-
* Simulation Core
* Copyright 2014, Caper Holdings LLC
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
*/

#include <SimCore/Export.h>
#include <SimCoreGeneratedVersion.h>
#include <string>

SIMCORE_EXPORT const char* GetSimCoreRevision()
{
   return SIMCORE_SVN_REVISION;
}

namespace SimCore
{
   SIMCORE_EXPORT std::string GetSimCoreRevision()
   {
      return SIMCORE_SVN_REVISION;
   }

   SIMCORE_EXPORT std::string GetSimCoreBuildDate()
   {
      return SIMCORE_BUILD_DATE;
   }

   SIMCORE_EXPORT std::string GetSimCoreCommitDate()
   {
      return SIMCORE_SVN_DATE;
   }
}
