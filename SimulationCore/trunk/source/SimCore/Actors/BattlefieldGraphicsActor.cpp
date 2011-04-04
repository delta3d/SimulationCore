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
#include <dtGame/gamemanager.h>
#include <dtCore/scene.h>

#include <dtUtil/getsetmacros.h>
#include <dtUtil/stringutils.h>
#include <dtDAL/propertymacros.h>
#include <dtDAL/arrayactorpropertycomplex.h>
#include <dtCore/shadermanager.h>

#include <osg/Geometry>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Material>
#include <osg/BlendFunc>
#include <osg/RenderInfo>
#include <osg/StateSet>
#include <osg/CullFace>

#include <osgUtil/DelaunayTriangulator>


namespace SimCore
{

   namespace Actors
   {
      IMPLEMENT_ENUM(BattlefieldGraphicsTypeEnum);

      const std::string BattlefieldGraphicsTypeEnum::CONFIG_PREFIX("SimCore.BattlefieldGrapicsActor.Color.");

      BattlefieldGraphicsTypeEnum::BattlefieldGraphicsTypeEnum(const std::string& name, const osg::Vec3& defaultColor)
      : dtUtil::Enumeration(name)
      , mDefaultColor(defaultColor)
      {
         AddInstance(this);
      }

      osg::Vec3 BattlefieldGraphicsTypeEnum::GetColor(dtUtil::ConfigProperties& config)
      {
         osg::Vec3 result = mDefaultColor;
         const std::string color = config.GetConfigPropertyValue(CONFIG_PREFIX + GetName(), "");
         if (!color.empty())
         {
            result = dtUtil::ToType<osg::Vec3>(color);
         }
         return result;
      }

