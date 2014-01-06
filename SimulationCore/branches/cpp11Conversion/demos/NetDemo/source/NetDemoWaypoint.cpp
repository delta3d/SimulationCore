/* -*-c++-*-
* Using 'The MIT License'
* Copyright (C) 2009, Alion Science and Technology Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* @author Bradley Anderegg
*/

#include <NetDemoWaypoint.h>

namespace NetDemo
{

   /////////////////////////////////////////////////////////////////////////////
   std::shared_ptr<dtDAL::ObjectType> NetDemoWaypointTypes::CHECKPOINT(new dtDAL::ObjectType("Checkpoint", "NetDemoWaypointTypes"));


   /////////////////////////////////////////////////////////////////////////////
   //NetDemoWaypoint
   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint()
      : BaseClass(NetDemoWaypointTypes::CHECKPOINT.get())
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const osg::Vec3& pos )
      : BaseClass(NetDemoWaypointTypes::CHECKPOINT.get())
      , mPosition(pos)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const dtUtil::RefString& name )
      : BaseClass(NetDemoWaypointTypes::CHECKPOINT.get())
      , mName(name)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const osg::Vec3& pos, const dtUtil::RefString& name )
      : BaseClass(NetDemoWaypointTypes::CHECKPOINT.get())
      , mName(name)
      , mPosition(pos)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const dtDAL::ObjectType* ot )
      : BaseClass(ot)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const dtDAL::ObjectType* ot, const osg::Vec3& pos )
      : BaseClass(ot)
      , mPosition(pos)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const dtDAL::ObjectType* ot, const dtUtil::RefString& name )
      : BaseClass(ot)
      , mName(name)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::NetDemoWaypoint( const dtDAL::ObjectType* ot, const osg::Vec3& pos, const dtUtil::RefString& name )
      : BaseClass(ot)
      , mName(name)
      , mPosition(pos)
   {

   }

   /////////////////////////////////////////////////////////////////////////////
   NetDemoWaypoint::~NetDemoWaypoint()
   {
   }

   //////////////////////////////////////////////////////////////////////////
   std::string NetDemoWaypoint::ToString() const
   {
      return mName.Get();
   }

   //////////////////////////////////////////////////////////////////////////
   void NetDemoWaypoint::SetName( const std::string& name )
   {
      mName = name;
   }

   //////////////////////////////////////////////////////////////////////////
   const std::string& NetDemoWaypoint::GetName() const
   {
      return mName.Get();
   }

   //////////////////////////////////////////////////////////////////////////
   std::string NetDemoWaypoint::GetNameCopy() const
   {
      return mName.Get();
   }

   //////////////////////////////////////////////////////////////////////////
   const osg::Vec3& NetDemoWaypoint::GetPosition() const
   {
      return mPosition;
   }

   //////////////////////////////////////////////////////////////////////////
   void NetDemoWaypoint::SetPosition( const osg::Vec3& pVec )
   {
      mPosition = pVec;
   }

   /////////////////////////////////////////////////////////////////////////////
   void NetDemoWaypoint::CreateProperties(dtAI::WaypointPropertyBase& container )
   {
      BaseClass::CreateProperties(container);

      static const dtUtil::RefString Property_WaypointName("WaypointName");
      static const dtUtil::RefString Desc_WaypointName("Name of Waypoint");

      static const dtUtil::RefString Property_CheckpointNum("CheckpointNum");
      static const dtUtil::RefString Desc_Checkpoint("The checkpoint in the order that the AI attacks them");

      static const dtUtil::RefString WaypointGroup("NetDemoWaypoint");

      container.CreateProperty<std::string>(Property_WaypointName, Property_WaypointName, 
         &NetDemoWaypoint::GetNameCopy, &NetDemoWaypoint::SetName, Desc_WaypointName, WaypointGroup);


      container.CreateProperty<int>(Property_CheckpointNum, Property_CheckpointNum, 
         &NetDemoWaypoint::GetCheckpoint, &NetDemoWaypoint::SetCheckpoint, Desc_Checkpoint, WaypointGroup);
   }

   int NetDemoWaypoint::GetCheckpoint() const
   {
      return mCheckpointNum;
   }

   void NetDemoWaypoint::SetCheckpoint(int c)
   {
      mCheckpointNum = c;
   }
} //namespace NetDemo
