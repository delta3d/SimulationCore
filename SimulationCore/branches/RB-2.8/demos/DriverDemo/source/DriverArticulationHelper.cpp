/*
 * Copyright, 2008, Alion Science and Technology Corporation, all rights reserved.
 * 
 * See the .h file for complete licensing information.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * @author Curtiss Murphy
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtUtil/nodecollector.h>
#include <dtCore/namedparameter.h>
#include <dtGame/deadreckoninghelper.h>
#include <dtUtil/mathdefines.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <DriverArticulationHelper.h>
#include <GameAppComponent.h>

#include <cmath>

namespace DriverDemo
{
   ////////////////////////////////////////////////////////////////////////////////
   // ARTICULATION HELPER CODE
   ////////////////////////////////////////////////////////////////////////////////
   const std::string DriverArticulationHelper::DOF_NAME_WEAPON("dof_gun_01");
   const std::string DriverArticulationHelper::DOF_NAME_TURRET("dof_turret_01");

   const std::string DriverArticulationHelper::ARTIC_NAME_TURRET_HEADING("TurretHeading");
   const std::string DriverArticulationHelper::ARTIC_NAME_WEAPON_ELEVATION("WeaponElevation");

   ////////////////////////////////////////////////////////////////////////////////
   DriverArticulationHelper::DriverArticulationHelper()
      : mEnableLocalDR(true),
      mThresholdTurretHeading(3.0f),
      mThresholdWeaponElevation(3.0f),
      mLastTurretHeading(0.0f),
      mLastWeaponElevation(0.0f),
      mTotalChangeTurretHeading(0.0f),
      mTotalChangeWeaponElevation(0.0f),
      mChangeCountTurretHeading(0),
      mChangeCountWeaponElevation(0)
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   DriverArticulationHelper::~DriverArticulationHelper()
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   dtCore::RefPtr<dtCore::NamedGroupParameter> DriverArticulationHelper::BuildGroupProperty()
   {
      dtCore::RefPtr<dtCore::NamedGroupParameter> articArrayProp 
         = new dtCore::NamedGroupParameter( GetArticulationArrayPropertyName() );

      if( mDOFTurret.valid() )
      {
         // NOTE: Headings are reversed in Delta HPR.
         //       Negate it to correct it for out-going data.
         mLastTurretHeading = -mDOFTurret->getCurrentHPR().x();
      }

      if( mDOFWeapon.valid() )
      {
         mLastWeaponElevation = mDOFWeapon->getCurrentHPR().y();
      }
   
      AddArticulatedParameter( *articArrayProp, SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH,
         ARTIC_NAME_TURRET_HEADING, 
         mLastTurretHeading, mChangeCountTurretHeading, 
         0.0f, 0,
         DOF_NAME_TURRET );

      // DEBUG: Turret Heading
      //std::cout << "\tTurretHeading(" << mChangeCountTurretHeading << "):\t" 
      //   << osg::RadiansToDegrees( mLastTurretHeading ) << std::endl;
   
      AddArticulatedParameter( *articArrayProp, SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION,
         ARTIC_NAME_WEAPON_ELEVATION,
         mLastWeaponElevation, mChangeCountWeaponElevation,
         0.0f, 0,
         DOF_NAME_WEAPON, DOF_NAME_TURRET );

      // DEBUG: Weapon Elevation
      //std::cout << "\tTurretWeaponElevation(" << mChangeCountWeaponElevation << "):\t" 
      //   << osg::RadiansToDegrees( mLastWeaponElevation ) << "\n" << std::endl;

      return articArrayProp;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverArticulationHelper::UpdateDOFReferences( dtUtil::NodeCollector* nodeCollector )
   {
      if( nodeCollector == NULL )
      {
         mDOFTurret = NULL;
         mDOFWeapon = NULL;
      }
      else
      {
         mDOFTurret = nodeCollector->GetDOFTransform( DOF_NAME_TURRET );
         mDOFWeapon = nodeCollector->GetDOFTransform( DOF_NAME_WEAPON );
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverArticulationHelper::HasDOF( osgSim::DOFTransform& dof ) const
   {
      return mDOFTurret.get() == &dof || mDOFWeapon.get() == &dof;
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DriverArticulationHelper::HasDOFMetric( 
      const osgSim::DOFTransform& dof, const std::string& metricName ) const
   {
      if( mDOFTurret.get() == &dof )
      {
         return SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH.GetName() == metricName
            || SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH.GetRelatedRateMetricName() == metricName;
      }
      else if( mDOFWeapon.get() == &dof )
      {
         return SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION.GetName() == metricName
            || SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION.GetRelatedRateMetricName() == metricName;
      }

      return false;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverArticulationHelper::HandleUpdatedDOFOrientation( 
      const osgSim::DOFTransform& dof, const osg::Vec3& hprChange, const osg::Vec3& hprCurrent )
   {
      // Determine if the DOF specified is a DOF referenced by this helper
      if( mDOFTurret.get() == &dof )
      {
         // Determine if the accumulated change is large enough
         mTotalChangeTurretHeading += hprChange.x();
         if( std::abs(mTotalChangeTurretHeading*0.5f) > mThresholdTurretHeading )
         {
            // Reset the change and increase the change count.
            mTotalChangeTurretHeading = 0.0f;
            ++mChangeCountTurretHeading;
            SetDirty( true );

         }
      }
      else if( mDOFWeapon.get() == &dof )
      {
         // Determine if the accumulated change is large enough
         mTotalChangeWeaponElevation += hprChange.y();
         if( std::abs(mTotalChangeWeaponElevation*0.5f) > mThresholdWeaponElevation )
         {
            // Reset the change and increase the change count.
            mTotalChangeWeaponElevation = 0.0f;
            ++mChangeCountWeaponElevation;
            SetDirty( true );

         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DriverArticulationHelper::DeadReckonArticulation(
      const std::string& dofName, const osg::Vec3& position, const osg::Vec3& velocity )
   {
      if( mEntity.valid() )
      {
         dtGame::DeadReckoningHelper* drHelper = NULL;
         mEntity->GetComponent(drHelper);
         drHelper->AddToDeadReckonDOF( dofName, position, velocity );
      }
   }

}
