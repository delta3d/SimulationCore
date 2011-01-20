/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
* Curtiss Murphy
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osgSim/DOFTransform>
#include <dtUtil/nodecollector.h>
#include <dtUtil/mathdefines.h>
#include <dtDAL/namedparameter.h>
#include <dtGame/deadreckoninghelper.h>
//#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Components/DefaultFlexibleArticulationHelper.h>

#include <cmath>

namespace SimCore
{
namespace Components
{
   ////////////////////////////////////////////////////////////////////////////////
   // ARTICULATION HELPER CODE
   ////////////////////////////////////////////////////////////////////////////////
   //const std::string DefaultFlexibleArticulationHelper::DOF_NAME_WEAPON("dof_gun"); // Append _01
   //const std::string DefaultFlexibleArticulationHelper::DOF_NAME_TURRET("dof_turret"); // Append _01, _02
   //const std::string DefaultFlexibleArticulationHelper::ARTIC_NAME_TURRET_HEADING("TurretHeading");
   //const std::string DefaultFlexibleArticulationHelper::ARTIC_NAME_WEAPON_ELEVATION("WeaponElevation");

   ////////////////////////////////////////////////////////////////////////////////
   DefaultFlexibleArticulationHelper::DefaultFlexibleArticulationHelper()
      : mThresholdHeading(3.0f)
      , mThresholdElevation(3.0f)
   {
   }

   ////////////////////////////////////////////////////////////////////////////////
   DefaultFlexibleArticulationHelper::~DefaultFlexibleArticulationHelper()
   {
      if (!mArticEntries.empty())
      {
         mArticEntries.clear();
      }
   }


