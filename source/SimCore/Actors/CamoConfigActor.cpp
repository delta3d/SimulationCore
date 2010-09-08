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
#include <SimCore/Actors/CamoConfigActor.h>

// XML Loading
#include <stack>
#include <dtDAL/project.h>
#include <dtDAL/propertymacros.h>
#include <dtUtil/xercesparser.h>
#include <dtUtil/xercesutils.h>
#include <xercesc/sax2/ContentHandler.hpp>  // for a base class
#include <xercesc/util/XMLString.hpp>

#if XERCES_VERSION_MAJOR < 3
#ifndef XMLSize_t
#define XMLSize_t unsigned
#endif
#endif



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // CAMO CONFIG HANDLER CODE - For parsing the Camo Config information.
      //////////////////////////////////////////////////////////////////////////
      static const dtUtil::RefString ELEMENT_CAMO("Camo");
      static const dtUtil::RefString ELEMENT_COLOR1("Color1");
      static const dtUtil::RefString ELEMENT_COLOR2("Color2");
      static const dtUtil::RefString ELEMENT_COLOR3("Color3");
      static const dtUtil::RefString ELEMENT_COLOR4("Color4");
      static const dtUtil::RefString ELEMENT_PATTERN_SCALE("PatternScale");
      static const dtUtil::RefString ELEMENT_PATTERN("PatternTexture");
      static const dtUtil::RefString ELEMENT_CONCEAL_MESH("ConcealMesh");
      static const dtUtil::RefString ELEMENT_DEFAULT_PATTERN("DefaultPatternTexture");
      static const dtUtil::RefString ATTR_CAMO_NAME("name");
      static const dtUtil::RefString ATTR_CAMO_ID("id");
      static const dtUtil::RefString ATTR_COLOR_VALUE("value");

      //////////////////////////////////////////////////////////////////////////
      class CamoConfigHandler : public XERCES_CPP_NAMESPACE_QUALIFIER ContentHandler
      {
         public:

            CamoConfigHandler(CamoConfigActor* actor)
               : mConfigActor(actor)
            {
            }

            virtual ~CamoConfigHandler() {}

            // Inherited Abstract Methods            
            virtual void endDocument() {}
            virtual void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length) {}
            virtual void processingInstruction(const XMLCh* const target, const XMLCh* const data) {}
            virtual void setDocumentLocator(const XERCES_CPP_NAMESPACE_QUALIFIER Locator* const locator) {}
            virtual void startDocument() {}
            virtual void startPrefixMapping(const	XMLCh* const prefix,const XMLCh* const uri) {}
            virtual void endPrefixMapping(const XMLCh* const prefix) {}
            virtual void skippedEntity(const XMLCh* const name) {}

            // Overridden Methods
            virtual void characters(const XMLCh* const chars, const XMLSize_t length);

            virtual void startElement(const XMLCh* const uri,
               const XMLCh* const localname,
               const XMLCh* const qname,
               const XERCES_CPP_NAMESPACE_QUALIFIER Attributes& attrs);

            virtual void endElement(const XMLCh* const uri,
               const XMLCh* const localname,
               const XMLCh* const qname);

         private:
            typedef std::stack<std::string> ElementStack;
            ElementStack mElements;

            dtCore::RefPtr<CamoConfigActor> mConfigActor;
            dtCore::RefPtr<CamoParams> mCurCamoParams;
            dtDAL::ResourceDescriptor mDefaultPatternTexture;
      };



      //////////////////////////////////////////////////////////////////////////
      // CAMO PARAETERS CODE
      //////////////////////////////////////////////////////////////////////////
      CamoParams::CamoParams()
         : mId(0)
         , mPatternScale(1.0f)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      CamoParams::~CamoParams()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR(CamoParams, std::string, Name);
      DT_IMPLEMENT_ACCESSOR(CamoParams, CamoParams::CamoId, Id);
      DT_IMPLEMENT_ACCESSOR(CamoParams, float, PatternScale);
      DT_IMPLEMENT_ACCESSOR(CamoParams, osg::Vec4, Color1);
      DT_IMPLEMENT_ACCESSOR(CamoParams, osg::Vec4, Color2);
      DT_IMPLEMENT_ACCESSOR(CamoParams, osg::Vec4, Color3);
      DT_IMPLEMENT_ACCESSOR(CamoParams, osg::Vec4, Color4);
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoParams, dtDAL::ResourceDescriptor, PatternTexture); // setter implemented below
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoParams, dtDAL::ResourceDescriptor, ConcealMesh); // setter implemented below

      //////////////////////////////////////////////////////////////////////////
      void CamoParams::SetPatternTexture(const dtDAL::ResourceDescriptor& file)
      {
         mPatternTexture = file;
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoParams::SetConcealMesh(const dtDAL::ResourceDescriptor& file)
      {
         mConcealMesh = file;
      }



      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      CamoConfigActor::CamoConfigActor()
         : CamoConfigActor::BaseClass("CamoConfigActor")
      {
      }

      //////////////////////////////////////////////////////////////////////////
      CamoConfigActor::~CamoConfigActor()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ACCESSOR_GETTER(CamoConfigActor, std::string, ConfigFile); // setter implemented below.

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActor::SetConfigFile(const std::string& file)
      {
         mConfigFile = file;

         if( ! file.empty())
         {
            LoadConfigFile(mConfigFile);
         }
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned CamoConfigActor::GetCamoParamsList(CamoParamsList& outList)
      {
         CamoParamsMap::const_iterator curIter = mCamoMap.begin();
         CamoParamsMap::const_iterator endIter = mCamoMap.end();
         for( ; curIter != endIter; ++curIter)
         {
            outList.push_back(curIter->second.get());
         }

         return unsigned(outList.size());
      }

      //////////////////////////////////////////////////////////////////////////
      unsigned CamoConfigActor::GetCamoParamsCount() const
      {
         return unsigned(mCamoMap.size());
      }

      //////////////////////////////////////////////////////////////////////////
      const CamoParams* CamoConfigActor::GetCamoParamsByName(const std::string& name) const
      {
         const CamoParams* params = NULL;
         CamoParamsMap::const_iterator curIter = mCamoMap.begin();
         CamoParamsMap::const_iterator endIter = mCamoMap.end();

         for( ; curIter != endIter; ++curIter)
         {
            if(curIter->second->GetName() == name)
            {
               params = curIter->second.get();
               break;
            }
         }

         return params;
      }

      //////////////////////////////////////////////////////////////////////////
      const CamoParams* CamoConfigActor::GetCamoParamsByCamoId(CamoParams::CamoId id) const
      {
         const CamoParams* params = NULL;
         CamoParamsMap::const_iterator foundIter = mCamoMap.find(id);

         if(foundIter != mCamoMap.end())
         {
            params = foundIter->second.get();
         }

         return params;
      }

      //////////////////////////////////////////////////////////////////////////
      bool CamoConfigActor::AddCamoParams(CamoParams& params)
      {
         bool success = mCamoMap.insert(CamoParamsMapPair(params.GetId(), &params)).second;

         if( ! success)
         {
            const CamoParams* existingCamo = GetCamoParamsByCamoId(params.GetId());

            std::ostringstream oss;
            oss << "Could not add camo \"" << params.GetName() << "\" because ";
            if(existingCamo != NULL)
            {
               oss << "camo \"" << existingCamo->GetName()
                  << "\" already has id " << existingCamo->GetId() << ".\n\n";
            }
            else
            {
               oss << "of some unknown reason.\n\n";
            }
            LOG_WARNING(oss.str());
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool CamoConfigActor::RemoveCamoParams(CamoParams& params)
      {
         bool success = false;

         CamoParamsMap::iterator foundIter = mCamoMap.find(params.GetId());
         if(foundIter != mCamoMap.end())
         {
            mCamoMap.erase(foundIter);
            success = true;
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      bool CamoConfigActor::LoadConfigFile()
      {
         return LoadConfigFile(mConfigFile);
      }

      //////////////////////////////////////////////////////////////////////////
      bool CamoConfigActor::LoadConfigFile(const std::string& file)
      {
         bool success = false;

         try
         {
            std::string filePath = dtDAL::Project::GetInstance().GetResourcePath(file);
            
            if(dtUtil::FileUtils::GetInstance().FileExists(filePath))
            {
               // Remove old camo parameters.
               Clear();

               // Read in new camo parameters
               CamoConfigHandler handler(this);
               dtUtil::XercesParser parser;
               success = parser.Parse(filePath, handler);
            }
            else
            {
               LOG_WARNING("CamoConfigActor could not load the file \""+ file +"\"");
            }
         }
         catch(dtUtil::Exception& e)
         {
            LOG_ERROR(e.ToString());
         }

         return success;
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActor::Clear()
      {
         mCamoMap.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      osg::Node* CamoConfigActor::GetOSGNode()
      {
         return NULL;
      }
      
      //////////////////////////////////////////////////////////////////////////
      const osg::Node* CamoConfigActor::GetOSGNode() const
      {
         return NULL;
      }



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      const dtUtil::RefString CamoConfigActorProxy::CLASS_NAME("SimCore::Actors::CamoConfigActor");
      const dtUtil::RefString CamoConfigActorProxy::PROPERTY_CONFIG_FILE("Config File");

      //////////////////////////////////////////////////////////////////////////
      CamoConfigActorProxy::CamoConfigActorProxy()
      {
         SetClassName(CamoConfigActorProxy::CLASS_NAME);
      }

      //////////////////////////////////////////////////////////////////////////
      CamoConfigActorProxy::~CamoConfigActorProxy()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorProxy::CreateActor()
      {
         SetActor(*new CamoConfigActor());
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigActorProxy::BuildPropertyMap()
      {
         // NOTE: Not calling BaseClass::BuildPropertyMap since has unwanted properties.
         CamoConfigActor* actor = NULL;
         GetActor(actor);

         typedef dtDAL::PropertyRegHelper<CamoConfigActorProxy&, CamoConfigActor> PropRegType;
         PropRegType propRegHelper(*this, actor, "Camo Config");

         // FILE PROPERTIES (as string since resource types do not allow generic xml files)
         DT_REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            ConfigFile,
            PROPERTY_CONFIG_FILE,
            PROPERTY_CONFIG_FILE,
            "The file that contains the camo parameters for one or more camo patterns.",
            PropRegType, propRegHelper);
      }



      //////////////////////////////////////////////////////////////////////////
      // CAMO CONFIG HANDLER CODE - For parsing the Camo Config information.
      //////////////////////////////////////////////////////////////////////////
      osg::Vec4 ConvertToVec4(const std::string& value)
      {
         std::istringstream ss;
         osg::Vec4 defaultVec;
         ss.str(value);
         ss >> defaultVec.x();
         ss >> defaultVec.y();
         ss >> defaultVec.z();
         ss >> defaultVec.w();

         return defaultVec;
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigHandler::startElement(const XMLCh* const uri,
         const XMLCh* const localname,
         const XMLCh* const qname,
         const XERCES_CPP_NAMESPACE_QUALIFIER Attributes& attrs)
      {
         dtUtil::XMLStringConverter elementName(localname);
         std::string element = elementName.ToString();
         mElements.push(element);

         dtUtil::AttributeSearch attrsearch;
         dtUtil::AttributeSearch::ResultMap results = attrsearch(attrs);
         dtUtil::AttributeSearch::ResultMap::iterator iter = results.end();

         if(element == ELEMENT_CAMO)
         {
            mCurCamoParams = new CamoParams;
            mCurCamoParams->SetPatternTexture(mDefaultPatternTexture);

            iter = results.find(ATTR_CAMO_NAME);
            if(iter != results.end())
            {
               mCurCamoParams->SetName(iter->second);
            }

            iter = results.find(ATTR_CAMO_ID);
            if(iter != results.end())
            {
               CamoParams::CamoId id
                  = CamoParams::CamoId(dtUtil::ToUnsignedInt(iter->second));
               mCurCamoParams->SetId(id);
            }
         }
         else if(mCurCamoParams.valid())
         {
            iter = results.find(ATTR_COLOR_VALUE);

            if(element == ELEMENT_COLOR1)
            {
               if(iter != results.end())
               {
                  mCurCamoParams->SetColor1(ConvertToVec4(iter->second));
               }
            }
            else if(element == ELEMENT_COLOR2)
            {
               if(iter != results.end())
               {
                  mCurCamoParams->SetColor2(ConvertToVec4(iter->second));
               }
            }
            else if(element == ELEMENT_COLOR3)
            {
               if(iter != results.end())
               {
                  mCurCamoParams->SetColor3(ConvertToVec4(iter->second));
               }
            }
            else if(element == ELEMENT_COLOR4)
            {
               if(iter != results.end())
               {
                  mCurCamoParams->SetColor4(ConvertToVec4(iter->second));
               }
            }
            else if(element == ELEMENT_CONCEAL_MESH)
            {
               // Getting ready to set a mesh.
               // Nullify the default in case the mesh was not specified on purpose.
               dtDAL::ResourceDescriptor empty;
               mCurCamoParams->SetConcealMesh(empty);
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigHandler::endElement(const XMLCh* const uri,
         const XMLCh* const localname,
         const XMLCh* const qname)
      {
         dtUtil::XMLStringConverter elementName(localname);
         std::string element = elementName.ToString();
         mElements.pop();

         if(element == ELEMENT_CAMO)
         {
            if(mCurCamoParams.valid())
            {
               if( ! mConfigActor->AddCamoParams(*mCurCamoParams))
               {
                  std::ostringstream oss;
                  oss << "CamoConfigActor \"" << mConfigActor->GetName() << "\" ("
                     << mConfigActor->GetUniqueId().ToString() << ") could not add camo \""
                     << mCurCamoParams->GetName() <<"\" (" << mCurCamoParams->GetId() << ").\n\n";
                  LOG_WARNING(oss.str());
               }
               mCurCamoParams = NULL;
            }
         }
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoConfigHandler::characters(const XMLCh* const chars, const XMLSize_t length)
      {
         const std::string& element = mElements.top();

         // Is this text for some default pattern???
         if(element == ELEMENT_DEFAULT_PATTERN)
         {
            mDefaultPatternTexture = dtUtil::XMLStringConverter(chars).ToString();
         }
         // Otherwise, the text is for some part of the current camo parameters
         else if(mCurCamoParams.valid())
         {
            if(element == ELEMENT_PATTERN)
            {
               mCurCamoParams->SetPatternTexture(
                  dtUtil::XMLStringConverter(chars).ToString());
            }
            else if(element == ELEMENT_CONCEAL_MESH)
            {
               mCurCamoParams->SetConcealMesh(
                  dtUtil::XMLStringConverter(chars).ToString());
            }
            else if(element == ELEMENT_PATTERN_SCALE)
            {
               float value = dtUtil::ToFloat(dtUtil::XMLStringConverter(chars).ToString());
               mCurCamoParams->SetPatternScale(value);
            }
         }
      }

   }
}
