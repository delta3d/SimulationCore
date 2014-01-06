/* -*-c++-*-
* Driver Demo - DriverArticulationHelper (.cpp & .h) - Using 'The MIT License'
* Copyright (C) 2008, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
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
#include <dtUtil/refcountedbase.h>
#include <dtUtil/refcountedbase.h>
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

         virtual std::shared_ptr<dtDAL::NamedGroupParameter> BuildGroupProperty();

         virtual void UpdateDOFReferences( dtUtil::NodeCollector* nodeCollector );

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

         std::weak_ptr<osgSim::DOFTransform> mDOFTurret;
         std::weak_ptr<osgSim::DOFTransform> mDOFWeapon;
         std::weak_ptr<SimCore::Actors::BaseEntity> mEntity;
   };
} //namespace

#endif
