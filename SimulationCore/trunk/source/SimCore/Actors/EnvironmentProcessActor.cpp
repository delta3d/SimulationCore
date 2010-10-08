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
#include <dtGame/gamemanager.h>
#include <dtDAL/propertymacros.h>
#include <dtDAL/namedgroupparameter.inl> //needed for Get and Set Value methods.

#include <climits>
#include <algorithm>

namespace SimCore
{
   namespace Actors
   {
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_INDEX("Index");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_TYPE_CODE("TypeCode");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_LOCATION("Location");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_ORIGINATION_LOCATION("OriginationLocation");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_ORIENTATION("Orientation");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_VELOCITY("Velocity");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_ANGULAR_VELOCITY("AngularVelocity");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_DIMENSION("Dimension");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_DIMENSION_RATE("DimensionRate");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_CENTROID_HEIGHT("CentroidHeight");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_RADIUS("Radius");
      const dtUtil::RefString EnvironmentProcessActorProxy::PARAM_RADIUS_RATE("RadiusRate");

      ////////////////////////////////////////////
      EnvironmentProcessActorProxy::EnvironmentProcessActorProxy()
      : mSequenceNumber(USHRT_MAX)
      , mLastUpdateSequenceNumber(USHRT_MAX)
      , mRecords()
      {
         SetClassName("SimCore::Actors::EnvironmentProcessActorProxy");
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

         // Sequence number needs to be updated first so that it's already update when the records are updated.
         // so it thus must be defined first.
         DT_REGISTER_PROPERTY_WITH_LABEL(SequenceNumber, "Sequence Number",
                  "The update sequence number of this environment process.",
                  PropRegHelperType, propRegHelper);

         DT_REGISTER_PROPERTY_WITH_LABEL(Records, "Environment Process Records",
                  "The data set of records describing this environment process.",
                  PropRegHelperType, propRegHelper);
      }

      ////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(EnvironmentProcessActorProxy, int, SequenceNumber);
      DT_IMPLEMENT_ACCESSOR(EnvironmentProcessActorProxy, int, LastUpdateSequenceNumber);

      ////////////////////////////////////////////
      const EnvironmentProcessActorProxy::CreatedActorList& EnvironmentProcessActorProxy::GetCreatedActors()
      {
         return mCreatedActors;
      }

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::ClearCreatedActors()
      {
         EnvironmentProcessActorProxy::CreatedActorList::iterator i, iend;
         i = mCreatedActors.begin();
         iend = mCreatedActors.end();
         for (; i != iend; ++i)
         {
            SimpleMovingShapeActorProxy& curActor = *(i->second);
            if (curActor.IsInGM())
            {
               GetGameManager()->DeleteActor(curActor);
            }
         }

         mCreatedActors.clear();
      }

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::OnRecordsChange(const RecordList& records)
      {
         if (mLastUpdateSequenceNumber != mSequenceNumber || mSequenceNumber == USHRT_MAX)
         {
            std::for_each(records.begin(), records.end(), dtUtil::MakeFunctor(&EnvironmentProcessActorProxy::OnRecordChange, this));
            mLastUpdateSequenceNumber = mSequenceNumber;
         }
      }

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::OnRecordChange(const dtCore::RefPtr<dtDAL::NamedGroupParameter>& record)
      {
         int index = record->GetValue(PARAM_INDEX, -1);
         int typeCode = record->GetValue(PARAM_TYPE_CODE, -1);

         const osg::Vec3d defaultLoc;
         const osg::Vec3 defaultVec3;

         record->GetValue(PARAM_LOCATION, defaultLoc);
         record->GetValue(PARAM_ORIGINATION_LOCATION, defaultLoc);
         record->GetValue(PARAM_DIMENSION, defaultVec3);
         record->GetValue(PARAM_DIMENSION_RATE, defaultVec3);
         record->GetValue(PARAM_ORIENTATION, defaultVec3);
         record->GetValue(PARAM_VELOCITY, defaultVec3);
         record->GetValue(PARAM_ANGULAR_VELOCITY, defaultVec3);
         record->GetValue(PARAM_CENTROID_HEIGHT, 0.0f);

      }

      ////////////////////////////////////////////
      ////////////////////////////////////////////
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
      ////////////////////////////////////////////

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::SetRecords(const dtDAL::NamedGroupParameter& groupParam)
      {
         mRecords.clear();
         mRecords.reserve(groupParam.GetParameterCount());
         AddRecordFunc addFunc(mRecords);
         groupParam.ForEachParameter(addFunc);

         OnRecordsChange(mRecords);
      }

      //////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::NamedGroupParameter> EnvironmentProcessActorProxy::GetRecords() const
      {
         return new dtDAL::NamedGroupParameter("Records", mRecords);
      }

   }

}
