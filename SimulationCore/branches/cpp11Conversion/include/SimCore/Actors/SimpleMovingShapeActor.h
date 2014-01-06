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
 * David Guthrie
 */

#ifndef SIMPLEMOVINGSHAPEACTOR_H_
#define SIMPLEMOVINGSHAPEACTOR_H_

#include <SimCore/Export.h>
#include <dtGame/gameactorproxy.h>
#include <dtUtil/getsetmacros.h>
#include <SimCore/Components/VolumeRenderingComponent.h>
#include <dtGame/deadreckoninghelper.h>
#include <osg/MatrixTransform>


namespace SimCore
{

   namespace Actors
   {

      class SIMCORE_EXPORT SimpleMovingShapeActorProxy : public dtGame::GameActorProxy
      {
      public:
         typedef dtGame::GameActorProxy BaseClass;

         class SIMCORE_EXPORT SimpleShapeDRHelper : public dtGame::DeadReckoningHelper
         {
            public:
               typedef dtGame::DeadReckoningHelper BaseClass;

               SimpleShapeDRHelper();

               virtual bool DoDR(dtGame::GameActor& gameActor, dtCore::Transform& xform, dtUtil::Log* pLogger, dtGame::BaseGroundClamper::GroundClampRangeType*& gcType);

               virtual void CalculateSmoothingTimes(const dtCore::Transform& xform);

               virtual void IncrementTimeSinceUpdate(float simTimeDelta, float curSimulationTime);

               float mCurTimeDelta;
               DeadReckoningHelper::DRVec3Util mDRScale;

         };

         static const dtUtil::RefString PROPERTY_LAST_KNOWN_DIMENSIONS;
         static const dtUtil::RefString PROPERTY_DIMENSIONS_VELOCITY_VECTOR;

         SimpleMovingShapeActorProxy();

         virtual void BuildActorComponents();

         void SetLastKnownDimension(const osg::Vec3& r);
         const osg::Vec3& GetLastKnownDimension() const;

         void SetLastKnownDimensionVelocity(const osg::Vec3& r);
         const osg::Vec3& GetLastKnownDimensionVelocity() const;

         DT_DECLARE_ACCESSOR(dtCore::UniqueId, Owner);
         DT_DECLARE_ACCESSOR(int, Index);

         /** @return the volume based on the dimensions an the shape represented.
          *  defaults to an ellipsoid.
          */
         virtual float CalculateVolume() const;

         void SetCurrentDimensions(const osg::Vec3& dim);
         const osg::Vec3& GetCurrentDimensions() const;

         void SetShapeColor(const osg::Vec3& color);
         const osg::Vec3& GetShapeColor() const;

         void SetDensityMultiplier(float density);
         float GetDensityMultiplier() const;

         void SetMass(float milligrams);
         float GetMass() const;

      protected:
         virtual ~SimpleMovingShapeActorProxy();

         virtual void OnEnteredWorld();

         //virtual void OnRemovedFromWorld() { }

         virtual void CreateDrawable();
         virtual void BuildPropertyMap();

         unsigned ComputeNumParticles(const osg::Vec3& dims);
         float ComputeDensity();

      private:

         bool mIsCreated;
         float mMass;
         float mDensityMultiplier;
         osg::Vec3 mDimensions;
         osg::Vec3 mShapeColor;
         std::shared_ptr<SimpleShapeDRHelper> mDRHelper;
         std::shared_ptr<SimCore::Components::VolumeRenderingComponent::ShapeVolumeRecord> mShapeVolume;


      };

   }

}

#endif /* SIMPLEMOVINGSHAPEACTOR_H_ */
