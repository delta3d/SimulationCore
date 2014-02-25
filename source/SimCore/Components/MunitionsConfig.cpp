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

#include <prefix/SimCorePrefix.h>

#include <xercesc/framework/LocalFileInputSource.hpp>
#if XERCES_VERSION_MAJOR < 3
#   include <xercesc/internal/XMLGrammarPoolImpl.hpp>
#endif
#include <xercesc/sax2/XMLReaderFactory.hpp>

#include <dtUtil/exception.h>
#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/datapathutils.h>

#include <dtCore/scene.h>

#include <dtCore/project.h>

#include <SimCore/Components/DamageHelper.h>
#include <SimCore/Components/MunitionsComponent.h>
#include <SimCore/Components/MunitionsConfig.h>

namespace SimCore
{
   namespace Components
   {

      //////////////////////////////////////////////////////////////////////////
      // String Constants
      //////////////////////////////////////////////////////////////////////////
      const std::string MunitionsConfig::LOG_NAME("MunitionsConfig.cpp");
      const std::string MunitionsConfig::A_NAME("name");
      const std::string MunitionsConfig::E_ACCUMULATION("accumulationFactor");  // Accumulation Factor
      const std::string MunitionsConfig::E_ANGLE_FALL("angleOfFall");    // Angle-Of-Fall
      const std::string MunitionsConfig::E_DAMAGE_PROB("damageProbability");  // Damage Probability
      const std::string MunitionsConfig::E_DAMAGE_NONE("none");
      const std::string MunitionsConfig::E_DAMAGE_N("N");     // same as NONE
      const std::string MunitionsConfig::E_DAMAGE_MOBILITY("mobility");
      const std::string MunitionsConfig::E_DAMAGE_M("M");     // same as MOBILITY
      const std::string MunitionsConfig::E_DAMAGE_FIRE("firepower");  // Firepower
      const std::string MunitionsConfig::E_DAMAGE_F("F");     // same as FIRE
      const std::string MunitionsConfig::E_DAMAGE_MOBILITY_FIRE("mobilityFirepower");  // MobilityFirepower
      const std::string MunitionsConfig::E_DAMAGE_MF("MF");    // same as MOBILITY_FIRE
      const std::string MunitionsConfig::E_DAMAGE_KILL("kill");
      const std::string MunitionsConfig::E_DAMAGE_K("K");     // same as KILL
      const std::string MunitionsConfig::E_ENTITY("entityClass");        // Entity Class
      const std::string MunitionsConfig::E_FORCE("force");
      const std::string MunitionsConfig::E_MUNITION("munition");
      const std::string MunitionsConfig::E_MUNITIONS_MAPPING("munitionsMapping");
      const std::string MunitionsConfig::E_RANGES_LETHAL("lethalRanges"); // Lethal Ranges
      const std::string MunitionsConfig::E_RANGE("range");
      const std::string MunitionsConfig::E_RANGE_FORWARD("forwardRange");
      const std::string MunitionsConfig::E_RANGE_DEFLECT("deflectRange");
      const std::string MunitionsConfig::E_RANGE_CUTOFF("rangeCutoff");
      const std::string MunitionsConfig::V_RANGE_1_3("1/3");
      const std::string MunitionsConfig::V_RANGE_2_3("2/3");
      const std::string MunitionsConfig::V_RANGE_MAX("Max");

