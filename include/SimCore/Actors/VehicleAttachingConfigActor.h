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
#ifndef _VEHICLE_ATTACHING_CONFIG_ACTOR_
#define _VEHICLE_ATTACHING_CONFIG_ACTOR_

#include <SimCore/Export.h>
#include <dtGame/gameactor.h>
#include <dtUtil/getsetmacros.h>
#include <dtCore/resourcedescriptor.h>

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

            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, InsideModelResourceGood);
            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, InsideModelResourceDamaged);
            DT_DECLARE_ACCESSOR(dtCore::ResourceDescriptor, InsideModelResourceDestroyed);

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
      };

      /////////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT VehicleAttachingConfigActorProxy : public dtGame::GameActorProxy
      {
         public:
            VehicleAttachingConfigActorProxy();
            virtual void BuildPropertyMap();

         protected:
            virtual ~VehicleAttachingConfigActorProxy();
            void CreateDrawable();
            virtual void OnEnteredWorld();
      };
   }
}
#endif
