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
* @author Allen Danklefsen
*/
#ifndef LM_OPENFLIGHT_TERRAIN_ACTOR
#define LM_OPENFLIGHT_TERRAIN_ACTOR

#include <SimCore/Export.h>
#include <SimCore/Actors/IGActor.h>
#include <osg/Group>

namespace SimCore
{
   namespace Actors
   {
      class SIMCORE_EXPORT OpenFlightToIVETerrainActor : public IGActor
      {
      public:

         /// Constructor
         OpenFlightToIVETerrainActor(dtGame::GameActorProxy& proxy);

         /**
         * Loads a mesh file which contains terrain using the configured base name and directory.
         */
         virtual void LoadTerrain();

         virtual void AddedToScene(dtCore::Scene* scene);

         //////////////////////////////////////////////////////////////////
         // Properties
         float GetPagingMinX() const {return mPaging_Min_X;}
         float GetPagingMinY() const {return mPaging_Min_Y;}
         float GetPagingMaxX() const {return mPaging_Max_X;}
         float GetPagingMaxY() const {return mPaging_Max_Y;}
         float GetPagingDelta() const {return mPaging_Delta;}
         float GetPagingRadius() const {return mPaging_Radius;}
         float GetPagingRange() const {return mPaging_Range;}
         const std::string& GetPagingBaseName() const {return mPaging_BaseName;}
         float GetPagingExpiringDelay() const {return mPaging_ExpiringDelay;}
         float GetPagingFPSTarget() const {return mPaging_Frame_Rate_Targeted;}
         bool  GetPagingPrecompile() const {return mPaging_Precompile;}
         int   GetMaxObjectsToCompile() const {return mMaximumObjectsToCompile;}
         float GetZOffsetforTerrain() const {return mZOffsetForTerrain;}
         const std::string& GetTerrainPath() const {return mTerrainPath;}

         void SetPagingMinX(float value){mPaging_Min_X = value;}
         void SetPagingMinY(float value){mPaging_Min_Y = value;}
         void SetPagingMaxX(float value){mPaging_Max_X = value;}
         void SetPagingMaxY(float value){mPaging_Max_Y = value;}
         void SetPagingDelta(float value){mPaging_Delta = value;}
         void SetPagingRadius(float value){mPaging_Radius = value;}
         void SetPagingRange(float value){mPaging_Range = value;}
         void SetTerrainPath(const std::string& value);
         void SetPagingExpiringDelay(float value){mPaging_ExpiringDelay = value;}
         void SetPagingFPSTarget(float value){mPaging_Frame_Rate_Targeted = value;}
         void SetPagingPrecompile(bool value) {mPaging_Precompile = value;}
         void SetMaxObjectsToCompile(int value){mMaximumObjectsToCompile = value;}
         void SetZOffsetforTerrain(float value){mZOffsetForTerrain = value;}
         void SetPagingBaseName(const std::string& value);
         //////////////////////////////////////////////////////////////////

      protected:

         /// Destructor
         virtual ~OpenFlightToIVETerrainActor();

         dtCore::RefPtr<osg::Group> mGroupNodeForTerrain;

         //////////////////////////////////////
         /// Properties
         float       mPaging_Min_X;
         float       mPaging_Min_Y;
         float       mPaging_Max_X;
         float       mPaging_Max_Y;
         float       mPaging_Delta;
         float       mPaging_Radius;
         float       mPaging_Range;
         std::string mPaging_BaseName;
         float       mPaging_ExpiringDelay;
         float       mPaging_Frame_Rate_Targeted;
         bool        mPaging_Precompile;
         int         mMaximumObjectsToCompile;
         float       mZOffsetForTerrain;
         std::string mTerrainPath;
         bool        mNeedToLoad;
         //////////////////////////////////////
      };

      class SIMCORE_EXPORT OpenFlightToIVETerrainActorProxy : public dtGame::GameActorProxy
      {
         public:
            typedef dtGame::GameActorProxy BaseClass;

            /**
            * Constructor
            */
            OpenFlightToIVETerrainActorProxy();

            /**
            * Adds the properties to the actor.
            */
            virtual void BuildPropertyMap();

            /// Creates the actor we are encapsulating
            virtual void CreateDrawable();

            /**
            *  We override this method to clear the database pager.
            */
            virtual void RemovedFromScene(dtCore::Scene* scene);

         protected:

            /**
            * Destructor
            */
            virtual ~OpenFlightToIVETerrainActorProxy() { }
      };
   }
}

#endif
