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
* @author Curtiss Murphy
* 
*/

#ifndef _SIMCORE_UTILITIES_H_
#define _SIMCORE_UTILITIES_H_

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <SimCore/Export.h>

#include <dtGame/gamemanager.h>
#include <dtGame/exceptionenum.h>
#include <SimCore/CollisionGroupEnum.h>

namespace dtPhysics
{
   class PhysicsObject;
}

namespace SimCore
{
namespace Utils
{

   extern SIMCORE_EXPORT const std::string CONFIG_PROP_ADDITIONAL_MAP;


   ////////////////////////////////////////////////////////////////////////////////
   // This file provides general utility methods that are useful in various places 
   // but don't have a particular home.
   ////////////////////////////////////////////////////////////////////////////////


   // Looks up the additional maps from the config file. 
   //    Ex: <Property Name="AdditionalMap1">DriverPrototypes</Property> 
   void SIMCORE_EXPORT GetAdditionalMaps(dtGame::GameManager& gm, std::vector<std::string>& toFill);

   /**
    * Attempts to load the maps - using passed in map PLUS any additional maps in config.
    * @return if it needed to do a map change or false if it's job could be completed without one.
    */
   bool SIMCORE_EXPORT LoadMaps(dtGame::GameManager& gm, const std::string& baseMapName);

   /// A simple utility to check the DeveloperMode config parameter.
   bool SIMCORE_EXPORT IsDevModeOn(dtGame::GameManager& gm); 

   /// Calls the GM CreateActorFromPrototype() method, but throws an exception if there's an error. 
   template <typename T>
   void CreateActorFromPrototypeWithException(dtGame::GameManager& gm, const std::string& prototypeName, 
      dtCore::RefPtr<T>& proxy, const std::string& errorMsg = "")
   {
      gm.CreateActorFromPrototype(prototypeName, proxy);
      if (proxy == NULL)
      {
         std::string errorText = "Failed to create actor from prototype named [" + prototypeName + "]. " + errorMsg;
         throw dtGame::InvalidParameterException(errorText, __FILE__, __LINE__);
      }
   }

   /**
    * Uses an isector to check if an object's transform is too high or two low and should be adjusted to be on the terrain.
    * @param dropHeight The height above the highest terrain point found to set the transform if it is object
    * @param maxDepthBelow the distance below the terrain at which the transform should not be moved back up, or < 0 to ignore.
    * @param maxHeightAbove The point above terrain above which point it should not move it back down, or < 0 to ignore
    * @return true if it moves the transform.
    */
   bool SIMCORE_EXPORT KeepTransformOnGround(dtCore::Transform& xform, dtCore::Transformable& terrainActor, float dropHeight,
      float maxDepthBelow = -1.0f, float maxHeightAbove = -1.0f);

   /**
    * Uses an isector to check if an object's transform is too high or two low and should be adjusted to be on the terrain.
    * @param dropHeight The height above the highest terrain point found to set the transform if it is object
    * @param maxDepthBelow The depth below the lowest point hit at which point it should move it back up, or < 0 to ignore
    * @param maxHeightAbove The depth above the highest point at which point it should move it back up, or < 0 to ignore
    * @return true if it moves the actor.
    */
   bool SIMCORE_EXPORT KeepActorOnGround(dtCore::Transformable& actor, dtCore::Transformable& terrainActor,
            float dropHeight = 0.5f, float maxDepthBelow = 5.0f, float maxHeightAbove = 20.0f);

   /**
    * Uses a physics ray cast to check if an object's transform is too high or two low and should be adjusted to be on the terrain.
    * @param transformToUpdate current transform of the body of actor that should be updated to keep it on the ground
    *                          if necessary.
    * @param bodyHeight The height of the body.  This is used because it ignore lower hits if one smaller that the height of the body is found, i.e. it won't fit in there.
    * @param dropHeight The height above the highest terrain point found to set the transform if it is adject
    * @param maxDepthBelow The depth below the lowest point hit at which point it should move it back up, or < 0 to ignore
    * @param maxHeightAbove The depth above the highest point at which point it should move it back up, or < 0 to ignore
    * @param testRange the range (half above and half below) to use for a raycast.  Anything less than 0 will be set to the default.
    * @param collisionFlags Physics collision flags to use as a filter for the raycast.
    * @return true if an adjustment was made.
    */
   bool SIMCORE_EXPORT KeepBodyOnGround(dtPhysics::TransformType& transformToUpdate, float bodyHeight = 1.0f, float dropHeight = 0.5f,
            float maxDepthBelow = 5.0f, float maxHeightAbove = 20.0f, float testRange = 1000.0f,
            dtPhysics::CollisionGroupFilter collisionFlags = 1 << SimCore::CollisionGroup::GROUP_TERRAIN);
}
}

#endif
