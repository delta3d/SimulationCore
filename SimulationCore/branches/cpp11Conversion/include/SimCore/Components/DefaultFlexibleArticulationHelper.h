/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
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
* @author Curtiss Murphy
*/
#ifndef _DEFAULT_FLEXIBLE_ARTICULATION_HELPER_H_
#define _DEFAULT_FLEXIBLE_ARTICULATION_HELPER_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <dtUtil/refcountedbase.h>
#include <SimCore/Components/ArticulationHelper.h>

//#include <osgSim/DOFTransform>
#include <dtUtil/refcountedbase.h>
#include <SimCore/Actors/BaseEntity.h>

#include <cmath>

////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class NodeCollector;
}

namespace dtDAL
{
   class NamedGroupParameter;
}



namespace SimCore
{
   namespace Components
   {
      /**
       * DefaultFlexibleArticulationHelper - This helper class provides everything most apps should
       * needed to dead reckon and publish articulations over a network. To use, create the helper, set
       * your entity, and then add each of the articulation dof's that you need. Once you set this up, it
       * will automatically examine the articulated dof's on your Entity. If any of them change, it will mark
       * them as dirty and cause them to be published in the next actor update. Use like this:
       *
       *    std::shared_ptr<DefaultFlexibleArticulationHelper> articHelper = new DefaultFlexibleArticulationHelper;
       *    articHelper->SetEntity(yourEntity);
       *    articHelper->AddArticulation("dof_gun_01", ARTIC_TYPE_BOTH);
       *    articHelper->AddArticulation("dof_door_left_01", ARTIC_TYPE_HEADING);
       *    yourEntity->SetArticulationHelper(articHelper.get()); // Do this last or else the node's can't be found
       */
      class SIMCORE_EXPORT DefaultFlexibleArticulationHelper : public SimCore::Components::ArticulationHelper
      {
         public:
            enum ARTICULATION_TYPE {ARTIC_TYPE_HEADING = 0, ARTIC_TYPE_ELEVATION, ARTIC_TYPE_BOTH};

            ///////////////////////////////////////////////////////////
            /// A simple data class used by the DefaultFlexibleArticulationHelper
            class ArticulationEntry : public std::enable_shared_from_this
            {
            public:
               ArticulationEntry() { };

               std::string mDOFName;
               std::string mParentDOFName;
               bool mIsHeading; // Can be both heading and/or elevation
               bool mIsElevation; // Can be both heading and/or elevation
               float mTotalChangeHeading;
               float mTotalChangeElevation;
               float mLastHeading;
               float mLastElevation;
               unsigned mChangeCountHeading;
               unsigned mChangeCountElevation;
               std::weak_ptr<osgSim::DOFTransform> mDOF;
            protected:
               virtual ~ArticulationEntry() { } ;
            };
            ///////////////////////////////////////////////////////////


            DefaultFlexibleArticulationHelper();

            /**
             * Adds an artic to be managed by this helper. Use this to add a turret, gun, door, or whatever.
             * Make sure that a dof node matching the dofName exists in the non-damaged model of your entity
             * Call this once per artic on your entity. See class definition on how to use this.
             * @param parentDOFName An optional name of the parent dof node of this dof (if important)
             */
            void AddArticulation(const std::string &dofName, ARTICULATION_TYPE articType, const std::string &parentDOFName="");

            virtual std::shared_ptr<dtDAL::NamedGroupParameter> BuildGroupProperty();
            virtual void UpdateDOFReferences( dtUtil::NodeCollector* nodeCollector );
            virtual bool HasDOF( osgSim::DOFTransform& dof ) const;
            virtual bool HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const;
            virtual void HandleUpdatedDOFOrientation( const osgSim::DOFTransform& dof, const osg::Vec3& hprChange, const osg::Vec3& hprCurrent );

            /// Set the angle (in degrees) that the dof's heading can change without being marked as "dirty"
            void SetThresholdHeading( float threshold ) { mThresholdHeading = std::abs(threshold); }
            /// Get the angle (in degrees) that the dof's heading can change without being marked as "dirty"
            float GetThresholdHeading() const  { return mThresholdHeading; }

            /// Set the angle (in degrees) that the dof's elev can move without being marked as "dirty"
            void SetThresholdElevation(float threshold) { mThresholdElevation = std::abs(threshold); }
            /// Get the angle (in degrees) that the dof's elev can move without being marked as "dirty"
            float GetThresholdElevation() const { return mThresholdElevation; }

            void SetEntity(SimCore::Actors::BaseEntity* entity) { mEntity = entity; }
            SimCore::Actors::BaseEntity* GetEntity() { return mEntity.get(); }
            const SimCore::Actors::BaseEntity* GetEntity() const { return mEntity.get(); }

         protected:
            virtual ~DefaultFlexibleArticulationHelper();

            void DeadReckonArticulation( const std::string& dofName,
               const osg::Vec3& position, const osg::Vec3& velocity );

         private:

            float mThresholdHeading;   // degrees
            float mThresholdElevation; // degrees

            std::weak_ptr<SimCore::Actors::BaseEntity> mEntity;
            std::vector<std::shared_ptr<ArticulationEntry> > mArticEntries;
      };
   }
} //namespace

#endif
