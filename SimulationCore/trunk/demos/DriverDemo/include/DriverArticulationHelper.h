/* -*-c++-*-
* Driver Demo
* Copyright (C) 2008, Alion Science and Technology Corporation
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
* @author Curtiss Murphy
*/
#ifndef _DRIVER_ARTICULATION_HELPER_H_
#define _DRIVER_ARTICULATION_HELPER_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <DriverExport.h>
#include <osgSim/DOFTransform>
#include <dtCore/refptr.h>
#include <dtCore/observerptr.h>
#include <SimCore/Components/ArticulationHelper.h>
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



namespace DriverDemo
{
   ////////////////////////////////////////////////////////////////////////////////
   // ARTICULATION HELPER CODE
   ////////////////////////////////////////////////////////////////////////////////
   class DRIVER_DEMO_EXPORT DriverArticulationHelper : public SimCore::Components::ArticulationHelper
   {
      public:
         // The common DOF names found on most vehicle models 
         static const std::string DOF_NAME_WEAPON;
         static const std::string DOF_NAME_TURRET;

         // The names of the articulated parameters used in the articulation array
         static const std::string ARTIC_NAME_TURRET_HEADING;
         static const std::string ARTIC_NAME_WEAPON_ELEVATION;

         DriverArticulationHelper();

         //virtual void SetControlState( SimCore::Actors::ControlStateActor* controlState );
         //virtual const SimCore::Actors::ControlStateActor* GetControlState() const;

         virtual dtCore::RefPtr<dtDAL::NamedGroupParameter> BuildGroupProperty();

         virtual void UpdateDOFReferences( dtCore::NodeCollector* nodeCollector );

         virtual bool HasDOF( osgSim::DOFTransform& dof ) const;

         virtual bool HasDOFMetric( const osgSim::DOFTransform& dof, const std::string& metricName ) const;

         virtual void HandleUpdatedDOFOrientation( const osgSim::DOFTransform& dof, const osg::Vec3& hprChange, const osg::Vec3& hprCurrent );

         //virtual void NotifyControlStateUpdate();

         // Set the angle (in degrees) that the articulation can move without marking
         // this articulation helper as "dirty"
         // @param threshold Angle (degrees) that a DOF can rotate without being marked as "dirty"
         void SetThresholdTurretHeading( float threshold )
         {
            mThresholdTurretHeading = std::abs(threshold);
         }
         float GetThresholdTurretHeading() const
         {
            return mThresholdTurretHeading;
         }

         // Set the angle (in degrees) that the articulation can move without marking
         // this articulation helper as "dirty"
         // @param threshold Angle (degrees) that a DOF can rotate without being marked as "dirty"
         void SetThresholdWeaponElevation( float threshold )
         {
            mThresholdWeaponElevation = std::abs(threshold);
         }
         float GetThresholdWeaponElevation() const
         {
            return mThresholdWeaponElevation;
         }

         void SetEntity( SimCore::Actors::BaseEntity* entity )
         {
            mEntity = entity;
         }
         SimCore::Actors::BaseEntity* GetEntity()
         {
            return mEntity.get();
         }
         const SimCore::Actors::BaseEntity* GetEntity() const
         {
            return mEntity.get();
         }

         void SetLocalDeadReckoningEnabled( bool enabled )
         {
            mEnableLocalDR = enabled;
         }
         bool IsLocalDeadReckoningEnabled() const
         {
            return mEnableLocalDR;
         }

      protected:
         virtual ~DriverArticulationHelper();

         void DeadReckonArticulation( const std::string& dofName,
            const osg::Vec3& position, const osg::Vec3& velocity );

      private:
         bool mEnableLocalDR;

         float mThresholdTurretHeading;   // degrees
         float mThresholdWeaponElevation; // degrees

         float mLastTurretHeading;
         float mLastWeaponElevation;

         float mTotalChangeTurretHeading;
         float mTotalChangeWeaponElevation;
         
         unsigned mChangeCountTurretHeading;
         unsigned mChangeCountWeaponElevation;

         dtCore::ObserverPtr<osgSim::DOFTransform> mDOFTurret;
         dtCore::ObserverPtr<osgSim::DOFTransform> mDOFWeapon;
         dtCore::ObserverPtr<SimCore::Actors::BaseEntity> mEntity;
   };
} //namespace

#endif
