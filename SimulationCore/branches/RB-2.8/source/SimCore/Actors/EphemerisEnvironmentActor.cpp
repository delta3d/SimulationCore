/* -*-c++-*-
 * SimulationCore
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
 *
 * David Guthrie
 */

#include <prefix/SimCorePrefix.h>
#include <SimCore/Actors/EphemerisEnvironmentActor.h>
#include <osgEphemeris/EphemerisData.h>
#include <dtCore/shadermanager.h>
#include <ctime>

#include <osg/Depth>
#include <osg/Fog>
#include <osg/StateSet>
#include <osg/Projection>
#include <SimCore/Components/RenderingSupportComponent.h>

namespace SimCore
{
   struct FogBoundingBoxCallback: public osg::Drawable::ComputeBoundingBoxCallback
   {
      virtual osg::BoundingBox computeBound(const osg::Drawable& d) const
      {
         return osg::BoundingBox();
      }
   };


   class BBVisitor : public osg::NodeVisitor
   {
   public:

      BBVisitor(): osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
      {

      }

      virtual void apply(osg::Geode& geode)
      {
         unsigned pNumDrawables = geode.getNumDrawables();
         for(unsigned i = 0; i < pNumDrawables; ++i)
         {
            osg::Drawable* draw = geode.getDrawable(i);
            draw->setComputeBoundingBoxCallback(new FogBoundingBoxCallback());
         }

      }
   };

   namespace Actors
   {

      EphemerisEnvironmentActor::EphemerisEnvironmentActor(dtGame::GameActorProxy& proxy)
      : BaseClass(proxy)
      , mEphemerisModel(new osgEphemeris::EphemerisModel())
      , mFogSphere(0)
      , mFogSphereEyePointTransform(new MoveWithEyePointTransform())

      {
         mEphemerisModel->setSkyDomeRadius( 499.0f );
         mEphemerisModel->setSunLightNum(0);
         mEphemerisModel->setMoveWithEyePoint(true);

         osgEphemeris::DateTime osgDT(true);
         mEphemerisModel->setDateTime(osgDT);

         //FogSphere SetUp
         mFogSphere = new osgEphemeris::Sphere( 497.0f,
            osgEphemeris::Sphere::TessLow,
            osgEphemeris::Sphere::OuterOrientation,
            osgEphemeris::Sphere::BothHemispheres,
            false
            );

         // Change render order and depth writing.
         osg::StateSet* states = mEphemerisModel->getOrCreateStateSet();
         osg::Depth* depthState = new osg::Depth(osg::Depth::ALWAYS, 1.0f , 1.0f );
         states->setAttributeAndModes(depthState);
         states->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
         states->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
         states->setRenderBinDetails( SimCore::Components::RenderingSupportComponent::RENDER_BIN_ENVIRONMENT, "RenderBin" );

		 //disable shadows on the ephemeris 
		 mEphemerisModel->setNodeMask(SimCore::Components::RenderingSupportComponent::DISABLE_SHADOW_NODE_MASK);

         //Set up the Fog Sphere so that it can be rendered
         osg::StateSet* fogSphereStates = mFogSphere->getOrCreateStateSet();
         osg::Depth* depthFogState = new osg::Depth(osg::Depth::ALWAYS, 1.0f , 1.0f );
         fogSphereStates->setAttributeAndModes(depthFogState);
         fogSphereStates->setAttributeAndModes(&GetFog());
         fogSphereStates->setMode(GL_FOG, osg::StateAttribute::ON);
         fogSphereStates->setMode(GL_LIGHTING, osg::StateAttribute::ON);
         fogSphereStates->setMode(GL_BLEND, osg::StateAttribute::ON);
         fogSphereStates->setRenderBinDetails( SimCore::Components::RenderingSupportComponent::RENDER_BIN_ENVIRONMENT + 2, "RenderBin" );

		 //disable shadows on the fog sphere
		 mFogSphere->setNodeMask(SimCore::Components::RenderingSupportComponent::DISABLE_SHADOW_NODE_MASK);

      }

      /////////////////////////////////////////////////////////////
      EphemerisEnvironmentActor::~EphemerisEnvironmentActor()
      {
//         LOGN_DEBUG("EphemerisEnvironmentActor", "Deleting EphemerisData");
//         osgEphemeris::EphemerisData* ephem = mEphemerisModel->getEphemerisData();
//         delete ephem;
      }

