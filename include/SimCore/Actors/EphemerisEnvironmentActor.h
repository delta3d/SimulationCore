/* -*-c++-*-
 * SimulationCore@SimulationCore
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
 * david
 */

#ifndef EPHEMERIS_ENVIRONMENT_ACTOR
#define EPHEMERIS_ENVIRONMENT_ACTOR

#include <SimCore/Export.h>
#include <SimCore/Actors/IGEnvironmentActor.h>

#include <osgEphemeris/EphemerisModel.h>

namespace SimCore
{
   namespace Actors
   {

      class SIMCORE_EXPORT EphemerisEnvironmentActor: public SimCore::Actors::IGEnvironmentActor
      {
      public:
         typedef SimCore::Actors::IGEnvironmentActor BaseClass;
         EphemerisEnvironmentActor(dtGame::GameActorProxy& parent);

         virtual void OnEnteredWorld();

         void SetEphemerisFog(bool fog_toggle );

         osgEphemeris::EphemerisModel* GetEphemerisModel();

         virtual void SetLatitudeAndLongitude( float latitude, float longitude );

         virtual void OnTimeChanged();

         osg::Transform* GetFogSphere();

         /*virtual*/ osg::Vec3d GetSunPosition() const;

         void BindShader(const std::string& shader, osg::Node* g);

      protected:
         virtual ~EphemerisEnvironmentActor();
      private:

         //The following class is take from the osgEphemeris class under the following License
         /* -*-c++-*- OpenSceneGraph - Ephemeris Model Copyright (C) 2005 Don Burns
          *
          * This library is open source and may be redistributed and/or modified under
          * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
          * (at your option) any later version.  The full license is in LICENSE file
          * included with this distribution, and on the openscenegraph.org website.
          *
          * This library is distributed in the hope that it will be useful,
          * but WITHOUT ANY WARRANTY; without even the implied warranty of
          * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
          * OpenSceneGraph Public License for more details.
          */
         class MoveWithEyePointTransform : public osg::Transform
         {
         public:
            MoveWithEyePointTransform()
            :mEnabled(true)
            {}

            void setCenter( osg::Vec3 center ) { mCenter = center; }
            void enable()  { mEnabled = true; }
            void disable() { mEnabled = false; }
            bool isEnabled() const { return mEnabled; }

            virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;
         private:
            bool mEnabled;
            osg::Vec3 mCenter;
         };


         dtCore::RefPtr<osgEphemeris::EphemerisModel> mEphemerisModel;
         //used to create a Fog Line between the Ephemeris Model and the Terrain Horizon
         dtCore::RefPtr<osgEphemeris::Sphere>      mFogSphere;
         dtCore::RefPtr<MoveWithEyePointTransform> mFogSphereEyePointTransform;
      };

      class EphemerisEnvironmentActorProxy: public SimCore::Actors::IGEnvironmentActorProxy
      {
      public:
         EphemerisEnvironmentActorProxy();
         /// Creates the actor
         virtual void CreateDrawable();
      protected:
         virtual ~EphemerisEnvironmentActorProxy();
      };

   }

}

#endif /* EPHEMERIS_ENVIRONMENT_ACTOR */
