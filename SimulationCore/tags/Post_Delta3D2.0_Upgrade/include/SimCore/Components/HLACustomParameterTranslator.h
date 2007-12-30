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

#ifndef _MUNITION_PARAMETER_TRANSLATOR_H_
#define _MUNITION_PARAMETER_TRANSLATOR_H_



////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
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
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // DVTE ATTRIBUTE TYPE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HLACustomAttributeType : public dtHLAGM::AttributeType
      {
         DECLARE_ENUM(HLACustomAttributeType);

         public:
            static const HLACustomAttributeType MUNITION_TYPE;
            static const HLACustomAttributeType DISCRETE_CONTROL_ARRAY_TYPE;
            static const HLACustomAttributeType CONTINUOUS_CONTROL_ARRAY_TYPE;
            static const HLACustomAttributeType VEC3F_TYPE;
            static const HLACustomAttributeType VEC3D_TYPE;

         private:
            HLACustomAttributeType(const std::string& name, unsigned char id, size_t encodedLength)
               : AttributeType(name, id, encodedLength)
            {
               AddInstance(this);
            }

            virtual ~HLACustomAttributeType() {}
      };



      //////////////////////////////////////////////////////////////////////////
      // DVTE PARAMETER TRANSLATOR
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HLACustomParameterTranslator : public dtHLAGM::ParameterTranslator
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

            // This function allows the translator access to the table that
            // maps munition names to the munition DIS identifiers.
            // The table is loaded and managed by the MunitionsComponent.
            void SetMunitionTypeTable( MunitionTypeTable* table ) { mMunitionTypeTable = table; }
            const MunitionTypeTable* GetMunitionTypeTable() const { return mMunitionTypeTable.get(); }

         protected:
            dtUtil::Log* mLogger;

            virtual ~HLACustomParameterTranslator();

         private:
            // This is a weak pointer to the MunitionsComponent's loaded
            // MunitionTypeTable. This object holds the munition types
            // that map munition names to their network DIS identifiers.
            // This translator will use this table to convert munition names
            // to and from binary DIS identifiers.
            dtCore::ObserverPtr<MunitionTypeTable> mMunitionTypeTable;

      };

   }
}

#endif
