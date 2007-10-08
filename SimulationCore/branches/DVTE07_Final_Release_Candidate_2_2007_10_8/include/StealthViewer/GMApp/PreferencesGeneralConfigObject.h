/* -*-c++-*-
 * Copyright, 2006, Alion Science and Technology Corporation, all rights reserved.
 * 
 *    Alion Science and Technology Corporation
 *    5365 Robin Hood Road
 *    Norfolk, VA 23513
 *    (757) 857-5670, www.alionscience.com
 *
 * This software was developed by Alion Science and Technology Corporation under circumstances in which the U. S. Government may have rights in the software.
 * 
 * @author Eddie Johnson
 */
#ifndef _PREFERENCES_GENERAL_CONFIG_OBJECT_H_
#define _PREFERENCES_GENERAL_CONFIG_OBJECT_H_

#include <dtUtil/enumeration.h>

#include <StealthViewer/GMApp/ConfigurationObjectInterface.h>
#include <StealthViewer/GMApp/Export.h>

namespace dtGame
{
   class GameActorProxy;
}

namespace StealthGM
{
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

               AttachMode(const std::string &name) : dtUtil::Enumeration(name)
               {
                  AddInstance(this);
               }
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

               PerformanceMode(const std::string &name) : dtUtil::Enumeration(name)
               {
                  AddInstance(this);
               }
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
          * Sets the near clipping plane
          * @param plane The new plane
          */
         void SetNearClippingPlane(double plane) { mNearClippingPlane = plane; SetIsUpdated(true); }

         /**
          * Gets the near clipping plane
          * @return mNearClippingPlane
          */
         double GetNearClippingPlane() const { return mNearClippingPlane; }

         /**
          * Sets the far clipping plane
          * @param plane The new plane
          */
         void SetFarClippingPlane(double plane) { mFarClippingPlane = plane; SetIsUpdated(true); }

         /**
          * Gets the far clipping plane
          * @return mFarClippingPlane
          */
         double GetFarClippingPlane() const { return mFarClippingPlane; }

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
         void AttachToActor(const dtGame::GameActorProxy &proxy) { mAttachProxy = &proxy; SetIsUpdated(true); }
      
         /**
          * Sets connecting to a federation on startup
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

      protected:

         /// Destructor
         virtual ~PreferencesGeneralConfigObject();

      private:

         const AttachMode *mAttachMode;
         bool mEnableCameraCollision;
         const PerformanceMode *mPerformanceMode;
         float mLODScale;
         double mNearClippingPlane;
         double mFarClippingPlane;
         bool mShowAdvancedOptions;
         const dtGame::GameActorProxy *mAttachProxy;
         bool mReconnectOnStartup;
         std::string mStartupConnectionName;
         bool mAutoRefreshEntityInfo;
   };
}
#endif
