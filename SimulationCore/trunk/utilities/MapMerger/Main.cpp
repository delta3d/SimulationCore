/* -*-c++-*-
 * Simulation Core - allTests\Main.cpp (.h & .cpp) - Using 'The MIT License'
 * Copyright (C) 2005-2008, Alion Science and Technology Corporation
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
 * This software was developed by Alion Science and Technology Corporation under
 * circumstances in which the U. S. Government may have rights in the software.
 *
 * @author David Guthrie
 */
#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>

#include <dtCore/timer.h>
#include <dtCore/deltawin.h>
#include <dtCore/globals.h>

#include <dtDAL/actorproxy.h>
#include <dtDAL/project.h>
#include <dtDAL/gameevent.h>
#include <dtDAL/gameeventmanager.h>
#include <dtDAL/map.h>
#include <dtDAL/librarymanager.h>

#include <dtAudio/audiomanager.h>

#include <set>
#include <vector>

#include <iostream>
#include <cstdlib>

void Usage()
{
   std::cerr << "Usage: MapMerger <projectPath> <mapFrom> <mapTo>" << std::endl;
   exit(1);
}

template <typename container>
void PrintMissing(const container& c)
{
   typedef typename container::const_iterator citor;
   citor i, iend;
   i = c.begin();
   iend = c.end();

   for (; i != iend; ++i)
   {
      const std::string& s = *i;
      std::cerr << "   " << s << "\n";
   }
   std::cerr << std::endl;
}

void PrintMissingData(dtDAL::Map& m)
{
   std::cerr << m.GetName() << " tried to load the following actor types, but they were not found.\n";
   PrintMissing(m.GetMissingActorTypes());

   std::cerr << m.GetName() << " tried to load the following libraries, but they were not found.\n";
   PrintMissing(m.GetMissingLibraries());
   exit(-1);
}

void MergeLibraries(dtDAL::Map& mapFrom, dtDAL::Map& mapTo)
{
   const std::vector<std::string>& libs = mapFrom.GetAllLibraries();
   std::vector<std::string>::const_iterator i, iend;
   i = libs.begin();
   iend= libs.end();
   //TODO: This doesn't solve ordering issues.
   for (; i != iend; ++i)
   {
      const std::string& libName = *i;
      if (!mapTo.HasLibrary(libName))
      {
         mapTo.AddLibrary(libName, mapFrom.GetLibraryVersion(libName));
      }
   }
}

void MergeProxies(dtDAL::Map& mapFrom, dtDAL::Map& mapTo)
{
   std::vector<dtCore::RefPtr<dtDAL::ActorProxy> > proxiesFrom;
   mapFrom.GetAllProxies(proxiesFrom);
   std::vector<dtCore::RefPtr<dtDAL::ActorProxy> >::iterator i, iend;
   i = proxiesFrom.begin();
   iend = proxiesFrom.end();
   for (; i != iend; ++i)
   {
      dtDAL::ActorProxy& mapFromProxyCurrent = **i;
      dtDAL::ActorProxy* foundProxy = mapTo.GetProxyById(mapFromProxyCurrent.GetId());
      if (foundProxy != NULL)
      {
         mapTo.RemoveProxy(*foundProxy);
      }

      mapTo.AddProxy(mapFromProxyCurrent);
   }
}

void MergeEvents(dtDAL::Map& mapFrom, dtDAL::Map& mapTo)
{
   std::vector<dtDAL::GameEvent* > mapFromEvents;
   mapFrom.GetEventManager().GetAllEvents(mapFromEvents);

   dtDAL::GameEventManager& mapToEventMan = mapTo.GetEventManager();
   std::vector<dtDAL::GameEvent* >::iterator i, iend;
   i = mapFromEvents.begin();
   iend = mapFromEvents.end();
   for (; i != iend; i++)
   {
      dtDAL::GameEvent* curEvent =  *i;
      dtDAL::GameEvent* foundEvent = mapToEventMan.FindEvent(curEvent->GetUniqueId());
      if (foundEvent != NULL)
      {
         mapToEventMan.RemoveEvent(foundEvent->GetUniqueId());
      }

      mapToEventMan.AddEvent(*curEvent);
   }
}

int main (int argc, char* argv[])
{
   const std::string executable = argv[0];
   std::string projectPath;
   std::string mapFromName;
   std::string mapToName;

   std::string currArg;
   for (int arg = 1; arg < argc; ++arg)
   {
      currArg = argv[arg];
      if (projectPath.empty())
      {
         projectPath = currArg;
      }
      else if (mapFromName.empty())
      {
         mapFromName = currArg;
      }
      else if (mapToName.empty())
      {
         mapToName = currArg;
      }
      else
      {
         std::cerr << "Ignoring argument: " << currArg << std::endl;
      }
   }

   if (projectPath.empty() || mapFromName.empty() || mapToName.empty())
   {
      Usage();
   }

   dtAudio::AudioManager::Instantiate();
   dtAudio::AudioManager::GetInstance().Config(AudioConfigData(32));

   try
   {
      dtDAL::Project::GetInstance().SetContext(projectPath);

      dtDAL::Map& mapFrom = dtDAL::Project::GetInstance().GetMap(mapFromName);
      dtDAL::Map& mapTo = dtDAL::Project::GetInstance().GetMap(mapToName);

      if (mapFrom.HasLoadingErrors())
      {
         PrintMissingData(mapFrom);
      }

      if (mapTo.HasLoadingErrors())
      {
         PrintMissingData(mapTo);
      }

      MergeLibraries(mapFrom, mapTo);
      MergeProxies(mapFrom, mapTo);
      MergeEvents(mapFrom, mapTo);

      dtDAL::Project::GetInstance().SaveMap(mapTo);

      dtAudio::AudioManager::Destroy();
   }
   catch(const dtUtil::Exception& ex)
   {
      dtAudio::AudioManager::Destroy();
      LOG_ERROR(ex.ToString());
      return 1;
   }

}

