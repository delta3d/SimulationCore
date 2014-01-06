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

#ifndef NETDEMO_WAYPOINT_H
#define NETDEMO_WAYPOINT_H

#include <DemoExport.h>
#include <dtUtil/refcountedbase.h>
#include <dtAI/waypointinterface.h>
#include <dtAI/waypointpropertycontainer.h>

#include <string>

namespace NetDemo
{
   class NETDEMO_EXPORT NetDemoWaypointTypes
   {
   public:
      static std::shared_ptr<dtDAL::ObjectType> CHECKPOINT;
   };


   class NETDEMO_EXPORT NetDemoWaypoint : public std::enable_shared_from_this, public dtAI::WaypointInterface
   {
   public:
      typedef WaypointInterface BaseClass;

   public:
      NetDemoWaypoint();
      NetDemoWaypoint(const osg::Vec3& pos);
      NetDemoWaypoint(const dtUtil::RefString& name);
      NetDemoWaypoint(const osg::Vec3& pos, const dtUtil::RefString& name);
      /*virtual*/ ~NetDemoWaypoint();

      void SetName(const std::string& name);
      const std::string& GetName() const;
      std::string GetNameCopy() const;

      int GetCheckpoint() const;
      void SetCheckpoint(int c);


      /*virtual*/ std::string ToString() const;

      /*virtual*/ const osg::Vec3& GetPosition() const;
      /*virtual*/ void SetPosition(const osg::Vec3& pVec);

      /*virtual*/ void CreateProperties(dtAI::WaypointPropertyBase& container);

      virtual void ref() const
      {
         std::enable_shared_from_this::ref();
      }

      virtual void unref() const
      {
         std::enable_shared_from_this::unref();
      }

   protected:
      // these allow derivatives of named waypoint to pass in their derivated object type
      NetDemoWaypoint(const dtDAL::ObjectType* ot);
      NetDemoWaypoint(const dtDAL::ObjectType* ot, const osg::Vec3& pos);
      NetDemoWaypoint(const dtDAL::ObjectType* ot, const dtUtil::RefString& name);
      NetDemoWaypoint(const dtDAL::ObjectType* ot, const osg::Vec3& pos, const dtUtil::RefString& name);

   private:
      dtUtil::RefString mName;

      int mCheckpointNum;
      osg::Vec3 mPosition;
   };

} //namespace NetDemo


#endif //NETDEMO_WAYPOINT_H
