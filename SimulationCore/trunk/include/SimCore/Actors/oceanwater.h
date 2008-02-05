/* 
* Delta3D Open Source Game and Simulation Engine 
* Copyright (C) 2004-2008 MOVES Institute 
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
* @author Bradley Anderegg, modded by allen danklefsen
*/

#ifndef __OCEAN_WATER_H__
#define __OCEAN_WATER_H__

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/PositionAttitudeTransform>
#include <osg/Program>
#include <osg/Vec3>
#include <osg/Vec4>

#include <SimCore/Export.h>

#include <dtCore/refptr.h>
#include <dtGame/gameactor.h>
#include <NxAgeiaPrimitivePhysicsHelper.h>

namespace SimCore
{
   namespace Actors
   {
#ifdef AGEIA_PHYSICS
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT OceanWater: public dtGame::GameActor,
                                       public dtAgeiaPhysX::NxAgeiaPhysicsInterface
#else
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT OceanWater: public dtGame::GameActor
#endif
      {
         public:

            OceanWater(dtGame::GameActorProxy& proxy);

            /**
            * This method is an invokable called when an object is local and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickLocal(const dtGame::Message &tickMessage);

            /**
            * This method is an invokable called when an object is remote and
            * receives a tick.
            * @param tickMessage A message containing tick related information.
            */
            virtual void TickRemote(const dtGame::Message &tickMessage);

            // Called when the actor has been added to the game manager.
            // You can respond to OnEnteredWorld on either the proxy or actor or both.
            virtual void OnEnteredWorld();

            void Update(double dt);

            void SetCenter(const osg::Vec2& pCenter){mCenter = pCenter;}
            osg::Vec2 GetCenter(){return mCenter;}

            void SetSize(const osg::Vec2& pSize){mSize = pSize;}
            osg::Vec2 GetSize(){return mSize;};

            void SetResolution(const osg::Vec2& pRes){};
            osg::Vec2 GetResolution(){return mResolution;}

            void SetHeight(float pHeight){mWaterHeight = pHeight;}
            float GetHeight(){return mWaterHeight;}
            
#ifdef AGEIA_PHYSICS

            /// Corresponds to the AGEIA_FLAGS_PRE_UPDATE flag
            virtual void AgeiaPrePhysicsUpdate();         

            /// Corresponds to the AGEIA_FLAGS_POST_UPDATE
            virtual void AgeiaPostPhysicsUpdate();

            /// Corresponds to the AGEIA_FLAGS_GET_COLLISION_REPORT
            virtual void AgeiaCollisionReport(dtAgeiaPhysX::ContactReport& contactReport, 
               NxActor& ourSelf, NxActor& whatWeHit){}

            // You would have to make a new raycast to get this report,
            // so no flag associated with it.
            virtual void AgeiaRaycastReport(const NxRaycastHit& hit, const NxActor& ourSelf, 
               const NxActor& whatWeHit){}

            // returns the physics helper for use
            dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper* GetPhysicsHelper() {return mPhysicsHelper.get();}
#endif

         protected:
            virtual ~OceanWater(){}

         private:

#ifdef AGEIA_PHYSICS
            dtCore::RefPtr<dtAgeiaPhysX::NxAgeiaPrimitivePhysicsHelper> mPhysicsHelper;
#endif

            void CreateGeometry();

            osg::Vec2 mSize;
            osg::Vec2 mResolution;
            osg::Vec2 mTexShift;
            osg::Vec3 mWater;
            osg::Vec2 mCenter;
            float     mWaterHeight; 
            float     mElapsedTime;
            float     mDeltaTime;
            float     mWaterSpeed;

            dtCore::RefPtr<osg::Geometry> mGeometry;
            dtCore::RefPtr<osg::Geode>	   mGeode;
      };

      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT OceanWaterActorProxy: public dtGame::GameActorProxy
      {
         public:
            OceanWaterActorProxy(){SetClassName("OceanWater");}

            void BuildPropertyMap();
            //bool IsPlaceable() const { return false; }

            void OnEnteredWorld();
           
         protected:
            virtual ~OceanWaterActorProxy(){}

            void CreateActor();
      };
   } // namespace
}// namespace
#endif //__OCEAN_WATER_H__