      //////////////////////////////////////////////////////////////////////////
      // Munition Config Code
      //////////////////////////////////////////////////////////////////////////
      MunitionsConfig::MunitionsConfig() : dtCore::Base("MunitionsConfig"),
         mLevel(LEVEL_ROOT),
         mLastLevel(LEVEL_ROOT),
         mFoundTable(false)
      {
         try
         {
            xercesc::XMLPlatformUtils::Initialize();
         }
         catch (const xercesc::XMLException& toCatch)
         {
            //if this happens, something is very very wrong.
            char* message = xercesc::XMLString::transcode( toCatch.getMessage() );
            std::string msg(message);
            LOG_ERROR("Error during parser initialization!: "+ msg)
               xercesc::XMLString::release( &message );
         }

         mLogger = &dtUtil::Log::GetInstance(LOG_NAME);

         mXercesParser = xercesc::XMLReaderFactory::createXMLReader();

         mXercesParser->setFeature(xercesc::XMLUni::fgSAX2CoreValidation, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgXercesDynamic, false);

         mXercesParser->setFeature(xercesc::XMLUni::fgSAX2CoreNameSpaces, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgXercesSchema, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgXercesSchemaFullChecking, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgXercesValidationErrorAsFatal, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgSAX2CoreNameSpacePrefixes, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgXercesUseCachedGrammarInParse, true);
         mXercesParser->setFeature(xercesc::XMLUni::fgXercesCacheGrammarFromParse, true);

         std::string schemaFileName = dtCore::Project::GetInstance().GetResourcePath(dtCore::ResourceDescriptor("Configs:MunitionsConfig.xsd"));

         if (!dtUtil::FileUtils::GetInstance().FileExists(schemaFileName))
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__,
               "Error, unable to load required file \"Configs/MunitionsConfig.xsd\".  Aborting.");
         }

