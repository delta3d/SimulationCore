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

#ifndef NETDEMO_MESSAGE_TYPES_H
#define NETDEMO_MESSAGE_TYPES_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtGame/messagetype.h>
#include "DemoExport.h"



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtGame
{
   class MessageFactory;
}



namespace NetDemo
{
   /////////////////////////////////////////////////////////////////////////////
   // CODE
   /////////////////////////////////////////////////////////////////////////////
   class NETDEMO_EXPORT MessageType : public dtGame::MessageType
   {
      DECLARE_ENUM(MessageType);
      public:
         typedef dtGame::MessageType BaseClass;

         static const int NETDEMO_MESSAGE_TYPE_ID = 2048;

         static const MessageType OPTION_NEXT;
         static const MessageType OPTION_PREV;
         static const MessageType OPTION_SELECT;

         static void RegisterMessageTypes(dtGame::MessageFactory& factory);

      protected:
         MessageType(
            const std::string &name,
            const std::string &category,
            const std::string &description,
            const unsigned short messageId);

         virtual ~MessageType();
   };

}

#endif
