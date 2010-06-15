/* -*-c++-*-
* Simulation Core
* Copyright 2010, Alion Science and Technology
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
*
* @author Chris Rodgers
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <dtDAL/enginepropertytypes.h>
#include <dtDAL/propertymacros.h>
#include <dtGame/gameactor.h>
#include <dtGame/gamemanager.h>
#include <SimCore/ActComps/CamoPaintStateActComp.h>
#include <SimCore/Actors/CamoConfigActor.h>
#include <SimCore/Actors/EntityActorRegistry.h>



namespace SimCore
{
   namespace ActComps
   {
      //////////////////////////////////////////////////////////////////////////
      // CONSTANTS
      //////////////////////////////////////////////////////////////////////////
      const dtGame::ActorComponent::ACType CamoPaintStateActComp::TYPE("CamoPaintStateActComp");
      const dtUtil::RefString CamoPaintStateActComp::PROPERTY_CAMO_ID("Camo Id");



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      CamoPaintStateActComp::CamoPaintStateActComp()
         : BaseClass(TYPE)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      CamoPaintStateActComp::~CamoPaintStateActComp()
      {
      }



      //////////////////////////////////////////////////////////////////////////
      // PROPERTY MACROS
      // These macros define the Getter method body for each property
      //////////////////////////////////////////////////////////////////////////
      IMPLEMENT_PROPERTY_GETTER(CamoPaintStateActComp, int, CamoId); // Setter is implemented below
      
      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::SetCamoId(int camoId)
      {
         const SimCore::Actors::CamoParams* camo = GetCamoParameters(camoId);
         if(camo != NULL)
         {
            SetPaintColor1(camo->GetColor1());
            SetPaintColor2(camo->GetColor2());
            SetPaintColor3(camo->GetColor3());
            SetPaintColor4(camo->GetColor4());
            SetPatternTexture(camo->GetPatternTexture());
         }
      }

      //////////////////////////////////////////////////////////////////////////
      const SimCore::Actors::CamoParams* CamoPaintStateActComp::GetCamoParameters(int camoId)
      {
         using namespace SimCore::Actors;

         const CamoParams* camo = NULL;

         dtGame::GameActor* actor = NULL;
         GetOwner(actor);
         if(actor != NULL)
         {
            dtGame::GameManager* gm = actor->GetGameActorProxy().GetGameManager();
            if(gm != NULL)
            {
               dtDAL::ActorProxy* proxy = NULL;
               gm->FindActorByType(*EntityActorRegistry::CAMO_CONFIG_ACTOR_TYPE, proxy);

               CamoConfigActor* actor = NULL;
               if(proxy != NULL)
               {
                  proxy->GetActor(actor);
               }

               if(actor != NULL)
               {
                  camo = actor->GetCamoParamsByCamoId(camoId);
               }
            }
         }

         return camo;
      }

      //////////////////////////////////////////////////////////////////////////
      void CamoPaintStateActComp::BuildPropertyMap()
      {
         BaseClass::BuildPropertyMap();

         typedef dtDAL::PropertyRegHelper<CamoPaintStateActComp&, CamoPaintStateActComp> PropRegType;
         PropRegType propRegHelper(*this, this, "Camo Paint Id");

         // INT PROPERTIES
         REGISTER_PROPERTY_WITH_NAME_AND_LABEL(
            CamoId,
            PROPERTY_CAMO_ID,
            PROPERTY_CAMO_ID,
            "Id of the camo object (from the Camo Config Actor) that specifies the color parameters and pattern texture.",
            PropRegType, propRegHelper);
      }

   } // ActComps namespace
} // SimCore namespace
