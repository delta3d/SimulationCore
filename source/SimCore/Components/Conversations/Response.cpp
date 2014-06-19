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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/Components/Conversations/Response.h>
#include <dtCore/gameevent.h>
#include <dtUtil/stringutils.h>



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // ENUMERATION CODE
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(ResponseType);
      ResponseType ResponseType::RESPONSE_WRONG("WRONG");
      ResponseType ResponseType::RESPONSE_MOVE_TO_NEXT("MOVE_TO_NEXT");
      ResponseType ResponseType::RESPONSE_CLOSE_UI("CLOSEUI");
      ResponseType ResponseType::RESPONSE_WRONG_ACKNOWLEDGED("WRONG_ACKNOWLEDGED");



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      Response::Response()
         : mResponseType(&ResponseType::RESPONSE_MOVE_TO_NEXT)
         , mEnabled(true)
         , mId()
         , mResponseText()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      Response::~Response()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void Response::SetResponseType( const ResponseType& pType )
      {
         mResponseType = &pType;
      }

      //////////////////////////////////////////////////////////////////////////
      const ResponseType& Response::GetResponseType() const
      {
         return *mResponseType;
      }

      //////////////////////////////////////////////////////////////////////////
      void Response::SetResponseText( const std::string& pText )
      {
         mResponseText = pText;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Response::GetResponseText() const
      {
         return mResponseText;
      }

      //////////////////////////////////////////////////////////////////////////
      bool Response::GetEnable() const
      {
         return mEnabled;
      }

      //////////////////////////////////////////////////////////////////////////
      void Response::SetEnable( bool b )
      {
         mEnabled = b;
      }

      //////////////////////////////////////////////////////////////////////////
      void Response::SetId(int i)
      {
         mId = dtCore::UniqueId(dtUtil::ToString(i));
      }

      //////////////////////////////////////////////////////////////////////////
      const dtCore::UniqueId& Response::GetId() const
      {
         return mId;
      }

   }
}