      /////////////////////////////////////////////////////////////
      void EphemerisEnvironmentActor::OnEnteredWorld()
      {
         BaseClass::OnEnteredWorld();
         dtCore::Environment& coreEnv = GetCoreEnvironment();
         //osg::Matrix projectMat;
         //projectMat.makePerspective(60, 1.5, 10, 996);
         //dtCore::RefPtr<osg::Projection> projection = new osg::Projection(projectMat);
         //coreEnv.GetOSGNode()->asGroup()->addChild(projection.get());
         osg::Group* envGroup = coreEnv.GetOSGNode()->asGroup();
         envGroup->addChild(mEphemerisModel.get());

         //here we setup the FogSphere's state set and add it to the scene
         envGroup->addChild(mFogSphereEyePointTransform.get());
         mFogSphereEyePointTransform->addChild(mFogSphere.get());

         //we give all the drawables on the fog sphere a large bounding box to ensure it is always rendered
         size_t sizeOfNH = mFogSphere->getNorthernHemisphere()->getNumDrawables();
         for(size_t i = 0; i < sizeOfNH; ++i)
         {
            mFogSphere->getNorthernHemisphere()->getDrawable(i)->setComputeBoundingBoxCallback(new FogBoundingBoxCallback());
         }

         //we give all the drawables on the fog sphere a large bounding box to ensure it is always rendered
         size_t sizeOfSH = mFogSphere->getSouthernHemisphere()->getNumDrawables();
         for(size_t i = 0; i < sizeOfSH; ++i)
         {
            mFogSphere->getSouthernHemisphere()->getDrawable(i)->setComputeBoundingBoxCallback(new FogBoundingBoxCallback());
         }

         mFogSphereEyePointTransform->setCenter(mEphemerisModel->getSkyDomeCenter());

         //this little hack will create a large bounding volume for the ephemeris to ensure it doesn't
         //get culled out
         BBVisitor bbv;
         mEphemerisModel->traverse(bbv);


         dtCore::ShaderManager::GetInstance().UnassignShaderFromNode(*mFogSphere.get());

         //First get the shader group assigned to this actor.
         const dtCore::ShaderGroup *shaderGroup =
         dtCore::ShaderManager::GetInstance().FindShaderGroupPrototype("EphemerisFogGroup");

         if (shaderGroup == NULL)
         {
            LOG_INFO("Could not find shader group EphemerisFogGroup");
            return;
         }

         const dtCore::ShaderProgram *defaultShader = shaderGroup->GetDefaultShader();

         try
         {
            if (defaultShader != NULL)
            {
               dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*defaultShader, *mFogSphere.get());
            }
            else
            {
               LOG_WARNING("Could not find a default shader in shader group: EphemerisFogGroup");
            }
         }
         catch (const dtUtil::Exception &e)
         {
            LOG_WARNING("Caught Exception while assigning shader: " + e.ToString());
         }
      }

      /////////////////////////////////////////////////////////////
      void EphemerisEnvironmentActor::SetEphemerisFog(bool fog_toggle)
      {
         if(fog_toggle == true)
         {
            mFogSphere->setNodeMask(~0);
         }
         else
         {
            mFogSphere->setNodeMask(0);
         }
      }

      /////////////////////////////////////////////////////////////
      osgEphemeris::EphemerisModel* EphemerisEnvironmentActor::GetEphemerisModel()
      {
         return mEphemerisModel.get();
      }

      /////////////////////////////////////////////////////////////
      void EphemerisEnvironmentActor::SetLatitudeAndLongitude(float latitude, float longitude)
      {
         //this updates the sky and sun colors
         if(mEphemerisModel.valid())
         {
            mEphemerisModel->setLatitudeLongitude(latitude, longitude);
         }

         BaseClass::SetLatitudeAndLongitude(latitude, longitude);
      }

      /////////////////////////////////////////////////////////////
      osg::Transform* EphemerisEnvironmentActor::GetFogSphere()
      {
         return mFogSphereEyePointTransform.get();
      }

      /////////////////////////////////////////////////////////////
      void EphemerisEnvironmentActor::OnTimeChanged()
      {
         BaseClass::OnTimeChanged();

         //update the ephemeris with the proper time
         mEphemerisModel->setAutoDateTime( false );

         osgEphemeris::EphemerisData* ephem = mEphemerisModel->getEphemerisData();

         dtUtil::DateTime dt(GetDateTime().GetGMTTime());

         LOGN_DEBUG("IGEnvironmentActor.cpp", dt.ToString());
         if (ephem != NULL)
         {

            ephem->dateTime.setYear(dt.GetYear()); // DateTime uses _actual_ year (not since 1900)
            ephem->dateTime.setMonth(dt.GetMonth());    // DateTime numbers months from 1 to 12, not 0 to 11
            ephem->dateTime.setDayOfMonth(dt.GetDay()); // DateTime numbers days from 1 to 31, not 0 to 30
            ephem->dateTime.setHour(dt.GetHour());
            ephem->dateTime.setMinute(dt.GetMinute());
            ephem->dateTime.setSecond(int(dt.GetSecond()));
         }
         else
         {
            LOG_ERROR("Ephemeris Data is NULL");
         }
      }

      /////////////////////////////////////////////////////////////
      osg::Vec3d EphemerisEnvironmentActor::GetSunPosition() const
      {
         if(mEphemerisModel.valid())
         {
            return mEphemerisModel->getSunPosition();
         }
         else
         {
            return BaseClass::GetSunPosition();
         }
      }

      /////////////////////////////////////////////////////////////
      bool EphemerisEnvironmentActor::MoveWithEyePointTransform::computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
      {
         if( mEnabled )
         {
            osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
            if (cv)
            {
               osg::Vec3 eyePointLocal = cv->getEyeLocal();
               matrix.preMult(osg::Matrix::translate(
                  osg::Vec3( eyePointLocal.x(), eyePointLocal.y(), eyePointLocal.z()) - mCenter));
            }
         }
         return true;
      }

      /////////////////////////////////////////////////////////////
      /////////// Actor Proxy             /////////////////////////
      /////////////////////////////////////////////////////////////
      EphemerisEnvironmentActorProxy::EphemerisEnvironmentActorProxy()
      {
         SetClassName("SimCore::Actors::EphemerisEnvironmentActor");
         SetHideDTCorePhysicsProps(true);
      }

      /////////////////////////////////////////////////////////////
      void EphemerisEnvironmentActorProxy::CreateDrawable()
      {
         SetDrawable( *new EphemerisEnvironmentActor(*this) );
      }

      /////////////////////////////////////////////////////////////
      EphemerisEnvironmentActorProxy::~EphemerisEnvironmentActorProxy()
      {
      }
   }

}
