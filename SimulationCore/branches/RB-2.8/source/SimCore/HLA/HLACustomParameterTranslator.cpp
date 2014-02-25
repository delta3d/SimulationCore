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
 * @author Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <string>
#include <osg/Endian>

#include <dtCore/scene.h>
#include <dtHLAGM/onetoonemapping.h>
#include <dtHLAGM/distypes.h>
#include <dtHLAGM/objectruntimemappinginfo.h>
#include <dtHLAGM/onetoonemapping.h>
#include <dtHLAGM/onetomanymapping.h>

#include <dtDAL/namedparameter.h>

#include <dtUtil/log.h>
#include <SimCore/Actors/MunitionTypeActor.h>
#include <SimCore/Actors/ControlStateActor.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Array2DParser.h>
#include <SimCore/HLA/HLACustomParameterTranslator.h>

namespace SimCore
{
   namespace HLA
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

      // This attribute type is used in capturing 3-float structures, such
      // as position and velocity from the network that should not go
      // through any coordinate conversion.
      const HLACustomAttributeType HLACustomAttributeType::VEC3F_TYPE("VEC3F_TYPE", 1, 12 );
      const HLACustomAttributeType HLACustomAttributeType::VEC3D_TYPE("VEC3D_TYPE", 1, 24 );

      const HLACustomAttributeType HLACustomAttributeType::MILLISECOND_TIME_TYPE("MILLISECOND_TIME_TYPE", 1, 4 );

      // DYNAMIC ARRAY TYPES
      const HLACustomAttributeType HLACustomAttributeType::SHORT_ARRAY_2D_TYPE("SHORT_ARRAY_2D_TYPE", 1, (1024+4) ); // Size is NOT constant.
      const HLACustomAttributeType HLACustomAttributeType::FLOAT_ARRAY_2D_TYPE("FLOAT_ARRAY_2D_TYPE", 1, (1024+4) ); // Size is NOT constant.



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
         if (controlParams.empty() )
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
            if (curControlParam == NULL)
               continue;

            encodingControl.SetByGroupParameter( *curControlParam );
            encodingControl.Encode( &buffer[bufferOffset] );

            // Step forward into buffer to write the next control
            bufferOffset += controlSize;
            encodingControl.Clear();