         XMLCh* schemaFileNameXMLCh = xercesc::XMLString::transcode(schemaFileName.c_str());
         xercesc::LocalFileInputSource inputSource(schemaFileNameXMLCh);
         //cache the schema
         mXercesParser->loadGrammar(inputSource, xercesc::Grammar::SchemaGrammarType, true);
         xercesc::XMLString::release(&schemaFileNameXMLCh);
      }

      //////////////////////////////////////////////////////////////////////////
      MunitionsConfig::~MunitionsConfig()
      {
         delete mXercesParser;
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned int MunitionsConfig::LoadMunitionTables( const std::string& filePath, 
         std::vector<dtCore::RefPtr<MunitionDamageTable> >& outTables )
      {
         unsigned int successes = 0;

         try
         {
            // TODO this should take a resource descriptor
            // Locate the xml file
            std::string path = dtUtil::FindFileInPathList(filePath);
            if (!dtUtil::FileUtils::GetInstance().FileExists(path))
            {
               mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__,
                  "XML configuration file \"%s\" not found.  Aborting.", filePath.c_str());
            }

            // Setup for parsing
            mXercesParser->setContentHandler(this);
            mXercesParser->setErrorHandler(this);
            mXercesParser->setEntityResolver(this);

            // Begin the parsing
            mXercesParser->parse(path.c_str());
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,  __LINE__, 
               "Parsing complete.\n");

            // Return all tables
            successes = mOutTables.size();
            std::vector<dtCore::RefPtr<MunitionDamageTable> >::iterator tableIter = mOutTables.begin();
            for( ; tableIter != mOutTables.end(); ++tableIter )
            {
               outTables.push_back( tableIter->get() );
            }
            mOutTables.clear();
         }
         catch (const xercesc::OutOfMemoryException&)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__, 
               "Ran out of memory parsing!");
         }
         catch (const xercesc::XMLException& toCatch)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__, 
               "Error during parsing! %ls :\n",
               toCatch.getMessage());
         }
         catch (const xercesc::SAXParseException& toCatch)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__, 
               "Error during SAX parsing! %ls :\n",
               toCatch.getMessage());
         }
         catch (const dtUtil::Exception& toCatch)
         {
            mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__, 
               "Unknown error! %ls :\n",
               toCatch.ToString().c_str());
         }

         return successes;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::startDocument()
      {
         mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
            "Parsing Munitions Config Document Started.\n");
         resetDocument();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::startElement
         (
         const XMLCh*  const  uri,
         const XMLCh*  const  localname,
         const XMLCh*  const  qname,
         const xercesc::Attributes& attrs
         )
      {
         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__, __LINE__,
               "Found element \"%s\" ", dtUtil::XMLStringConverter(localname).c_str());
         }

         dtUtil::XMLStringConverter localNameConverter(localname);
         std::string curElement(localNameConverter.c_str());

         mElements.push( curElement );

         // Update current parse level and any other states
         // This will also create data objects that need to
         // capture values from further parsing.
         Update( curElement, true );

         // Look for a name attribute
         if( IsNamedElement( curElement ) )
         {
            dtUtil::AttributeSearch as;
            dtUtil::AttributeSearch::ResultMap rMap = as(attrs);
            dtUtil::AttributeSearch::ResultMap::iterator itor = rMap.find(A_NAME);

            // If a name was pulled, send it to be processed.
            // The current parse level will control where the
            // name is set.
            if( itor != rMap.end() )
            {
               SetData( mLevel, curElement, itor->second );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::characters(const XMLCh* const chars, const XMLSize_t length)
      {
         dtUtil::XMLStringConverter charsConverter(chars);
         const std::string sChars(charsConverter.c_str());
         const std::string& curElement = mElements.top();

         SetData( mLevel, curElement, sChars );
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname)
      {
         dtUtil::XMLStringConverter localNameConverter(localname);

         const std::string& lname = mElements.top();
         const std::string curElement(localNameConverter.c_str());

         if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,  __LINE__,
               "Ending element: \"%s\"", lname.c_str());
         }

         if (lname != curElement)
         {
            std::ostringstream ss;
            ss << "Attempting to pop mElements off of stack and the element "
               << "at the top (" << lname << ") is not the same as the element ending ("
               << curElement << ").";
         }

         // TODO: parse

         else if (mLogger->IsLevelEnabled(dtUtil::Log::LOG_DEBUG))
         {
            mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,
               "Ending unknown element %s within the global context.",
               curElement.c_str()
               );
         }

         // Update current parse level and any other states.
         // This helps update to the correct level when exiting an element
         Update( curElement, false );

         mElements.pop();
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::endDocument()
      {
         mLogger->LogMessage(dtUtil::Log::LOG_DEBUG, __FUNCTION__,  __LINE__,
            "Parsing HLA FOM Configuration Document Ended.\n");
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsConfig::IsLeafElement( const std::string& element ) const
      {
         return element == E_DAMAGE_N
            || element == E_DAMAGE_M
            || element == E_DAMAGE_F
            || element == E_DAMAGE_MF
            || element == E_DAMAGE_K
            || element == E_ANGLE_FALL
            || element == E_ACCUMULATION
            || element == E_FORCE
            || element == E_RANGE_CUTOFF;
      }

      //////////////////////////////////////////////////////////////////////////
      bool MunitionsConfig::IsNamedElement( const std::string& element ) const
      {
         return element == E_ENTITY
            || element == E_MUNITION
            || element == E_RANGE;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::Update( const std::string& element, bool isStartElement )
      {
         mLastLevel = mLevel;

         if( element == E_MUNITIONS_MAPPING ) { mLevel = LEVEL_ROOT; }
         if( element == E_ENTITY ) { mLevel = LEVEL_ENTITY; }
         if( element == E_MUNITION ) { mLevel = LEVEL_MUNITION; }
         if( element == E_DAMAGE_PROB ) { mLevel = LEVEL_DAMAGE_PROBS; }
         if( element == E_RANGES_LETHAL ) { mLevel = LEVEL_LETHAL_RANGES; }
         if( element == E_RANGE ) { mLevel = LEVEL_SUB_RANGE; }
         if( element == E_RANGE_FORWARD ) { mLevel = LEVEL_RANGE_FORWARD; }
         if( element == E_RANGE_DEFLECT ) { mLevel = LEVEL_RANGE_DEFLECT; }

         // Determine if the parsing has just left a munition declaration
         if( element == E_MUNITION && ! isStartElement )
         {
            if( mCurMunitionDamage.valid() && mCurMunitionTable.valid() )
            {
               // Determine if the damage probabilities are Direct or Indirect
               if( mCurProbs.valid() )
               {
                  // Use the last used range name to determine if this
                  // munition is Indirect vs. Direct Fire.
                  if( ! mCurMunitionRangeName.empty() )
                  {
                     mCurMunitionDamage->SetIndirectFireProbabilities( *mCurProbs ); 
                     mCurMunitionRangeName = ""; // clear the range name.
                  }
                  else
                  {
                     mCurMunitionDamage->SetDirectFireProbabilities( *mCurProbs ); 
                  }
                  mCurProbs = NULL;
               }
               mCurMunitionTable->AddMunitionDamage( mCurMunitionDamage.get() );
               mCurMunitionDamage = NULL;
            }
         }
         // Determine if parsing is entering a munition declaration
         else if( element == E_MUNITION && isStartElement )
         {
            mCurMunitionDamage = new MunitionDamage("");
            mCurProbs = new DamageProbability(""); // This could be for Direct or Indirect fire.
         }
         // Determine if parsing is entering a new entity class declaration
         else if( element == E_ENTITY && isStartElement )
         {
            bool isDefault = mOutTables.empty();
            mCurMunitionTable = new MunitionDamageTable("", isDefault);
         }
         // Determine if parsing is leaving an entity class declaration
         else if( element == E_ENTITY && ! isStartElement )
         {
            if( mCurMunitionTable.valid() )
            {
               mOutTables.push_back( mCurMunitionTable.get() );
               mCurMunitionTable = NULL;
               mFoundTable = false;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::SetData( ParseLevel level, const std::string& element, const std::string& data )
      {
         // Avoid any processing since there is not table to capture data.
         if( ! mCurMunitionTable.valid() ) { return; }

         // Determine if the data was meant for the table itself
         if( level == LEVEL_ENTITY )
         {
            if( mCurMunitionTable.valid() )
            {
               if( ! mFoundTable )
               {
                  mCurMunitionTable->SetName( data );
                  mFoundTable = true;
               }
            }
         }
         // Most data that is set is passed to the MunitionDamage
         else if( mCurMunitionDamage.valid() )
         {
            if( level == LEVEL_MUNITION )
            {
               if( element == E_MUNITION )
               {
                  if(mCurMunitionDamage->GetName().empty())
                  {
                     mCurMunitionDamage->SetName( data );
                  }
               }
               else if( element == E_FORCE )
               {
                  mCurMunitionDamage->SetNewtonForce( atof( data.c_str() ) );
               }
               else if( element == E_ACCUMULATION )
               {
                  mCurMunitionDamage->SetAccumulationFactor( atof( data.c_str() ) );
               }
            }
            else if( level == LEVEL_DAMAGE_PROBS )
            {
               float value = atof( data.c_str() );
               if( element == E_DAMAGE_N ) { mCurProbs->SetNoDamage( value ); }
               else if( element == E_DAMAGE_M ) { mCurProbs->SetMobilityDamage( value ); }
               else if( element == E_DAMAGE_F ) { mCurProbs->SetFirepowerDamage( value ); }
               else if( element == E_DAMAGE_MF ) { mCurProbs->SetMobilityFirepowerDamage( value ); }
               else if( element == E_DAMAGE_K ) { mCurProbs->SetKillDamage( value ); }
            }
            else if( level == LEVEL_LETHAL_RANGES )
            {
               if( element == E_RANGE_CUTOFF )
               {
                  mCurMunitionDamage->SetCutoffRange( atof( data.c_str() ) );
               }
            }
            else if( level == LEVEL_SUB_RANGE )
            {
               if( data == V_RANGE_1_3 
                  || data == V_RANGE_2_3 
                  || data == V_RANGE_MAX )
               {
                  mCurMunitionRangeName = data;
               }
               else if( element == E_ANGLE_FALL )
               {
                  SetRangeData( level, element, mCurMunitionRangeName, data );
               }
            }
            else if( level == LEVEL_RANGE_FORWARD 
               || level == LEVEL_RANGE_DEFLECT )
            {
               SetRangeData( level, element, mCurMunitionRangeName, data );
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::SetRangeData( ParseLevel level, const std::string& element, 
         const std::string& rangeName, const std::string& data )
      {
         dtCore::RefPtr<DamageRanges> curDamageRanges;

         // Get or create a set of damage ranges specified by rangeName.
         if( rangeName == V_RANGE_1_3 )
         {
            curDamageRanges = mCurMunitionDamage->GetDamageRanges1_3();
            if( ! curDamageRanges.valid() )
            {
               curDamageRanges = new DamageRanges(V_RANGE_1_3);
               mCurMunitionDamage->SetDamageRanges1_3( curDamageRanges ); // copies the set ranges
               // Re-obtain the internal damage range pointer
               curDamageRanges = mCurMunitionDamage->GetDamageRanges1_3();
            }
         }
         else if( rangeName == V_RANGE_2_3 )
         {
            curDamageRanges = mCurMunitionDamage->GetDamageRanges2_3();
            if( ! curDamageRanges.valid() )
            {
               curDamageRanges = new DamageRanges(V_RANGE_2_3);
               mCurMunitionDamage->SetDamageRanges2_3( curDamageRanges ); // copies the set ranges
               // Re-obtain the internal damage range pointer
               curDamageRanges = mCurMunitionDamage->GetDamageRanges2_3();
            }
         }
         else if( rangeName == V_RANGE_MAX )
         {
            curDamageRanges = mCurMunitionDamage->GetDamageRangesMax();
            if( ! curDamageRanges.valid() )
            {
               curDamageRanges = new DamageRanges(V_RANGE_MAX);
               mCurMunitionDamage->SetDamageRangesMax( curDamageRanges ); // copies the set ranges
               // Re-obtain the internal damage range pointer
               curDamageRanges = mCurMunitionDamage->GetDamageRangesMax();
            }
         }

         // If this data if for angle-of-fall, set it and return
         if( level == LEVEL_SUB_RANGE && element == E_ANGLE_FALL )
         {
            curDamageRanges->SetAngleOfFall( atof(data.c_str()) );
            return;
         }

         // Get ready to capture a reference to Forward/Deflect ranges
         osg::Vec4* ranges = NULL;
         if( level == LEVEL_RANGE_FORWARD ) { ranges = &curDamageRanges->GetForwardRanges(); }
         if( level == LEVEL_RANGE_DEFLECT ) { ranges = &curDamageRanges->GetDeflectRanges(); }
         if( ranges == NULL ) { return; }

         // Set an individual range
         if( element == E_DAMAGE_M )
         {
            (*ranges)[0] = atof(data.c_str());
         }
         else if( element == E_DAMAGE_F )
         {
            (*ranges)[1] = atof(data.c_str());
         }
         else if( element == E_DAMAGE_MF )
         {
            (*ranges)[2] = atof(data.c_str());
         }
         else if( element == E_DAMAGE_K )
         {
            (*ranges)[3] = atof(data.c_str());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::error(const xercesc::SAXParseException& exc)
      {
         mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__,  __LINE__,
            "ERROR %d:%d - %s:%s - %s", exc.getLineNumber(),
            exc.getColumnNumber(), dtUtil::XMLStringConverter(exc.getPublicId()).c_str(),
            dtUtil::XMLStringConverter(exc.getSystemId()).c_str(),
            dtUtil::XMLStringConverter(exc.getMessage()).c_str());
         throw exc;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::fatalError(const xercesc::SAXParseException& exc)
      {
         mLogger->LogMessage(dtUtil::Log::LOG_ERROR, __FUNCTION__, __LINE__,
            "FATAL-ERROR %d:%d - %s:%s - %s", exc.getLineNumber(),
            exc.getColumnNumber(), dtUtil::XMLStringConverter(exc.getPublicId()).c_str(),
            dtUtil::XMLStringConverter(exc.getSystemId()).c_str(),
            dtUtil::XMLStringConverter(exc.getMessage()).c_str());
         throw exc;
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::warning(const xercesc::SAXParseException& exc)
      {
         mLogger->LogMessage(dtUtil::Log::LOG_WARNING, __FUNCTION__, __LINE__,
            "WARNING %d:%d - %s:%s - %s", exc.getLineNumber(),
            exc.getColumnNumber(), dtUtil::XMLStringConverter(exc.getPublicId()).c_str(),
            dtUtil::XMLStringConverter(exc.getSystemId()).c_str(),
            dtUtil::XMLStringConverter(exc.getMessage()).c_str());
      }

      //////////////////////////////////////////////////////////////////////////
      void MunitionsConfig::resetDocument()
      {
         resetErrors();
      }

   }
}
