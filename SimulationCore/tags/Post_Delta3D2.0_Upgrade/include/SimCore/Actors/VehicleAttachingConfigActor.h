/*
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2005, BMH Associates, Inc.
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
 * @author Allen Danklefsen
 */
#ifndef _VEHICLE_ATTACHING_CONFIG_ACTOR_
#define _VEHICLE_ATTACHING_CONFIG_ACTOR_

#include <SimCore/Export.h>
#include <dtGame/gameactor.h>

namespace SimCore
{
   namespace Actors
   {
      /////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT VehicleAttachingConfigActor : public dtGame::GameActor
      {
         public:

            /// Constructor
            VehicleAttachingConfigActor(dtGame::GameActorProxy &proxy);

            //////////////////////////////////////////////////////////////////////////////////
            osg::Vec3 GetSeatPosition() {return mSeatPosition;}
            void SetSeatPosition(const osg::Vec3& seatPosition) {mSeatPosition = seatPosition;}

            //////////////////////////////////////////////////////////////////////////////////
            bool GetUsesInsideModel() {return mUseInsideModel;}
            void SetUsesInsideModel(bool value) {mUseInsideModel = value;}

            //////////////////////////////////////////////////////////////////////////////////
            std::string GetInsideModelResourceGood() {return mInsideModelResourceGood;}
            void SetInsideModelResourceGood(const std::string& value) {mInsideModelResourceGood = value;}
            std::string GetInsideModelResourceDamaged() {return mInsideModelResourceDamaged;}
            void SetInsideModelResourceDamaged(const std::string& value) {mInsideModelResourceDamaged = value;}
            std::string GetInsideModelResourceDestroyed() {return mInsideModelResourceDestroyed;}
            void SetInsideModelResourceDestroyed(const std::string& value) {mInsideModelResourceDestroyed = value;}

            //////////////////////////////////////////////////////////////////////////////////
            osg::Vec3 GetRotationOffSet() {return mRotationOffSet;}
            void SetRotationOffSet(const osg::Vec3& value){mRotationOffSet = value;}

         protected:

            /// Destructor
            virtual ~VehicleAttachingConfigActor();

         private:
            osg::Vec3   mSeatPosition;
            osg::Vec3   mRotationOffSet;
            bool        mUseInsideModel;
            std::string mInsideModelResourceGood;
            std::string mInsideModelResourceDamaged;
            std::string mInsideModelResourceDestroyed;
      };

      /////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT VehicleAttachingConfigActorProxy : public dtGame::GameActorProxy
      {
         public:
            VehicleAttachingConfigActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~VehicleAttachingConfigActorProxy();
            void CreateActor();
            virtual void OnEnteredWorld();
      };
   }
}
#endif