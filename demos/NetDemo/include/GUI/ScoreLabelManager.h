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

#ifndef NETDEMO_SCORE_LABEL_MANAGER_H
#define NETDEMO_SCORE_LABEL_MANAGER_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <dtUtil/refcountedbase.h>
#include <dtABC/labelactor.h>
#include "DemoExport.h"



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace osg
{
   class Group;
}

namespace dtCore
{
   class Camera;
}



namespace NetDemo
{
   namespace GUI
   {
      //////////////////////////////////////////////////////////////////////////
      // SCORE LABEL CODE
      //////////////////////////////////////////////////////////////////////////
      class NETDEMO_EXPORT ScoreLabel : public dtABC::LabelActor
      {
         public:
            typedef dtABC::LabelActor BaseClass;

            static const float DISPLACEMENT_HEIGHT; // In normalized scree coordinate measurement.
            static const float DEFAULT_LIFE_TIME;
            static const osg::Vec2 DEFAULT_SIZE;
            static const osg::Vec4 DEFAULT_COLOR;

            ScoreLabel();

            void SetWorldPoint(const osg::Vec3& worldPoint);
            const osg::Vec3& GetWorldPoint() const;

            void SetLifeTime(float lifeTime);
            float GetLifeTime() const;

            void SetScore(int score);

            void Update(float timeDelta);

            bool IsLive() const;

            float GetDisplacement() const;

         protected:
            virtual ~ScoreLabel();

         private:
            float mLifeTime;
            float mRemainingLifeTime;
            osg::Vec3 mWorldPoint;
      };



      //////////////////////////////////////////////////////////////////////////
      // SCORE LABEL MANAGER CODE
      //////////////////////////////////////////////////////////////////////////
      class NETDEMO_EXPORT ScoreLabelManager : public std::enable_shared_from_this
      {
         public:
            typedef std::enable_shared_from_this BaseClass;

            static const int DEFAULT_MAX_LABEL_COUNT = 20;

            ScoreLabelManager();

            void SetGuiLayer(osg::Group& layer);

            void SetCamera(dtCore::Camera& camera);

            void SetMaxLabelCount(int maxLabelCount);

            void AddScoreLabel(const osg::Vec3& objectWorldPoint,
               int score, float lifeTime);

            void Update(float timeDelta);

            void Clear();

            void SetEnabled(bool enabled);
            bool IsEnabled() const;

         protected:
            virtual ~ScoreLabelManager();

            void RemoveNodeFromParents(osg::Node& node);

         private:
            int mMaxLabelCount;
            osg::ref_ptr<osg::Group> mLabelLayer;
            std::shared_ptr<dtCore::Camera> mCamera;
            
            typedef std::list<std::shared_ptr<ScoreLabel> > LabelList;
            LabelList mLabelList;
            LabelList mLabelRecycleBin;
      };

   } // END - GUI namespace
} // END - NetDemo namespace

#endif
