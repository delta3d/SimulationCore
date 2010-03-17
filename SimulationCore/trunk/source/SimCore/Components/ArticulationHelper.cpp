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
 * @author Chris Rodgers, Curtiss Murphy
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <osgSim/DOFTransform>
#include <dtDAL/namedparameter.h>
#include <dtDAL/groupactorproperty.h>
#include <dtGame/deadreckoninghelper.h>
#include <SimCore/Components/ArticulationHelper.h>


namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // ENUMERATION CODE
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(ArticulationMetricType);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_UNKNOWN("Unknown Metric","Unknown Rate Metric");
      ArticulationMetricType ArticulationMetricType::ARTICULATE_POSITION(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_POSITION,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_POSITIONRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_EXTENSION(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_EXTENSION,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_EXTENSIONRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_X(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_X,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_XRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_Y(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_Y,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_YRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_Z(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_Z,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ZRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_AZIMUTH(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_AZIMUTH,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_AZIMUTHRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_ELEVATION(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ELEVATION,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ELEVATIONRATE);
      ArticulationMetricType ArticulationMetricType::ARTICULATE_ROTATION(dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ROTATION,
         dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ROTATIONRATE);

      //////////////////////////////////////////////////////////////////////////
      ArticulationMetricType::ArticulationMetricType( const std::string &name, const std::string& relatedRateName )
         : dtUtil::Enumeration(name),
         mRelatedRateName(relatedRateName)
      {
         AddInstance(this);
      }



      //////////////////////////////////////////////////////////////////////////
      // ARTICULATION HELPER CODE
      //////////////////////////////////////////////////////////////////////////
      const std::string ArticulationHelper::PROPERTY_NAME_ARTICULATED_ARRAY("Articulated Parameters Array");
      const std::string ArticulationHelper::PARAM_NAME_SUFFIX_RATE("Rate");
      const std::string ArticulationHelper::PARAM_NAME_PREFIX_ARTICULATED("Articulated");
      const std::string ArticulationHelper::PARAM_NAME_PREFIX_ATTACHED("Attached");
      const std::string ArticulationHelper::PARAM_NAME_CHANGE("Change");
      const std::string ArticulationHelper::PARAM_NAME_DOF("OurName");
      const std::string ArticulationHelper::PARAM_NAME_DOF_PARENT("OurParent");
      const std::string ArticulationHelper::PARAM_NAME_DOF_ROOT("other"); // default enumerated value in VisitMapping.xml

      //////////////////////////////////////////////////////////////////////////
      ArticulationHelper::ArticulationHelper()
         : mIsDirty(false)
         , mArticArrayPropName(PROPERTY_NAME_ARTICULATED_ARRAY)
         , mPublishReverseHeading(false)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      ArticulationHelper::~ArticulationHelper()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ArticulationHelper::HandleUpdatedDOF( const osgSim::DOFTransform& dof,
         const osg::Vec3& transChange, const osg::Vec3& hprChange, const osg::Vec3& hprCurrent )
      {
         HandleUpdatedDOFTranslation( dof, transChange );
         HandleUpdatedDOFOrientation( dof, hprChange, hprCurrent );
      }

      //////////////////////////////////////////////////////////////////////////
      bool ArticulationHelper::AddArticulatedParameter(
         dtDAL::NamedGroupParameter& outArticArrayProp,
         ArticulationMetricType& articulationType,
         const std::string& paramName,
         float value, unsigned valueChangeCount,
         float rate, unsigned rateChangeCount,
         const std::string& dofName, const std::string& dofParentName )
      {
         if( articulationType == ArticulationMetricType::ARTICULATE_UNKNOWN )
            return false;

         std::string extendedParamName( PARAM_NAME_PREFIX_ARTICULATED + paramName );

         // Add the value
         dtCore::RefPtr<dtDAL::NamedGroupParameter> articParam;
         // --- Create the parameter if it does not already exist
         articParam = new dtGame::GroupMessageParameter(extendedParamName);
         articParam->AddParameter( *new dtDAL::NamedFloatParameter( articulationType.GetName(), value ) );
         articParam->AddParameter( *new dtDAL::NamedUnsignedShortIntParameter( PARAM_NAME_CHANGE, valueChangeCount ) );
         articParam->AddParameter( *new dtDAL::NamedStringParameter( PARAM_NAME_DOF, dofName ) );
         articParam->AddParameter( *new dtDAL::NamedStringParameter( PARAM_NAME_DOF_PARENT, dofParentName ) );
         outArticArrayProp.AddParameter( *articParam );

         // Add the rate
         extendedParamName = extendedParamName + PARAM_NAME_SUFFIX_RATE;
         // --- Create the parameter if it does not already exist
         articParam = new dtGame::GroupMessageParameter(extendedParamName);
         articParam->AddParameter( *new dtDAL::NamedFloatParameter(
            articulationType.GetRelatedRateMetricName(), rate ) );
         articParam->AddParameter( *new dtDAL::NamedUnsignedShortIntParameter( PARAM_NAME_CHANGE, rateChangeCount ) );
         articParam->AddParameter( *new dtDAL::NamedStringParameter( PARAM_NAME_DOF, dofName ) );
         articParam->AddParameter( *new dtDAL::NamedStringParameter( PARAM_NAME_DOF_PARENT, dofParentName ) );
         outArticArrayProp.AddParameter( *articParam );

         return true;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      // Function moved from Entity and modified to fit the helper
      class DRDOFDataFromHLA : public osg::Referenced
      {
         public:
            bool mRemove;
            osg::Vec3 mVelocity;
            osg::Vec3 mPosition;
            std::string mMetricName;

            DRDOFDataFromHLA()
               : mRemove(false)
            {
            }

         protected:
            virtual ~DRDOFDataFromHLA() {}
      };

      ////////////////////////////////////////////////////////////////////////////////////
      void ArticulationHelper::HandleArticulatedParametersArray(
         const dtDAL::NamedGroupParameter& articArrayParam,
         dtUtil::NodeCollector& nodeCollector, dtGame::DeadReckoningHelper& deadReckoningHelper )
      {
         if(nodeCollector.GetTransformNodeMap().empty())
         {
            return;
         }

         typedef std::map<std::string,dtCore::RefPtr<DRDOFDataFromHLA> > NameToDOFDataMap;
         NameToDOFDataMap toSendList;
         std::vector<const dtGame::MessageParameter*> toFill;
         articArrayParam.GetParameters(toFill);
         std::vector<const dtGame::MessageParameter*>::iterator iter = toFill.begin();

         for(;iter != toFill.end(); ++iter)
         {
            if((*iter)->GetDataType() == dtDAL::DataType::GROUP)
            {
               char switchLetter = (*iter)->GetName()[1]; // "ArticulatedPartMessageParam"
               if(switchLetter == 'r')
               {
                  bool isNotRate = false;
                  float value = 0.0f;
                  std::string dofName;
                  osgSim::DOFTransform *dof;
                  const dtGame::GroupMessageParameter& curGroupParam
                     = *static_cast<const dtGame::GroupMessageParameter*>(*iter);

                  if(GetArticulationDOFName(curGroupParam, dofName))
                  {
                     if((dof = nodeCollector.GetDOFTransform(dofName)) != NULL)
                     {
                        dtCore::RefPtr<DRDOFDataFromHLA> dofData = NULL;
                        NameToDOFDataMap::iterator iterDRDOF = toSendList.find(dofName);

                        bool hadToMakeNew = iterDRDOF == toSendList.end();

                        if( ! hadToMakeNew )
                        {
                           dofData = iterDRDOF->second.get();
                        }
                        else
                        {
                           dofData = new DRDOFDataFromHLA;
                        }

                        osg::Vec3::value_type* dataMetricField = NULL;
                        const std::string* metricName = NULL;
                        if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_AZIMUTH, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_AZIMUTH;
                           dataMetricField = &dofData->mPosition[0];
                           isNotRate = true;
                        }
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_AZIMUTHRATE, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_AZIMUTHRATE;
                           dataMetricField = &dofData->mVelocity[0];
                        }
                        else if(GetArticulation(curGroupParam, dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ELEVATION, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ELEVATION;
                           dataMetricField = &dofData->mPosition[1];
                           isNotRate = true;
                        }
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ELEVATIONRATE, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ELEVATIONRATE;
                           dataMetricField = &dofData->mVelocity[1];
                        }
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ROTATION, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ROTATION;
                           dataMetricField = &dofData->mPosition[2];
                           isNotRate = true;
                        }
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ROTATIONRATE, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ROTATIONRATE;
                           dataMetricField = &dofData->mVelocity[2];
                        }
                        /*else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_POSITION, value))
                        {}
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_POSITIONRATE, value))
                        {}*/
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_EXTENSION, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_EXTENSION;
                           dataMetricField = &dofData->mPosition[1];
                           isNotRate = true;
                        }
                        /*else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_EXTENSIONRATE, value))
                        {
                           metricName = &dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_EXTENSIONRATE;
                           dataMetricField = &dofData->mVelocity[1];
                        }
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_X, value))
                        {}
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_XRATE, value))
                        {}
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_Y, value))
                        {}
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_YRATE, value))
                        {}
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_Z, value))
                        {}
                        else if(GetArticulation(curGroupParam,  dtGame::DeadReckoningHelper::DeadReckoningDOF::REPRESENATION_ZRATE, value))
                        {}*/

                        // Determine if this helper owns the metric to the specified DOF.
                        // If so, avoid sending it to the dead reckoning helper.
                        if( ! HasDOFMetric( *dof, *metricName ) )
                        {
                           // Apply the value change to the DOF data object.
                           if( dataMetricField != NULL )
                           {
                              *dataMetricField = value;

                              // Record the name of the non-rate part of the metric.
                              if( isNotRate )
                              {
                                 dofData->mMetricName = *metricName;
                              }
                           }
                        }
                        else
                        {
                           dofData->mRemove = true;
                        }

                        // Add the metric data to the list if it was not already there.
                        if(hadToMakeNew)
                        {
                           // Add the captured data to list to be fed to the
                           // dead reckoning helper.
                           toSendList.insert(std::make_pair(dofName, dofData));
                        }
                     }
                  }
               }
               else if(switchLetter == 't') // "AttachedPartMessageParam"
               {
                  // handle attachment
               }
            }
         }

         // after first for loop go through and delete all old messages
         for(NameToDOFDataMap::iterator iterDRDOF = toSendList.begin();
            iterDRDOF != toSendList.end();
            ++iterDRDOF)
         {
            if( iterDRDOF->second.valid() )
            {
               if( iterDRDOF->second->mRemove )
               {
                  deadReckoningHelper.RemoveAllDRDOFByName( iterDRDOF->first );
               }
               else
               {
                  deadReckoningHelper.AddToDeadReckonDOF(iterDRDOF->first,
                     iterDRDOF->second->mPosition, iterDRDOF->second->mVelocity,
                     iterDRDOF->second->mMetricName);
               }
            }
         }
         toSendList.clear();
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool ArticulationHelper::GetArticulation(const dtGame::GroupMessageParameter& articParam,
         const std::string& metricTypeName, float& outValue)
      {
         const dtGame::FloatMessageParameter* metricParam
            = dynamic_cast<const dtGame::FloatMessageParameter*>
            (articParam.GetParameter(metricTypeName));

         if(metricParam != NULL)
         {
            outValue = metricParam->GetValue();
            return true;
         }
         return false;
      }

      ////////////////////////////////////////////////////////////////////////////////////
      bool ArticulationHelper::GetArticulationDOFName(
         const dtGame::GroupMessageParameter& articParam, std::string& outName)
      {
         const dtGame::StringMessageParameter* dofNameParam
            = dynamic_cast<const dtGame::StringMessageParameter*>
            (articParam.GetParameter(SimCore::Components::ArticulationHelper::PARAM_NAME_DOF));

         if(dofNameParam != NULL)
         {
            outName = dofNameParam->GetValue();
            return true;
         }
         return false;
      }
   }
}
