/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
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
 *
 * David Guthrie
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/EnvironmentProcessActor.h>
#include <dtGame/gameactor.h>
#include <dtDAL/propertymacros.h>

#include <climits>

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////
      EnvironmentProcessActorProxy::EnvironmentProcessActorProxy()
      : mSequenceNumber(USHRT_MAX)
      , mRecords()
      {
      }

      ////////////////////////////////////////////
      EnvironmentProcessActorProxy::~EnvironmentProcessActorProxy()
      {
      }

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::CreateActor()
      {
         SetActor(*new dtGame::GameActor(*this));
      }

      //////////////////////////////////////////////////////////////////////////////
      void EnvironmentProcessActorProxy::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUPNAME("EnvironmentProcess");

         typedef dtDAL::PropertyRegHelper<EnvironmentProcessActorProxy&, EnvironmentProcessActorProxy> PropRegHelperType;
         PropRegHelperType propRegHelper(*this, this, GROUPNAME);

         DT_REGISTER_PROPERTY_WITH_LABEL(SequenceNumber, "Sequence Number",
                  "The update sequence number of this environment process.",
                  PropRegHelperType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_LABEL(Records, "Environment Process Records",
                  "The data set of records describing this environment process.",
                  PropRegHelperType, propRegHelper);
      }

      ////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(EnvironmentProcessActorProxy, int, SequenceNumber);


      class AddRecordFunc
      {
      public:
         AddRecordFunc(EnvironmentProcessActorProxy::RecordList& records)
         : mRecords(records)
         {}

         void operator() (const dtCore::RefPtr<dtDAL::NamedParameter>& param)
         {
            if (param->GetDataType() == dtDAL::DataType::GROUP)
            {
               mRecords.push_back(new dtDAL::NamedGroupParameter(static_cast<const dtDAL::NamedGroupParameter&>(*param)));
            }
         }

         EnvironmentProcessActorProxy::RecordList& mRecords;
      };

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::SetRecords(const dtDAL::NamedGroupParameter& groupParam)
      {
         mRecords.clear();
         mRecords.reserve(groupParam.GetParameterCount());
         AddRecordFunc addFunc(mRecords);
         groupParam.ForEachParameter(addFunc);
      }

      //////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::NamedGroupParameter> EnvironmentProcessActorProxy::GetRecords() const
      {
         return new dtDAL::NamedGroupParameter("Records", mRecords);
      }


   }

}
