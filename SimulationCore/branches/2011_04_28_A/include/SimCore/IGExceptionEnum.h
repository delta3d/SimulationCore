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
 * @author David Guthrie
 */

#include <SimCore/Export.h>
#include <dtUtil/enumeration.h>
#include <dtUtil/exception.h>

 namespace SimCore
 {
   /**
    * @brief class holding types of exceptions used in the Image Generator namespace.
    */
   class SIMCORE_EXPORT IGExceptionEnum : public dtUtil::Enumeration
   {
      DECLARE_ENUM(IGExceptionEnum);
      public:

         static IGExceptionEnum INVALID_CONNECTION_DATA;

      protected:
         IGExceptionEnum(const std::string& name);
   };

   class SIMCORE_EXPORT IGException : public dtUtil::Exception
   {
   public:
      IGException(const std::string& message, const std::string& filename, unsigned int linenum);
      virtual ~IGException() {};
   };
}
