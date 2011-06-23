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
 * @author Eddie Johnson
 * @author Curtiss Murphy
 */
#ifndef _PREFERENCES_GENERAL_CONFIG_OBJECT_H_
#define _PREFERENCES_GENERAL_CONFIG_OBJECT_H_

#include <dtUtil/enumeration.h>
#include <dtCore/refptr.h>
#include <dtCore/uniqueid.h>

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

#include <osg/Vec2f>
#include <osg/Vec3>

namespace dtGame
{
   class GameActorProxy;
}

namespace StealthGM
{
   class StealthInputComponent;


   class STEALTH_GAME_EXPORT PreferencesGeneralConfigObject : public ConfigurationObjectInterface
   {
      public:

         class STEALTH_GAME_EXPORT AttachMode : public dtUtil::Enumeration
         {
            DECLARE_ENUM(AttachMode);

            public:

               static const AttachMode FIRST_PERSON;
               static const AttachMode THIRD_PERSON;

            private:
               AttachMode(const std::string& name);
            };

         class STEALTH_GAME_EXPORT PerformanceMode : public dtUtil::Enumeration
         {
            DECLARE_ENUM(PerformanceMode);

            public:

               static const PerformanceMode BEST_GRAPHICS;
               static const PerformanceMode BETTER_GRAPHICS;
               static const PerformanceMode DEFAULT;
               static const PerformanceMode BETTER_SPEED;
               static const PerformanceMode BEST_SPEED;

            private:
               PerformanceMode(const std::string& name);
         };

         /// Constructor
         PreferencesGeneralConfigObject();

         /**
          * Overridden base class method to apply the changes made to this class to the
          * game manager
          */
         virtual void ApplyChanges(dtGame::GameManager &gameManager);

         /**
          * Sets the attach mode
          * @param newMode The new attach mode
          */
         void SetAttachMode(const AttachMode &newMode) { mAttachMode = &newMode; SetIsUpdated(true); }

         /**
          * Set overload
          */
         void SetAttachMode(const std::string &mode);

         /**
          * Returns the current attach mode
          * @return mAttachMode
          */
         const AttachMode& GetAttachMode() const { return *mAttachMode; }

         /**
          * Sets camera collision
          * @param enable True to enable, false to disable
          */
         void SetCameraCollision(bool enable) { mEnableCameraCollision = enable; SetIsUpdated(true); }

         /**
          * Returns true if camera collision is enabled
          * @return mEnableCameraCollision
          */
         bool GetEnableCameraCollision() const { return mEnableCameraCollision; }

         /**
          * Sets the performance mode
          * @param mode The performance mode
          */
         void SetPerformanceMode(const PerformanceMode &mode) { mPerformanceMode = &mode; SetIsUpdated(true); }

         /**
          * Set overload
          */
         void SetPerformanceMode(const std::string &mode);

         /**
          * Returns the current performance mode
          * @return mPerformanceMode
          */
         const PerformanceMode& GetPerformanceMode() const { return *mPerformanceMode; }

         /**
          * Sets the LOD scale
          * @param scale The new scale
          */
         void SetLODScale(float scale) { mLODScale = scale; SetIsUpdated(true); }

         /**
          * Gets the LOD scale
          * @return mLODScale
          */
         float GetLODScale() const { return mLODScale; }

         /**
          * Sets showing the advanced options
          * @param enable True if showing, false to disable
          */
         void SetShowAdvancedOptions(bool enable) { mShowAdvancedOptions = enable; SetIsUpdated(true); }

         /**
          * Returns showing advanced options
          * @param mShowAdvancedOptions
          */
         bool GetShowAdvancedOptions() const { return mShowAdvancedOptions; }

         /**
          * Sends an attach to actor message on the next tick
          * @param proxy The proxy to attach to
          */
         void AttachToActor(const dtCore::UniqueId& id) { mAttachActorId = id; SetIsUpdated(true); }

         /**
          * Tells the Stealth Actor to detach - sends an attach with a NULL actor.
          */
         void DetachFromActor() { mDetachFromActor = true; SetIsUpdated(true); }

         /// Sets the node name to use as an attach point.  Empty means attach to the entity as a whole.
         void SetAttachPointNodeName(const std::string& name);

         /// @return the node name to use as an attach point.  Empty means attach to the entity as a whole.
         const std::string& GetAttachPointNodeName() const;

         /// Sets the initial rotation offset in reference to the entity to which to attach.
         void SetInitialAttachRotationHPR(const osg::Vec3& hpr);
         /// @return the initial rotation offset in reference to the entity to which to attach.
         const osg::Vec3& GetInitialAttachRotationHPR() const;

         /// Resets the attachment of the stealth actor to it the current actor is attached to.
         void Reattach();

         /**
          * Checks to see if the stealth actor is currently attached.
          * @return true if the stealth actor is attached.
          */
         bool IsStealthActorCurrentlyAttached();

         /**
          * Sets connecting to a network on startup
          * @param enable True to connect, false to start ordinarily
          * @param name The name of the connection
          */
         void SetReconnectOnStartup(bool enable, const std::string &name)
         {
            mReconnectOnStartup = enable;
            SetStartupConnectionName(name);
         }

         /**
          * Returns true to reconnect on start up
          * @return mReconnectOnStartup
          */
         bool GetReconnectOnStartup() const { return mReconnectOnStartup; }

         /**
          * Sets the connection name to reconnect to
          * @param name The connection name
          */
         void SetStartupConnectionName(const std::string &name) { mStartupConnectionName = name;}

         /**
          * Gets the connection name to reconnect to
          * @return mStartupConnectionName
          */
         const std::string& GetStartupConnectionName() const { return mStartupConnectionName; }

         /**
          * Sets auto refreshing the entity info window
          * @param enable True to enable, false otherwise
          */
         void SetAutoRefreshEntityInfoWindow(bool enable) { mAutoRefreshEntityInfo = enable; }

         /**
          * Gets refreshing the entity info window
          * @return mAutoRefreshEntityInfo
          */
         bool GetAutoRefreshEntityInfoWindow() const { return mAutoRefreshEntityInfo; }

         void SetShouldAutoAttachToEntity(bool);
         bool GetShouldAutoAttachToEntity() const;

         void SetAutoAttachEntityCallsign(const std::string& callsign);
         const std::string& GetAutoAttachEntityCallsign() const;

      protected:

         /// Destructor
         virtual ~PreferencesGeneralConfigObject();

         void AttachOrDetach(dtGame::GameManager& gameManager);
      private:

         const AttachMode* mAttachMode;
         bool mEnableCameraCollision;
         const PerformanceMode* mPerformanceMode;

         float mLODScale;
         bool mShowAdvancedOptions;
         dtCore::UniqueId mAttachActorId;
         bool mReconnectOnStartup;
         std::string mStartupConnectionName;
         bool mAutoRefreshEntityInfo;
         bool mDetachFromActor;
         dtCore::RefPtr<StealthInputComponent> mInputComponent;

         std::string mAttachPointNodeName;
         osg::Vec3 mInitialAttachRotationHPR;

         bool mShouldAutoAttachToEntity;
         std::string mAutoAttachEntityCallsign;
   };
}
#endif
