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
#include <SimCore/Actors/IGActor.h>
#include <dtGame/gameactorproxy.h>
#include <dtUtil/getsetmacros.h>
#include <dtUtil/enumeration.h>
#include <dtUtil/configproperties.h>

#include <osg/Vec3>
#include <osg/Geode>
#include <osg/Array>

namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT BattlefieldGraphicsTypeEnum : public dtUtil::Enumeration
      {
      public:
         DECLARE_ENUM(BattlefieldGraphicsTypeEnum);

         static const std::string CONFIG_PREFIX;

         static BattlefieldGraphicsTypeEnum UNASSIGNED;
         static BattlefieldGraphicsTypeEnum AIR_CORRIDOR;
         static BattlefieldGraphicsTypeEnum AIR_SPACE_COORDINATION_AREA;
         static BattlefieldGraphicsTypeEnum COORDINATED_FIRE_LINE;
         static BattlefieldGraphicsTypeEnum FREE_FIRE_AREA;
         static BattlefieldGraphicsTypeEnum FIRE_SUPPORT_COORDINATION_LINE;
         static BattlefieldGraphicsTypeEnum NO_FIRE_AREA;
         static BattlefieldGraphicsTypeEnum RESTRICTIVE_FIRE_AREA;
         static BattlefieldGraphicsTypeEnum RESTRICTIVE_FIRE_LINE;
         static BattlefieldGraphicsTypeEnum TARGET;
         static BattlefieldGraphicsTypeEnum TARGET_BUILDUP_AREA;
         static BattlefieldGraphicsTypeEnum ZONE_OF_RESPONSIBILITY;

         /// @return the color for the given enum.  This will lookup the color in the config, or return the default if not found.
         osg::Vec3 GetColor(dtUtil::ConfigProperties& config);

         const osg::Vec3& GetDefaultColor() const;

      private:
         BattlefieldGraphicsTypeEnum(const std::string& name, const osg::Vec3& defaultColor);
         osg::Vec3 mDefaultColor;
      };


      class SIMCORE_EXPORT BattlefieldGraphicsActorProxy : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;

         BattlefieldGraphicsActorProxy();

         virtual void BuildActorComponents();

         DT_DECLARE_ACCESSOR(dtUtil::EnumerationPointer<BattlefieldGraphicsTypeEnum>, Type);

         DT_DECLARE_ACCESSOR_INLINE(bool, Closed);

         DT_DECLARE_ACCESSOR_INLINE(float, Radius);

         DT_DECLARE_ACCESSOR_INLINE(float, MinAltitude);
         DT_DECLARE_ACCESSOR_INLINE(float, MaxAltitude);

         DT_DECLARE_ARRAY_ACCESSOR(osg::Vec3, Point, Points);

         void SetEnableTopGeometry(bool b);
         bool GetEnableTopGeometry() const;

         static void SetGlobalEnableTopGeometry(bool b, dtGame::GameManager& gm);
         static bool GetGlobalEnableTopGeometry();
            
      protected:
         virtual ~BattlefieldGraphicsActorProxy();

         virtual void OnEnteredWorld();
         virtual void OnRemovedFromWorld();

         virtual void CreateDrawable();
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
         void AssignShader(osg::Geode* node);

         void CreateMultiGeometry();
         void CreateClosedTop(std::vector<osg::Vec3>& points, bool createGeometry = true);
         void AddTriangle(osg::Vec3Array& geom, const osg::Vec3& point, float minHeight, float maxHeight);
         void AddQuadGeometry(const osg::Vec3& from, const osg::Vec3& to, float minHeight, float maxHeight);
         void CreateClosedGeometry(std::vector<osg::Vec3>& points);
         void CreateClosedGeometry(std::vector<osg::Vec3>& points, float minHeight, float maxHeight, bool top);

         void CheckPointPairsForIntersections(std::vector<osg::Vec3>& points, std::vector<std::vector<osg::Vec3> >& tops);
         bool Intersects(const osg::Vec3& line1From, const osg::Vec3& line1To, const osg::Vec3& line2From, const osg::Vec3& line2To, osg::Vec3& intersectPoint);
         bool CheckUpdate();

         bool mEnableTopGeometry;
         static bool mEnableTopGeometryGlobal;
         dtCore::RefPtr<osg::Geode> mGeode;
         dtCore::RefPtr<osg::Geode> mTopGeode;

      };

      class SIMCORE_EXPORT BattlefieldGraphicsDrawable : public SimCore::Actors::IGActor
      {
      public:
         BattlefieldGraphicsDrawable(dtGame::GameActorProxy& owner);

         ///@return true if this actor should be visible based on the visibility options given.
         virtual bool ShouldBeVisible(const SimCore::VisibilityOptions& vo);
      private:
         ~BattlefieldGraphicsDrawable();
      };
   }

}

#endif /* BATTLEFIELDGRAPHICS_ACTOR_H_ */
