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

#include <map>

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

            struct DynamicLight: public osg::Referenced
            {                          
               protected:
                  /*virtual*/ ~DynamicLight(){}
                  DynamicLight(const DynamicLight&); //un-implemented
                  DynamicLight& operator=(const DynamicLight&); //un-implemented

               public:
                  DynamicLight() : mID(++mLightCounter){}

                  float mIntensity;          
                  float mSaturationIntensity;
                  osg::Vec3 mColor;
                  osg::Vec3 mPosition;
                  osg::Vec3 mAttenuation;

                  LightID mID;
                  static LightID mLightCounter;
                  
                  dtCore::ObserverPtr<dtCore::Transformable> mTarget;

            };

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

            void SetPosition(DynamicLight* dl);

            void InitializeCullVisitor();
            void InitializeFrameBuffer();

            virtual void ProcessTick(const dtGame::TickMessage &msg);

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

            typedef std::map<LightID, dtCore::RefPtr<DynamicLight> > ID_To_Light_Map; 
            ID_To_Light_Map mLights;
      };
   } // namespace
} // namespace
#endif