      const osg::Vec3& BattlefieldGraphicsTypeEnum::GetDefaultColor() const
      {
         return mDefaultColor;
      }

      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::UNASSIGNED("UNASSIGNED", osg::Vec3());
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::AIR_CORRIDOR("AIR_CORRIDOR", osg::Vec3(0.5f, 0.5f, 1.0f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::AIR_SPACE_COORDINATION_AREA("AIR_SPACE_COORDINATION_AREA", osg::Vec3(0.5f, 0.5f, 1.0f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::COORDINATED_FIRE_LINE("COORDINATED_FIRE_LINE", osg::Vec3(1.0f, 0.5f, 0.5f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::FREE_FIRE_AREA("FREE_FIRE_AREA", osg::Vec3(1.0f, 0.5f, 0.5f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::FIRE_SUPPORT_COORDINATION_LINE("FIRE_SUPPORT_COORDINATION_LINE", osg::Vec3(0.5f, 0.5f, 1.0f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::NO_FIRE_AREA("NO_FIRE_AREA", osg::Vec3(0.5f, 1.0f, 0.5f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::RESTRICTIVE_FIRE_AREA("RESTRICTIVE_FIRE_AREA", osg::Vec3(0.5f, 1.0f, 1.0f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::RESTRICTIVE_FIRE_LINE("RESTRICTIVE_FIRE_LINE", osg::Vec3(0.5f, 1.0f, 1.0f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::TARGET("TARGET", osg::Vec3(1.0f, 0.5f, 0.5f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::TARGET_BUILDUP_AREA("TARGET_BUILDUP_AREA", osg::Vec3(1.0f, 0.5f, 0.5f));
      BattlefieldGraphicsTypeEnum BattlefieldGraphicsTypeEnum::ZONE_OF_RESPONSIBILITY("ZONE_OF_RESPONSIBILITY", osg::Vec3(0.9f, 0.9f, 0.9f));

      ////////////////////////////////////////////////////////////////////////////
      DT_IMPLEMENT_ARRAY_ACCESSOR(BattlefieldGraphicsActorProxy, osg::Vec3, Point, Points, osg::Vec3());

      ////////////////////////////////////////////////////////////////////////////
      BattlefieldGraphicsActorProxy::BattlefieldGraphicsActorProxy()
         : mType(&BattlefieldGraphicsTypeEnum::UNASSIGNED)
         , mClosed(false)
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

      DT_IMPLEMENT_ACCESSOR(BattlefieldGraphicsActorProxy, dtUtil::EnumerationPointer<BattlefieldGraphicsTypeEnum>, Type);

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
         osg::Vec4 color(GetType().GetColor(GetGameManager()->GetConfiguration()), 0.5f);

         mGeode = new osg::Geode();

         osg::StateSet* ss = mGeode->getOrCreateStateSet();
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);

         dtCore::RefPtr<osg::BlendFunc> blendFunc = new osg::BlendFunc();
         blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
         ss->setAttributeAndModes(blendFunc);
         ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

         ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

         dtCore::RefPtr<osg::CullFace> cull = new osg::CullFace();
         cull->setMode(osg::CullFace::BACK);
         ss->setAttributeAndModes(cull, osg::StateAttribute::OFF);

         AssignShader();

         //assign uniforms
         osg::Uniform* particleColor = ss->getOrCreateUniform("color", osg::Uniform::FLOAT_VEC4);
         particleColor->set(color);

         if(mPoints.size() == 1 && mRadius > 0.0001f)
         {
            //use a cylinder
            float minZ = mMinAltitude;
            float maxZ = mMaxAltitude;
            float diff = maxZ - minZ;

            osg::Vec3 center = mPoints[0];
            center[2] = minZ + (0.5f * diff);

            dtCore::RefPtr<osg::Cylinder> shape = new osg::Cylinder(center, mRadius, diff);
            dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(shape);
            //shapeDrawable->setColor(color);
            mGeode->addDrawable(shapeDrawable);
            //GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
            GetGameManager()->GetScene().GetSceneNode()->addChild(mGeode.get());


         }
         else if(mPoints.size() > 1)
         {
            int numVerts = 0;

            dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry();
            dtCore::RefPtr<osg::Vec3Array> vectorArray = new osg::Vec3Array();
            

            std::vector<osg::Vec3>::iterator iter = mPoints.begin();
            std::vector<osg::Vec3>::iterator iterEnd = mPoints.end();

            osg::Vec3 point1;
            osg::Vec3 point2;

            for(;iter != iterEnd; ++iter)
            {
               point1 = *iter;

               AddTriangle(*vectorArray, point1, mMinAltitude, mMaxAltitude);

               numVerts += 2;
            
            }

            if(mClosed)
            {
               //connect the beginning and the end
               point1 = mPoints.front();
               
               AddTriangle(*vectorArray, point1, mMinAltitude, mMaxAltitude);

               numVerts += 2;
            }

            geom->setVertexArray(vectorArray.get());
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, numVerts));


            mGeode->addDrawable(geom.get());

            if(mClosed)
            {
               CreateClosedTop();
            }

            //GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
            GetGameManager()->GetScene().GetSceneNode()->addChild(mGeode.get());
         }

      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::AssignShader()
      {
         
         dtCore::ShaderManager& sm = dtCore::ShaderManager::GetInstance();

         const dtCore::ShaderGroup *shaderGroup = sm.FindShaderGroupPrototype("BattlefieldGraphicsGroup");

         if (shaderGroup == NULL)
         {
            LOG_INFO("Could not find shader group BattlefieldGraphicsGroup");
            return;
         }

         const dtCore::ShaderProgram *defaultShader = shaderGroup->GetDefaultShader();

         try
         {
            if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *mGeode);
            }
            else
            {
               LOG_WARNING("Could not find a default shader in shader group: BattlefieldGraphicsGroup");
            }
         }
         catch (const dtUtil::Exception &e)
         {
            LOG_WARNING("Caught Exception while assigning shader: " + e.ToString());
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateMultiGeometry()
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
            float minZ = mMinAltitude;
            float maxZ = mMaxAltitude;
            float diff = maxZ - minZ;

            osg::Vec3 center = mPoints[0];
            center[2] = minZ + (0.5f * diff);

            dtCore::RefPtr<osg::Cylinder> shape = new osg::Cylinder(center, mRadius, diff);
            dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(shape);
            shapeDrawable->setColor(color);
            mGeode->addDrawable(shapeDrawable);
            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());

         }
         else if(mPoints.size() > 1)
         {

            std::vector<osg::Vec3>::iterator iter = mPoints.begin();
            std::vector<osg::Vec3>::iterator iterEnd = mPoints.end();

            osg::Vec3 point1 = *iter;
            osg::Vec3 point2;
            ++iter;

            for(;iter != iterEnd; ++iter)
            {
               point2 = *iter;
               AddQuadGeometry(point1, point2, mMinAltitude, mMaxAltitude);

               point1 = point2;
            }

            if(mClosed)
            {
               //connect the beginning and the end
               AddQuadGeometry(mPoints.front(), mPoints.back(), mMinAltitude, mMaxAltitude);
               CreateClosedTop();
            }

            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
         }

      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateClosedTop()
      {
         int numVerts = mPoints.size();
         osg::Vec4 color(0.5f, 0.5f, 1.0f, 0.5f);

         dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry();
         dtCore::RefPtr<osg::Vec3Array> vectorArray = new osg::Vec3Array();
         vectorArray->reserve(numVerts);


         std::vector<osg::Vec3>::iterator iter = mPoints.begin();
         std::vector<osg::Vec3>::iterator iterEnd = mPoints.end();

         
         for(;iter != iterEnd; ++iter)
         {
            osg::Vec3 point1 = *iter;

            osg::Vec3 UL(point1.x(), point1.y(), mMaxAltitude);

            vectorArray->push_back(UL);
         }

         dtCore::RefPtr<osgUtil::DelaunayTriangulator> triangulator = new osgUtil::DelaunayTriangulator(vectorArray);
         bool result = triangulator->triangulate();
         if(result)
         {
            geom->setVertexArray(vectorArray.get());
            geom->addPrimitiveSet(triangulator->getTriangles());

            mGeode->addDrawable(geom.get());
         }
         else
         {
            LOG_ERROR("Unable to use DelaunayTriangulator to close top of volume");
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::AddTriangle(osg::Vec3Array& geom, const osg::Vec3& point, float minHeight, float maxHeight)
      {
         //the four corners are lower left , upper left, upper right, and lower right
         osg::Vec3 UR, LR;
         UR.set(point.x(), point.y(), maxHeight);
         LR.set(point.x(), point.y(), minHeight);

         geom.push_back(UR);
         geom.push_back(LR);
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::AddQuadGeometry(const osg::Vec3& from, const osg::Vec3& to, float minHeight, float maxHeight)
      {
         osg::Vec4 color(0.5f, 0.5f, 1.0f, 0.5f);

         dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry();
         dtCore::RefPtr<osg::Vec3Array> vectorArray = new osg::Vec3Array();

         osg::Vec3 LL, UL, UR, LR;
         LL.set(from.x(), from.y(), minHeight);
         UL.set(from.x(), from.y(), maxHeight);
         UR.set(to.x(), to.y(), maxHeight);
         LR.set(to.x(), to.y(), minHeight);

         vectorArray->push_back(LL);
         vectorArray->push_back(LR);
         vectorArray->push_back(UR);
         vectorArray->push_back(UL);

         geom->setVertexArray(vectorArray.get());
         geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));


         mGeode->addDrawable(geom.get());
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

         DT_REGISTER_PROPERTY(Type, "A type code defining what the graphic represents", PropRegHelperType, propRegHelper);

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

         
         dtCore::RefPtr<dtDAL::Vec3ActorProperty> vec3prop =
         new dtDAL::Vec3ActorProperty("NestedVec3",
                  "Nested Vec3",
                  dtDAL::Vec3ActorProperty::SetFuncType(arrayProp.get(), &Vec3ArrayPropType::SetCurrentValue),
                  dtDAL::Vec3ActorProperty::GetFuncType(arrayProp.get(), &Vec3ArrayPropType::GetCurrentValue),
                  "", GROUPNAME);

         arrayProp->SetArrayProperty(*vec3prop);

         AddProperty(arrayProp);

      }


   }
}
