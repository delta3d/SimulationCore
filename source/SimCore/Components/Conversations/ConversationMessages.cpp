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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <dtGame/messageparameter.h>
#include <SimCore/Components/Conversations/ConversationMessages.h>
#include <SimCore/Components/Conversations/Interaction.h>
#include <SimCore/Components/Conversations/Response.h>



namespace SimCore
{
   using namespace Components;

   /////////////////////////////////////////////////////////////////////////////
   // INTERACTION CHANGED MESSAGE
   /////////////////////////////////////////////////////////////////////////////
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_CONVERSATION_NAME("Conversation Name");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_INTERACTION_TEXT("Interaction Text");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_1("Response Text 1");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_2("Response Text 2");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_3("Response Text 3");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_4("Response Text 4");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_5("Response Text 5");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_6("Response Text 6");
   const dtUtil::RefString InteractionChangedMessage::PARAMETER_REPEAT_TEXT("Repeat Text");

   /////////////////////////////////////////////////////////////////////////////
   InteractionChangedMessage::InteractionChangedMessage()
      : dtGame::Message()
      , mConversationName(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_CONVERSATION_NAME, ""))
      , mInteractionText(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_INTERACTION_TEXT, ""))
      , mResponseText1(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_1, ""))
      , mResponseText2(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_2, ""))
      , mResponseText3(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_3, ""))
      , mResponseText4(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_4, ""))
      , mResponseText5(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_5, ""))
      , mResponseText6(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_RESPONSE_TEXT_6, ""))
      , mRepeatText(new dtGame::StringMessageParameter(InteractionChangedMessage::PARAMETER_REPEAT_TEXT, ""))
   {
      AddParameter(mConversationName.get());
      AddParameter(mInteractionText.get());
      AddParameter(mResponseText1.get());
      AddParameter(mResponseText2.get());
      AddParameter(mResponseText3.get());
      AddParameter(mResponseText4.get());
      AddParameter(mResponseText5.get());
      AddParameter(mResponseText6.get());
      AddParameter(mRepeatText.get());
   }

   /////////////////////////////////////////////////////////////////////////////
   InteractionChangedMessage::~InteractionChangedMessage()
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetConversationName( const std::string& name )
   {
      mConversationName->FromString( name );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetConversationName() const
   {
      return mConversationName->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetInteractionText( const std::string& text )
   {
      mInteractionText->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetInteractionText() const
   {
      return mInteractionText->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetResponseText1( const std::string& text )
   {
      mResponseText1->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetResponseText1() const
   {
      return mResponseText1->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetResponseText2( const std::string& text )
   {
      mResponseText2->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetResponseText2() const
   {
      return mResponseText2->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetResponseText3( const std::string& text )
   {
      mResponseText3->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetResponseText3() const
   {
      return mResponseText3->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetResponseText4( const std::string& text )
   {
      mResponseText4->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetResponseText4() const
   {
      return mResponseText4->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetResponseText5( const std::string& text )
   {
      mResponseText5->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetResponseText5() const
   {
      return mResponseText5->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetResponseText6( const std::string& text )
   {
      mResponseText6->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetResponseText6() const
   {
      return mResponseText6->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void InteractionChangedMessage::SetRepeatText(const std::string& text)
   {
      mRepeatText->FromString(text);
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& InteractionChangedMessage::GetRepeatText() const
   {
      return mRepeatText->GetValue();
   }



   /////////////////////////////////////////////////////////////////////////////
   // CONVERSATION RESPONSE MESSAGE
   /////////////////////////////////////////////////////////////////////////////
   const dtUtil::RefString ConversationResponseMessage::PARAMETER_CONVERSATION_NAME("Conversation Name");
   const dtUtil::RefString ConversationResponseMessage::PARAMETER_INTERACTION_TEXT("Interaction Text");
   const dtUtil::RefString ConversationResponseMessage::PARAMETER_RESPONSE_TEXT("Response Text");
   const dtUtil::RefString ConversationResponseMessage::PARAMETER_RESPONSE_TYPE("Response Type");

   /////////////////////////////////////////////////////////////////////////////
   ConversationResponseMessage::ConversationResponseMessage()
      : dtGame::Message()
      , mConversationName(new dtGame::StringMessageParameter(ConversationResponseMessage::PARAMETER_CONVERSATION_NAME, ""))
      , mInteractionText(new dtGame::StringMessageParameter(ConversationResponseMessage::PARAMETER_INTERACTION_TEXT, ""))
      , mResponseText(new dtGame::StringMessageParameter(ConversationResponseMessage::PARAMETER_RESPONSE_TEXT, ""))
      , mResponseType(new dtGame::EnumMessageParameter(ConversationResponseMessage::PARAMETER_RESPONSE_TYPE,
                                                    ResponseType::RESPONSE_MOVE_TO_NEXT.GetName()))
   {
      AddParameter(mConversationName.get());
      AddParameter(mInteractionText.get());
      AddParameter(mResponseText.get());
      AddParameter(mResponseType.get());
   }

   /////////////////////////////////////////////////////////////////////////////
   ConversationResponseMessage::~ConversationResponseMessage()
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void ConversationResponseMessage::SetResponseText( const std::string& response )
   {
      mResponseText->FromString(response);
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& ConversationResponseMessage::GetResponseText() const
   {
      return mResponseText->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void ConversationResponseMessage::SetResponseType(const ResponseType& responseType )
   {
      mResponseType->SetValue(responseType.GetName());
   }

   /////////////////////////////////////////////////////////////////////////////
   const ResponseType& ConversationResponseMessage::GetResponseType() const
   {
      return *ResponseType::GetValueForName(mResponseType->GetValue());
   }

   /////////////////////////////////////////////////////////////////////////////
   void ConversationResponseMessage::SetInteractionText( const std::string& text )
   {
      mInteractionText->FromString( text );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& ConversationResponseMessage::GetInteractionText() const
   {
      return mInteractionText->GetValue();
   }

   /////////////////////////////////////////////////////////////////////////////
   void ConversationResponseMessage::SetConversationName( const std::string& name )
   {
      mConversationName->FromString( name );
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& ConversationResponseMessage::GetConversationName() const
   {
      return mConversationName->GetValue();
   }

}
