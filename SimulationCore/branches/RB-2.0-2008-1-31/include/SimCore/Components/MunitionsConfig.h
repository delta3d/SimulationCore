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

#ifndef _MUNITION_CONFIG_H_
#define _MUNITION_CONFIG_H_

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/ContentHandler.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
namespace xercesc_dt = XERCES_CPP_NAMESPACE;

#include <SimCore/Export.h>

#include <dtCore/base.h>
#include <dtUtil/xercesutils.h>
#include <stack>


namespace dtUtil
{
   class Log;
}

namespace SimCore
{
   namespace Components
   {
      class DamageProbability;
      class MunitionDamage;
      class MunitionDamageTable;

      //////////////////////////////////////////////////////////////////////////
      // Munition Config Code
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT MunitionsConfig : public xercesc_dt::ContentHandler, 
         public xercesc_dt::ErrorHandler, public xercesc_dt::EntityResolver, public dtCore::Base
      {
      public:
         MunitionsConfig();

         unsigned int LoadMunitionTables( const std::string& filePath, 
            std::vector<dtCore::RefPtr<MunitionDamageTable> >& outTables );



         // Error Handler inherited interface:
         virtual void error(const xercesc_dt::SAXParseException& exc);
         virtual void fatalError(const xercesc_dt::SAXParseException& exc);
         virtual void warning(const xercesc_dt::SAXParseException& exc);
         virtual void resetErrors() {}
         


         // Content Handler inherited interface:
         virtual void startDocument();

         virtual void startElement
            (
            const XMLCh*  const  uri,
            const XMLCh*  const  localname,
            const XMLCh*  const  qname,
            const xercesc_dt::Attributes& attrs
            );
         virtual void endElement(
            const XMLCh* const uri, 
            const XMLCh* const localname, 
            const XMLCh* const qname);

         virtual void endDocument();

         virtual void characters(const XMLCh* const chars, const unsigned int length);

         virtual void resetDocument();


         // NOTE: The following group of functions override inherited pure virtual functions
         // and will thus allow instancing of this class.
         virtual void ignorableWhitespace(const XMLCh* const chars, const unsigned int length) {}
         virtual void processingInstruction(const   XMLCh* const target, const XMLCh* const   data) {}
         virtual void startPrefixMapping(const XMLCh* const prefix, const XMLCh* const uri){}
         virtual void endPrefixMapping(const XMLCh* const prefix){}
         virtual void skippedEntity(const XMLCh* const name){}
         virtual void setDocumentLocator(const xercesc_dt::Locator* const locator){}

         // Entity Resolver inherited interface:
         virtual xercesc_dt::InputSource* resolveEntity(
            const XMLCh* const publicId,
            const XMLCh* const systemId) { return NULL; }

      protected:

         enum ParseLevel
         {
            // NOTE: Trailing comments list the elements at
            // the specified parse level.
            // Leaf elements should not have a level assigned.
            LEVEL_ROOT, // entityClass
            LEVEL_ENTITY, // munition
            LEVEL_MUNITION, // force, accumulationFactor, damageProbability, lethalRanges
            LEVEL_DAMAGE_PROBS, // N, M, F, MF, K
            LEVEL_LETHAL_RANGES, // rangeCutoff, range
            LEVEL_SUB_RANGE, // angleOfFall, forwardRange, deflectRange
            LEVEL_RANGE_FORWARD, // M, F, MF, K
            LEVEL_RANGE_DEFLECT // M, F, MF, K
         };

         virtual ~MunitionsConfig();

         bool IsLeafElement( const std::string& element ) const;

         bool IsNamedElement( const std::string& element ) const;

         void Update( const std::string& element, bool isStartElement );

         // This high-level function re-routes data
         void SetData( ParseLevel level, const std::string& element, const std::string& data );

         // NOTE: This is called by SetData.
         // This function handles the gritty work of choosing, creating, and setting
         // of individual damage ranges.
         void SetRangeData( ParseLevel level, const std::string& element, 
            const std::string& rangeName, const std::string& data );

      private:

         dtCore::RefPtr<dtUtil::Log> mLogger;
         xercesc_dt::SAX2XMLReader* mXercesParser;
         ParseLevel mLevel;
         ParseLevel mLastLevel;

         std::stack<std::string> mElements;
         bool mFoundTable;

         std::string mCurMunitionRangeName;

         dtCore::RefPtr<DamageProbability> mCurProbs;
         dtCore::RefPtr<MunitionDamage> mCurMunitionDamage;
         dtCore::RefPtr<MunitionDamageTable> mCurMunitionTable;
         std::vector<dtCore::RefPtr<MunitionDamageTable> > mOutTables;

         static const std::string LOG_NAME;

         // NOTE: Prefixes are as follows:
         //    E_ = Element
         //    A_ = Attribute
         //    V_ = Value
         static const std::string A_NAME;
         static const std::string E_ACCUMULATION;  // Accumulation Factor
         static const std::string E_ANGLE_FALL;    // Angle-Of-Fall
         static const std::string E_DAMAGE_PROB;  // Damage Probability
         static const std::string E_DAMAGE_NONE;
         static const std::string E_DAMAGE_N;     // same as NONE
         static const std::string E_DAMAGE_MOBILITY;
         static const std::string E_DAMAGE_M;     // same as MOBILITY
         static const std::string E_DAMAGE_FIRE;  // Firepower
         static const std::string E_DAMAGE_F;     // same as FIRE
         static const std::string E_DAMAGE_MOBILITY_FIRE;  // MobilityFirepower
         static const std::string E_DAMAGE_MF;    // same as MOBILITY_FIRE
         static const std::string E_DAMAGE_KILL;
         static const std::string E_DAMAGE_K;     // same as KILL
         static const std::string E_ENTITY;        // Entity Class
         static const std::string E_FORCE;
         static const std::string E_MUNITION;
         static const std::string E_MUNITIONS_MAPPING;
         static const std::string E_RANGES_LETHAL; // Lethal Ranges
         static const std::string E_RANGE;
         static const std::string E_RANGE_FORWARD;
         static const std::string E_RANGE_DEFLECT;
         static const std::string E_RANGE_CUTOFF;
         static const std::string V_RANGE_1_3;
         static const std::string V_RANGE_2_3;
         static const std::string V_RANGE_MAX;

      };

   }
}

#endif