   ////////////////////////////////////////////////////////////////////////////////
   void DefaultFlexibleArticulationHelper::AddArticulation(const std::string &dofName, ARTICULATION_TYPE articType, 
      const std::string &parentDOFName)
   {
      if (!dofName.empty())
      {
         dtCore::RefPtr<ArticulationEntry> newArtic = new ArticulationEntry();
         newArtic->mDOFName = dofName;
         newArtic->mParentDOFName = parentDOFName;
         newArtic->mIsHeading = (articType == ARTIC_TYPE_HEADING || articType == ARTIC_TYPE_BOTH);
         newArtic->mIsElevation = (articType == ARTIC_TYPE_ELEVATION || articType == ARTIC_TYPE_BOTH);
         newArtic->mTotalChangeHeading = 0.0f;
         newArtic->mTotalChangeElevation = 0.0f;
         newArtic->mLastHeading = 0.0f;
         newArtic->mLastElevation = 0.0f;
         newArtic->mChangeCountHeading = 0;
         newArtic->mChangeCountElevation = 0;

         mArticEntries.push_back(newArtic);
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   dtCore::RefPtr<dtDAL::NamedGroupParameter> DefaultFlexibleArticulationHelper::BuildGroupProperty()
   {
      // NOTE - The group params are built on the fly. 
      // This method is called when the entity needs to do an actor update message. 
      dtCore::RefPtr<dtDAL::NamedGroupParameter> articArrayProp 
         = new dtDAL::NamedGroupParameter(GetArticulationArrayPropertyName());

      if(!mArticEntries.empty())
      {
         for (unsigned int i = 0; i < (unsigned int) mArticEntries.size(); i ++)
         {
            ArticulationEntry *articEntry = mArticEntries[i].get();
            if (articEntry->mDOF.valid())
            {
               // HEADING
               if (articEntry->mIsHeading)
               {
                  articEntry->mLastHeading = articEntry->mDOF->getCurrentHPR().x();
                  // NOTE: Headings are reversed in some networks, such as HLA. See Platform::SetArticulationHelper() for more info. 
                  if (IsPublishReverseHeading())
                  {
                     articEntry->mLastHeading *= -1.0f;
                  }

                  AddArticulatedParameter(*articArrayProp, SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH,
                     articEntry->mDOFName + "_Heading", articEntry->mLastHeading, articEntry->mChangeCountHeading, 
                     0.0f, 0, articEntry->mDOFName, articEntry->mParentDOFName);
               }

               // ELEVATION
               if (articEntry->mIsElevation)
               {
                  articEntry->mLastElevation = articEntry->mDOF->getCurrentHPR().y();
                  AddArticulatedParameter(*articArrayProp, SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION,
                     articEntry->mDOFName + "_Elevation", articEntry->mLastElevation, articEntry->mChangeCountElevation, 
                     0.0f, 0, articEntry->mDOFName, articEntry->mParentDOFName);
               }
            }
         }
      }

      return articArrayProp;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DefaultFlexibleArticulationHelper::UpdateDOFReferences(dtUtil::NodeCollector* nodeCollector)
   {
      if(!mArticEntries.empty())
      {
         for (unsigned int i = 0; i < mArticEntries.size(); i ++)
         {
            ArticulationEntry *articEntry = mArticEntries[i].get();
            if(nodeCollector == NULL)
            {
               articEntry->mDOF = NULL;
            }
            else 
            {
               articEntry->mDOF = nodeCollector->GetDOFTransform(articEntry->mDOFName);
            }

         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DefaultFlexibleArticulationHelper::HasDOF(osgSim::DOFTransform& dof) const
   {
      bool result = false;
      if(!mArticEntries.empty())
      {
         for (unsigned int i = 0; !result && i < mArticEntries.size(); i ++)
         {
            ArticulationEntry *articEntry = mArticEntries[i].get();
            if(&dof == articEntry->mDOF.get())
            {
               result = true;
            }
         }
      }
      return result;
   }

   ////////////////////////////////////////////////////////////////////////////////
   bool DefaultFlexibleArticulationHelper::HasDOFMetric( 
      const osgSim::DOFTransform& dof, const std::string& metricName) const
   {
      bool result = false;
      if(!mArticEntries.empty())
      {
         // Spin through our artics - look for a matching DOF that has same Elev/Heading type
         for (unsigned int i = 0; !result && i < mArticEntries.size(); i ++)
         {
            ArticulationEntry *articEntry = mArticEntries[i].get();
            if(&dof == articEntry->mDOF.get())
            {
               bool metricIsHeading = SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH.GetName() == metricName
                  || SimCore::Components::ArticulationMetricType::ARTICULATE_AZIMUTH.GetRelatedRateMetricName() == metricName;
               bool metricIsElevation = SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION.GetName() == metricName
                  || SimCore::Components::ArticulationMetricType::ARTICULATE_ELEVATION.GetRelatedRateMetricName() == metricName;
               // Check heading & elevation - if either match, then it's a match
               if ((articEntry->mIsHeading && metricIsHeading) || (articEntry->mIsElevation && metricIsElevation))
               {
                  result = true;
               }
            }
         }
      }
      return result;
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DefaultFlexibleArticulationHelper::HandleUpdatedDOFOrientation( 
      const osgSim::DOFTransform& dof, const osg::Vec3& hprChange, const osg::Vec3& hprCurrent )
   {
      if(!mArticEntries.empty())
      {
         // Spin through our artics - look for a matching DOF that has same Elev/Heading type
         for (unsigned int i = 0; i < mArticEntries.size(); i ++)
         {
            ArticulationEntry *articEntry = mArticEntries[i].get();
            if (articEntry->mDOF == &dof)
            {
               if (articEntry->mIsHeading)
               {
                  // Determine if the accumulated change is large enough
                  articEntry->mTotalChangeHeading += hprChange.x();
                  if(std::abs(articEntry->mTotalChangeHeading*0.5f) > mThresholdHeading)
                  {
                     // Reset the change and increase the change count.
                     articEntry->mTotalChangeHeading = 0.0f;
                     ++articEntry->mChangeCountHeading;
                     SetDirty(true);
                  }
               }
               if (articEntry->mIsElevation)
               {
                  // Determine if the accumulated change is large enough
                  articEntry->mTotalChangeElevation += hprChange.x();
                  if(std::abs(articEntry->mTotalChangeElevation*0.5f) > mThresholdElevation)
                  {
                     // Reset the change and increase the change count.
                     articEntry->mTotalChangeElevation = 0.0f;
                     ++articEntry->mChangeCountElevation;
                     SetDirty(true);
                  }
               }
            }
         }
      }
   }

   ////////////////////////////////////////////////////////////////////////////////
   void DefaultFlexibleArticulationHelper::DeadReckonArticulation(
      const std::string& dofName, const osg::Vec3& position, const osg::Vec3& velocity )
   {
      if(mEntity.valid())
      {
         dtGame::DeadReckoningHelper* drHelper = NULL;
         mEntity->GetComponent(drHelper);
         drHelper->AddToDeadReckonDOF(dofName, position, velocity);
      }
   }

}
}
