/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2009, Alion Science and Technology, BMH Operation
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
 * Chris Rodgers
 */

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <sstream>
#include <dtCore/camera.h>
#include <dtCore/transform.h>
#include "GUI/ScoreLabelManager.h"



namespace NetDemo
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // SCORE LABEL CODE
      //////////////////////////////////////////////////////////////////////////
      const float ScoreLabel::DISPLACEMENT_HEIGHT = 0.1f;
      const float ScoreLabel::DEFAULT_LIFE_TIME = 2.0f;
      const osg::Vec2 ScoreLabel::DEFAULT_SIZE(0.2f, 0.05f);
      const osg::Vec4 ScoreLabel::DEFAULT_COLOR(0.0f, 1.0f, 0.0f, 1.0f);

      //////////////////////////////////////////////////////////////////////////
      ScoreLabel::ScoreLabel()
         : BaseClass()
         , mLifeTime(DEFAULT_LIFE_TIME)
         , mRemainingLifeTime(DEFAULT_LIFE_TIME)
      {
         SetTextColor(DEFAULT_COLOR);
         //SetBackSize(DEFAULT_SIZE);
         SetFontSize(DEFAULT_SIZE.y());
         SetBackVisible(false);

         osg::StateSet* ss = GetOSGNode()->getOrCreateStateSet();
         ss->setMode(GL_BLEND, osg::StateAttribute::ON);
         ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
         ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
         ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
      }

      //////////////////////////////////////////////////////////////////////////
      ScoreLabel::~ScoreLabel()
      {
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabel::SetWorldPoint(const osg::Vec3& worldPoint)
      {
         mWorldPoint = worldPoint;
      }

      //////////////////////////////////////////////////////////////////////////
      const osg::Vec3& ScoreLabel::GetWorldPoint() const
      {
         return mWorldPoint;
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabel::SetLifeTime(float lifeTime)
      {
         mLifeTime = lifeTime;
         mRemainingLifeTime = lifeTime;
      }

      //////////////////////////////////////////////////////////////////////////
      float ScoreLabel::GetLifeTime() const
      {
         return mLifeTime;
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabel::SetScore(int score)
      {
         std::stringstream ss;
         ss << (score<0.0f?"-":"+") << score;
         SetText(ss.str());
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabel::Update(float timeDelta)
      {
         mRemainingLifeTime -= timeDelta;
      }

      //////////////////////////////////////////////////////////////////////////
      bool ScoreLabel::IsLive() const
      {
         return mRemainingLifeTime > 0.0f;
      }

      //////////////////////////////////////////////////////////////////////////
      float ScoreLabel::GetDisplacement() const
      {
         float ratio = 0.0f;
         
         if(mLifeTime != 0.0f)
         {
            ratio = 1.0f - mRemainingLifeTime / mLifeTime;
         }

         return ratio * DISPLACEMENT_HEIGHT;
      }



      //////////////////////////////////////////////////////////////////////////
      // SCORE LABEL MANAGER CODE
      //////////////////////////////////////////////////////////////////////////
      ScoreLabelManager::ScoreLabelManager()
         : mMaxLabelCount(DEFAULT_MAX_LABEL_COUNT)
         , mLabelLayer(new osg::Group)
      {
      }

      //////////////////////////////////////////////////////////////////////////
      ScoreLabelManager::~ScoreLabelManager()
      {
         Clear();
         RemoveNodeFromParents(*mLabelLayer);
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::SetGuiLayer(osg::Group& layer)
      {
         RemoveNodeFromParents(*mLabelLayer);
         layer.addChild(mLabelLayer.get());
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::SetCamera(dtCore::Camera& camera)
      {
         mCamera = &camera;
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::SetMaxLabelCount(int maxLabelCount)
      {
         mMaxLabelCount = maxLabelCount;
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::AddScoreLabel(const osg::Vec3& objectWorldPoint,
         int score, float lifeTime)
      {
         dtCore::RefPtr<ScoreLabel> label;

         // Get or create a label.
         if(mMaxLabelCount <= int(mLabelList.size()))
         {
            label = mLabelList.front();
            mLabelList.pop_front();
         }
         // Recycle a dead label.
         else if(mLabelRecycleBin.size() > 0)
         {
            label = mLabelRecycleBin.front();
            mLabelRecycleBin.pop_front();
            mLabelLayer->addChild(label->GetOSGNode());
         }
         // No labels are free but the label
         // limits permits one to be created.
         else
         {
            label = new ScoreLabel;
            mLabelLayer->addChild(label->GetOSGNode());
         }

         // Setup and add the label.
         label->SetScore(score);
         label->SetLifeTime(lifeTime);
         label->SetWorldPoint(objectWorldPoint);
         mLabelList.push_back(label);
      }

      //////////////////////////////////////////////////////////////////////////
      template<typename T_Ptr >
      struct RemoveLabelPred
      {
         bool operator() (const T_Ptr& ptr) const
         {
            return ! ptr->IsLive();
         }
      };

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::Update(float timeDelta)
      {
         LabelList::iterator curIter = mLabelList.begin();
         LabelList::iterator endList = mLabelList.end();

         // Update all the labels.
         if(mCamera.valid())
         {
            // Declare loop variables.
            ScoreLabel* curLabel = NULL;
            dtCore::Transform curXform;
            osg::Vec3d curScreenPoint;

            for( ; curIter != endList; ++curIter)
            {
               curLabel = curIter->get();
               curLabel->Update(timeDelta);

               // Remove the label if it is dead.
               if( ! curLabel->IsLive())
               {
                  // Remove the label from view.
                  mLabelLayer->removeChild(curLabel->GetOSGNode());

                  // Add the reference to the recycle bin. It will
                  // be removed from the current list later in this method.
                  mLabelRecycleBin.push_back(curLabel);
               }
               else // Otherwise, update its position.
               {
                  // Convert the label's world point to a normalized
                  // screen point.
                  mCamera->ConvertWorldCoordinateToScreenCoordinate(
                     curLabel->GetWorldPoint(), curScreenPoint);
                  curScreenPoint.z() = curScreenPoint.z() > 0.0f ? 0.5f : -0.5f;
                  curLabel->GetOSGNode()->setNodeMask(curScreenPoint.z() > 0.0f ? 0xFFFFFFFF : 0x0);

                  // Update the Y displacement.
                  curScreenPoint.y() += curLabel->GetDisplacement();

                  // Set the label's physical position to the screen point.
                  curXform.SetTranslation(curScreenPoint);
                  curLabel->SetTransform(curXform);
               }
            }
         }

         // Send dead labels to the recycle bin.
         RemoveLabelPred<dtCore::RefPtr<ScoreLabel> > removeIfPred;
         mLabelList.remove_if(removeIfPred);
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::Clear()
      {
         // Detach all the labels from the layer.
         mLabelLayer->removeChildren(0, mLabelLayer->getNumChildren());

         // Clear all references from the queues.
         mLabelList.clear();
         mLabelRecycleBin.clear();
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::SetEnabled(bool enabled)
      {
         mLabelLayer->setNodeMask(enabled ? 0xFFFFFFFF : 0x0);
      }

      //////////////////////////////////////////////////////////////////////////
      bool ScoreLabelManager::IsEnabled() const
      {
         return mLabelLayer->getNodeMask() != 0x0;
      }

      //////////////////////////////////////////////////////////////////////////
      void ScoreLabelManager::RemoveNodeFromParents(osg::Node& node)
      {
         typedef osg::Node::ParentList ParentList;
         ParentList parentList = node.getParents();

         ParentList::iterator curIter = parentList.begin();
         ParentList::iterator endList = parentList.end();
         for( ; curIter != endList; ++curIter)
         {
            (*curIter)->removeChild(&node);
         }
      }

   } // END - GUI namespace
} // END - NetDemo namespace
