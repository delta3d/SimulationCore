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

#include <dtDAL/plugin_export.h>
#include <dtDAL/actorproxy.h>

//#include <dtDAL/plugin_export.h>
#include <dtCore/refptr.h>
#include <dtCore/deltadrawable.h>

namespace SimCore
{
   namespace Actors
   {
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT OceanWater: public dtGame::GameActor
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

            //void Init(const osg::Vec2& pCenter, const osg::Vec2& pSize, const osg::Vec2& pResolution, float pHeight);
            
            void Update(double dt);

            /*void SetWaterColor(const osg::Vec3& pColor);
            osg::Vec3 GetWaterColor(){return mWaterColorVec;};*/

            void SetCenter(const osg::Vec2& pCenter){mCenter = pCenter;}
            osg::Vec2 GetCenter(){return mCenter;}

            void SetSize(const osg::Vec2& pSize){mSize = pSize;}
            osg::Vec2 GetSize(){return mSize;};

            void SetResolution(const osg::Vec2& pRes){};
            osg::Vec2 GetResolution(){return mResolution;}

            void SetHeight(float pHeight){mWaterHeight = pHeight;}
            float GetHeight(){return mWaterHeight;}

            //void Reset(){Init(mCenter, mSize, mResolution, mWaterHeight);};

           /* void SetLightPos(const osg::Vec3& pLightPos);
            void SetEyePos(const osg::Vec3& pEyePos);*/

            /*const osg::Node* GetOSGNode() const{ return mNode.get();};
            osg::Node* GetOSGNode() { return mNode.get();};*/

         protected:
            virtual ~OceanWater(){}

         private:

            //void CreateNodes();
            void CreateGeometry();

            osg::Vec2                                 mSize;
            osg::Vec2                                 mResolution;
            osg::Vec2                                 mTexShift;
            osg::Vec3                                 mWater;
            osg::Vec2                                 mCenter;
            //osg::Vec3                                 mWaterColorVec;
            float                                     mWaterHeight; 
            float                                     mElapsedTime;
            float                                     mDeltaTime;
            float                                     mWaterSpeed;

            dtCore::RefPtr<osg::Geometry>                     mGeometry;
            dtCore::RefPtr<osg::Geode>		                    mGeode;

            //dtCore::RefPtr<osg::PositionAttitudeTransform>    mXform; 
            /*dtCore::RefPtr<osg::Uniform>                      mLightPos; 
            dtCore::RefPtr<osg::Uniform>                      mEyePos;
            dtCore::RefPtr<osg::Uniform>                      mTexInc;
            dtCore::RefPtr<osg::Uniform>                      mTexIncPrev;
            dtCore::RefPtr<osg::Uniform>                      mWaterColor;
            dtCore::RefPtr<osg::Uniform>                      mBlend;
            dtCore::RefPtr<osg::Uniform>                      mTextureRepeat;*/

            //dtCore::RefPtr<osg::Program>                      mProg; 
            //dtCore::RefPtr<osg::Group> mNode;

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

