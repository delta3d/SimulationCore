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

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT EnvironmentProcessActorProxy : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;
         typedef std::vector<dtCore::RefPtr<dtDAL::NamedGroupParameter> > RecordList;

         EnvironmentProcessActorProxy();

         DT_DECLARE_ACCESSOR(int, SequenceNumber);

         void SetRecords(const dtDAL::NamedGroupParameter& groupParam);
         dtCore::RefPtr<dtDAL::NamedGroupParameter> GetRecords() const;

         virtual void OnRecordsChange(dtDAL::NamedGroupParameter& records) = 0;

      protected:

         virtual ~EnvironmentProcessActorProxy();

         virtual void CreateActor();
         virtual void BuildPropertyMap();

      private:
         RecordList mRecords;
      };

   }

}

#endif /* ENVIRONMENTPROCESSACTORPROXY_H_ */
