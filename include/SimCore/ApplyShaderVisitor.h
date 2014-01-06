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

#ifndef SIMCORE_APPLY_SHADER_VISITOR_H
#define SIMCORE_APPLY_SHADER_VISITOR_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <osg/NodeVisitor>
#include <dtUtil/refcountedbase.h>
#include <SimCore/Export.h>



////////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////
namespace dtCore
{
   class ShaderProgram;
}

namespace dtUtil
{
   class RefString;
}



namespace SimCore
{
   /////////////////////////////////////////////////////////////////////////////
   // CLASS CODE
   /////////////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT ApplyShaderVisitor : public osg::NodeVisitor
   {
      public:
         typedef osg::NodeVisitor BaseClass;

         typedef std::map<dtUtil::RefString, std::shared_ptr<dtCore::ShaderProgram> > ShaderMap;

         ApplyShaderVisitor();

         /**
          * Set the current shader to be applied to any matched nodes
          * over the current node tree traversal.
          * @param shaderName Name of the requested shader as it appears
          *        in the shaderDefs.xml.
          */
         void SetShaderName(const std::string& shaderName);
         const std::string& GetShaderName() const;

         /**
          * Set the current shader to be applied to any matched nodes
          * over the current node tree traversal.
          * @param shaderGroup Name of the shader group of the requested shader
          *        as it appears in the shaderDefs.xml.
          */
         void SetShaderGroup(const std::string& shaderGroup);
         const std::string& GetShaderGroup() const;

         /**
          * Add one or more node names to search for over a single
          * node tree traversal.
          * @param nodeName The exact name of the node to be found (case-sensitive).
          * @return TRUE if the name was added, FALSE if it already exists
          *         in the name list.
          */
         bool AddNodeName(const std::string& nodeName);

         /**
          * Remove a node name from the internal node name search list.
          * @param nodeName Name to be removed from the search list.
          * @return TRUE if the name existed and was removed.
          */
         bool RemoveNodeName(const std::string& nodeName);

         /**
          * Determine if the name exists in the internal node name search list.
          * @return TRUE if the node exists in the search list.
          */
         bool HasNodeName(const std::string& nodeName) const ;

         /**
          * These methods allow one to acquire direct references to the
          * applied shader instances AFTER a node tree was traversed.
          * The map keys on the node names used in the node name search.
          * @return Direct reference to the internal node name search container,
          *         whose values are references to the shader instances created
          *         for any matching nodes after a successful node tree traversal.
          */
         ShaderMap& GetShaderMap();
         const ShaderMap& GetShaderMap() const;

         /**
          * Convenience method for clearing the node name search container.
          * @return Number of items that where removed.
          */
         int ClearNodeNames();

         /**
          * Override method that performs the node matching and shader attachment
          * over a node tree traversal. To start a node traversal, pass this visitor
          * into the "accept" method called from the root OSG node of the tree/scene
          * to be searched.
          */
         virtual void apply(osg::Node& node);

      private:
         dtUtil::RefString mShaderName;
         dtUtil::RefString mShaderGroup;

         ShaderMap mShaderMap;
   };

} // END - SimCore namespace

#endif
