/* -*-c++-*-
* Delta3D Open Source Game and Simulation Engine
* Copyright (C) 2006, Alion Science and Technology, BMH Operation
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
* @author Allen Danklefsen, with guest appearance by Bradley Anderegg
*/

#ifndef _RENDERING_COMPONENT_
#define _RENDERING_COMPONENT_

// project includes needed
#include <SimCore/Export.h>
#include <dtGame/gmcomponent.h>

#include <SimCore/AgeiaTerrainCullVisitor.h>
#include <osgUtil/CullVisitor>
#include <osgUtil/SceneView>
#include <osgUtil/StateGraph>
#include <osgUtil/RenderStage>

#include <dtUtil/functor.h>

#include <vector>

namespace dtGame
{
   class GameManager;
   class TickMessage;
   class ActorPublishedMessage;
   class ActorDeletedMessage;
   class ActorUpdateMessage;
}

namespace dtCore
{
   class Camera;
   class DeltaDrawable;
   class Transformable;
}
namespace osg
{
   class CameraNode;
}

namespace SimCore
{
   namespace Actors
   {
      class DynamicLightPrototypeProxy;
   }

   namespace Components
   {
      //class in cpp
      class UpdateViewCallback;

      ///////////////////////////////////////////////////////
      //    The Component
      ///////////////////////////////////////////////////////
      class SIMCORE_EXPORT RenderingSupportComponent : public dtGame::GMComponent
      {
         public:
            typedef unsigned LightID;
            static const unsigned MAX_LIGHTS;

            struct SIMCORE_EXPORT DynamicLight: public osg::Referenced
            {                          
               protected:
                  /*virtual*/ ~DynamicLight(){}
                  DynamicLight(const DynamicLight&); //un-implemented
                  DynamicLight& operator=(const DynamicLight&); //un-implemented

               public:

                  DynamicLight();

                  //this flag is used internally for removing, but I suppose if you want to set it you can
                  bool mDeleteMe;

                  //these variables effect the way the light appears..
                  float mIntensity; //the intensity is a multiplier of the effect of the light, can be used to disable or enable a light, typically 1 or 0
                  float mSaturationIntensity; //this is currently not used for performance
                  osg::Vec3 mColor;
                  osg::Vec3 mPosition;
                  osg::Vec3 mAttenuation; //this controls how far the light is visible from the vec3 represents (constant, linear, quadratic) attentions

                  //these variables effect the simulation of the light
                  bool mFlicker; //a flickering light will automatically increase and decrease its intensity relative to the flicker scale
                  float mFlickerScale;  //the flicker scale should be the maximum increase or decrease in the light intensity in a single frame
                                        //and also the maximum it will vary from its original intensity
                  
                  bool mAutoDeleteAfterMaxTime; //using this flag will set the light to be automatically removed after the number of seconds
                  float mMaxTime;            //specified by mMaxTime, this can be used in conjunction with Fade Out
                  
                  bool mFadeOut;      //if this is set to true we will gradually decrease our intensity over the time specified
                  float mFadeOutTime; //NOTE: if used in accordance with mMaxTime OR mAutoDeleteLightOnTargetNull then the fade out will
                                      //  occur after the object is marked for deletion.  So if MaxTime = 1.0 and FadeOutTime = 0.5
                                      //  the light will be at 100% intensity for 1.5 seconds and then fade from 100% to 0% over 0.5 seconds

                  float mRadius;      //this is used to determine how far away we are from the light, it pretty much makes the light into a
                                      //bounding sphere

                  LightID mID;
                  static LightID mLightCounter;
                  
                  bool mAutoDeleteLightOnTargetNull; //setting this flag will auto delete the light when the target becomes NULL, this
                                                     //can be used in conjunction with Fade Out

                  dtCore::ObserverPtr<dtCore::Transformable> mTarget;

            };


            typedef std::vector<dtCore::RefPtr<DynamicLight> > LightArray; 

            class RenderFeature: public osg::Referenced
            {
            public:
               virtual void SetEnable(bool pEnable) = 0;
               virtual void Init(osg::Group* parent, dtCore::Camera* cam) = 0;
               virtual void Update() = 0;

            };

            static const std::string& DEFAULT_NAME;

            /// Constructor
            RenderingSupportComponent(const std::string &name = DEFAULT_NAME);

            // Convenience method to add a new dynamic light by looking it up from the prototypes. Returns the unique dynamic light instance
            DynamicLight *AddDynamicLightByPrototypeName(const std::string prototypeName);

            LightID AddDynamicLight(DynamicLight*);
            void RemoveDynamicLight(LightID id);
            DynamicLight* GetDynamicLight(LightID id);

            bool GetEnableNVGS();
            void SetEnableNVGS(bool pEnable);

            bool GetEnableCullVisitor();
            void SetEnableCullVisitor(bool pEnable);
            
            void SetNVGS(RenderFeature* rf);
            const RenderFeature* GetNVGS() const;

            void SetEnableDynamicLights(bool);
            bool GetEnableDynamicLights() const;

            void SetGUI(dtCore::DeltaDrawable* gui);

            /**
            * Processes messages sent from the Game Manager
            * @param The message to process
            * @see dtGame::GameManager
            */
            virtual void ProcessMessage(const dtGame::Message &msg);

            // loads cull visitor stuff.
            void OnAddedToGM();

            // so its decoupled from tick local, and tick local is not cluttered
            // with future stuff.
            bool UpdateCullVisitor();

            //this function is used internally by a class in the cpp file
            //which calculates the view matrix on the cull callback
            void UpdateViewMatrix(const osg::Matrix&);

         protected:
            /// Destructor
            virtual ~RenderingSupportComponent(void);

            void RemoveLight(LightArray::iterator);
            DynamicLight* FindLight(LightID id);

            void SetPosition(DynamicLight* dl);

            void InitializeCullVisitor();
            void InitializeFrameBuffer();

            // Finds all the dynamic light actor prototypes in the GM and holds onto them
            void LoadPrototypes();

            virtual void ProcessTick(const dtGame::TickMessage &msg);

            void UpdateDynamicLights(float dt);

         public:
            //here we define constants for defining the render bins
            //so we don't have hard coded render bin numbers all over the place
            static const int RENDER_BIN_ENVIRONMENT       = -2;
            static const int RENDER_BIN_SKY_AND_ATMOSPHERE=  9;
            static const int RENDER_BIN_PRECIPITATION     = 11;
            static const int RENDER_BIN_TRANSPARENT       = 10;
            static const int RENDER_BIN_HWMVV_INTERIOR    = 15;
            static const int RENDER_BIN_HUD               = 20;
            static const int RENDER_BIN_MINIMAP           = 25;

         private:
            bool mEnableDynamicLights;
            bool mEnableCullVisitor;
            bool mEnableNVGS;
            dtCore::RefPtr<osg::CameraNode> mGUIRoot;
            dtCore::RefPtr<osg::CameraNode> mNVGSRoot;
            dtCore::RefPtr<RenderFeature> mNVGS;
            dtCore::RefPtr<SimCore::AgeiaTerrainCullVisitor> mCullVisitor;
            dtCore::RefPtr<UpdateViewCallback> mViewCallback;

            // list of dynamic light actor prototypes
            std::map<const std::string, dtCore::RefPtr<SimCore::Actors::DynamicLightPrototypeProxy> > mDynamicLightPrototypes;

            LightArray mLights;
      };
   } // namespace
} // namespace
#endif