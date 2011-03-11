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
#ifndef SIMCORE_INTERACTION_H
#define SIMCORE_INTERACTION_H

#include <osg/Referenced>
#include <dtCore/refptr.h>
#include <string>
#include <vector>

#include <SimCore/Export.h>

#include <dtUtil/enumeration.h>


namespace SimCore
{
   namespace Components
   {
      class Response;

      class SIMCORE_EXPORT InteractionType : public dtUtil::Enumeration
      {
         DECLARE_ENUM(InteractionType)

         public:
            static InteractionType INTERACTION_NORMAL;
            static InteractionType INTERACTION_ERROR;
            static InteractionType INTERACTION_COMMAND;
            static InteractionType INTERACTION_QUESTION;

         private:
            InteractionType( const std::string& name )
               : dtUtil::Enumeration(name)
            {
               AddInstance(this);
            }
      };
      

      class SIMCORE_EXPORT Interaction : public osg::Referenced
      {
         public:
            typedef std::pair<dtCore::RefPtr<Response>, dtCore::RefPtr<Interaction> > ResponseInteractionPair;
            typedef std::vector<ResponseInteractionPair> ResponseArray;

         public:
            Interaction();

            void SetName(const std::string& name);
            const std::string& GetName() const;

            void SetCharacterName(const std::string& name);
            const std::string& GetCharacterName() const;

            void SetEnable(bool b);
            bool GetEnable() const;

            const std::string& GetInteractionText() const;
            void SetInteractionText(const std::string& text);

            void SetSoundHandle(const std::string& soundName);
            const std::string& GetSoundHandle() const;

            /// Repeat Text is a placeholder for some UI's that allow audio and a text option to ask 'please repeat last audio'. 
            void SetRepeatText(const std::string& newValue);
            /// Repeat Text is a placeholder for some UI's that allow audio and a text option to ask 'please repeat last audio'. 
            const std::string& GetRepeatText() const;

            void SetInteractionType(const InteractionType& interactionType);
            const InteractionType& GetInteractionType() const;

            void AddResponse(Response& pResponse);
            void AddResponse(Response& r, Interaction& i);
            void AddResponse(ResponseInteractionPair& ri_pair);
            const ResponseArray& GetResponses() const;

            void SetRandomizeResponses(bool b);
            bool GetRandomizeResponses() const;

            bool HasResponse(const Response& r) const;
            bool HasCloseUI() const;     

            void SetForceCloseResponseLast(bool b);
            bool GetForceCloseResponseLast() const;

            Interaction* GetBranch(const Response& r);

         protected:
            /*virtual*/ ~Interaction();

         private:

            bool mRandomizeAnswers, mEnabled, mForceCloseResponseLast;
            const InteractionType* mType;

            std::string mName;
            std::string mCharacterName;
            std::string mInteractionText;
            std::string mSoundHandle;
            std::string mRepeatText;

            ResponseArray mResponses;
      };

   }
}

#endif
