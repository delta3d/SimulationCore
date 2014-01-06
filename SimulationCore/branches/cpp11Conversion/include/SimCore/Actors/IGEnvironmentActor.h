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
#include <dtCore/environment.h>
#include <dtUtil/datetime.h>

//#include <SimCore/Actors/LensFlareDrawable.h>

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
      class LensFlareDrawable;

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
         void SetWind( const osg::Vec3& wind );
         const osg::Vec3& GetWind() const;

         virtual void EnableCloudPlane( bool enable );

         virtual bool IsCloudPlaneEnabled() const;

         const dtUtil::DateTime& GetDateTime() const;
         void SetDateTime(const dtUtil::DateTime& dateTime);

         void SetTimeFromSystem();

         //Set the density on the environments fog
         void SetDensity (float density);

         //Get the density on the environments fog
         float GetDensity();

         /// @return true if the fog is enabled
         bool IsFogEnabled() const;

         /// Turn the Fog on and off on the environment
         void SetFogEnabled(bool enable);

         void SetFogColor(const osg::Vec3& color);
         const osg::Vec3 GetFogColor();

         // to be called from a motion model
         void SetSkyDomesCenter(const osg::Vec3& position);

         void SetFogMode( dtCore::Environment::FogMode mode );

         void SetFogNear( float val );

         // Sets and Gets the visibility from the underlying environment actor
         void SetVisibility( float distance );
         float GetVisibility ();

         virtual void SetLatitudeAndLongitude( float latitude, float longitude );

         // Used to change the Cloud Plane texture to some Cloud Texture file that already exists
         // @param int That represents some numbered file
         void ChangeClouds(int, float, float);

         // Gets the number of the file that is currently being used for the cloud texture
         // @return The number of the Cloud texture file that is currently being used.
         int GetCloudCoverage() const;

         void SetTimeAndDateString( const std::string &timeAndDate );
         bool SetTimeAndDate( std::istringstream& iss );

         std::string GetTimeAndDateString() const;

         dtCore::CloudPlane* GetCloudPlane(){return mCloudPlane.get();};

         bool GetEnableLensFlare() const;
         void SetEnableLensFlare(bool b);
         LensFlareDrawable* GetLensFlareDrawable();
         const LensFlareDrawable* GetLensFlareDrawable() const;

         virtual osg::Vec3d GetSunPosition() const;

         bool GetInitializeSystemClock() const;
         void SetInitializeSystemClock(bool enable);

      protected:

         // Destructor
         virtual ~IGEnvironmentActor();

         void SetupCloudPlanes();

         //this function is called when the time on the dtCore::Environment changes
         virtual void OnTimeChanged();

         dtCore::Environment& GetCoreEnvironment();
         osg::Fog& GetFog();

         void InitLensFlare();

      private:

         bool mEnableCloudPlane, mEnableLensFlare, mInitSystemClock;
         time_t mCurrTime;
         osg::Vec3 mWind;
         std::shared_ptr<dtCore::CloudPlane> mCloudPlane;
         std::shared_ptr<dtCore::Environment> mEnvironment;
         std::shared_ptr<LensFlareDrawable> mLensFlare;
         osg::ref_ptr<osg::Fog> mFog;

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
         virtual void CreateDrawable();

         /// Adds the properties associated with this actor
         virtual void BuildPropertyMap();

         /// Called to build the invokables associated with a proxy
         virtual void BuildInvokables();

         /// Called when a proxy is added to the game manager
         virtual void OnEnteredWorld();

         //we override this to make the environment actor global.. ie return false
         /*virtual*/ bool IsPlaceable() const;

      protected:

         /// Destructor
         virtual ~IGEnvironmentActorProxy();

      private:

      };

   }
}

#endif