            // Avoid buffer over flow
            if (maxSize < bufferOffset + controlSize )
               break;
         }

         // Round off buffer to the nearest multiple of control size
         if (maxSize % controlSize > 0 )
         {
            // The buffer offset should be indexing the byte after the last control.
            // The index should be the total buffer size.
            maxSize = bufferOffset;
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapFromGroupParamToControlArray(
         const dtHLAGM::AttributeType& type,
         char* buffer,
         size_t& maxSize,
         const dtDAL::NamedGroupParameter& parameter ) const
      {
         if (type == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE )
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
         if (limit == 0 )
            return;

         // Iterate through the buffer, appending new controls
         dtCore::RefPtr<dtDAL::NamedGroupParameter> controlParam;
         unsigned bufferOffset = 0;
         while( bufferOffset + controlSize <= size )
         {
            decodingControl.Decode( &buffer[bufferOffset] );
            controlParam = decodingControl.GetAsGroupParameter();

            if (controlParam.valid() && ! controlParam->GetName()->empty() )
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
         if (type == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE )
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

         if (parameterDataType != paramDef.GetGameType() )
         {
            mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,
               "Warning, the Message Parameter DataType for \"%s\" is \"%s\", but the mapping configuration says it should be \"%s\"",
               parameter.GetName().c_str(), parameterDataType.GetName().c_str(),
               paramDef.GetGameType().GetName().c_str());
         }

         // Outgoing Munition Type
         if (hlaType == HLACustomAttributeType::MUNITION_TYPE )
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

            if (! mMunitionTypeTable.valid() )
            {
               mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
                  "No MunitionTypeTable is present to map munition \"%s\" to a DIS identifier.",
                  munitionName.c_str());
               return;
            }

            const SimCore::Actors::MunitionTypeActor* munitionType
               = mMunitionTypeTable->GetMunitionType( munitionName );

            if (munitionType != NULL )
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
         else if (hlaType == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE
            || hlaType == HLACustomAttributeType::DISCRETE_CONTROL_ARRAY_TYPE)
         {
            if (parameterDataType == dtDAL::DataType::GROUP)
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
         else if(hlaType == HLACustomAttributeType::VEC3F_TYPE
            || hlaType == HLACustomAttributeType::VEC3D_TYPE )
         {
            MapFromParamToVec3(buffer, maxSize, parameter, parameter.GetDataType());
         }
         else if(hlaType == HLACustomAttributeType::MILLISECOND_TIME_TYPE)
         {
            MapFromParamToTime(buffer, maxSize, parameter, parameter.GetDataType());
         }
         else if(hlaType == HLACustomAttributeType::FLOAT_ARRAY_2D_TYPE)
         {
            MapFromParamToFloatArray2D(buffer, maxSize, parameter);
         }
         else if(hlaType == HLACustomAttributeType::SHORT_ARRAY_2D_TYPE)
         {
            MapFromParamToShortArray2D(buffer, maxSize, parameter);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapToMessageParameters(const char* buffer, size_t size,
         std::vector<dtCore::RefPtr<dtGame::MessageParameter> >& parameters,
         const dtHLAGM::OneToManyMapping& mapping) const
      {

         const dtHLAGM::AttributeType& hlaType = mapping.GetHLAType();

         if (parameters.empty())
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to map from HLA mapping %s because no message parameters were passed into the mapping method.",
               mapping.GetHLAName().c_str());
            return;
         }

         if (mapping.GetParameterDefinitions().empty())
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
            if (! mMunitionTypeTable.valid()
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

            // Log an error if the type was not found
            if (munitionType == NULL )
            {
               std::ostringstream oss;
               oss << "Munition \"" << dis << "\" could NOT be found in munition table." << std::endl;
               mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__, oss.str().c_str() );
            }
         }
         // Incoming Control Type Array
         else if (hlaType == HLACustomAttributeType::CONTINUOUS_CONTROL_ARRAY_TYPE
            || hlaType == HLACustomAttributeType::DISCRETE_CONTROL_ARRAY_TYPE)
         {
            if (parameterDataType == dtDAL::DataType::GROUP )
            {
               dtDAL::NamedGroupParameter& groupParam
                  = static_cast<dtDAL::NamedGroupParameter&>(parameter);
               MapToGroupParamFromControlArray( hlaType, buffer, size, groupParam );
            }
            else
            {
               std::ostringstream oss;
               oss << "HLA type: \"" << hlaType.GetName() << "\" may not by mapped to anything but a group parameter. "
                        "It is currently mapped to a \"" << parameterDataType.GetName() << "\"." << std::endl;
               mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__, oss.str().c_str() );
            }
         }
         else if(hlaType == HLACustomAttributeType::VEC3F_TYPE
            || hlaType == HLACustomAttributeType::VEC3D_TYPE)
         {
            MapToParamFromVec3(buffer, size, parameter, parameter.GetDataType());
         }
         else if(hlaType == HLACustomAttributeType::MILLISECOND_TIME_TYPE)
         {
            MapToParamFromTime(buffer, size, parameter, parameter.GetDataType());
         }
         else if(hlaType == HLACustomAttributeType::FLOAT_ARRAY_2D_TYPE)
         {
            MapToParamFromFloatArray2D(buffer, size, parameter);
         }
         else if(hlaType == HLACustomAttributeType::SHORT_ARRAY_2D_TYPE)
         {
            MapToParamFromShortArray2D(buffer, size, parameter);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const HLACustomAttributeType* DetermineHLAVec3AtributeType( unsigned bufferSize )
      {
         if (bufferSize >= 24 )
         {
            return &HLACustomAttributeType::VEC3D_TYPE;
         }
         else if (bufferSize >= 12 )
         {
            return &HLACustomAttributeType::VEC3F_TYPE;
         }
         return NULL;
      }

      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapToParamFromVec3(
         const char* buffer,
         const size_t size,
         dtGame::MessageParameter& parameter,
         const dtDAL::DataType& parameterDataType ) const
      {
         osg::Vec3d position;

         const HLACustomAttributeType* hlaType = DetermineHLAVec3AtributeType(size);

         if (hlaType == NULL)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to decode Vec3 because buffer size is i%",
               size);
            return;
         }

         if (*hlaType == HLACustomAttributeType::VEC3D_TYPE)
         {
            dtHLAGM::WorldCoordinate wc;
            wc.Decode(buffer);
            position.set(wc.GetX(), wc.GetY(), wc.GetZ());
         }
         else if (*hlaType == HLACustomAttributeType::VEC3F_TYPE)
         {
            dtHLAGM::VelocityVector wc;
            wc.Decode(buffer);
            position.set(wc.GetX(), wc.GetY(), wc.GetZ());
         }

         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
            "Vec3 has been decoded to %lf %lf %lf",
            position.x(), position.y(), position.z());

         if (parameterDataType == dtDAL::DataType::VEC3)
         {
            static_cast<dtGame::Vec3MessageParameter&>(parameter).SetValue(position);
         }
         else if (parameterDataType == dtDAL::DataType::VEC3D)
         {
            static_cast<dtGame::Vec3dMessageParameter&>(parameter).SetValue(position);
         }
         else if (parameterDataType == dtDAL::DataType::VEC3F)
         {
            static_cast<dtGame::Vec3fMessageParameter&>(parameter).SetValue(
               osg::Vec3f(position.x(), position.y(), position.z()));
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapFromParamToVec3(
         char* buffer,
         const size_t maxSize,
         const dtGame::MessageParameter& parameter,
         const dtDAL::DataType& parameterDataType) const
      {
         osg::Vec3d position;

         const HLACustomAttributeType* hlaType = NULL;

         if (maxSize >= 24 )
         {
            hlaType = &HLACustomAttributeType::VEC3D_TYPE;
         }
         else if (maxSize >= 12 )
         {
            hlaType = &HLACustomAttributeType::VEC3F_TYPE;
         }

         if (hlaType == NULL )
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
               "Unable to encode Vec3 because buffer size is i%",
               maxSize);
            return;
         }

         if (parameterDataType == dtDAL::DataType::VEC3)
         {
            position = static_cast<const dtGame::Vec3MessageParameter&>(parameter).GetValue();
         }
         else if (parameterDataType == dtDAL::DataType::VEC3F)
         {
            osg::Vec3f posTemp = static_cast<const dtGame::Vec3fMessageParameter&>(parameter).GetValue();
            position.x() = posTemp.x();
            position.y() = posTemp.y();
            position.z() = posTemp.z();
         }
         else if (parameterDataType == dtDAL::DataType::VEC3D)
         {
            osg::Vec3d posTemp = static_cast<const dtGame::Vec3dMessageParameter&>(parameter).GetValue();
            //We're loosing precision here if a Vec3 is not compiled as a vec3d, but the
            //coordinate converter doesn't support Vec3d directly.
            position.x() = posTemp.x();
            position.y() = posTemp.y();
            position.z() = posTemp.z();
         }
         else
         {
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
               "The incoming parameter \"%s\" is not of a supported type \"%s\" for conversion to \"%s\"",
               parameter.GetName().c_str(), parameterDataType.GetName().c_str(),
               HLACustomAttributeType::VEC3F_TYPE.GetName().c_str());
         }

         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
               "Vec3 is %f %f %f",
               position.x(), position.y(), position.z());
         }

         bool errorEncoding = false;

         if (*hlaType == HLACustomAttributeType::VEC3D_TYPE )
         {
            dtHLAGM::WorldCoordinate wc( position.x(), position.y(), position.z() );
            if (maxSize >= wc.EncodedLength())
               wc.Encode(buffer);
            else
               errorEncoding = true;
         }
         else if (*hlaType == HLACustomAttributeType::VEC3F_TYPE)
         {
            dtHLAGM::VelocityVector wc( position.x(), position.y(), position.z() );
            if (maxSize >= wc.EncodedLength())
               wc.Encode(buffer);
            else
               errorEncoding = true;
         }

         if (errorEncoding )
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
            "Not enough space was allocated in the buffer to convert",
            position.x(), position.y(), position.z());
      }

      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapToParamFromTime(
         const char* buffer,
         const size_t size,
         dtGame::MessageParameter& parameter,
         const dtDAL::DataType& parameterDataType ) const
      {
         if (size < HLACustomAttributeType::MILLISECOND_TIME_TYPE.GetEncodedLength())
         {
            return;
         }

         ///ewww, const cast, but the data stream won't use a non-const pointer.
         dtUtil::DataStream ds(const_cast<char *>(buffer), size, false);
         ds.SetForceLittleEndian(false);
         if (parameterDataType == dtDAL::DataType::DOUBLE)
         {
            dtGame::DoubleMessageParameter& dmp = static_cast<dtGame::DoubleMessageParameter&>(parameter);
            unsigned int value;
            ds >> value;
            dmp.SetValue(double(value) / 1000.0);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapFromParamToTime(
         char* buffer,
         size_t& maxSize,
         const dtGame::MessageParameter& parameter,
         const dtDAL::DataType& parameterDataType) const
      {
         if (maxSize < HLACustomAttributeType::MILLISECOND_TIME_TYPE.GetEncodedLength())
         {
            maxSize = 0;
            return;
         }

         //this needs a way to prevent resizing.
         dtUtil::DataStream ds(buffer, maxSize, false);
         ds.SetForceLittleEndian(false);
         ds.ClearBuffer();
         if (parameterDataType == dtDAL::DataType::DOUBLE)
         {
            const dtGame::DoubleMessageParameter& dmp = static_cast<const dtGame::DoubleMessageParameter&>(parameter);
            ds << (unsigned long)(dmp.GetValue() * 1000.0);
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapToParamFromFloatArray2D(
         const char* buffer,
         const size_t maxSize,
         dtGame::MessageParameter& parameter) const
      {
         const dtDAL::DataType& paramType = parameter.GetDataType();

         if (paramType == dtDAL::DataType::FLOAT
            || paramType == dtDAL::DataType::DOUBLE)
         {
            Array2DParser<float> floatArray;
            floatArray.Decode(buffer, maxSize);

            if (floatArray.GetRows()== 0)
            {
               return;
            }

            if (paramType == dtDAL::DataType::DOUBLE)
            {
               static_cast<dtGame::DoubleMessageParameter&>(parameter)
                  .SetValue( double(floatArray.GetValue(0,0)));
            }
            else // FLOAT
            {
               static_cast<dtGame::FloatMessageParameter&>(parameter)
                  .SetValue( floatArray.GetValue(0,0));
            }
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapFromParamToFloatArray2D(
         char* buffer,
         size_t& maxSize,
         const dtGame::MessageParameter& parameter ) const
      {
         const dtDAL::DataType& paramType = parameter.GetDataType();

         Array2DParser<float> floatArray;
         floatArray.SetColumns(1);

         std::vector<float>& arrayData = floatArray.GetData();

         float value = 0.0f;

         if( paramType == dtDAL::DataType::FLOAT )
         {
            value = static_cast<const dtGame::FloatMessageParameter&>
               (parameter).GetValue();
         }
         else if( paramType == dtDAL::DataType::DOUBLE )
         {
            value = float(static_cast<const dtGame::DoubleMessageParameter&>
               (parameter).GetValue());
         }

         arrayData.push_back(value);
         maxSize = floatArray.Encode(buffer, maxSize);
      }

      /////////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapToParamFromShortArray2D(
         const char* buffer,
         const size_t maxSize,
         dtGame::MessageParameter& parameter) const
      {
         const dtDAL::DataType& paramType = parameter.GetDataType();

         // Integral Types
         if (paramType == dtDAL::DataType::SHORTINT
            || paramType == dtDAL::DataType::USHORTINT
            || paramType == dtDAL::DataType::INT
            || paramType == dtDAL::DataType::UINT)
         {
            Array2DParser<short> shortArray;
            shortArray.Decode(buffer, maxSize);

            if (shortArray.GetRows()== 0)
            {
               return;
            }

            if (paramType == dtDAL::DataType::USHORTINT)
            {
               static_cast<dtGame::UnsignedShortIntMessageParameter&>(parameter)
                  .SetValue((unsigned short)(shortArray.GetValue(0, 0)));
            }
            else if (paramType == dtDAL::DataType::SHORTINT)
            {
               static_cast<dtGame::ShortIntMessageParameter&>(parameter)
                  .SetValue(shortArray.GetValue(0, 0));
            }
            else if (paramType == dtDAL::DataType::UINT)
            {
               static_cast<dtGame::UnsignedIntMessageParameter&>(parameter)
                  .SetValue((unsigned int)(shortArray.GetValue(0, 0)));
            }
            else // INT
            {
               static_cast<dtGame::IntMessageParameter&>(parameter)
                  .SetValue(int(shortArray.GetValue(0, 0)));
            }
         }
         // String Types
         else if (paramType == dtDAL::DataType::ENUMERATION)
         {
            // TODO:
         }
      }

      /////////////////////////////////////////////////////////////////////////////
      void HLACustomParameterTranslator::MapFromParamToShortArray2D(
         char* buffer,
         size_t& maxSize,
         const dtGame::MessageParameter& parameter) const
      {
         const dtDAL::DataType& paramType = parameter.GetDataType();

         Array2DParser<short> shortArray;
         shortArray.SetColumns(1);

         std::vector<short>& arrayData = shortArray.GetData();

         short value = 0;

         if (paramType == dtDAL::DataType::SHORTINT
            || paramType == dtDAL::DataType::USHORTINT
            || paramType == dtDAL::DataType::INT
            || paramType == dtDAL::DataType::UINT)
         {
            if (paramType == dtDAL::DataType::UINT)
            {
               value = short(static_cast<const dtGame::UnsignedIntMessageParameter&>
                  (parameter).GetValue());
            }
            else if (paramType == dtDAL::DataType::INT)
            {
               value = short(static_cast<const dtGame::IntMessageParameter&>
                  (parameter).GetValue());
            }
            else if (paramType == dtDAL::DataType::USHORTINT)
            {
               value = short(static_cast<const dtGame::UnsignedShortIntMessageParameter&>
                  (parameter).GetValue());
            }
            else // SHORTINT
            {
               value = static_cast<const dtGame::ShortIntMessageParameter&>
                  (parameter).GetValue();
            }
         }
         // String Types
         else if (paramType == dtDAL::DataType::ENUMERATION)
         {
            // TODO:
         }

         arrayData.push_back(value);
         maxSize = shortArray.Encode(buffer, maxSize);
      }

   }
}
