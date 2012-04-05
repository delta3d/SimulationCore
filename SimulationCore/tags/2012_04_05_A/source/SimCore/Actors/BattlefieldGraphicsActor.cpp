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
#include <SimCore/VisibilityOptions.h>
#include <SimCore/Actors/EntityActorRegistry.h>
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

#include <cmath>

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

      bool BattlefieldGraphicsActorProxy::mEnableTopGeometryGlobal = true;

      ////////////////////////////////////////////////////////////////////////////
      BattlefieldGraphicsActorProxy::BattlefieldGraphicsActorProxy()
         : mType(&BattlefieldGraphicsTypeEnum::UNASSIGNED)
         , mClosed(false)
         , mRadius(0.0f)
         , mMinAltitude(0.0f)
         , mMaxAltitude(100.0f)
         , mDirtyFlag(false)
         , mEnableTopGeometry(false)
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

         AssignShader(mGeode.get());

         //assign uniforms
         osg::Vec4 color(GetType().GetColor(GetGameManager()->GetConfiguration()), 0.5f);

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

            //create body
            dtCore::RefPtr<osg::TessellationHints> hints = new osg::TessellationHints();
            hints->setCreateBottom(false);
            hints->setCreateTop(false);

            dtCore::RefPtr<osg::Cylinder> shape = new osg::Cylinder(center, mRadius, diff);
            dtCore::RefPtr<osg::ShapeDrawable> shapeDrawable = new osg::ShapeDrawable(shape, hints);
            mGeode->addDrawable(shapeDrawable);

            //create top
            dtCore::RefPtr<osg::TessellationHints> hintsTop = new osg::TessellationHints();
            hintsTop->setCreateBody(false);

            dtCore::RefPtr<osg::Cylinder> shapeTop = new osg::Cylinder(center, mRadius, diff);
            dtCore::RefPtr<osg::ShapeDrawable> shapeDrawableTop = new osg::ShapeDrawable(shape, hintsTop);
            
            CreateClosedTop(mPoints, false);
            mTopGeode->addDrawable(shapeDrawableTop);
            SetEnableTopGeometry(mEnableTopGeometryGlobal);


            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
            GetGameActor().GetOSGNode()->asGroup()->addChild(mTopGeode.get());

         }
         else if(mPoints.size() > 1 && mRadius > 0.0001f)
         {
            CreateClosedGeometry(mPoints);

            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
            GetGameActor().GetOSGNode()->asGroup()->addChild(mTopGeode.get());

         }
         else if(mPoints.size() > 1)
         {
            int numVerts = 0;

            dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry();
            dtCore::RefPtr<osg::Vec3Array> vectorArray = new osg::Vec3Array();
            

            std::vector<osg::Vec3>::iterator iter = mPoints.begin();
            std::vector<osg::Vec3>::iterator iterEnd = mPoints.end();

            osg::Vec3 point1;

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

               CreateClosedTop(mPoints);
               SetEnableTopGeometry(mEnableTopGeometryGlobal);
            }

            geom->setVertexArray(vectorArray.get());
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, numVerts));


            mGeode->addDrawable(geom.get());

            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
            GetGameActor().GetOSGNode()->asGroup()->addChild(mTopGeode.get());
         }

      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::AssignShader(osg::Geode* node)
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
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *node);
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

               CreateClosedTop(mPoints);
               SetEnableTopGeometry(mEnableTopGeometryGlobal);
            }


            GetGameActor().GetOSGNode()->asGroup()->addChild(mGeode.get());
         }

      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateClosedGeometry(std::vector<osg::Vec3>& points)
      { 
         if(points.size() > 1)
         {
            std::vector<osg::Vec3> frontPoints;
            std::vector<osg::Vec3> backPoints;

            std::vector<osg::Vec3>::iterator iter = points.begin();
            std::vector<osg::Vec3>::iterator iterEnd = points.end();

            osg::Vec3 point1 = *iter;
            osg::Vec3 point2;
            ++iter;

            osg::Matrix rot;
            rot.makeRotate(osg::PI_2, osg::Vec3(0.0f, 0.0f, 1.0f));

            bool first = true;
            for(;iter != iterEnd; ++iter)
            {
               point2 = *iter;

               osg::Vec3 point1A, point1B;
               osg::Vec3 point2A, point2B;

               osg::Vec3 diff = point2 - point1;
               diff.normalize();

               osg::Vec3 offset = rot.preMult(diff);
               
               point1A = point1 + (offset * mRadius * 0.5f);
               frontPoints.push_back(point1A);
            
               point1B = point1 - (offset * mRadius * 0.5f);
               backPoints.push_back(point1B);

               point2A = point2 + (offset * mRadius * 0.5f);
               frontPoints.push_back(point2A);
               

               point2B = point2 - (offset * mRadius * 0.5f);
               backPoints.push_back(point2B);

               point1 = point2;
               first = !first;
            }

            //remove duplicates, find tops
            std::vector<std::vector<osg::Vec3> > topGeometry;
            topGeometry.assign(std::min(frontPoints.size(), backPoints.size()), std::vector<osg::Vec3>());
            CheckPointPairsForIntersections(frontPoints, topGeometry);
            CheckPointPairsForIntersections(backPoints, topGeometry);

            //create geometry
            CreateClosedGeometry(frontPoints, mMinAltitude, mMaxAltitude, mClosed);
            CreateClosedGeometry(backPoints, mMinAltitude, mMaxAltitude, mClosed);

            std::vector<std::vector<osg::Vec3> >::iterator vec_iter = topGeometry.begin();
            std::vector<std::vector<osg::Vec3> >::iterator vec_iterEnd = topGeometry.end();
            
            for(;vec_iter != vec_iterEnd; ++vec_iter)
            {
               std::vector<osg::Vec3>& vecArray = *vec_iter;
               if(vecArray.size() > 2)
               {
                  CreateClosedTop(*vec_iter);
               }
            }

            ////create top
            //std::vector<osg::Vec3>::iterator iterFront = frontPoints.begin();
            //std::vector<osg::Vec3>::iterator iterBack = backPoints.begin();

            //std::vector<osg::Vec3>::iterator iterFrontEnd = frontPoints.end();
            //std::vector<osg::Vec3>::iterator iterBackEnd = backPoints.end();

            //osg::Vec3 point1Front = *iterFront;
            //osg::Vec3 point2Front;
            //++iterFront;

            //osg::Vec3 point1Back = *iterBack;
            //osg::Vec3 point2Back;
            //++iterBack;

            //for(;iterBack != iterBackEnd && iterFront != iterFrontEnd; ++iterBack, ++iterFront)
            //{
            //   point2Front = *iterFront;
            //   point2Back = *iterBack;

            //   std::vector<osg::Vec3> combinedPoints;
            //   
            //   combinedPoints.push_back(point1Front);
            //   combinedPoints.push_back(point1Back);
            //   combinedPoints.push_back(point2Front);
            //   combinedPoints.push_back(point2Back);

            //   CreateClosedTop(combinedPoints);

            //   point1Front = point2Front;
            //   point1Back = point2Back;
            //}

            SetEnableTopGeometry(mEnableTopGeometryGlobal);
            
         }
      } 

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateClosedGeometry(std::vector<osg::Vec3>& points, float minHeight, float maxHeight, bool top)
      {
         if(points.size() > 1)
         {
            std::vector<osg::Vec3>::iterator iter = points.begin();
            std::vector<osg::Vec3>::iterator iterEnd = points.end();

            osg::Vec3 point1 = *iter;
            osg::Vec3 point2;
            ++iter;

            for(;iter != iterEnd; ++iter)
            {
               point2 = *iter;
               AddQuadGeometry(point1, point2, minHeight, maxHeight);

               point1 = point2;
            }
         }
      }

      ////////////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CreateClosedTop(std::vector<osg::Vec3>& points, bool createGeometry) 
      {
         if(!mTopGeode.valid())
         {
            mTopGeode = new osg::Geode();
         }

         osg::StateSet* ss = mTopGeode->getOrCreateStateSet();
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);

         dtCore::RefPtr<osg::BlendFunc> blendFunc = new osg::BlendFunc();
         blendFunc->setFunction(osg::BlendFunc::SRC_ALPHA ,osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
         ss->setAttributeAndModes(blendFunc);
         ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

         ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

         dtCore::RefPtr<osg::CullFace> cull = new osg::CullFace();
         cull->setMode(osg::CullFace::BACK);
         ss->setAttributeAndModes(cull, osg::StateAttribute::OFF);

         AssignShader(mTopGeode.get());

         //assign uniforms
         osg::Vec4 color(GetType().GetColor(GetGameManager()->GetConfiguration()), 0.5f);

         osg::Uniform* particleColor = ss->getOrCreateUniform("color", osg::Uniform::FLOAT_VEC4);
         particleColor->set(color);

         if(createGeometry)
         {

            int numVerts = points.size();
            dtCore::RefPtr<osg::Geometry> geom = new osg::Geometry();
            dtCore::RefPtr<osg::Vec3Array> vectorArray = new osg::Vec3Array();
            vectorArray->reserve(numVerts);


            std::vector<osg::Vec3>::iterator iter = points.begin();
            std::vector<osg::Vec3>::iterator iterEnd = points.end();

            
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

               mTopGeode->addDrawable(geom.get());
            }
            else
            {
               LOG_ERROR("Unable to use DelaunayTriangulator to close top of volume");
            }
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
      void BattlefieldGraphicsActorProxy::CreateDrawable()
      {
         SetDrawable(*new BattlefieldGraphicsDrawable(*this));
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

      /////////////////////////////////////////////////////////////////////
      bool BattlefieldGraphicsActorProxy::CheckUpdate()
      {
         //todo-
         return false;
      }

      /////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::CheckPointPairsForIntersections(std::vector<osg::Vec3>& points, std::vector<std::vector<osg::Vec3> >& tops)
      {
         if(points.size() > 3)
         {
            std::vector<osg::Vec3> newPoints;
            std::vector<osg::Vec3>::iterator iter = points.begin();
            std::vector<osg::Vec3>::iterator iterEnd = points.end();

            osg::Vec3 point1, point2, point3, point4, lastPoint;
            point1 = *iter;
            ++iter;

            point2 = *iter;
            ++iter;

            newPoints.push_back(point1);
            tops[0].push_back(point1);

            int i = 0;
            for(;iter != iterEnd;++i)
            {
               point3 = *iter;
               
               ++iter;
               if(iter != iterEnd)
               {
                  point4 = *iter; 

                  //assign to get Z value since intersection is done in 2D
                  osg::Vec3 resultingIntersection = point2;
                  if(Intersects(point1, point2, point3, point4, resultingIntersection))
                  {
                     //swap point 2 with intersection
                     point2 = resultingIntersection;
                     point3 = resultingIntersection;

                     newPoints.push_back(point2);
                     
                     tops[i].push_back(point2);
                  }
                  else
                  {
                     newPoints.push_back(point2);
                     newPoints.push_back(point3);
                     
                     tops[i].push_back(point2);
                     tops[i].push_back(point3);
                  }

                  point1 = point3;
                  tops[i + 1].push_back(point1);

                  point2 = point4;
                  lastPoint = point4;
                  ++iter;
               }
               else
               {
                  lastPoint = point3;
               }
            }

            newPoints.push_back(lastPoint);
            tops[i].push_back(lastPoint);

            points.swap(newPoints);
         }
         else
         {
            std::vector<osg::Vec3>::iterator iter = points.begin();
            std::vector<osg::Vec3>::iterator iterEnd = points.end();

            for(;iter != iterEnd; ++iter)
            {
               tops[0].push_back(*iter);
            }
         }
      }

      /////////////////////////////////////////////////////////////////////
      bool BattlefieldGraphicsActorProxy::Intersects(const osg::Vec3& line1From, const osg::Vec3& line1To, const osg::Vec3& line2From, const osg::Vec3& line2To, osg::Vec3& intersectPoint)
      {
         float denom = ((line2To.y() - line2From.y())*(line1To.x() - line1From.x())) -
            ((line2To.x() - line2From.x())*(line1To.y() - line1From.y()));

         float nume_a = ((line2To.x() - line2From.x())*(line1From.y() - line2From.y())) -
            ((line2To.y() - line2From.y())*(line1From.x() - line2From.x()));

         float nume_b = ((line1To.x() - line1From.x())*(line1From.y() - line2From.y())) -
            ((line1To.y() - line1From.y())*(line1From.x() - line2From.x()));

         if(std::abs(denom) > 0.00001f)
         {
            float ua = nume_a / denom;
            float ub = nume_b / denom;

            if(ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f)
            {
               // Get the intersection point- treated as 2D 
               intersectPoint[0] = line1From.x() + ua*(line1To.x() - line1From.x());
               intersectPoint[1] = line1From.y() + ua*(line1To.y() - line1From.y());

               return true;
            }
         }

         return false;
      }

      /////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::SetEnableTopGeometry(bool b)
      {
         mEnableTopGeometry = b;

         if(mTopGeode.valid())
         {
            if(mEnableTopGeometry)
            {
               mTopGeode->setNodeMask(0xFFFFFFFF);
            }
            else
            {
               mTopGeode->setNodeMask(0x0);
            }
         }
      }

      /////////////////////////////////////////////////////////////////////
      bool BattlefieldGraphicsActorProxy::GetEnableTopGeometry() const
      {
         return mEnableTopGeometry;
      }

      /////////////////////////////////////////////////////////////////////
      void BattlefieldGraphicsActorProxy::SetGlobalEnableTopGeometry(bool b, dtGame::GameManager& gm)
      {
         mEnableTopGeometryGlobal = b;

         std::vector<dtDAL::BaseActorObject*> vect;
         gm.FindActorsByType(*SimCore::Actors::EntityActorRegistry::BATTLEFIELD_GRAPHICS_ACTOR_TYPE, vect);

         std::vector<dtDAL::BaseActorObject*>::iterator iter = vect.begin();
         std::vector<dtDAL::BaseActorObject*>::iterator iterEnd = vect.end();

         for(;iter != iterEnd; ++iter)
         {
            BattlefieldGraphicsActorProxy* bfg = dynamic_cast<BattlefieldGraphicsActorProxy*>(*iter);
            if(bfg != NULL)
            {
               bfg->SetEnableTopGeometry(mEnableTopGeometryGlobal);
            }
         }

      }

      /////////////////////////////////////////////////////////////////////
      bool BattlefieldGraphicsActorProxy::GetGlobalEnableTopGeometry()
      {
         return mEnableTopGeometryGlobal;
      }

      /////////////////////////////////////////////////////////////////////
      BattlefieldGraphicsDrawable::BattlefieldGraphicsDrawable(dtGame::GameActorProxy& owner)
      : IGActor(owner)
      {
      }

      /////////////////////////////////////////////////////////////////////
      bool BattlefieldGraphicsDrawable::ShouldBeVisible(const SimCore::VisibilityOptions& vo)
      {
         return vo.GetBasicOptions().mBattlefieldGraphics;
      }

      /////////////////////////////////////////////////////////////////////
      BattlefieldGraphicsDrawable::~BattlefieldGraphicsDrawable()
      {

      }


   }
}
