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
#ifndef SIMCORE_CONVERSATION_H
#define SIMCORE_CONVERSATION_H

#include <dtCore/observerptr.h>
#include <SimCore/Export.h>
#include <osg/Referenced>
#include <vector>
#include <string>
#include <SimCore/Components/Conversations/Interaction.h>
#include <SimCore/Components/Conversations/Command.h>
#include <dtDAL/gameevent.h>

//for xml loading
#include <dtUtil/xercesparser.h>
#include <dtUtil/xercesutils.h>
#include <xercesc/sax2/ContentHandler.hpp>  // for a base class
#include <xercesc/util/XMLString.hpp>
#include <stack>
#include <map>


namespace dtDAL
{
   class GameEvent;
}

namespace SimCore
{
   namespace Components
   {
      class SIMCORE_EXPORT Conversation : public osg::Referenced
      {
         public:
            typedef std::vector< dtCore::RefPtr<Interaction> > InteractionArray;
            typedef std::multimap<dtCore::ObserverPtr<dtDAL::GameEvent>, dtCore::RefPtr<Command<void> > > EventToCommandMap;
            typedef std::multimap<const Response*, dtCore::ObserverPtr<dtDAL::GameEvent> > ResponseToEventMap;

            typedef dtUtil::Functor<void, TYPELIST_2(dtDAL::GameEvent*, Conversation*)> RegisterConversationFunctor;

         public:
            Conversation(RegisterConversationFunctor regFunc);

            const std::string& GetName() const;

            const std::string& GetCharacterName() const;

            bool LoadConversation(const std::string& filename);

            const Interaction* GetCurrentInteraction() const;

            Interaction* HandleResponse(const Response* r);

            void HandleGameEvent(dtDAL::GameEvent* ge);

            ResponseToEventMap& GetResponseEvents();

            void IncrementInteraction();

            void ResetConversation();

         protected:
            /*virtual*/ ~Conversation();
            void ClearData();

            dtDAL::GameEvent* LookupGameEvent(const std::string& str) const;

            void AddCommand(dtDAL::GameEvent* ge, Command<void>* com);
            void AddResponseEvent(Response* r, dtDAL::GameEvent* ge);

            template <class T>
            Command<void>* CreateCommand(T* i, bool enable)
            {
               typedef Command1<void, bool> Com;
               typedef dtUtil::Functor<void, TYPELIST_1(bool)> Func;
               return new Com(Func(i, &T::SetEnable), enable);
            }

         private:

            std::string mName;
            std::string mCharacterName;
            RegisterConversationFunctor mRegistrationFunctor;

            dtCore::ObserverPtr<Interaction> mCurrentInteraction;
            InteractionArray::iterator mCurrentInteractionIter;
            InteractionArray mInteractions;
            EventToCommandMap mCommands;
            ResponseToEventMap mResponseEvents;


            //for internal XML Loading data
            class XMLHandler : public XERCES_CPP_NAMESPACE_QUALIFIER ContentHandler
            {
            public:
               XMLHandler(Conversation* mgr): mManager(mgr)
               {
                  mInConversation = false;
                  mInInteraction = false;
                  mInResponse = false;
               }
               ~XMLHandler() {}

               // inherited pure virtual functions
               virtual void endDocument() {}
               virtual void ignorableWhitespace(const XMLCh* const chars, const unsigned int length) {}
               virtual void processingInstruction(const XMLCh* const target, const XMLCh* const data) {}
               virtual void setDocumentLocator(const XERCES_CPP_NAMESPACE_QUALIFIER Locator* const locator) {}
               virtual void startDocument() {}
               virtual void startPrefixMapping(const	XMLCh* const prefix,const XMLCh* const uri) {}
               virtual void endPrefixMapping(const XMLCh* const prefix) {}
               virtual void skippedEntity(const XMLCh* const name) {}

               virtual void characters(const XMLCh* const chars, const unsigned int length);

               virtual void startElement(const XMLCh* const uri,
                  const XMLCh* const localname,
                  const XMLCh* const qname,
                  const XERCES_CPP_NAMESPACE_QUALIFIER Attributes& attrs);

               virtual void endElement(const XMLCh* const uri,
                  const XMLCh* const localname,
                  const XMLCh* const qname);

            private:

               bool  mInConversation, mInInteraction, mInResponse;
               Conversation* mManager;

               typedef std::stack<std::string> ElementStack;
               ElementStack mElements;

               std::string mDefaultCloseUI;
               std::vector<std::string> mResponseOptions;

               typedef std::vector<std::pair<dtCore::RefPtr<Response>, dtCore::RefPtr<Interaction> > > ResponseInteractionArray;
               ResponseInteractionArray mResponses;
               ResponseInteractionArray mCloseUIResponses;

               dtCore::RefPtr<Interaction> mCurrentInteraction;
               dtCore::RefPtr<Interaction> mBranchInteraction;
               dtCore::RefPtr<Response> mCurrentResponse;
         };

         friend class XMLHandler;
      };

   }
}

#endif
