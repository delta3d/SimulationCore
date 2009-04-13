/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2008, Alion Science and Technology, BMH Operation
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
* Bradley Anderegg
*/
#ifndef SIMCORE_RESPONSE_H
#define SIMCORE_RESPONSE_H

#include <SimCore/Export.h>
#include <osg/Referenced>
#include <dtCore/observerptr.h>
#include <dtCore/uniqueid.h>
#include <dtUtil/enumeration.h>

#include <string>



namespace dtDAL
{
   class GameEvent;
}

namespace SimCore
{
   namespace Components
   {
      class SIMCORE_EXPORT ResponseType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(ResponseType);

         public:
            static ResponseType RESPONSE_WRONG;
            static ResponseType RESPONSE_MOVE_TO_NEXT;
            static ResponseType RESPONSE_CLOSE_UI;
            static ResponseType RESPONSE_WRONG_ACKNOWLEDGED;

         private:
            ResponseType( const std::string& name )
               : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
      };



      class SIMCORE_EXPORT Response : public osg::Referenced
      {
         public:
            Response();

            void SetResponseType(const ResponseType& pType);
            const ResponseType& GetResponseType() const;

            void SetResponseText(const std::string& pText);
            const std::string& GetResponseText() const;

            bool GetEnable() const;
            void SetEnable(bool b);

            void SetId(int i);
            const dtCore::UniqueId& GetId() const;

         protected:
            /*virtual*/ ~Response();

         private:

            const ResponseType* mResponseType;

            bool mEnabled;
            dtCore::UniqueId mId;
            std::string mResponseText;

      };
   
   }
}

#endif
