/* -*-c++-*-
 * SimulationCore
 * Copyright 2011, Alion Science and Technology
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
 * David Guthrie
 */

#ifndef WHEELACTCOMP_H_
#define WHEELACTCOMP_H_

#include <SimCore/Export.h>
#include <dtGame/actorcomponent.h>
#include <dtUtil/getsetmacros.h>
#include <dtUtil/refstring.h>
#include <osg/Matrix>
#include <osg/Vec2>

namespace dtUtil
{
   class NodeCollector;
}

namespace dtGame
{
   class TickMessage;
}

namespace SimCore
{
   namespace ActComps
   {
      class SIMCORE_EXPORT Axle : public osg::Referenced
      {
      public:
         Axle();

         DT_DECLARE_ACCESSOR(osg::Vec2, WheelWidthAndRadius)
         DT_DECLARE_ACCESSOR(bool, IsSteerable)
         DT_DECLARE_ACCESSOR(float, MaxSteerAngle)

         virtual unsigned GetNumWheels() const = 0;

         virtual void UpdateAxleRotation(float dt, float steerAngle, float speedmps) = 0;
         /// Use positive for jounce and negative for rebound.
         virtual void UpdateWheelPosition(unsigned whichWheel, float jounceRebound) = 0;

         /// Sets the world position of a wheel on the axle or in local space relative to it's parent transform.
         virtual void SetWheelBaseTransform(unsigned whichWheel, const osg::Matrix& xform, bool worldRelative = true) = 0;
         virtual void GetWheelBaseTransform(unsigned whichWheel, osg::Matrix& xform, bool worldRelative = true) = 0;

      protected:

         virtual ~Axle();
      };

      /**
       * Defines a set of wheels/axles for an actor.  It looks for wheels on model and can handle displaying the rotation
       * and steering for it.
       */
      class  SIMCORE_EXPORT WheelActComp : public dtGame::ActorComponent
      {
      public:
         static const dtGame::ActorComponent::ACType TYPE;

         WheelActComp();

         class AutoUpdateModeEnum : public dtUtil::Enumeration
         {
            DECLARE_ENUM(AutoUpdateModeEnum);
         public:
            static AutoUpdateModeEnum OFF;
            static AutoUpdateModeEnum REMOTE_ONLY;
            static AutoUpdateModeEnum LOCAL_AND_REMOTE;
         private:
            AutoUpdateModeEnum(const std::string& name);
         };

         /// Configure the actor component in OnEnteredWorld or wait for the actor to call it directly.
         DT_DECLARE_ACCESSOR(bool, AutoConfigure)

         /// True if this actor component should call update each tick.
         DT_DECLARE_ACCESSOR(dtUtil::EnumerationPointer<AutoUpdateModeEnum>, AutoUpdateMode)

         DT_DECLARE_ACCESSOR(dtUtil::RefString, WheelNodePrefix)

         DT_DECLARE_ACCESSOR(float, SteeringAngle)

         DT_DECLARE_ACCESSOR(float, MaxSteeringAngle)

         virtual void OnAddedToActor(dtGame::GameActor& actor);

         virtual void OnRemovedFromActor(dtGame::GameActor& actor);

         virtual void OnEnteredWorld();

         virtual void OnRemovedFromWorld();

         virtual void BuildPropertyMap();

         void Update(const dtGame::TickMessage& msg);

         void FindAxles(dtUtil::NodeCollector* nodeCollector = NULL);

         DT_DECLARE_ARRAY_ACCESSOR(dtCore::RefPtr<Axle>, Axle, Axles)

      protected:
         virtual ~WheelActComp();
      private:
         float mLastFrameHeading;
      };

   }

}

#endif /* WHEELACTCOMP_H_ */
