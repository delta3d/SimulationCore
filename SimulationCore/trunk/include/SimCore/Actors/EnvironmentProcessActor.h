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

#ifndef ENVIRONMENTPROCESSACTORPROXY_H_
#define ENVIRONMENTPROCESSACTORPROXY_H_

#include <SimCore/Export.h>

#include <dtGame/gameactorproxy.h>
#include <dtDAL/namedgroupparameter.h>

#include <dtUtil/getsetmacros.h>

#include <SimCore/Actors/SimpleMovingShapeActor.h>

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT EnvironmentProcessActorProxy : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;
         typedef std::vector<dtCore::RefPtr<dtDAL::NamedGroupParameter> > RecordList;
         typedef std::vector<dtCore::RefPtr<SimpleMovingShapeActorProxy> > CreatedActorList;

         enum EnvironmentRecordTypeCode
         {
            COMBICStateRecordType = 256,
            FlareStateRecordType = 259,
            BiologicalStateType = 4096,
            ChemVaporStateType = 4097,
            RadiologicalStateType = 4098,
            ChemLiquidStateType = 4099,
            BoundingSphereRecordType = 65536,
            UniformGeometryRecordType = 327680,
            PointRecord1Type = 655360,
            LineRecord1Type = 786432,
            SphereRecord1Type = 851968,
            EllipsoidRecord1Type = 1048576,
            ConeRecord1Type = 3145728,
            RectangularVolRecord1Type = 5242880,
            RectangularVolRecord3Type = 83886080,
            PointRecord2Type = 167772160,
            LineRecord2Type = 201326592,
            SphereRecord2Type = 218103808,
            EllipsoidRecord2Type = 268435456,
            ConeRecord2Type = 805306368,
            RectangularVolRecord2Type = 1342177280,
            GaussianPlumeRecordType = 1610612736,
            GaussianPuffRecordType = 1879048192,
            GaussianPuffRecordEXType = 1879048193
         };

         // This set of strings matches what's in the code in the dtHLA mapping, but since this actor
         // is completely decoupled and it defines an internal data structure, I couldn't share the data.
         static const dtUtil::RefString PARAM_INDEX;
         static const dtUtil::RefString PARAM_TYPE_CODE;
         static const dtUtil::RefString PARAM_LOCATION;
         static const dtUtil::RefString PARAM_ORIGINATION_LOCATION;
         static const dtUtil::RefString PARAM_ORIENTATION;
         static const dtUtil::RefString PARAM_VELOCITY;
         static const dtUtil::RefString PARAM_ANGULAR_VELOCITY;
         static const dtUtil::RefString PARAM_DIMENSION;
         static const dtUtil::RefString PARAM_DIMENSION_RATE;
         static const dtUtil::RefString PARAM_CENTROID_HEIGHT;
         static const dtUtil::RefString PARAM_RADIUS;
         static const dtUtil::RefString PARAM_RADIUS_RATE;

         static const dtUtil::RefString PARAM_AGENT_ENUM;
         static const dtUtil::RefString PARAM_GEOM_INDEX;
         static const dtUtil::RefString PARAM_TOTAL_MASS;
         static const dtUtil::RefString PARAM_MIN_SIZE;
         static const dtUtil::RefString PARAM_MAX_SIZE;
         static const dtUtil::RefString PARAM_AVG_MASS_PER_UNIT;
         static const dtUtil::RefString PARAM_PURITY;
         static const dtUtil::RefString PARAM_RADIOLOGCIAL_ACTIVITY;
         static const dtUtil::RefString PARAM_PROBABILITY;
         static const dtUtil::RefString PARAM_VIABILITY;

         static const dtUtil::RefString CONFIG_MULTIPLIER_PREFIX;
         static const dtUtil::RefString CONFIG_COLOR_PREFIX;

         EnvironmentProcessActorProxy();

         DT_DECLARE_ACCESSOR(bool, Active);

         DT_DECLARE_ACCESSOR(int, SequenceNumber);

         DT_DECLARE_ACCESSOR(int, LastUpdateSequenceNumber);

         void SetRecords(const dtDAL::NamedGroupParameter& groupParam);
         dtCore::RefPtr<dtDAL::NamedGroupParameter> GetRecords() const;

         const CreatedActorList& GetCreatedActors();

         void ClearCreatedActors();

      protected:
         // records passed in so that the method may be overridden more easily.
         virtual void OnRecordsChange(const RecordList& records);

         // This takes a ref ptr by reference because it's called from a functor
         virtual void OnRecordChange(const dtCore::RefPtr<dtDAL::NamedGroupParameter>& record);

         virtual void OnStateTypeChange(const dtCore::RefPtr<dtDAL::NamedGroupParameter>& record);

         virtual ~EnvironmentProcessActorProxy();

         virtual void CreateActor();
         virtual void BuildPropertyMap();

         virtual void OnRemovedFromWorld();

      private:
         RecordList mRecords;
         CreatedActorList mCreatedActors;
         CreatedActorList mCreatedActorsBuffer2;
         bool mChangeMarker; // True if the records have been updated, but the changes have not been propagated.
      };

   }

}

#endif /* ENVIRONMENTPROCESSACTORPROXY_H_ */
