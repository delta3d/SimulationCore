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
#include <SimCore/Components/Conversations/Interaction.h>
#include <SimCore/Components/Conversations/Response.h>
#include <dtDAL/gameevent.h>
#include <algorithm>



namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // ENUMERATION CODE
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(InteractionType);
      InteractionType InteractionType::INTERACTION_NORMAL("NORMAL");
      InteractionType InteractionType::INTERACTION_ERROR("ERROR");
      InteractionType InteractionType::INTERACTION_COMMAND("COMMAND");
      InteractionType InteractionType::INTERACTION_QUESTION("QUESTION");



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      Interaction::Interaction()
         : mRandomizeAnswers(true)
         , mEnabled(true)
         , mForceCloseResponseLast(true)
         , mType(&InteractionType::INTERACTION_NORMAL)
         , mName()
         , mInteractionText()
         , mSoundHandle()
         , mResponses()
         , mRepeatText()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      Interaction::~Interaction()
      {

      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetName( const std::string& name )
      {
         mName = name;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Interaction::GetName() const
      {
         return mName;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetCharacterName( const std::string& name )
      {
         mCharacterName = name;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Interaction::GetCharacterName() const
      {
         return mCharacterName;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetSoundHandle( const std::string& soundName )
      {
         mSoundHandle = soundName;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Interaction::GetSoundHandle() const
      {
         return mSoundHandle;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetRepeatText(const std::string& newValue)
      {
         mRepeatText = newValue;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Interaction::GetRepeatText() const
      {
         return mRepeatText;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetInteractionType( const InteractionType& interactionType )
      {
         mType = &interactionType;
      }

      //////////////////////////////////////////////////////////////////////////
      const InteractionType& Interaction::GetInteractionType() const
      {
         return *mType;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::AddResponse( Response& pResponse )
      {
         ResponseInteractionPair p(&pResponse, NULL);
         pResponse.SetId(mResponses.size());
         mResponses.push_back(p);
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::AddResponse(Response& r, Interaction& i)
      {
         ResponseInteractionPair p(&r, &i);
         r.SetId(mResponses.size());
         mResponses.push_back(p);
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::AddResponse( ResponseInteractionPair& ri_pair )
      {
         ri_pair.first->SetId(mResponses.size());
         mResponses.push_back(ri_pair);
      }

      //////////////////////////////////////////////////////////////////////////
      const Interaction::ResponseArray& Interaction::GetResponses() const
      {
         return mResponses;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetRandomizeResponses( bool b )
      {
         mRandomizeAnswers = b;
      }

      //////////////////////////////////////////////////////////////////////////
      bool Interaction::GetRandomizeResponses() const
      {
         return mRandomizeAnswers;
      }

      //////////////////////////////////////////////////////////////////////////
      const std::string& Interaction::GetInteractionText() const
      {
         return mInteractionText;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetInteractionText( const std::string& text )
      {
         mInteractionText = text;
      }

      //////////////////////////////////////////////////////////////////////////
      //used for HasResponse() and GetBranch() below
      struct funcMatchResponse
      {
         funcMatchResponse(const Response& r): mResponse(r){}

         template<class T>
         bool operator()(const T& response)
         {
            return response.first.get() == &mResponse;
         }

      private:
         const Response& mResponse;
      };

      //////////////////////////////////////////////////////////////////////////
      bool Interaction::HasResponse( const Response& r ) const
      {
         return std::find_if(mResponses.begin(), mResponses.end(), funcMatchResponse(r)) != mResponses.end();
      }

      //////////////////////////////////////////////////////////////////////////
      Interaction* Interaction::GetBranch(const Response& r) 
      {
         Interaction* i = NULL;

         ResponseArray::iterator iter = std::find_if(mResponses.begin(), mResponses.end(), funcMatchResponse(r));
         if(iter != mResponses.end())
         {
            i = (*iter).second.get();
         } 

         return i;
      }

      //////////////////////////////////////////////////////////////////////////
      //used for HasCloseUI() below
      struct funcMatchCloseUI
      {
         template<class T>
         bool operator()(const T& response)
         {
            return response.first->GetResponseType() == ResponseType::RESPONSE_CLOSE_UI;
         }
      };

      //////////////////////////////////////////////////////////////////////////
      bool Interaction::HasCloseUI() const
      {
         return std::find_if(mResponses.begin(), mResponses.end(), funcMatchCloseUI()) != mResponses.end();
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetEnable( bool b )
      {
         mEnabled = b;
      }

      //////////////////////////////////////////////////////////////////////////
      bool Interaction::GetEnable() const
      {
         return mEnabled;
      }

      //////////////////////////////////////////////////////////////////////////
      void Interaction::SetForceCloseResponseLast( bool b )
      {
         mForceCloseResponseLast = b;
      }

      //////////////////////////////////////////////////////////////////////////
      bool Interaction::GetForceCloseResponseLast() const
      {
         return mForceCloseResponseLast;
      }

   }
}
