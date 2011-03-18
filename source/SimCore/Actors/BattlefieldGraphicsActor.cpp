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

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/BattlefieldGraphicsActor.h>
#include <dtGame/gameactor.h>
#include <dtUtil/getsetmacros.h>
#include <dtDAL/propertymacros.h>
#include <dtDAL/arrayactorpropertycomplex.h>


namespace SimCore
{

   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ARRAY_ACCESSOR(BattlefieldGraphicsActorProxy, osg::Vec3, Point, Points, osg::Vec3());

      ////////////////////////////////////////////////////////////////////////////
      BattlefieldGraphicsActorProxy::BattlefieldGraphicsActorProxy()
         : mClosed(false)
         , mRadius(0.0f)
         , mMinAltitude(0.0f)
         , mMaxAltitude(100.0f)
      {
         SetClassName("SimCore::Actors::BattlefieldGraphicsActorProxy");
         SetHideDTCorePhysicsProps(true);
      }

      ////////////////////////////////////////////////////////////////////////////
      BattlefieldGraphicsActorProxy::~BattlefieldGraphicsActorProxy()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::BuildActorComponents()
      {
         BaseClass::BuildActorComponents();
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::OnEnteredWorld()
      {
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateActor()
      {
         SetActor(*new dtGame::GameActor(*this));
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::BuildPropertyMap()
      {
         static const dtUtil::RefString GROUPNAME("BattlefieldGraphics");

         typedef dtDAL::PropertyRegHelper<BattlefieldGraphicsActorProxy&, BattlefieldGraphicsActorProxy> PropRegHelperType;
         PropRegHelperType propRegHelper(*this, this, GROUPNAME);

         DT_REGISTER_PROPERTY(Closed, "A boolean which determines if the last point should connect back to the first point, ie if the shape is closed or not",
                                 PropRegHelperType, propRegHelper);

         
         DT_REGISTER_PROPERTY(Radius, "The radius of the shape, if the radius is set it implies there is only one point, which is then extruded to make a cylinder.",
                                 PropRegHelperType, propRegHelper);

         DT_REGISTER_PROPERTY(MinAltitude, "The starting height to extrude from.", PropRegHelperType, propRegHelper);


         DT_REGISTER_PROPERTY(MaxAltitude, "The maximum height that the volume is extruded to.", PropRegHelperType, propRegHelper);


         typedef dtDAL::ArrayActorPropertyComplex<osg::Vec3> Vec3ArrayPropType;
         dtCore::RefPtr<Vec3ArrayPropType> arrayProp =
            new Vec3ArrayPropType
               ("PointArray", "PointArray",
                Vec3ArrayPropType::SetFuncType(this, &BattlefieldGraphicsActorProxy::SetPoint),
                Vec3ArrayPropType::GetFuncType(this, &BattlefieldGraphicsActorProxy::GetPoint),
                Vec3ArrayPropType::GetSizeFuncType(this, &BattlefieldGraphicsActorProxy::GetNumPoints),
                Vec3ArrayPropType::InsertFuncType(this, &BattlefieldGraphicsActorProxy::InsertAPoint),
                Vec3ArrayPropType::RemoveFuncType(this, &BattlefieldGraphicsActorProxy::RemovePoint),
                "The list of points to extrude given the min and max altitude.",
                GROUPNAME

               );

         AddProperty(arrayProp);

      }


   }
}
