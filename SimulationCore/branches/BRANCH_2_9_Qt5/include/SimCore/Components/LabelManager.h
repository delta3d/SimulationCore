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
 *
 * @author Chris Rodgers
 * @author David Guthrie
 */

#ifndef SIMCORE_LABEL_MANAGER_H
#define SIMCORE_LABEL_MANAGER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <dtCore/observerptr.h>
#include <dtGame/gamemanager.h>

#include <OpenThreads/Mutex>

#include <map>
#include <vector>
////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Camera;
}

namespace dtCore
{
   class Camera;
   class Transformable;
}

namespace dtCore
{
   class BaseActorObject;
}

namespace SimCore
{
   namespace Components
   {
      class LabelManager;

      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class LabelUpdateTask : public dtGUI::GUI::GUITask
      {
      public:
         LabelUpdateTask(LabelManager& labelManager);

         /*virtual*/ void Update(float dt);

      protected:
         virtual ~LabelUpdateTask();

         dtCore::ObserverPtr<SimCore::Components::LabelManager> mLabelManager;
      };



      //////////////////////////////////////////////////////////////////////////
      // CLASS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDLabel : public SimCore::Components::HUDElement
      {
         public:
            static const std::string PROPERTY_COLOR;
            static const std::string PROPERTY_TEXT;
            static const std::string PROPERTY_LINE2;

            HUDLabel( const std::string& name, const std::string& type );

            HUDLabel( CEGUI::Window& window );

            /// Sizes the label to match the text if needed.
            void UpdateSize();

            /// Changes the label text.
            void SetText(const std::string& text);
            const std::string GetText() const;

            /// Changes the velocity label text.
            void SetLine2(const std::string& value);
            const std::string GetLine2() const;

            void SetColor(const osg::Vec3& color);

            void SetColor(const osg::Vec4& color);

            void GetColor(osg::Vec4& outColor ) const;

            void SetAlpha(float alpha);

            float GetAlpha() const;

            void SetZDepth(float depth);

            float GetZDepth() const;

            bool operator < (const HUDLabel& other) const;

            bool operator > (const HUDLabel& other) const;

         protected:
            virtual ~HUDLabel();

         private:
            float mZDepth;
            osg::Vec3 mVelocity;
            std::string mText, mLine2;
            bool mSized;
      };



      //////////////////////////////////////////////////////////////////////////
      // LABEL OPTIONS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LabelOptions
      {
         public:
            LabelOptions();
            ~LabelOptions();

            bool ShowLabels() const;
            void SetShowLabels(bool show);

            bool ShowDamageState() const;
            void SetShowDamageState(bool show);

            bool ShowLabelsForEntities() const;
            void SetShowLabelsForEntities(bool show);

            bool ShowLabelsForPositionReports() const;
            void SetShowLabelsForPositionReports(bool show);

            bool ShowLabelsForBlips() const;
            void SetShowLabelsForBlips(bool show);

            /// @return maximum visible distance of a label.  <= 0 means unlimited.
            float GetMaxLabelDistance() const;

            /**
             * This prevents doing sqrts, but you can't see if the value is < 0 by calling this method.
             * @return maximum visible distance of a label squared.
             */
            float GetMaxLabelDistance2() const;

            /// Sets the maximum visible distance of a label.  <= 0 means unlimited.
            void SetMaxLabelDistance(float distance);

            bool operator == (const LabelOptions& toCompare) const;
            bool operator != (const LabelOptions& toCompare) const { return !(*this == toCompare); }

            /// Used for the game manager find to see if the actor matches the options.
            bool operator() (dtCore::BaseActorObject& actor);
         private:
            float mMaxLabelDistance;
            float mMaxLabelDistance2;

            bool mShowDamageState : 1;
            bool mShowLabels : 1;
            bool mShowLabelsForEntities : 1;
            bool mShowLabelsForPositionReports : 1;
            bool mShowLabelsForBlips : 1;
      };



      //////////////////////////////////////////////////////////////////////////
      // LABEL MANAGER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LabelManager : public dtCore::Base//osg::Referenced
      {
         public:
            typedef std::map<dtCore::UniqueId, dtCore::RefPtr<SimCore::Components::HUDLabel> > LabelMap;

            LabelManager();

            void Init(dtGUI::GUI* gui);

            /**
             * Set the game manager so that the camera can be accessed as well
             * as entities involved with labels.
             */
            void SetGameManager(dtGame::GameManager* gm);
            dtGame::GameManager* GetGameManager();
            const dtGame::GameManager* GetGameManager() const;

            void SetOptions(const LabelOptions& options);
            const LabelOptions& GetOptions() const;

            void SetGUILayer(SimCore::Components::HUDElement* guiLayer);
            SimCore::Components::HUDElement* GetGUILayer();
            const SimCore::Components::HUDElement* GetGUILayer() const;

            dtCore::RefPtr<HUDLabel> GetOrCreateLabel(dtCore::BaseActorObject& actor);

            void AddLabel(SimCore::Components::HUDLabel& label);

            void Update(float dt);

            void UpdateFormatting(float dt);

            const std::string AssignLabelColor(const dtCore::BaseActorObject& actor, HUDLabel& label);

            const osg::Vec2 CalculateLabelScreenPosition(float boundRadius, const osg::Vec3& boundCenter,
                     dtCore::Camera& deltaCam, const osg::Vec3& screenPos, const osg::Vec2& labelSize);

            static void CalculateOnScreenBoundingBox(const osg::BoundingBox& bbox, const dtCore::Camera& cam,
                     osg::Vec3d& outBottomLeft, osg::Vec3d& outTopRight);

            static void CalculateBoundingBox(dtCore::Transformable& transformable, osg::BoundingBox& outBB);

            // TEMP:
            virtual void OnSystem(const dtUtil::RefString& phase, double deltaSim, double deltaReal)
;
            // TEMP:

            /// Called to update a single label on a single actor.
            void ApplyLabelToActor(dtCore::BaseActorObject& proxy, dtCore::Camera& deltaCamera, LabelMap& newLabels, std::string& nameBuffer);

         protected:
            virtual ~LabelManager();

         private:
            void ClearLabelsFromGUILayer();

            typedef std::vector<SimCore::Components::HUDLabel*> CEGUISortList;

            dtCore::ObserverPtr<dtGame::GameManager> mGM;
            LabelOptions mOptions;
            dtCore::RefPtr<SimCore::Components::HUDElement> mGUILayer;
            LabelMap mLastLabels;
            CEGUISortList mCEGUISortList;

            OpenThreads::Mutex mTaskMutex;
            float mTimeUntilSort;
      };

   }
}

#endif
