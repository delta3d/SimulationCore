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

#ifndef _CAMO_CONFIG_ACTOR_H_
#define _CAMO_CONFIG_ACTOR_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <osg/Vec4>
#include <dtUtil/getsetmacros.h>
#include <dtDAL/resourcedescriptor.h>
#include <dtGame/gameactor.h>



namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      // CAMO PARAETERS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT CamoParams : public osg::Referenced
      {
         public:
            typedef int CamoId;

            CamoParams();

            DECLARE_PROPERTY(std::string, Name);
            DECLARE_PROPERTY(CamoId, Id);
            DECLARE_PROPERTY(osg::Vec4, Color1);
            DECLARE_PROPERTY(osg::Vec4, Color2);
            DECLARE_PROPERTY(osg::Vec4, Color3);
            DECLARE_PROPERTY(osg::Vec4, Color4);
            DECLARE_PROPERTY(dtDAL::ResourceDescriptor, PatternTexture);

         protected:
            virtual ~CamoParams();
      };

      typedef std::list<dtCore::RefPtr<CamoParams> > CamoParamsList;



      //////////////////////////////////////////////////////////////////////////
      // ACTOR CODE
      //////////////////////////////////////////////////////////////////////////
      class CamoConfigActorProxy;
      class SIMCORE_EXPORT CamoConfigActor : public dtGame::GameActor
      {
         public:
            typedef dtGame::GameActor BaseClass;

            CamoConfigActor(CamoConfigActorProxy& proxy);

            DECLARE_PROPERTY(std::string, ConfigFile);

            unsigned GetCamoParamsList(CamoParamsList& outList);

            unsigned GetCamoParamsCount() const;

            const CamoParams* GetCamoParamsByName(const std::string& name) const;

            const CamoParams* GetCamoParamsByCamoId(CamoParams::CamoId id) const;

            bool AddCamoParams(CamoParams& params);
            bool RemoveCamoParams(CamoParams& params);

            bool LoadConfigFile();
            bool LoadConfigFile(const std::string& file);

            virtual void OnEnteredWorld();

            void Clear();

         protected:
            virtual ~CamoConfigActor();

         private:
            typedef std::map<CamoParams::CamoId, dtCore::RefPtr<CamoParams> > CamoParamsMap;
            typedef std::pair<CamoParams::CamoId, dtCore::RefPtr<CamoParams> > CamoParamsMapPair;
            CamoParamsMap mCamoMap;
      };



      //////////////////////////////////////////////////////////////////////////
      // PROXY CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT CamoConfigActorProxy : public dtGame::GameActorProxy
      {
         public:
            static const dtUtil::RefString CLASS_NAME;
            static const dtUtil::RefString PROPERTY_CONFIG_FILE;

            typedef dtGame::GameActorProxy BaseClass;

            CamoConfigActorProxy();

            virtual void CreateActor();

            virtual void BuildPropertyMap();

         protected:
            virtual ~CamoConfigActorProxy();
      };

   }
}

#endif