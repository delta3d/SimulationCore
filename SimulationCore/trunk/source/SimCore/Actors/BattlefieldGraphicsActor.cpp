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

#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/RenderInfo>
#include <osg/StateSet>

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
         CreateGeometry();
      }
      
      
      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::OnRemovedFromWorld()
      {
         CleanUp();
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CleanUp()
      {
         if(mGeode.valid())
         {
            GetGameActor().GetOSGNode()->asGroup()->removeChild(mGeode.get());
            mGeode = NULL;
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateGeometry()
      {
         osg::Vec4 color(0.5f, 0.5f, 1.0f, 0.5f);

         mGeode = new osg::Geode();

         osg::StateSet* ss = mGeode->getOrCreateStateSet();
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);

         osg::BlendFunc* blendFunc = new osg::BlendFunc();
         blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
         ss->setAttributeAndModes(blendFunc);
         ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);



         if(mPoints.size() == 1 && mRadius > 0.0001f)
         {
            //use a cylinder
            dtCore::RefPtr<osg::Cylinder> shape = new osg::Cylinder(mPoints.front(), mRadius, mMaxAltitude - mMinAltitude);
            dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(shape);
            shapeDrawable->setColor(color);
            mGeode->addDrawable(shapeDrawable);

         }
         else if(mPoints.size() > 1)
         {
            int numVerts = 4 * mPoints.size();

            if(mClosed)
            {
               numVerts += 4;
            }

            dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry();
            dtCore::RefPtr<osg::Vec3Array> vectorArray = new osg::Vec3Array();
            dtCore::RefPtr<osg::Vec4Array> colorArray = new osg::Vec4Array();
            vectorArray->reserve(numVerts);


            std::vector<osg::Vec3>::iterator iter = mPoints.begin();
            std::vector<osg::Vec3>::iterator iterEnd = mPoints.end();

            osg::Vec3 point1 = *iter;
            osg::Vec3 point2;
            ++iter;

            for(;iter != iterEnd; ++iter)
            {
               point2 = *iter;
               AddPlane(*vectorArray, point1, point2, mMinAltitude, mMaxAltitude);

               //temporary until we have a color source
               colorArray->push_back(color);
               colorArray->push_back(color);
               colorArray->push_back(color);
               colorArray->push_back(color);
               
               point1 = point2;
            }

            if(mClosed)
            {
               //connect the beginning and the end
               point1 = mPoints.front();
               point2 = mPoints.back();
               AddPlane(*vectorArray, point1, point2, mMinAltitude, mMaxAltitude);

               colorArray->push_back(color);
               colorArray->push_back(color);
               colorArray->push_back(color);
               colorArray->push_back(color);
            }

            geom->setVertexArray(vectorArray.get());
            geom->setColorArray(colorArray.get());
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4 * numVerts));

            mGeode->addDrawable(geom.get());
            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
         }

      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::AddPlane(osg::Vec3Array& geom, const osg::Vec3& from, const osg::Vec3& to, float minHeight, float maxHeight)
      {
         //the four corners are lower left , upper left, upper right, and lower right
         osg::Vec3 LL, UL, UR, LR;
         LL.set(from.x(), from.y(), minHeight);
         UL.set(from.x(), from.y(), maxHeight);
         UR.set(to.x(), to.y(), maxHeight);
         LR.set(to.x(), to.y(), minHeight);

         geom.push_back(LL);
         geom.push_back(UL);
         geom.push_back(UR);
         geom.push_back(LR);
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
