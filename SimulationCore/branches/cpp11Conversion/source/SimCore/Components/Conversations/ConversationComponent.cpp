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
#include <dtGame/basemessages.h>
#include <dtGame/messagetype.h>
#include <dtGame/messagefactory.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/fileutils.h>
#include <dtDAL/project.h>
#include <SimCore/Messages.h>
#include <SimCore/MessageType.h>
#include <SimCore/Components/Conversations/ConversationComponent.h>
#include <SimCore/Components/Conversations/ConversationMessages.h>



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string ConversationComponent::DEFAULT_NAME = "ConversationComponent";

      //////////////////////////////////////////////////////////////////////////
      ConversationComponent::ConversationComponent(const std::string &name)
         : dtGame::GMComponent(name)
      {

      }

      //////////////////////////////////////////////////////////////////////////
      ConversationComponent::~ConversationComponent()
      {
         ClearData();
      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::ClearData()
      {
         mCurrentConversation = nullptr;
         mConversations.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::OnAddedToGM()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::OnRemovedFromGM()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      const Conversation* ConversationComponent::GetCurrentConversation()
      {
         //we are adding special code here in case something goes wrong
         //to ensure we always have a valid conversation
         if(!mCurrentConversation.valid() && !mConversations.empty())
         {
            mCurrentConversation = (*mConversations.begin()).second.get();
            LOG_ERROR("Restarting to first conversation because the current conversation was nullptr");
         }

         return mCurrentConversation.get();
      }
     

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::ProcessMessage(const dtGame::Message& message)
      {
         if(message.GetMessageType() == dtGame::MessageType::TICK_LOCAL)
         {
         }
         else if(message.GetMessageType() == dtGame::MessageType::INFO_GAME_EVENT)
         {
            const dtGame::GameEventMessage& eventMessage
               = static_cast<const dtGame::GameEventMessage&>(message);

            //TODO- Investigate this const cast, even with a const iterator it still doesnt work without it
            dtDAL::GameEvent* ge = const_cast<dtDAL::GameEvent*>(eventMessage.GetGameEvent());

            //first send to the conversations
            SendGameEventToConversations(ge);

            bool interactionChanged = false;
            //now check to see if we need to switch interactions
            ConversationMap::const_iterator iter = mConversations.find(ge);
            if(iter != mConversations.end())
            {
               if((*iter).second.valid())
               {
                  interactionChanged = true;
                  mCurrentConversation = (*iter).second.get();
                  mCurrentConversation->ResetConversation();
               }
            }

            //a special check to see if the current interaction was disabled by the game event
            //in which case we need to increment our current conversations game event
            //NOTE: if we do not do this it will be a problem if we disable our current interaction between
            //  the conversation becoming current and the user opening the phone ui         
            if(mCurrentConversation.valid() && mCurrentConversation->GetCurrentInteraction() != nullptr)
            {
               if(!mCurrentConversation->GetCurrentInteraction()->GetEnable())
               {
                  mCurrentConversation->IncrementInteraction();
                  interactionChanged = true;
               }
            }

            //if we changed our interaction send a message
            if(interactionChanged && mCurrentConversation.valid() && mCurrentConversation->GetCurrentInteraction() != nullptr)
            {
               SendInteractionChangedMessage(mCurrentConversation->GetCurrentInteraction());
            }

         }
      }

      //////////////////////////////////////////////////////////////////////////
      bool ConversationComponent::HandleResponse(const Response* r)
      {
         bool success = false;

         if(r != nullptr && mCurrentConversation.valid() && mCurrentConversation->GetCurrentInteraction() != nullptr)
         {
            if(mCurrentConversation->GetCurrentInteraction()->HasResponse(*r))
            {
               success = true;

               //the response seems valid go ahead and send the response message
               SendConversationResponseMessage(r);

               //fire all response game events
               Conversation::ResponseToEventMap::iterator iter = mCurrentConversation->GetResponseEvents().lower_bound(r);
               Conversation::ResponseToEventMap::iterator endIter = mCurrentConversation->GetResponseEvents().upper_bound(r);
               
               for(; iter != endIter; ++iter)
               {
                  SendGameEvent(iter->second.get());
               }

               //we might need to return bool here if the interaction changed
               //for now we are returning either a new interaction or nullptr
               Interaction* i = mCurrentConversation->HandleResponse(r);
               if(i != nullptr)
               {
                  SendInteractionChangedMessage(i);
               }
            }
            else
            {
               LOG_ERROR("Invalid response for current conversation.");
            }
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::SendInteractionChangedMessage(const Interaction* i)
      {
         dtGame::GameManager& mgr = *GetGameManager();
         std::shared_ptr<dtGame::Message> msg = 
            mgr.GetMessageFactory().CreateMessage(MessageType::INTERACTION_CHANGED);

         InteractionChangedMessage& gem = static_cast<InteractionChangedMessage&>(*msg);
         
         //in the context that this is called above we do not need to nullptr check mCurrentConversation
         gem.SetConversationName(mCurrentConversation->GetName());
         gem.SetInteractionText(i->GetInteractionText());
         
         const Interaction::ResponseArray& ra = i->GetResponses();

         if(ra.size() > 0)
         {
            gem.SetResponseText1(ra[0].first->GetResponseText());
         }
         if(ra.size() > 1)
         {
            gem.SetResponseText2(ra[1].first->GetResponseText());
         }
         if(ra.size() > 2)
         {
            gem.SetResponseText3(ra[2].first->GetResponseText());
         }
         if(ra.size() > 3)
         {
            gem.SetResponseText4(ra[3].first->GetResponseText());
         }
         if(ra.size() > 4)
         {
            gem.SetResponseText5(ra[4].first->GetResponseText());
         }
         if(ra.size() > 5)
         {
            gem.SetResponseText6(ra[5].first->GetResponseText());
         }

         mgr.SendMessage(gem);

      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::SendConversationResponseMessage(const Response* r )
      {
         dtGame::GameManager& mgr = *GetGameManager();
         std::shared_ptr<dtGame::Message> msg = 
            mgr.GetMessageFactory().CreateMessage(MessageType::CONVERSATION_RESPONSE);

         ConversationResponseMessage& gem = static_cast<ConversationResponseMessage&>(*msg);
         gem.SetResponseText(r->GetResponseText());
         gem.SetResponseType(r->GetResponseType());
         mgr.SendMessage(gem);
      }

      //////////////////////////////////////////////////////////////////////////
      //TODO: THIS CODE IS ALSO IN WORLDITEMACTOR, AND THE INPUT COMPONENT, PERHAPS MOVE TO UTILS.H
      void ConversationComponent::SendGameEvent(const dtDAL::GameEvent* pEvent )
      {
         dtGame::GameManager& mgr = *GetGameManager();
         std::shared_ptr<dtGame::Message> msg = 
            mgr.GetMessageFactory().CreateMessage(dtGame::MessageType::INFO_GAME_EVENT);

         dtGame::GameEventMessage& gem = static_cast<dtGame::GameEventMessage&>(*msg);
         //why can't I send a const game event????
         gem.SetGameEvent(const_cast<dtDAL::GameEvent&>(*pEvent));
         mgr.SendMessage(gem);
      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::LoadConversationConfig( const std::string& filename )
      {
         //before we load the conversations seed the random number generator
         srand(time(nullptr));

         //attempt to load conversation xml
         std::string fileWithPath = dtDAL::Project::GetInstance().GetContext() + dtUtil::FileUtils::PATH_SEPARATOR + "Conversations" + dtUtil::FileUtils::PATH_SEPARATOR + filename;
         if(dtUtil::FileUtils::GetInstance().FileExists(fileWithPath))
         {
            XMLHandler handler(this);
            dtUtil::XercesParser parser;
            parser.Parse(fileWithPath, handler);
         }
         else
         {
            LOG_ERROR("Unable to find file '" + fileWithPath + "'.")         
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::RegisterConversation( dtDAL::GameEvent* ge, Conversation* conv )
      {
         if(mConversations.find(ge) != mConversations.end())
         {
            LOG_ERROR("Unable to register conversation '" + conv->GetName() + "' with GameEvent '" + ge->GetName() 
               + "' because it is already registered with conversation '" + mConversations.find(ge)->second->GetName() + "'");
         }
         else
         {
            mConversations.insert(std::make_pair(ge, conv));
         }
      }

      //////////////////////////////////////////////////////////////////////////
      struct funcSendGameEvents
      {
            funcSendGameEvents(dtDAL::GameEvent* ge): mEvent(ge){}

            template <class T>
            void operator()(T& t)
            {
               t.second->HandleGameEvent(mEvent);
            }

         private:
            dtDAL::GameEvent* mEvent;
      };

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::SendGameEventToConversations( dtDAL::GameEvent* ge )
      {
         std::for_each(mConversations.begin(), mConversations.end(), funcSendGameEvents(ge));
      }



      //////////////////////////////////////////////////////////////////////////
      // XML HANDLER CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string CONVERSATION_LIST_ELEMENT("ConversationList");
      const std::string CONVERSATION_ELEMENT("Conversation");
      const std::string FILE_ELEMENT("File");

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::XMLHandler::startElement(const XMLCh* const uri,
         const XMLCh* const localname,
         const XMLCh* const qname,
         const XERCES_CPP_NAMESPACE_QUALIFIER Attributes& attrs)
      {
         char* elementName = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(localname);
         std::string ename(elementName);
         XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release( &elementName );

         dtUtil::AttributeSearch attrsearch;
         dtUtil::AttributeSearch::ResultMap results = attrsearch( attrs );

         dtUtil::AttributeSearch::ResultMap::iterator typeiter = results.find(FILE_ELEMENT);

         if (ename == CONVERSATION_LIST_ELEMENT)
         {
            LOG_INFO("Parsing Conversation Config.");
         }
         else if(ename == CONVERSATION_ELEMENT)
         {
            if( typeiter != results.end() )
            {
               std::shared_ptr<Conversation> conv = new Conversation(Conversation::
                  RegisterConversationFunctor(mManager, &ConversationComponent::RegisterConversation));
               
               if(!conv->LoadConversation((*typeiter).second))
               {
                  LOG_ERROR("Error loading conversation file '" + (*typeiter).second + "'.");
               }
            }
            else
            {
               LOG_ERROR("Transition file not structured properly at Event, "+ename+", requires Type attributes.")
            }
         }

      }

      //////////////////////////////////////////////////////////////////////////
      void ConversationComponent::XMLHandler::endElement(const XMLCh* const uri,
         const XMLCh* const localname,
         const XMLCh* const qname)
      {  
         std::string elementStr = dtUtil::XMLStringConverter(localname).ToString();
         if(elementStr == CONVERSATION_LIST_ELEMENT)
         {
            LOG_INFO("Done parsing Conversation Config.");
         }
      }
   }
}
