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
#ifndef SIMCORE_CONVERSATION_COMPONENT_H
#define SIMCORE_CONVERSATION_COMPONENT_H

#include <dtGame/gmcomponent.h>
#include <SimCore/Export.h>
#include <SimCore/Components/Conversations/Conversation.h>
#include <SimCore/Components/Conversations/Response.h>

#include <dtCore/gameevent.h>
#include <map>

//for xml loading
#include <dtUtil/xercesparser.h>
#include <dtUtil/xercesutils.h>
#include <xercesc/sax2/ContentHandler.hpp>  // for a base class
#include <xercesc/util/XMLString.hpp>

#if XERCES_VERSION_MAJOR < 3
#ifndef XMLSize_t
#define XMLSize_t unsigned
#endif
#endif

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT ConversationComponent : public dtGame::GMComponent
      {
         public:

            typedef dtCore::RefPtr<Conversation> ConversationPtr;
            typedef std::map<const dtCore::GameEvent*, ConversationPtr> ConversationMap;

            static const dtCore::RefPtr<dtCore::SystemComponentType> TYPE;
            static const std::string DEFAULT_NAME;

            /// Constructor
            ConversationComponent(dtCore::SystemComponentType& type = *TYPE);

            /*virtual*/ void ProcessMessage(const dtGame::Message &message);


            void LoadConversationConfig(const std::string& filename);
            
            const Conversation* GetCurrentConversation();

            virtual bool HandleResponse(const Response* r);

         protected:

            /// Destructor
            /*virtual*/ ~ConversationComponent();

            /*virtual*/ void OnAddedToGM(); 

            /*virtual*/ void OnRemovedFromGM();

            void ClearData();
            void SendGameEvent(const dtCore::GameEvent* ge);
            void SendInteractionChangedMessage(const Interaction* i);
            void SendConversationResponseMessage(const Response* r);
            void SendGameEventToConversations(dtCore::GameEvent* ge);
            //called by class conversation
            void RegisterConversation(dtCore::GameEvent* ge, Conversation* conv);

         private:

            dtCore::ObserverPtr<Conversation> mCurrentConversation;

            ConversationMap mConversations;


            //for xml loading
            class XMLHandler : public XERCES_CPP_NAMESPACE_QUALIFIER ContentHandler
            {
            public:
               XMLHandler(ConversationComponent* mgr): mManager(mgr) {}
               ~XMLHandler() {}

               // inherited pure virtual functions            
               virtual void endDocument() {}
               virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length) {}
               virtual void processingInstruction(const XMLCh* const target, const XMLCh* const data) {}
               virtual void setDocumentLocator(const XERCES_CPP_NAMESPACE_QUALIFIER Locator* const locator) {}
               virtual void startDocument() {}
               virtual void startPrefixMapping(const	XMLCh* const prefix,const XMLCh* const uri) {}
               virtual void endPrefixMapping(const XMLCh* const prefix) {}
               virtual void skippedEntity(const XMLCh* const name) {}

               virtual void characters(const XMLCh* const chars, const XMLSize_t length){}

               virtual void startElement(const XMLCh* const uri,
                  const XMLCh* const localname,
                  const XMLCh* const qname,
                  const XERCES_CPP_NAMESPACE_QUALIFIER Attributes& attrs);

               virtual void endElement(const XMLCh* const uri,
                  const XMLCh* const localname,
                  const XMLCh* const qname);

            private:
               ConversationComponent* mManager;
            };
      };
   }
}

#endif
