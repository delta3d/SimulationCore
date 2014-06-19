/* -*-c++-*-
* Simulation Core
* Copyright 2009, Alion Science and Technology
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
*/

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <SimCore/Utilities.h>

#include <SimCore/IGExceptionEnum.h>
#include <SimCore/BaseGameEntryPoint.h>
#include <dtABC/application.h>
#include <dtCore/project.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/mathdefines.h>
#include <dtCore/batchisector.h>
#include <dtCore/transform.h>
#include <sstream>

#include <dtPhysics/palphysicsworld.h>

namespace SimCore
{
namespace Utils
{

   const std::string CONFIG_PROP_ADDITIONAL_MAP("AdditionalMap");

   ///////////////////////////////////////////////////////////////////////
   void GetAdditionalMaps(dtGame::GameManager& gm, std::vector<std::string>& toFill)
   {
      std::ostringstream oss;
      std::string addMap;
      unsigned i = 1;
      do
      {
         if (i > 1)
            toFill.push_back(addMap);

         oss << CONFIG_PROP_ADDITIONAL_MAP << i;
         addMap = gm.GetConfiguration().GetConfigPropertyValue(oss.str());
         oss.str("");
         ++i;
      }
      while (!addMap.empty());
   }


   ///////////////////////////////////////////////////////////////////////
   void LoadMaps(dtGame::GameManager& gm, std::string baseMapName)
   {
      // BaseMapName is not required, but if provided, it must exist
      if (!baseMapName.empty())
      {
         // Determine if the specified map is valid.
         typedef std::set<std::string> MapNameList;
         const MapNameList& names = dtCore::Project::GetInstance().GetMapNames();
         if( names.find( baseMapName ) == names.end() )
         {
            std::ostringstream oss;
            oss << "Cannot connect because \"" << baseMapName << "\" is not a valid map name." << std::endl;
            throw dtUtil::Exception( oss.str(), __FUNCTION__, __LINE__ );
         }
      }


      // Get the other map names used in loading prototypes
      // and other application data.
      std::vector<std::string> mapNames;

      if (!baseMapName.empty())
      {
         mapNames.push_back(baseMapName);
      }

      GetAdditionalMaps(gm, mapNames);

      try
      {
         gm.ChangeMapSet(mapNames, false);
      }
      catch(const dtUtil::Exception& e)
      {
         e.LogException(dtUtil::Log::LOG_ERROR);
         throw;
      }
   }

