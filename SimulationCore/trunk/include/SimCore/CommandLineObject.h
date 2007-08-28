/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen
*/

#ifndef _COMMAND_LINE_OBJECT_
#define _COMMAND_LINE_OBJECT_

// project includes needed
#include <SimCore/Export.h>

#include <dtCore/refptr.h>

#include <osg/Referenced>

#include <vector>
#include <string>

namespace dtDAL
{
   class NamedParameter;
}

namespace SimCore
{
   ///////////////////////////////////////////////////////
   //    The Object
   ///////////////////////////////////////////////////////
   class SIMCORE_EXPORT CommandLineObject : public osg::Referenced
   {
      public:
         /// Constructor
         CommandLineObject();
       
         // clear out the parameter list
         virtual void ClearParametersList();

         // i dont think you would ever remove a single parameter, or wouldnt
         // have the need to, without just clearing the whole paramter list
         // so no removeparam function, just add
         virtual void AddParameter(dtDAL::NamedParameter* namedParameter);

         // i dont think you would ever remove a single parameter, or wouldnt
         // have the need to, without just clearing the whole paramter list
         // so no removeparam function, just add
         virtual const dtDAL::NamedParameter* GetParameter(const std::string& checkValue) const;

      protected:
         /// Destructor
         virtual ~CommandLineObject();

      private:
         std::vector<dtCore::RefPtr<dtDAL::NamedParameter> > mParametersVec;
   };
} // namespace
#endif
