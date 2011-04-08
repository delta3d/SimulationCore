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
#include <dtDAL/gameevent.h>
#include <dtDAL/gameeventmanager.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/mathdefines.h>
#include <dtUtil/log.h>
#include <dtDAL/project.h>
#include <SimCore/Components/Conversations/Conversation.h>
#include <SimCore/Components/Conversations/Response.h>


namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      Conversation::Conversation(RegisterConversationFunctor regFunc)
         : mName()
         , mCharacterName()
         , mRegistrationFunctor(regFunc)
         , mCurrentInteraction(NULL)
         , mCurrentInteractionIter()
         , mInteractions()
      {
         mCurrentInteractionIter = mInteractions.end();
      }

      //////////////////////////////////////////////////////////////////////////
      Conversation::~Conversation()
      {
         ClearData();
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::ClearData()
      {
         mName.clear();
         mCurrentInteraction = NULL;
         mInteractions.clear();
         mCurrentInteractionIter = mInteractions.end();
         mCommands.clear();
         mResponseEvents.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::IncrementInteraction()
      {
         mCurrentInteraction = NULL;

         if(mCurrentInteractionIter != mInteractions.end())
         {
            ++mCurrentInteractionIter;
            if(mCurrentInteractionIter != mInteractions.end())
            {
               //if this interaction is not enabled, skip it
               if(!(*mCurrentInteractionIter)->GetEnable())
               {
                  IncrementInteraction();
               }
               else
               {
                  mCurrentInteraction = (*mCurrentInteractionIter).get();
               }
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const Interaction* Conversation::GetCurrentInteraction() const
      {
         return mCurrentInteraction.get();
      }

      //////////////////////////////////////////////////////////////////////////
      Interaction* Conversation::HandleResponse( const Response* r )
      {
         if(r != NULL && mCurrentInteraction.valid())
         {
            //ensure the response is valid for our current interaction
            if(mCurrentInteraction->HasResponse(*r))
            {
               const ResponseType& rt = r->GetResponseType();

               if(rt == ResponseType::RESPONSE_WRONG)
               {
                  Interaction* i = mCurrentInteraction->GetBranch(*r);
                  if(i != NULL)
                  {
                     mCurrentInteraction = i;
                  }
               }
               else if(rt == ResponseType::RESPONSE_MOVE_TO_NEXT)
               {
                  IncrementInteraction();
               }
               else if(rt == ResponseType::RESPONSE_CLOSE_UI)
               {
                  //don't think we need to do anything here
               }
               else if(rt == ResponseType::RESPONSE_WRONG_ACKNOWLEDGED)
               {
                  //just go back to where we came from
                  if(mCurrentInteractionIter != mInteractions.end())
                  {
                     mCurrentInteraction = (*mCurrentInteractionIter).get();
                  }
               }
            }
            else
            {
               LOG_ERROR("Response to handle does not belong to the current interaction.");
            }

         }

         return mCurrentInteraction.get();
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Conversation::GetName() const
      {
         return mName;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Conversation::GetCharacterName() const
      {
         return mCharacterName;
      }


      //////////////////////////////////////////////////////////////////////////
      bool Conversation::LoadConversation( const std::string& filename )
      {
         bool success = false;

         ClearData();

         //attempt to load conversation xml
         std::string fileWithPath = dtDAL::Project::GetInstance().GetContext() + dtUtil::FileUtils::PATH_SEPARATOR + "Conversations" + dtUtil::FileUtils::PATH_SEPARATOR + filename;
         if(dtUtil::FileUtils::GetInstance().FileExists(fileWithPath))
         {
            XMLHandler handler(this);
            dtUtil::XercesParser parser;
            success = parser.Parse(fileWithPath, handler);

            if(success && !mInteractions.empty())
            {
               mCurrentInteractionIter = mInteractions.begin();
               mCurrentInteraction = mInteractions.front().get();
            }
         }
         else
         {
            LOG_ERROR("Unable to find file '" + fileWithPath + "'.")
         }
         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      dtDAL::GameEvent* Conversation::LookupGameEvent(const std::string& str) const
      {
         dtDAL::GameEventManager& eventManager = dtDAL::GameEventManager::GetInstance();

         dtDAL::GameEvent* ge = eventManager.FindEvent(str);
         if(ge == NULL && str != "")
         {
            LOG_WARNING("Error loading conversation file, did not find GameEvent of name '" + str + "' in current map.")
         }
         return ge;
      }

      //////////////////////////////////////////////////////////////////////////
      struct funcExecuteCommands
      {
         template <typename _Type>
         void operator()(_Type& pCommand)
         {
            pCommand.second->operator()();
         }
      };

      //////////////////////////////////////////////////////////////////////////
      void Conversation::HandleGameEvent( dtDAL::GameEvent* ge )
      {
         std::for_each(mCommands.lower_bound(ge), mCommands.upper_bound(ge), funcExecuteCommands());
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::AddCommand(dtDAL::GameEvent* ge, Command<void>* com)
      {
         mCommands.insert(std::make_pair(ge, com));
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::AddResponseEvent(Response* r, dtDAL::GameEvent* ge)
      {
         mResponseEvents.insert(std::make_pair(r, ge));
      }

      //////////////////////////////////////////////////////////////////////////
      Conversation::ResponseToEventMap& Conversation::GetResponseEvents()
      {
         return mResponseEvents;
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::ResetConversation()
      {
         if(!mInteractions.empty())
         {
            mCurrentInteractionIter = mInteractions.begin();
            mCurrentInteraction = mInteractions.front().get();
         }
      }


      //////////////////////////////////////////////////////////////////////////
      //XML LOADING
      //////////////////////////////////////////////////////////////////////////
      const std::string CHARACTER_ELEMENT("Character");
      const std::string CONVERSATION_ELEMENT("Conversation");
      const std::string NAME_ELEMENT("Name");
      const std::string GAME_EVENT_ELEMENT("Event");
      const std::string TEXT_ELEMENT("Text");
      const std::string RAND_TEXT_ELEMENT("RandText");
      const std::string RESPONSE_ELEMENT("Response");
      const std::string INTERACTION_ELEMENT("Interaction");
      const std::string SOUND_FILE_ELEMENT("Audio");
      const std::string RANDOMIZE_ANSWERS_ELEMENT("RandomOrder");
      const std::string TYPE_ELEMENT("Type");
      const std::string DEFAULT_CLOSEUI_ELEMENT("DefaultCloseUIText");
      const std::string WRONG_FEEDBACK_ELEMENT("Wrong_Feedback");
      const std::string FEEDBACK_ELEMENT("Feedback");
      const std::string PLAYER_RESPONSE_ELEMENT("PlayerResponse");
      const std::string ENABLE_EVENT_ELEMENT("EnableEvent");
      const std::string DISABLE_EVENT_ELEMENT("DisableEvent");
      const std::string ENABLE_FLAG("Enabled");
      const std::string FORCE_CLOSE_RESPONSE_LAST_FLAG("ForceCloseResponseLast");
      const std::string REPEAT_ELEMENT("Repeat"); // Used at interaction and wrong_feedback level.

      //////////////////////////////////////////////////////////////////////////
      void Conversation::XMLHandler::startElement(const XMLCh* const uri,
         const XMLCh* const localname,
         const XMLCh* const qname,
         const XERCES_CPP_NAMESPACE_QUALIFIER Attributes& attrs)
      {
         dtUtil::XMLStringConverter elementName(localname);
         std::string elementStr = elementName.ToString();

         dtUtil::AttributeSearch attrsearch;
         dtUtil::AttributeSearch::ResultMap results = attrsearch( attrs );

         dtUtil::AttributeSearch::ResultMap::iterator typeiter = results.find(TYPE_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator textiter = results.find(TEXT_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator eventiter = results.find(GAME_EVENT_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator randomiter = results.find(RANDOMIZE_ANSWERS_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator audioiter = results.find(SOUND_FILE_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator feedbackiter = results.find(FEEDBACK_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator playerresponseiter = results.find(PLAYER_RESPONSE_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator enableeventiter = results.find(ENABLE_EVENT_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator disableeventiter = results.find(DISABLE_EVENT_ELEMENT);
         dtUtil::AttributeSearch::ResultMap::iterator enableflagiter = results.find(ENABLE_FLAG);
         dtUtil::AttributeSearch::ResultMap::iterator forcecloselastiter = results.find(FORCE_CLOSE_RESPONSE_LAST_FLAG);
         dtUtil::AttributeSearch::ResultMap::iterator repeatiter = results.find(REPEAT_ELEMENT);

         if(elementStr == CONVERSATION_ELEMENT)
         {
            mInConversation = true;
            mInInteraction = false;
            mInResponse = false;
            mDefaultCloseUI.clear();
         }
         else if(elementStr == INTERACTION_ELEMENT)
         {
            mCurrentInteraction = new Interaction();
            mInInteraction = true;

            if(typeiter != results.end())
            {
               InteractionType* rt = InteractionType::GetValueForName((*typeiter).second);
               if(rt != NULL)
               {
                  mCurrentInteraction->SetInteractionType(*rt);
               }
            }

            if(randomiter != results.end())
            {
               bool b = dtUtil::ToType<bool>((*randomiter).second);
               mCurrentInteraction->SetRandomizeResponses(b);
            }

            if(audioiter != results.end())
            {
               mCurrentInteraction->SetSoundHandle((*audioiter).second);
            }

            if(enableeventiter != results.end())
            {
               std::string str((*enableeventiter).second);
               dtDAL::GameEvent* ge = mManager->LookupGameEvent(str);
               if(ge != NULL)
               {
                  mManager->AddCommand(ge, mManager->CreateCommand(mCurrentInteraction.get(), true));
               }
            }

            if(disableeventiter != results.end())
            {
               std::string str((*disableeventiter).second);
               dtDAL::GameEvent* ge = mManager->LookupGameEvent(str);
               if(ge != NULL)
               {
                  mManager->AddCommand(ge, mManager->CreateCommand(mCurrentInteraction.get(), false));
               }
            }

            if(enableflagiter != results.end())
            {
               bool b = dtUtil::ToType<bool>((*enableflagiter).second);
               mCurrentInteraction->SetEnable(b);
            }

            if(forcecloselastiter != results.end())
            {
               bool b = dtUtil::ToType<bool>((*forcecloselastiter).second);
               mCurrentInteraction->SetForceCloseResponseLast(b);
            }

            if(repeatiter != results.end())
            {
               mCurrentInteraction->SetRepeatText((*repeatiter).second);
            }

         }
         else if(elementStr == RESPONSE_ELEMENT)
         {
            mCurrentResponse = new Response();
            mInResponse = true;

            if(typeiter != results.end())
            {
               ResponseType* rt = ResponseType::GetValueForName((*typeiter).second);
               if(rt != NULL)
               {
                  mCurrentResponse->SetResponseType(*rt);
               }
            }

            if(eventiter != results.end())
            {
               dtDAL::GameEvent* ge = mManager->LookupGameEvent((*eventiter).second);
               if(ge != NULL)
               {
                  mManager->AddResponseEvent(mCurrentResponse.get(), ge);
               }
            }

            if(textiter != results.end())
            {
               std::string str((*textiter).second);
               mResponseOptions.push_back(str);
            }

            if(enableeventiter != results.end())
            {
               std::string str((*enableeventiter).second);
               dtDAL::GameEvent* ge = mManager->LookupGameEvent(str);
               if(ge != NULL)
               {
                  mManager->AddCommand(ge, mManager->CreateCommand(mCurrentResponse.get(), true));
               }
            }

            if(disableeventiter != results.end())
            {
               std::string str((*disableeventiter).second);
               dtDAL::GameEvent* ge = mManager->LookupGameEvent(str);
               if(ge != NULL)
               {
                  mManager->AddCommand(ge, mManager->CreateCommand(mCurrentResponse.get(), false));
               }
            }

            if(enableflagiter != results.end())
            {
               bool b = dtUtil::ToType<bool>((*enableflagiter).second);
               mCurrentResponse->SetEnable(b);
            }
         }
         else if(elementStr == WRONG_FEEDBACK_ELEMENT)
         {
            //these fields are not optional
            if(feedbackiter != results.end() && playerresponseiter != results.end())
            {
               mBranchInteraction = new Interaction();

               //feedback
               std::string str((*feedbackiter).second);
               mBranchInteraction->SetInteractionText(str);

               //player response
               std::string str1((*playerresponseiter).second);
               Response* r = new Response();
               r->SetResponseText(str1);
               r->SetResponseType(ResponseType::RESPONSE_WRONG_ACKNOWLEDGED);
               mBranchInteraction->AddResponse(*r);

               if(eventiter != results.end())
               {
                  std::string str2((*eventiter).second);
                  dtDAL::GameEvent* ge = mManager->LookupGameEvent(str2);
                  if(ge != NULL)
                  {
                     mManager->AddResponseEvent(r, ge);
                  }
               }

               if(audioiter != results.end())
               {
                  std::string str2((*audioiter).second);
                  mBranchInteraction->SetSoundHandle(str2);
               }

               if(repeatiter != results.end())
               {
                  mBranchInteraction->SetRepeatText((*repeatiter).second);
               }

            }
            else
            {
               LOG_ERROR("Unable to add wrong branch because required fields Feedback, and PlayerResponse were not found.");
            }
         }

         mElements.push(elementStr);
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::XMLHandler::endElement(const XMLCh* const uri,
         const XMLCh* const localname,
         const XMLCh* const qname)
      {
         std::string elementStr = dtUtil::XMLStringConverter(localname).ToString();
         if(elementStr == CONVERSATION_ELEMENT)
         {
            mInConversation = false;

            //we are done, error check here
         }
         else if(elementStr == INTERACTION_ELEMENT)
         {
            //TODO ERROR CHECK HERE
            if(mCurrentInteraction.valid())
            {
               //RANDOMIZE BEFORE ADDING CLOSE UI
               while(!mResponses.empty())
               {
                  int index = 0;

                  if(mCurrentInteraction->GetRandomizeResponses())
                  {
                     index = dtUtil::RandRange(0U, unsigned(mResponses.size()) - 1U);
                  }

                  mCurrentInteraction->AddResponse(mResponses[index]);
                  mResponses.erase(std::remove(mResponses.begin(), mResponses.end(), mResponses[index]), mResponses.end());
               }

               //now if the force close ui response last flag is set, then it will put all the close ui responses
               //in a different container, so if there are any responses in that container add them now
               ResponseInteractionArray::iterator iter = mCloseUIResponses.begin();
               ResponseInteractionArray::iterator iterEnd = mCloseUIResponses.end();
               for(;iter != iterEnd; ++iter)
               {
                  mCurrentInteraction->AddResponse(*iter);
               }

               mCloseUIResponses.clear();

               //make default close ui response
               if(!mDefaultCloseUI.empty() && !mCurrentInteraction->HasCloseUI())
               {
                  Response* r = new Response();
                  r->SetResponseType(ResponseType::RESPONSE_CLOSE_UI);
                  r->SetResponseText(mDefaultCloseUI);
                  mCurrentInteraction->AddResponse(*r);
               }

               //add current interaction
               mManager->mInteractions.push_back(mCurrentInteraction);
               mCurrentInteraction = NULL;
            }

            mInInteraction = false;
         }
         else if(elementStr == RESPONSE_ELEMENT)
         {
            //TODO ERROR CHECK HERE
            if(mCurrentInteraction.valid() && mCurrentResponse.valid())
            {
               //pick a random response
               if(!mResponseOptions.empty())
               {
                  int index = dtUtil::RandRange(0U, unsigned(mResponseOptions.size()) - 1U);
                  mCurrentResponse->SetResponseText(mResponseOptions[index]);
                  mResponseOptions.clear();
               }
               else
               {
                  //print error, no response text
               }

               //add response to interaction
               //if(mBranchInteraction.valid())
               //{
               //   mCurrentInteraction->AddResponse(*mCurrentResponse, *mBranchInteraction);
               //   mBranchInteraction = NULL;
               //}
               //else
               //{
               //   mCurrentInteraction->AddResponse(*mCurrentResponse);
               //}

               std::pair<dtCore::RefPtr<Response>, dtCore::RefPtr<Interaction> > responseElement;
               responseElement.first = mCurrentResponse.get();
               responseElement.second = mBranchInteraction.get();

               //if this is a close ui response and the interaction wants to force it last
               //add it to a separate container
               if(mCurrentResponse->GetResponseType() == ResponseType::RESPONSE_CLOSE_UI && mCurrentInteraction->GetForceCloseResponseLast())
               {
                  mCloseUIResponses.push_back(responseElement);
               }
               else
               {
                  mResponses.push_back(responseElement);
               }

               mBranchInteraction = NULL;
               mCurrentResponse = NULL;
            }

            mInResponse = false;
         }
         else if(elementStr == WRONG_FEEDBACK_ELEMENT)
         {
            //TODO:
         }
         mElements.pop();
      }

      //////////////////////////////////////////////////////////////////////////
      void Conversation::XMLHandler::characters(const XMLCh* const chars, const XMLSize_t length)
      {
         std::string& topEl = mElements.top();

         if(topEl == CONVERSATION_ELEMENT)
         {
            //TODO ERROR CHECK HERE
         }
         else if(topEl == CHARACTER_ELEMENT)
         {
            const std::string characterName(dtUtil::XMLStringConverter(chars).ToString());

            // Is the character specific to an interaction, as if the
            // character is butting in?
            if(mInInteraction)
            {
               mCurrentInteraction->SetCharacterName( characterName );
            }
            // Is the character defined for the whole conversation at
            // the global conversation level?
            else if(mInConversation)
            {
               mManager->mCharacterName = characterName;
            }
         }
         else if(topEl == NAME_ELEMENT)
         {
            std::string name = dtUtil::XMLStringConverter(chars).ToString();
            if(mInInteraction)
            {
               mManager->mName = name;
            }
            else if(mInConversation)
            {
               //TODO ERROR CHECK HERE
               if(mCurrentInteraction.valid())
               {
                  mCurrentInteraction->SetName(name);
               }
            }
            else
            {
               //PRINT ERROR- out of conversation scope
            }
         }
         else if(topEl == GAME_EVENT_ELEMENT)
         {
            std::string gameEventString = dtUtil::XMLStringConverter(chars).ToString();
            if(!gameEventString.empty())
            {
               dtDAL::GameEvent* ge = mManager->LookupGameEvent(gameEventString);
               if(ge != NULL)
               {
                  if(mInConversation && !mInResponse)
                  {
                     mManager->mRegistrationFunctor(ge, mManager);
                  }
                  else if(mInResponse && mCurrentResponse.valid())
                  {
                     mManager->AddResponseEvent(mCurrentResponse.get(), ge);
                  }
               }
            }
         }
         else if(topEl == TEXT_ELEMENT)
         {
            std::string str = dtUtil::XMLStringConverter(chars).ToString();

            if(mInInteraction && !mInResponse)
            {
               if(mCurrentInteraction.valid())
               {
                  mCurrentInteraction->SetInteractionText(str);
               }
            }
            else
            {
               //print error
            }
         }
         else if(topEl == RAND_TEXT_ELEMENT)
         {
            std::string str(dtUtil::XMLStringConverter(chars).ToString());
            if(mInResponse)
            {
               mResponseOptions.push_back(str);
            }
         }
         else if(topEl == DEFAULT_CLOSEUI_ELEMENT)
         {
            if(mInConversation && !mInInteraction && !mInResponse)
            {
               mDefaultCloseUI = dtUtil::XMLStringConverter(chars).ToString();
            }
            else
            {
               //print error
            }
         }

      }

   }
}
