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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix.h>
#include <dtCore/shadermanager.h>
#include <dtCore/shaderprogram.h>
#include <dtUtil/refstring.h>
#include <SimCore/ApplyShaderVisitor.h>



namespace SimCore
{
   /////////////////////////////////////////////////////////////////////////////
   // CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   ApplyShaderVisitor::ApplyShaderVisitor()
      : BaseClass(BaseClass::TRAVERSE_ALL_CHILDREN)
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   void ApplyShaderVisitor::SetShaderName(const std::string& shaderName)
   {
      mShaderName = shaderName;
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& ApplyShaderVisitor::GetShaderName() const
   {
      return mShaderName;
   }

   /////////////////////////////////////////////////////////////////////////////
   void ApplyShaderVisitor::SetShaderGroup(const std::string& shaderGroup)
   {
      mShaderGroup = shaderGroup;
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::string& ApplyShaderVisitor::GetShaderGroup() const
   {
      return mShaderGroup;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool ApplyShaderVisitor::AddNodeName(const std::string& nodeName)
   {
      bool success = false;

      if( ! HasNodeName(nodeName))
      {
         // Create an item keyed on nodeName and with a value of NULL.
         // The value will be assigned later during a node tree traversal.
         mShaderMap[nodeName] = NULL;

         success = true;
      }

      return success;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool ApplyShaderVisitor::RemoveNodeName(const std::string& nodeName)
   {
      ShaderMap::iterator foundIter = mShaderMap.find(nodeName);

      bool found = foundIter != mShaderMap.end();

      if(found)
      {
         mShaderMap.erase(foundIter);
      }

      return found;
   }

   /////////////////////////////////////////////////////////////////////////////
   bool ApplyShaderVisitor::HasNodeName(const std::string& nodeName) const
   {
      return mShaderMap.find(nodeName) != mShaderMap.end();
   }

   /////////////////////////////////////////////////////////////////////////////
   ApplyShaderVisitor::ShaderMap& ApplyShaderVisitor::GetShaderMap()
   {
      return mShaderMap;
   }

   /////////////////////////////////////////////////////////////////////////////
   const ApplyShaderVisitor::ShaderMap& ApplyShaderVisitor::GetShaderMap() const
   {
      return mShaderMap;
   }

   /////////////////////////////////////////////////////////////////////////////
   int ApplyShaderVisitor::ClearNodeNames()
   {
      int numItems = int(mShaderMap.size());
      mShaderMap.clear();
      return numItems;
   }

   /////////////////////////////////////////////////////////////////////////////
   void ApplyShaderVisitor::apply(osg::Node& node)
   {
      const std::string& nodeName = node.getName();

      // If this node's name matches the ones to be found...
      ShaderMap::iterator foundIter = mShaderMap.find(nodeName);
      if(foundIter != mShaderMap.end())
      {
         // ...attach the shader.
         dtCore::RefPtr<dtCore::ShaderProgram> protoShader
            = dtCore::ShaderManager::GetInstance().FindShaderPrototype(mShaderName, mShaderGroup);
         if(protoShader.valid())
         {
            // Maintain a reference to the new shader for the found node,
            // in case the caller code needs to dynamically control the
            // shader instances over the program lifetime.
            foundIter->second
               = dtCore::ShaderManager::GetInstance().AssignShaderFromPrototype(*protoShader, node);

            if( ! foundIter->second.valid())
            {
               LOG_ERROR("Could not create and attach shader \"" + mShaderName
                  + "\" of group \"" + mShaderGroup + "\" to node \"" + nodeName + "\".");
            }
         }
      }

      traverse(node);
   }

} // END - SimCore namespace
