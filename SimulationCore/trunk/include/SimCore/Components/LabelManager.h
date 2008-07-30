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

#ifndef SIMCORE_LABEL_MANAGER_H
#define SIMCORE_LABEL_MANAGER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>
#include <SimCore/Components/BaseHUDElements.h>
#include <dtCore/observerptr.h>

////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Camera;
}

namespace dtGame
{
   class GameManager;
}

namespace SimCore
{
   namespace Components
   {
      //////////////////////////////////////////////////////////////////////////
      // LABEL CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT HUDLabel : public SimCore::Components::HUDElement
      {
         public:
            static const std::string PROPERTY_COLOR;
            static const std::string PROPERTY_TEXT;
            static const std::string PROPERTY_LINE2;

            HUDLabel( const std::string& name, const std::string& type );

            HUDLabel( CEGUI::Window& window );

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

            bool HUDLabel::operator > (const HUDLabel& other) const;

         protected:
            virtual ~HUDLabel();

         private:
            float mZDepth;
            osg::Vec3 mVelocity;
      };



      //////////////////////////////////////////////////////////////////////////
      // LABEL OPTIONS CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LabelOptions
      {
         public:
            LabelOptions();
            ~LabelOptions();

            bool ShowDamageState() const;
            void SetShowDamageState(bool show);

            float GetMaxLabelDistance() const;
            float GetMaxLabelDistance2() const;
            void SetMaxLabelDistance(float distance);

            bool operator == (const LabelOptions& toCompare) const;

            /// Used for the game manager find to see if the actor matches the options.
            bool operator() (dtDAL::ActorProxy& actor);
         private:
            float mMaxLabelDistance;
            float mMaxLabelDistance2;

            bool mShowDamageState;
      };



      //////////////////////////////////////////////////////////////////////////
      // LABEL MANAGER CODE
      //////////////////////////////////////////////////////////////////////////
      class SIMCORE_EXPORT LabelManager : public dtCore::Base//osg::Referenced
      {
         public:

            LabelManager();

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

            dtCore::RefPtr<HUDLabel> GetOrCreateLabel(dtDAL::ActorProxy& actor);

            void AddLabel(SimCore::Components::HUDLabel& label);

            void Update(float timeDelta);


            const std::string AssignLabelColor(const dtDAL::ActorProxy& actor, HUDLabel& label);

            const osg::Vec2 CalculateLabelScreenPosition(dtCore::Transformable& transformable,
                     dtCore::Camera& deltaCam, const osg::Vec3& screenPos, const osg::Vec2& labelSize);

            static void CalculateOnScreenBoundingBox(const osg::BoundingBox& bbox, const dtCore::Camera& cam,
                     osg::Vec3d& outBottomLeft, osg::Vec3d& outTopRight);

            static void CalculateBoundingBox(dtCore::Transformable& transformable, osg::BoundingBox& outBB);

            // TEMP:
            virtual void OnMessage(MessageData* data);
            // TEMP:

         protected:
            virtual ~LabelManager();

         private:
            void ClearLabelsFromGUILayer();

            typedef std::map<dtCore::UniqueId, dtCore::RefPtr<SimCore::Components::HUDLabel> > LabelMap;
            typedef std::vector<SimCore::Components::HUDLabel*> CEGUISortList;

            dtCore::ObserverPtr<dtGame::GameManager> mGM;
            LabelOptions mOptions;
            dtCore::RefPtr<SimCore::Components::HUDElement> mGUILayer;
            LabelMap mLastLabels;
            CEGUISortList mCEGUISortList;
      };

   }
}

#endif
