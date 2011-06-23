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

#ifndef _HLA_CUSTOM_PARAMETER_TRANSLATOR_H_
#define _HLA_CUSTOM_PARAMETER_TRANSLATOR_H_



////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/HLA/Export.h>
#include <dtCore/observerptr.h>
#include <dtHLAGM/parametertranslator.h>
#include <SimCore/Components/MunitionsComponent.h>


////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtUtil
{
   class Log;
}

namespace dtDAL
{
   class DataType;
   class NamedGroupParameter;
}

namespace dtHLAGM
{
   class ObjectRuntimeMappingInfo;
   class OneToManyMapping;
}

namespace SimCore
{
   namespace HLA
   {
      /**
       * @brief Sim Core additionl attribute types.
       */
      class SIMCORE_HLA_EXPORT HLACustomAttributeType : public dtHLAGM::AttributeType
      {
         DECLARE_ENUM(HLACustomAttributeType);

         public:
            static const HLACustomAttributeType MUNITION_TYPE;
            static const HLACustomAttributeType DISCRETE_CONTROL_ARRAY_TYPE;
            static const HLACustomAttributeType CONTINUOUS_CONTROL_ARRAY_TYPE;
            static const HLACustomAttributeType VEC3F_TYPE;
            static const HLACustomAttributeType VEC3D_TYPE;
            static const HLACustomAttributeType MILLISECOND_TIME_TYPE;
            static const HLACustomAttributeType SHORT_ARRAY_2D_TYPE;
            static const HLACustomAttributeType FLOAT_ARRAY_2D_TYPE;

         private:
            HLACustomAttributeType(const std::string& name, unsigned char id, size_t encodedLength)
               : AttributeType(name, id, encodedLength)
            {
               AddInstance(this);
            }

            virtual ~HLACustomAttributeType() {}
      };


      /**
       * @brief Sim Core additional parameter translator
       */
      class SIMCORE_HLA_EXPORT HLACustomParameterTranslator : public dtHLAGM::ParameterTranslator
      {
         public:
            HLACustomParameterTranslator();

            virtual void MapToMessageParameters(const char* buffer, size_t size,
               std::vector<dtCore::RefPtr<dtGame::MessageParameter> >& parameters,
               const dtHLAGM::OneToManyMapping& mapping) const;

            virtual void MapFromMessageParameters(char* buffer, size_t& maxSize,
               std::vector<dtCore::RefPtr<const dtGame::MessageParameter> >& parameters,
               const dtHLAGM::OneToManyMapping& mapping) const;

            virtual const dtHLAGM::AttributeType& GetAttributeTypeForName(const std::string& name) const;

            virtual bool TranslatesAttributeType(const dtHLAGM::AttributeType& type) const;

            void MapFromGroupParamToControlArray(
               const dtHLAGM::AttributeType& type,
               char* buffer,
               size_t& maxSize,
               const dtDAL::NamedGroupParameter& parameter ) const;

            void MapToGroupParamFromControlArray(
               const dtHLAGM::AttributeType& type,
               const char* buffer,
               const size_t size,
               dtDAL::NamedGroupParameter& parameter ) const;

            void MapToParamFromVec3(
               const char* buffer,
               const size_t size,
               dtGame::MessageParameter& parameter,
               const dtDAL::DataType& parameterDataType ) const;

            void MapFromParamToVec3(
               char* buffer,
               const size_t maxSize,
               const dtGame::MessageParameter& parameter,
               const dtDAL::DataType& parameterDataType) const;

            void MapToParamFromTime(
               const char* buffer,
               const size_t size,
               dtGame::MessageParameter& parameter,
               const dtDAL::DataType& parameterDataType ) const;

            void MapFromParamToTime(
               char* buffer,
               size_t& maxSize,
               const dtGame::MessageParameter& parameter,
               const dtDAL::DataType& parameterDataType) const;

            void MapToParamFromFloatArray2D(
               const char* buffer,
               const size_t maxSize,
               dtGame::MessageParameter& parameter) const;

            void MapFromParamToFloatArray2D(
               char* buffer,
               size_t& maxSize,
               const dtGame::MessageParameter& parameter) const;

            void MapToParamFromShortArray2D(
               const char* buffer,
               const size_t maxSize,
               dtGame::MessageParameter& parameter) const;

            void MapFromParamToShortArray2D(
               char* buffer,
               size_t& maxSize,
               const dtGame::MessageParameter& parameter) const;


            // This function allows the translator access to the table that
            // maps munition names to the munition DIS identifiers.
            // The table is loaded and managed by the MunitionsComponent.
            void SetMunitionTypeTable( SimCore::Components::MunitionTypeTable* table ) { mMunitionTypeTable = table; }
            const SimCore::Components::MunitionTypeTable* GetMunitionTypeTable() const { return mMunitionTypeTable.get(); }

         protected:
            dtUtil::Log* mLogger;

            virtual ~HLACustomParameterTranslator();

         private:
            // This is a weak pointer to the MunitionsComponent's loaded
            // MunitionTypeTable. This object holds the munition types
            // that map munition names to their network DIS identifiers.
            // This translator will use this table to convert munition names
            // to and from binary DIS identifiers.
            dtCore::ObserverPtr<SimCore::Components::MunitionTypeTable> mMunitionTypeTable;

      };

   }
}

#endif
