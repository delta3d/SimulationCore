/*
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 * Alion Science and Technology Corporation
 * 5365 Robin Hood Road
 * Norfolk, VA 23513
 * (757) 857-5670, www.alionscience.com
 * 
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/dvteprefix-src.h>
#include <osg/Endian>

#include <string>

#include <dtUtil/log.h>

#include <dtCore/scene.h>

#include <dtHLAGM/onetoonemapping.h>
#include <dtHLAGM/distypes.h>
#include <dtHLAGM/objectruntimemappinginfo.h>

#include <SimCore/Components/HLACustomParameterTranslator.h>
#include <SimCore/Components/MunitionsComponent.h>

#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/ControlStateActor.h>

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // Attribute Type Code
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_ENUM(HLACustomAttributeType);

      // This attribute type is similar to RPRAttributeType::ENTITY_TYPE
      // found in dtHLAGM::RPRParameterTranslator.
      const HLACustomAttributeType HLACustomAttributeType::MUNITION_TYPE("MUNITION_TYPE", 1, 8);

      // These attribute types are for control states.
      const HLACustomAttributeType HLACustomAttributeType::DISCRETE_CONTROL_ARRAY_TYPE("DISCRETE_CONTROL_ARRAY_TYPE", 1, 
         SimCore::Actors::DiscreteControl::CONTROL_BYTE_SIZE * 5 );
      const HLACustomAttributeType HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE("CONTINUOUS_CONTROL_ARRAY_TYPE", 1, 
         SimCore::Actors::ContinuousControl::CONTROL_BYTE_SIZE * 5 );



      //////////////////////////////////////////////////////////////////////////
      // Parameter Translator Code
      //////////////////////////////////////////////////////////////////////////
      HLACustomParameterTranslator::HLACustomParameterTranslator()
      {
         mLogger = &dtUtil::Log::GetInstance("parametertranslator.cpp");
      }

      //////////////////////////////////////////////////////////////////////////
      HLACustomParameterTranslator::~HLACustomParameterTranslator()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      template<class T_ControlType>
      void MapFromGroupParamToControlArray_Template(
         T_ControlType& encodingControl,
         char* buffer,
         size_t& maxSize,
         const dtDAL::NamedGroupParameter& parameter )
      {
         // Access all the sub-group parameters that represent the controls
         std::vector<const dtDAL::NamedParameter*> controlParams;
         parameter.GetParameters( controlParams );

         // Avoid any unnecessary processing
         if( controlParams.empty() )
         {
            maxSize = 0;
            return;
         }

         // Determine the type of control array that is being written
         const unsigned controlSize = encodingControl.GetByteSize();

         // Iterate through all controls and write them to the buffer
         const dtDAL::NamedGroupParameter* curControlParam = NULL;
         unsigned bufferOffset = 0;
         const unsigned limit = controlParams.size();
         maxSize = controlSize * limit;
         for( unsigned i = 0; i < limit; ++i )
         {
            // Obtain the current parameter as a group parameter.
            curControlParam = static_cast<const dtDAL::NamedGroupParameter*>(controlParams[i]);

            // Avoid processing this parameter if NULL. This should not happen.
            if( curControlParam == NULL)
               continue;

            encodingControl.SetByGroupParameter( *curControlParam );
            encodingControl.Encode( &buffer[bufferOffset] );

            // Step forward into buffer to write the next control
            bufferOffset += controlSize;
            encodingControl.Clear();

            // Avoid buffer over flow
            if( maxSize < bufferOffset + controlSize )
               break;
         }

         // Round off buffer to the nearest multiple of control size
         if( maxSize % controlSize > 0 )
         {
            // The buffer offset should be indexing the byte after the last control.
            // The index should be the total buffer size.
            maxSize = bufferOffset;
         }
      }

      void HLACustomParameterTranslator::MapFromGroupParamToControlArray(
         const dtHLAGM::AttributeType& type,
         char* buffer,
         size_t& maxSize,
         const dtDAL::NamedGroupParameter& parameter ) const
      {
         if( type == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE )
         {
            dtCore::RefPtr<SimCore::Actors::ContinuousControl> encodingControl = new SimCore::Actors::ContinuousControl;
            MapFromGroupParamToControlArray_Template( *encodingControl, buffer, maxSize, parameter );
         }
         else
         {
            dtCore::RefPtr<SimCore::Actors::DiscreteControl> encodingControl = new SimCore::Actors::DiscreteControl;
            MapFromGroupParamToControlArray_Template( *encodingControl, buffer, maxSize, parameter );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      template<class T_ControlType>
      void MapToGroupParamFromControlArray_Template(
         T_ControlType& decodingControl,
         const char* buffer, 
         const size_t size,
         dtDAL::NamedGroupParameter& parameter )
      {
         // Determine the control type size.
         const unsigned controlSize = decodingControl.GetByteSize(); 

         // Determine the number of controls to be read.
         const unsigned limit = size / controlSize;

         // Avoid processing a zero length buffer.
         // This is highly unlikely, but this is here for data quality assurance.
         if( limit == 0 )
            return;

         // Iterate through the buffer, appending new controls
         dtCore::RefPtr<dtDAL::NamedGroupParameter> controlParam;
         unsigned bufferOffset = 0;
         while( bufferOffset + controlSize <= size )
         {
            decodingControl.Decode( &buffer[bufferOffset] );
            controlParam = decodingControl.GetAsGroupParameter();

            if( controlParam.valid() )
            {
               parameter.AddParameter( *controlParam );
            }

            // Step forward into the buffer to read the next control
            bufferOffset += controlSize;
            decodingControl.Clear();
            controlParam = NULL;
         }
      }

      void HLACustomParameterTranslator::MapToGroupParamFromControlArray(
         const dtHLAGM::AttributeType& type,
         const char* buffer, 
         const size_t size,
         dtDAL::NamedGroupParameter& parameter ) const
      {
         if( type == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE )
         {
            dtCore::RefPtr<SimCore::Actors::ContinuousControl> decodingControl = new SimCore::Actors::ContinuousControl;
            MapToGroupParamFromControlArray_Template( *decodingControl, buffer, size, parameter );
         }
         else
         {
            dtCore::RefPtr<SimCore::Actors::DiscreteControl> decodingControl = new SimCore::Actors::DiscreteControl;
            MapToGroupParamFromControlArray_Template( *decodingControl, buffer, size, parameter );
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const dtHLAGM::AttributeType& HLACustomParameterTranslator::GetAttributeTypeForName(const std::string& name) const
      {
         dtUtil::Enumeration* enumVal = HLACustomAttributeType::GetValueForName(name);

         if (enumVal == NULL)
            return dtHLAGM::AttributeType::UNKNOWN;
         else
            return static_cast<const dtHLAGM::AttributeType&>(*enumVal);
      }

      //////////////////////////////////////////////////////////////////////////
      bool HLACustomParameterTranslator::TranslatesAttributeType(const dtHLAGM::AttributeType& type) const
      {
         return dynamic_cast<const HLACustomAttributeType*>(&type) != NULL;
      }


      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapFromMessageParameters(char* buffer, size_t& maxSize,
         std::vector<dtCore::RefPtr<const dtGame::MessageParameter> >& parameters, 
         const dtHLAGM::OneToManyMapping& mapping) const
      {
         const dtHLAGM::AttributeType& hlaType = mapping.GetHLAType();

         if (parameters.size() == 0)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to map from Game to HLA mapping %s because no message parameters were passed into the mapping method.",
               mapping.GetHLAName().c_str());

            return;
         }

         if (mapping.GetParameterDefinitions().size() == 0)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to map to HLA mapping %s because no parameter mappings are added to the mapping object.",
               mapping.GetHLAName().c_str());
            return;
         }

         // All the current mappings use only one parameter.
         const dtGame::MessageParameter& parameter = *parameters[0];
         const dtDAL::DataType& parameterDataType  = parameter.GetDataType();
         const dtHLAGM::OneToManyMapping::ParameterDefinition& paramDef = mapping.GetParameterDefinitions()[0];

         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
            "Mapping values from game mapping %s to HLA mapping %s.",
            paramDef.GetGameName().c_str(), mapping.GetHLAName().c_str());

         parameter.WriteToLog(*mLogger);

         if( parameterDataType != paramDef.GetGameType() )
         {
            mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,
               "Warning, the Message Parameter DataType for \"%s\" is \"%s\", but the mapping configuration says it should be \"%s\"",
               parameter.GetName().c_str(), parameterDataType.GetName().c_str(), 
               paramDef.GetGameType().GetName().c_str());
         }

         // Outgoing Munition Type
         if( hlaType == HLACustomAttributeType::MUNITION_TYPE )
         {
            if (parameter.GetDataType() != dtDAL::DataType::STRING)
            {
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "The incoming parameter \"%s\" is not of a supported type \"%s\" for conversion to a DIS identifier."
                  "Only string parameters are supported.",
                  parameter.GetName().c_str(), parameter.GetDataType().GetName().c_str());
               return;
            } 

            const std::string& munitionName = static_cast<const dtGame::StringMessageParameter&>(parameter).GetValue();

            if( ! mMunitionTypeTable.valid() )
            {
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "No MunitionTypeTable is present to map munition \"%s\" to a DIS identifier.",
                  munitionName.c_str());
               return;
            }

            const SimCore::Actors::MunitionTypeActor* munitionType 
               = mMunitionTypeTable->GetMunitionType( munitionName );

            if( munitionType != NULL )
            {
               const SimCore::Actors::DISIdentifier& extendedDis = munitionType->GetDISIdentifier();
               
               dtHLAGM::EntityType dis( extendedDis.GetKind(), extendedDis.GetDomain(), 
                  extendedDis.GetCountry(), extendedDis.GetCategory(),
                  extendedDis.GetSubcategory(), extendedDis.GetSpecific(), 
                  extendedDis.GetExtra());

               dis.Encode(buffer);
            }
         }
         // Outgoing Control Type Array
         else if( hlaType == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE
            || hlaType == HLACustomAttributeType::DISCRETE_CONTROL_ARRAY_TYPE )
         {
            if( parameterDataType == dtDAL::DataType::GROUP )
            {
               const dtDAL::NamedGroupParameter& groupParam 
                  = static_cast<const dtDAL::NamedGroupParameter&>(parameter);
               MapFromGroupParamToControlArray( hlaType, buffer, maxSize, groupParam );
            }
            else
            {
               // TODO: Log Error
            }
         }
      }
         
      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapToMessageParameters(const char* buffer, size_t size,
         std::vector<dtCore::RefPtr<dtGame::MessageParameter> >& parameters, 
         const dtHLAGM::OneToManyMapping& mapping) const
      {
         
         const dtHLAGM::AttributeType& hlaType = mapping.GetHLAType();

         if (parameters.size() == 0)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to map from HLA mapping %s because no message parameters were passed into the mapping method.",
               mapping.GetHLAName().c_str());
            return;
         }

         if (mapping.GetParameterDefinitions().size() == 0)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to map from HLA mapping %s because no parameter mappings are added to the mapping object.",
               mapping.GetHLAName().c_str());
            return;
         }

         // All the current mappings use only one parameter.
         dtGame::MessageParameter& parameter = *parameters[0];
         const dtHLAGM::OneToManyMapping::ParameterDefinition& paramDef = mapping.GetParameterDefinitions()[0];

         const dtDAL::DataType& parameterDataType = parameter.GetDataType();

         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
               "Mapping values from HLA mapping %s to game mapping %s",
               mapping.GetHLAName().c_str(), paramDef.GetGameName().c_str());

         // Incoming Munition Type
         if ( hlaType == HLACustomAttributeType::MUNITION_TYPE )
         {
            // Ensure that there is a table to look up a munition name
            if( ! mMunitionTypeTable.valid() 
               || parameterDataType != dtDAL::DataType::STRING ) { return; }

            // Read in DIS binary data
            dtHLAGM::EntityType dis;
            dis.Decode(buffer);

            SimCore::Actors::DISIdentifier disExtended( dis.GetKind(), dis.GetDomain(), dis.GetCountry(),
               dis.GetCategory(), dis.GetSubcategory(), dis.GetSpecific(), dis.GetExtra() );

            const SimCore::Actors::MunitionTypeActor* munitionType 
               = mMunitionTypeTable->GetMunitionTypeByDIS( disExtended );

            static_cast<dtGame::StringMessageParameter&>(parameter).SetValue(
               munitionType != NULL ? munitionType->GetName() : "" );
         }
         // Incoming Control Type Array
         else if( hlaType == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE
            || hlaType == HLACustomAttributeType::DISCRETE_CONTROL_ARRAY_TYPE )
         {
            if( parameterDataType == dtDAL::DataType::GROUP )
            {
               dtDAL::NamedGroupParameter& groupParam 
                  = static_cast<dtDAL::NamedGroupParameter&>(parameter);
               MapToGroupParamFromControlArray( hlaType, buffer, size, groupParam );
            }
            else
            {
               // TODO: Log Error
            }
         }
      }
       
   }
}
