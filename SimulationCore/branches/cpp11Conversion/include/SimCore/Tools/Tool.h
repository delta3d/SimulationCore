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
 */
#ifndef DVTE_TOOL
#define DVTE_TOOL

#include <osg/Referenced>
#include <dtUtil/coordinates.h>
#include <dtCore/refptr.h>
#include <SimCore/Export.h>

// Foward declaration
namespace CEGUI
{
   class Window;
}

namespace SimCore 
{
   namespace Actors
   {
      class StealthActor;
   }
   
   namespace Tools
   {
      class SIMCORE_EXPORT Tool : public osg::Referenced
      {
         public:

            /// Constructor
            Tool(CEGUI::Window *window);

            /**
             * Enables a tool
             * @param enable True to enable, false to disable
             */
            virtual void Enable(bool enable) { mIsEnabled = enable; }

            /**
             * Returns true if this is enabled
             * @return mIsEnabled
             */
            bool IsEnabled() const { return mIsEnabled; }

            /**
             * Retrieves the player that has this tool
             * @return mPlayerActor
             */
            SimCore::Actors::StealthActor* GetPlayerActor() { return mStealthActor; }

            /**
             * const version of the accessor function
             * @return const mPlayerActor
             */
            const SimCore::Actors::StealthActor* GetPlayerActor() const { return mStealthActor; }

            /**
             * Sets the player actor of this tool
             * @param newPlayer The new player
             */
            void SetPlayerActor(SimCore::Actors::StealthActor* newPlayer) { mStealthActor = newPlayer; }

            /**
             * Sets the coordinate converter for this tool
             * @param coords The converter to set
             */
            void SetCoordinateConverter(const dtUtil::Coordinates &coords) { mCoordinates = coords; }

            /**
             * Gets the coordinates of this tool
             * @return mCoordinates
             */
            const dtUtil::Coordinates& GetCoordinateConverter() const { return mCoordinates; }

            /**
             * Gets the coordinates of this tool
             * @return mCoordinates
             */
            dtUtil::Coordinates& GetCoordinateConverter() { return mCoordinates; }

            /**
             * Calculates the mils either positive or negative
             * @param distance, The distance
             * @param elevation, The elevation
             * @return The mils
             */
            float CalculateDegrees(const float distance, const float elevation);

         protected:

            /**
             * Protected helper method to pad a number to 4 digits
             * @param num The number to pad
             * @return A std::string padded out
             */
            std::string PadNumber(int num);

            /// Destructor
            virtual ~Tool();

            CEGUI::Window *mMainWindow;

         private:

            bool mIsEnabled;

            SimCore::Actors::StealthActor* mStealthActor;
            dtUtil::Coordinates mCoordinates;
      };
   }
}

#endif
