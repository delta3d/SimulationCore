/* -*-c++-*-
* Simulation Core
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
 * @author Chris Rodgers
 */
#ifndef _BASIC_ENVIRONMENT_ACTOR_H_
#define _BASIC_ENVIRONMENT_ACTOR_H_

#include <SimCore/Export.h>
#include <dtGame/environmentactor.h>
#include <ctime>
#include <dtCore/environment.h>
#include <osgEphemeris/EphemerisModel>
#include <dtUtil/datetime.h>

namespace osg
{
   class Fog;
}

namespace dtCore
{
   class CloudPlane;
   class DeltaDrawable;
}

namespace SimCore
{
   namespace Actors
   {
      ////////////////////////////////////////////////////////////////////////
      // Actor Code
      ////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT IGEnvironmentActor : public dtGame::IEnvGameActor
      {
      public:

         typedef dtGame::IEnvGameActor BaseClass;
         
         // Constructor
         IGEnvironmentActor( dtGame::GameActorProxy& proxy );

         // Adds an actor proxy to the internal hierarchy of the environment
         // @param actor The proxy to add/
         virtual void AddActor( dtCore::DeltaDrawable& actor );

         // Removes an actor proxy from the internal hierarchy
         // @param actor The proxy to remove
         virtual void RemoveActor( dtCore::DeltaDrawable& actor );

         // Removes all actors associated with this environment
         virtual void RemoveAllActors();

         virtual void OnEnteredWorld();

         // Called to see if this environment has the specified proxy
         // @param proxy The proxy to look for
         // @return True if it contains it, false if not
         virtual bool ContainsActor( dtCore::DeltaDrawable& actor ) const;

         // Gets all the actors associated with this environment
         // @param outActorList The vector to fill
         virtual void GetAllActors( std::vector<dtCore::DeltaDrawable*>& outActorList );

         virtual unsigned int GetNumEnvironmentChildren() const;

         // Sets the current wind force
         void SetWind( const osg::Vec3& wind ) { mWind = wind; }
         osg::Vec3 GetWind() const { return mWind; }

         virtual void EnableCloudPlane( bool enable );

         virtual bool IsCloudPlaneEnabled() const;

         const dtUtil::DateTime& GetDateTime() const;
         void SetDateTime(const dtUtil::DateTime& dateTime);

         void SetTimeFromSystem();

         //Turn the Fog on and off on the environment
         void EnableFog(bool enable);

         //Set the density on the environments fog
         void SetDensity (float density);

         //Get the density on the environments fog
         float GetDensity ();
 
         void SetEphemerisFog(bool fog_toggle );

         bool IsFogEnabled() const { return mEnvironment->GetFogEnable(); }

         void SetFogColor( const osg::Vec3& color );
         const osg::Vec3 GetFogColor() 
         {
            osg::Vec3 color;
            mEnvironment->GetFogColor(color);
            return color;
         }

         // to be called from a motion model
         void SetSkyDomesCenter(const osg::Vec3& position);

         osgEphemeris::EphemerisModel* GetEphemerisModel();

         void SetFogMode( dtCore::Environment::FogMode mode );

         void SetFogNear( float val );

         // Sets and Gets the visibility from the underlying environment actor
         void SetVisibility( float distance );
         float GetVisibility ();

         void SetLatitudeAndLongitude( float latitude, float longitude );

         osgEphemeris::Sphere* GetFogSphere() {return mFogSphere.get();}

         // Used to change the Cloud Plane texture to some Cloud Texture file that already exists
         // @param int That represents some numbered file
         void ChangeClouds(int, float, float);

         // Gets the number of the file that is currently being used for the cloud texture
         // @return The number of the Cloud texture file that is currently being used.
         int GetCloudCoverage() const;

         void SetTimeAndDateString( const std::string &timeAndDate );
         bool SetTimeAndDate( std::istringstream& iss );

         std::string GetTimeAndDateString() const;

      protected:

         // Destructor
         virtual ~IGEnvironmentActor();

         void SetupCloudPlanes();   

         //this function is called when the time on the dtCore::Environment changes
         virtual void OnTimeChanged();
      
      private:

         //The following class is take from the osgEphemeris class under the following Liscense
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
                MoveWithEyePointTransform():_enabled(true) {}

                void setCenter( osg::Vec3 center ) { _center = center; }
                void enable()  { _enabled = true; }
                void disable() { _enabled = false; }
                bool isEnabled() const { return _enabled; }

                virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const;
            private:
                bool _enabled;
                osg::Vec3 _center;
        };


         


         bool mEnableCloudPlane;
         time_t mCurrTime;
         osg::Vec3 mWind;
         dtCore::RefPtr<dtCore::CloudPlane> mCloudPlane;
         dtCore::RefPtr<dtCore::Environment> mEnvironment;
         dtCore::RefPtr<osgEphemeris::EphemerisModel> mEphemerisModel;
         
         //used to create a Fog Line between the Ephemeris Model and the Terrain Horizon
         dtCore::RefPtr<osgEphemeris::Sphere>      mFogSphere;
         dtCore::RefPtr<MoveWithEyePointTransform> mFogSphereEyePointTransform;
         dtCore::RefPtr<osg::Fog>                  mFog;

         int mCloudCoverage;
      };

      ////////////////////////////////////////////////////////////////////////
      // Proxy Code
      ////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT IGEnvironmentActorProxy : public dtGame::IEnvGameActorProxy
      {
      public:

         /// Constructor
         IGEnvironmentActorProxy();

         /// Creates the actor
         virtual void CreateActor() { SetActor( *new IGEnvironmentActor(*this) ); }

         /// Adds the properties associated with this actor
         virtual void BuildPropertyMap();

         /// Called to build the invokables associated with a proxy
         virtual void BuildInvokables();

         /// Called when a proxy is added to the game manager
         virtual void OnEnteredWorld();

      protected:

         /// Destructor
         virtual ~IGEnvironmentActorProxy();

      private:

      };

   }
}

#endif
