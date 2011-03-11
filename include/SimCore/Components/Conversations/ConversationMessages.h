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
 * Bradley Anderegg, Curtiss Murphy
 */

#ifndef SIMCORE_CONVERSATION_MESSAGES_H
#define SIMCORE_CONVERSATION_MESSAGES_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtGame/message.h>
#include <SimCore/Export.h>



/////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
/////////////////////////////////////////////////////////////////////////////
namespace SimCore
{
   namespace Components
   {
      class Interaction;
      class ResponseType;
   }
}



namespace SimCore
{
   using namespace Components;

   /////////////////////////////////////////////////////////////////////////////
   // INTERACTION CHANGED MESSAGE
   /////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT InteractionChangedMessage : public dtGame::Message
   {
      public:
         static const dtUtil::RefString PARAMETER_CONVERSATION_NAME;
         static const dtUtil::RefString PARAMETER_INTERACTION_TEXT;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT_1;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT_2;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT_3;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT_4;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT_5;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT_6;
         static const dtUtil::RefString PARAMETER_REPEAT_TEXT;

         /// Constructor
         InteractionChangedMessage();

         void SetConversationName( const std::string& name );
         const std::string& GetConversationName() const;

         void SetInteractionText( const std::string& text );
         const std::string& GetInteractionText() const;

         void SetResponseText1( const std::string& text );
         const std::string& GetResponseText1() const;

         void SetResponseText2( const std::string& text );
         const std::string& GetResponseText2() const;

         void SetResponseText3( const std::string& text );
         const std::string& GetResponseText3() const;

         void SetResponseText4( const std::string& text );
         const std::string& GetResponseText4() const;

         void SetResponseText5( const std::string& text );
         const std::string& GetResponseText5() const;

         void SetResponseText6( const std::string& text );
         const std::string& GetResponseText6() const;

         void SetRepeatText( const std::string& text );
         const std::string& GetRepeatText() const;

      protected:

         /// Destructor
         virtual ~InteractionChangedMessage();

      private:
         dtCore::RefPtr<dtGame::StringMessageParameter> mConversationName;
         dtCore::RefPtr<dtGame::StringMessageParameter> mInteractionText;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText1;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText2;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText3;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText4;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText5;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText6;
         dtCore::RefPtr<dtGame::StringMessageParameter> mRepeatText; 

   };



   /////////////////////////////////////////////////////////////////////////////
   // CONVERSATION RESPONSE MESSAGE
   /////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT ConversationResponseMessage : public dtGame::Message
   {
      public:

         static const dtUtil::RefString PARAMETER_CONVERSATION_NAME;
         static const dtUtil::RefString PARAMETER_INTERACTION_TEXT;
         static const dtUtil::RefString PARAMETER_RESPONSE_TEXT;
         static const dtUtil::RefString PARAMETER_RESPONSE_TYPE;

         /// Constructor
         ConversationResponseMessage();

         void SetResponseText(const std::string& response);
         const std::string& GetResponseText() const;

         void SetResponseType(const ResponseType& responseType);
         const ResponseType& GetResponseType() const;

         void SetInteractionText( const std::string& text );
         const std::string& GetInteractionText() const;

         void SetConversationName( const std::string& name );
         const std::string& GetConversationName() const;

      protected:

         /// Destructor
         virtual ~ConversationResponseMessage();

      private:
         dtCore::RefPtr<dtGame::StringMessageParameter> mConversationName;
         dtCore::RefPtr<dtGame::StringMessageParameter> mInteractionText;
         dtCore::RefPtr<dtGame::StringMessageParameter> mResponseText;
         dtCore::RefPtr<dtGame::EnumMessageParameter> mResponseType;

   };

}

#endif
