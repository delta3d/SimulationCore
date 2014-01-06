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
#include <prefix/SimCorePrefix.h>
#include <SimCore/Tools/Tool.h>
#include <SimCore/Actors/StealthActor.h>
#include <sstream>

#ifdef None
#undef None
#endif

#include <CEGUI/CEGUI.h>

namespace SimCore
{
   namespace Tools
   {
      Tool::Tool(CEGUI::Window *window) : 
         mMainWindow(window),
         mIsEnabled(false),
         mStealthActor(nullptr)
      {

      }

      Tool::~Tool()
      {

      }

      std::string Tool::PadNumber(int num)
      {
         std::ostringstream paddedNum;
         int absNum = abs(num);

         if (absNum == 0)
            paddedNum << "0000";
         else if (absNum < 10)
            paddedNum << "000" << absNum;
         else if (absNum < 100)
            paddedNum << "00" << absNum;
         else if (absNum < 1000)
            paddedNum << "0" << absNum;
         else
            paddedNum << absNum;

         if (num < 0)
            paddedNum.str("-" + paddedNum.str());

         return paddedNum.str();
      }

      float Tool::CalculateDegrees(const float distance, const float elevation)
      {
         if (distance == 0.0f)
         {
            LOG_ERROR("Cannot calculate mils with a distance of 0.");
            return 0.0f;
         }
         float radians = std::asin(elevation / distance);
         return osg::RadiansToDegrees(radians);
      }
   }
}
