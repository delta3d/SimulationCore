/* -*-c++-*-
 * SimulationCore
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
 * Bradley Anderegg
 */

#ifndef BATTLEFIELDGRAPHICS_ACTOR_H_
#define BATTLEFIELDGRAPHICS_ACTOR_H_

#include <SimCore/Export.h>
#include <dtGame/gameactorproxy.h>
#include <dtUtil/getsetmacros.h>

#include <osg/Vec3>
#include <osg/Geode>
#include <osg/Array>

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT BattlefieldGraphicsActorProxy : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;

         BattlefieldGraphicsActorProxy();

         virtual void BuildActorComponents();

         DT_DECLARE_ACCESSOR_INLINE(bool, Closed);

         DT_DECLARE_ACCESSOR_INLINE(float, Radius);

         DT_DECLARE_ACCESSOR_INLINE(float, MinAltitude);
         DT_DECLARE_ACCESSOR_INLINE(float, MaxAltitude);

         DT_DECLARE_ARRAY_ACCESSOR(osg::Vec3, Point, Points);

      protected:
         virtual ~BattlefieldGraphicsActorProxy();

         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();

         virtual void CreateActor();
         virtual void BuildPropertyMap();


      private:
         //for some reason the ArrayActorPropertyComplex get/set macro is trying to 
         //use the wrong insert function, as a temporary workaround I have made
         //an overload with a different name 
         void InsertAPoint(int index)
         {
            InsertPoint(index);
         }

         void CleanUp();
         void CreateGeometry();
         void CreateClosedTop();
         void AddTriangle(osg::Vec3Array& geom, const osg::Vec3& point, float minHeight, float maxHeight);

         dtCore::RefPtr<osg::Geode> mGeode;

      };

   }

}

#endif /* BATTLEFIELDGRAPHICS_ACTOR_H_ */
