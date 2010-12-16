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
#include <SimCore/Actors/SimpleMovingShapeActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
#include <dtGame/gamemanager.inl>
#include <dtDAL/propertymacros.h>
#include <dtDAL/namedgroupparameter.inl> //needed for Get and Set Value methods.

#include <dtGame/deadreckoninghelper.h>

#include <climits>
#include <algorithm>

//DELETE ME
#include <iostream>
#include <osg/io_utils>

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
      , mChangeMarker(false)
      {
         SetClassName("SimCore::Actors::EnvironmentProcessActorProxy");
         SetHideDTCorePhysicsProps(true);
         printf("%s\n","Created Environment Process");
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
         BaseClass::BuildPropertyMap();
         static const dtUtil::RefString GROUPNAME("EnvironmentProcess");

         typedef dtDAL::PropertyRegHelper<EnvironmentProcessActorProxy&, EnvironmentProcessActorProxy> PropRegHelperType;
         PropRegHelperType propRegHelper(*this, this, GROUPNAME);

         DT_REGISTER_PROPERTY_WITH_LABEL(Active, "Active",
                  "Whether this environment process is active.",
                  PropRegHelperType, propRegHelper);

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
      DT_IMPLEMENT_ACCESSOR(EnvironmentProcessActorProxy, bool, Active);
      DT_IMPLEMENT_ACCESSOR_GETTER(EnvironmentProcessActorProxy, int, SequenceNumber);
      DT_IMPLEMENT_ACCESSOR(EnvironmentProcessActorProxy, int, LastUpdateSequenceNumber);

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::SetSequenceNumber(int nextValue)
      {
         mSequenceNumber = nextValue;
         OnRecordsChange(mRecords);
      }

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
            SimpleMovingShapeActorProxy& curActor = **i;
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
         if (mChangeMarker && (mLastUpdateSequenceNumber != mSequenceNumber || mSequenceNumber == USHRT_MAX))
         {
            mCreatedActorsBuffer2.reserve(records.size());

            std::for_each(records.begin(), records.end(), dtUtil::MakeFunctor(&EnvironmentProcessActorProxy::OnRecordChange, this));
            mLastUpdateSequenceNumber = mSequenceNumber;
            mCreatedActors.swap(mCreatedActorsBuffer2);

            CreatedActorList::iterator i, iend;
            i = mCreatedActorsBuffer2.begin();
            iend = mCreatedActorsBuffer2.end();
            for (; i != iend; ++i)
            {
               GetGameManager()->DeleteActor(**i);
            }
            mCreatedActorsBuffer2.clear();
            mChangeMarker = false;
         }
      }

      class RemoveIndexFunc
      {
      public:
         RemoveIndexFunc(unsigned index)
         : mIndex(index)
         {
         }

         bool operator()(SimpleMovingShapeActorProxy* actor)
         {
            return unsigned(actor->GetIndex()) == mIndex;
         }
      private:
         unsigned mIndex;
      };

      ////////////////////////////////////////////
      void EnvironmentProcessActorProxy::OnRecordChange(const dtCore::RefPtr<dtDAL::NamedGroupParameter>& record)
      {
         unsigned index = record->GetValue(PARAM_INDEX, 0U);
         unsigned typeCode = record->GetValue(PARAM_TYPE_CODE, 0U);

         if (typeCode == GaussianPuffRecordType || typeCode == GaussianPuffRecordEXType)
         {
            const osg::Vec3d defaultLoc;
            const osg::Vec3 defaultVec3;

            dtCore::RefPtr<SimpleMovingShapeActorProxy> puff;

            if (!mCreatedActors.empty())
            {
               RemoveIndexFunc riFunc(index);

               CreatedActorList::iterator found = std::remove_if(mCreatedActors.begin(), mCreatedActors.end(), riFunc);

               if (found != mCreatedActors.end())
               {
                  puff = *found;
                  mCreatedActors.erase(found);
                  mCreatedActorsBuffer2.push_back(puff);
               }
            }

            if (!puff.valid())
            {
               GetGameManager()->CreateActor(*EntityActorRegistry::ENVIRONMENT_PROCESS_MOVING_SHAPE_ACTOR_TYPE, puff);
               puff->SetOwner(GetId());
               puff->SetIndex(index);
               GetGameManager()->AddActor(*puff, IsRemote(), false);
            }

            dtGame::DeadReckoningHelper* drAC = NULL;
            puff->GetGameActor().GetComponent(drAC);

            osg::Vec3d tempVec3d;
            osg::Vec3 tempVec3;

            tempVec3d = record->GetValue(PARAM_LOCATION, defaultLoc);
            drAC->SetLastKnownTranslation(tempVec3d);

            //record->GetValue(PARAM_ORIGINATION_LOCATION, defaultLoc);

            tempVec3 = record->GetValue(PARAM_DIMENSION, defaultVec3);
            puff->SetLastKnownDimension(tempVec3);
            std::cout << "Dimension" << tempVec3 << std::endl;
            tempVec3 = record->GetValue(PARAM_DIMENSION_RATE, defaultVec3);
            puff->SetLastKnownDimensionVelocity(tempVec3);
            std::cout << "Dimension Rate" << tempVec3 << std::endl;
            tempVec3 = record->GetValue(PARAM_ORIENTATION, defaultVec3);
            drAC->SetLastKnownRotation(tempVec3);
            tempVec3 = record->GetValue(PARAM_VELOCITY, defaultVec3);
            drAC->SetLastKnownVelocity(tempVec3);
            tempVec3 = record->GetValue(PARAM_ANGULAR_VELOCITY, defaultVec3);
            drAC->SetLastKnownAngularVelocity(tempVec3);

            //record->GetValue(PARAM_CENTROID_HEIGHT, 0.0f);
         }

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
         mChangeMarker = true;

         OnRecordsChange(mRecords);
      }

      //////////////////////////////////////////////
      dtCore::RefPtr<dtDAL::NamedGroupParameter> EnvironmentProcessActorProxy::GetRecords() const
      {
         return new dtDAL::NamedGroupParameter("Records", mRecords);
      }

   }

}