   ///////////////////////////////////////////////////////////////////////
   bool IsDevModeOn(dtGame::GameManager& gm)
   {
      bool result = false;
      std::string developerMode;
      developerMode = gm.GetConfiguration().GetConfigPropertyValue
         (SimCore::BaseGameEntryPoint::CONFIG_PROP_DEVELOPERMODE, "false");
      result = dtUtil::ToType<bool>(developerMode);
      return result;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   bool KeepTransformOnGround(dtCore::Transform& xform, dtCore::Transformable& terrainActor, float dropHeight,
      float maxDepthBelow, float maxHeightAbove)
   {
      // assume we are under earth unless we get proof otherwise.
      // because some checks could THINK we are under earth, especially if you drive / move
      // under a bridge or something
      bool underearth = maxDepthBelow >= 0.0f;
      bool tooHigh = maxHeightAbove >= 0.0f;

      osg::Vec3 pos;
      xform.GetTranslation(pos);

      osg::Vec3 hp;
      dtCore::RefPtr<dtCore::BatchIsector> iSector = new dtCore::BatchIsector();
      //iSector->SetScene( &GetGameActorProxy().GetGameManager()->GetScene() );
      iSector->SetQueryRoot(&terrainActor);
      dtCore::BatchIsector::SingleISector& SingleISector = iSector->EnableAndGetISector(0);
      osg::Vec3 endPos = pos;
      osg::Vec3 startPos = pos;
      startPos[2] -= underearth ? maxDepthBelow : 1000.0f;
      endPos[2] += tooHigh ? maxHeightAbove : 1000.0f;
      float offsettodo = maxDepthBelow;
      float tooHighOffset = maxHeightAbove;
      SingleISector.SetSectorAsLineSegment(startPos, endPos);
      if (iSector->Update(osg::Vec3(0.0f, 0.0f, 0.0f), true))
      {
         for (unsigned i = 0; (underearth || tooHigh) && i < SingleISector.GetNumberOfHits(); ++i)
         {
            SingleISector.GetHitPoint(hp, i);

            if (underearth && pos[2] + offsettodo > hp[2])
            {
               underearth = false;
            }

            if (tooHigh && pos[2] - tooHighOffset < hp[2])
            {
               tooHigh = false;
            }
         }
      }
      else
      {
         // Can't do anything if there is no ground
         underearth = false;
         tooHigh = false;
      }

      bool result = underearth || tooHigh;
      if (result)
      {
         // Setting to the highest position in either case.
         pos.z() = hp[2] + dropHeight;
         xform.SetTranslation(pos);
      }
      return result;
   }

   ///////////////////////////////////////////////////////////////////////////////////
   bool KeepActorOnGround(dtCore::Transformable& actor, dtCore::Transformable& terrainActor, float dropHeight,
            float maxDepthBelow, float maxHeightAbove)
   {
      dtCore::Transform xform;
      actor.GetTransform(xform);

      bool result = KeepTransformOnGround(xform, terrainActor, dropHeight, maxDepthBelow, maxHeightAbove);
      if (result)
      {
         actor.SetTransform(xform);
      }

      return result;
   }

   // From DR Helper.  Needs to move into a math header.
   /////////////////////////////////////////////////////////////////////////////
   void OrientTransform(dtCore::Transform& xform,
      osg::Matrix& rotation, const osg::Vec3& location, const osg::Vec3& normal)
   {
      osg::Vec3 oldNormal(0, 0, 1);

      osg::Vec3 newNormal = normal;
      if (oldNormal * normal < 0.0f)
      {
         newNormal = -normal;
      }

      oldNormal = osg::Matrix::transform3x3(oldNormal, rotation);
      osg::Matrix normalRot;
      normalRot.makeRotate(oldNormal, newNormal);

      rotation = rotation * normalRot;

      xform.Set(location, rotation);
   }

   ///////////////////////////////////////////////////////////////////////////////////
   bool KeepBodyOnGround(dtPhysics::TransformType& transformToUpdate, float bodyHeight, float dropHeight,
            float maxDepthBelow, float maxHeightAbove, float testRange, dtPhysics::CollisionGroupFilter collisionFlags)
   {
      // assume we are under earth unless we get proof otherwise.
      // because some checks could THINK we are under earth, especially if you drive / move
      // under a bridge or something
      bool underearth = maxDepthBelow >= 0.0f;
      bool tooHigh = maxHeightAbove >= 0.0f;

      if (testRange <= 0.0f)
      {
         testRange = 1000.0f;
      }

      dtPhysics::VectorType pos;
      transformToUpdate.GetTranslation(pos);

      osg::Vec3 normal(0.0, 0.0, 1.0);

      osg::Vec3 hp;
      osg::Vec3 endPos = pos;
      osg::Vec3 startPos = pos;
      startPos[2] -= testRange / 2.0f;
      endPos[2] += testRange / 2.0f;
      float offsettodo = maxDepthBelow;
      float tooHighOffset = maxHeightAbove;

      dtPhysics::RayCast ray;
      ray.SetCollisionGroupFilter(collisionFlags);
      ray.SetOrigin(startPos);
      ray.SetDirection(endPos - startPos);

      std::vector<dtPhysics::RayCast::Report> hits;
      hits.reserve(10);
      dtPhysics::PhysicsWorld::GetInstance().TraceRay(ray, hits, true);

      if (!hits.empty())
      {
         float shortestDistance = testRange;

         std::vector<dtPhysics::RayCast::Report>::const_iterator i, iend;
         i = hits.begin();
         iend = hits.end();
         for (; (underearth || tooHigh) && i != iend; ++i)
         {
            const dtPhysics::RayCast::Report& report = *i;

            float distance = dtUtil::Abs(pos.z() - report.mHitPos.z());
            //If the current best hit is below this hit pos, but the space is less than the height of the body, it won't fit
            // in there, so it has to be moved up to the higher level.
            bool bodyTooTallForCurrentHit = (report.mHitPos.z() > hp.z() && report.mHitPos.z() - hp.z() < bodyHeight);
            if (shortestDistance > distance || bodyTooTallForCurrentHit)
            {
               hp = report.mHitPos;
               shortestDistance = distance;
               normal = report.mHitNormal;

               if (underearth && pos[2] + offsettodo > report.mHitPos.z())
               {
                  underearth = false;
               }

               if (tooHigh && pos[2] - tooHighOffset < report.mHitPos.z())
               {
                  tooHigh = false;
               }
            }

         }
      }
      else
      {
         // Can't do anything if there is no ground
         underearth = false;
         tooHigh = false;
      }

      bool result = underearth || tooHigh;
      if (result)
      {
         // Setting to the highest position in either case.
         pos.z() = hp[2] + dropHeight;
         osg::Matrix rot;
         transformToUpdate.GetRotation(rot);
         OrientTransform(transformToUpdate, rot, pos, normal);
      }
      return result;

   }


}
}
