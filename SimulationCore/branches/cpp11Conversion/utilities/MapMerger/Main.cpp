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
 * @author David Guthrie, Curtiss Murphy
 */
#include <dtUtil/fileutils.h>
#include <dtUtil/log.h>
#include <dtUtil/exception.h>
#include <dtUtil/stringutils.h>
#include <dtUtil/datapathutils.h>

#include <dtCore/timer.h>
#include <dtCore/deltawin.h>

#include <dtDAL/actorproxy.h>
#include <dtDAL/actorproperty.h>
#include <dtDAL/project.h>
#include <dtDAL/gameevent.h>
#include <dtDAL/gameeventmanager.h>
#include <dtDAL/map.h>
#include <dtDAL/librarymanager.h>

#include <dtAudio/audiomanager.h>

#include <set>
#include <vector>

#include <iostream>
#include <fstream>
#include <cstdlib>


// global vars for mapMerger
static int mNumLibrariesAdded = 0;
static int mNumActorsAdded = 0;
static int mNumActorsChanged = 0;
static int mNumPropsChanged = 0;
static int mNumEventsAdded = 0; 
static int mNumEventsChanged = 0;
static int mNumNamesChanged = 0;

/////////////////////////////////////////////////////////////////////
void Usage()
{
   std::cerr << "Usage: MapMerger [-i ignorePropsFile] <projectPath> <mapFrom> <mapTo>" << std::endl;
   exit(1);
}

template <typename container>

/////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////
void PrintMissingData(dtDAL::Map& m)
{
   std::cerr << m.GetName() << " tried to load the following actor types, but they were not found.\n";
   PrintMissing(m.GetMissingActorTypes());

   std::cerr << m.GetName() << " tried to load the following libraries, but they were not found.\n";
   PrintMissing(m.GetMissingLibraries());
   exit(-1);
}

/////////////////////////////////////////////////////////////////////
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
         mNumLibrariesAdded ++;
      }
   }
}

/////////////////////////////////////////////////////////////////////
void SyncProperties(const dtDAL::ActorProxy& from, dtDAL::ActorProxy& to, std::set<std::string> ignorePropsSet)
{
   bool actorPropChanged = false;
   std::vector<const dtDAL::ActorProperty*> props;
   from.GetPropertyList(props);

   std::vector<const dtDAL::ActorProperty*>::const_iterator i, iend;
   i = props.begin();
   iend = props.end();
   for (; i != iend; ++i)
   {
      const dtDAL::ActorProperty& curProp = **i;
      if (curProp.IsReadOnly())
      {
         continue;
      }

      /// make sure the property name is not in the ignore set.
      if (ignorePropsSet.find(curProp.GetName()) == ignorePropsSet.end())
      {
         dtDAL::ActorProperty* toProp = to.GetProperty(curProp.GetName());
         if (toProp == nullptr)
         {
            std::cerr << "Proxies with the same Id, don't have the same properties??" << std::endl
               << "   Actor Id: \"" << from.GetId().ToString() << std::endl 
               << "   Property: \"" << curProp.GetName() << std::endl 
               << "   First ActorType: \"" << from.GetActorType().GetFullName() << std::endl 
               << "   Second ActorType: \"" << to.GetActorType().GetFullName() << "\""
               << std::endl << std::endl;
         }
         else
         {
            if (toProp->ToString() != curProp.ToString())
            {
               mNumPropsChanged ++;
               actorPropChanged = true;
            }
            toProp->CopyFrom(curProp);
         }
      }
   }

   // Copy over name also.
   if (to.GetName() != from.GetName())
   {
      to.SetName(from.GetName());
      mNumNamesChanged ++;
      actorPropChanged = true;
   }

   if (actorPropChanged)
   {
      mNumActorsChanged ++;
   }
}

/////////////////////////////////////////////////////////////////////
void MergeProxies(dtDAL::Map& mapFrom, dtDAL::Map& mapTo, std::set<std::string> ignorePropsSet)
{
   std::vector<std::shared_ptr<dtDAL::ActorProxy> > proxiesFrom;
   mapFrom.GetAllProxies(proxiesFrom);
   std::vector<std::shared_ptr<dtDAL::ActorProxy> >::iterator i, iend;
   i = proxiesFrom.begin();
   iend = proxiesFrom.end();
   for (; i != iend; ++i)
   {
      dtDAL::ActorProxy& mapFromProxyCurrent = **i;
      dtDAL::ActorProxy* foundProxy = mapTo.GetProxyById(mapFromProxyCurrent.GetId());
      if (foundProxy != nullptr)
      {
         SyncProperties(mapFromProxyCurrent, *foundProxy, ignorePropsSet);
      }
      else
      {
         mapTo.AddProxy(mapFromProxyCurrent);
         mNumActorsAdded ++;
      }
   }
}

/////////////////////////////////////////////////////////////////////
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
      if (foundEvent != nullptr)
      {
         if (foundEvent->GetDescription() != curEvent->GetDescription() && 
            foundEvent->GetName() != curEvent->GetName())
         {
            mNumEventsChanged ++;
         }

         foundEvent->SetDescription(curEvent->GetDescription());
         foundEvent->SetName(curEvent->GetName());
      }
      else
      {
         mapToEventMan.AddEvent(*curEvent);
         mNumEventsAdded ++;
      }

   }
}

/////////////////////////////////////////////////////////////////////
/** \brief A functor which tests if a character is part of a newline.
  * This "predicate" needed to have 'state', the locale member.
  */
class IsEOL : public std::unary_function<char, bool>
{
   public:
      bool operator()(char c) const { return c == '\n' || c == '\r'; }

private:
   std::locale mLocale;
};

/////////////////////////////////////////////////////////////////////
void ReadIgnoreProplist(const std::string& path, std::vector<std::string>& resultingList)
{
   dtUtil::FileInfo fi = dtUtil::FileUtils::GetInstance().GetFileInfo(path);
   if (fi.fileType != dtUtil::REGULAR_FILE)
   {
      throw dtUtil::Exception("Unable to open file \"" + path + "\" for a list of properties to ignore when copying from the base map.",
               __FILE__, __LINE__);
   }

   char* c = new char[fi.size + 1];

   std::ifstream ifs(path.c_str(), std::ios_base::in);
   ifs.read(c, fi.size);
   if (ifs.bad())
   {
      delete[] c;
      throw dtUtil::Exception("Unable to read from file \"" + path + "\" for a list of properties to ignore when copying from the base map.",
               __FILE__, __LINE__);

   }

   c[fi.size] = '\0';
   std::string data(c);

   dtUtil::StringTokenizer<IsEOL>::tokenize(resultingList, data);
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
int main (int argc, char* argv[])
{
   const std::string executable = argv[0];
   std::string projectPath;
   std::string mapFromName;
   std::string mapToName;

   std::string ignorePropsFile;
   std::string currArg;
   for (int arg = 1; arg < argc; ++arg)
   {
      currArg = argv[arg];
      if (currArg == "-i")
      {
         if (arg + 1 < argc)
         {
            ignorePropsFile = argv[arg + 1];
            ++arg;
         }
         else
         {
            std::cerr << "Passed -i without an argument.\n";
            Usage();
         }
      }
      else if (projectPath.empty())
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

   std::cout << "Map Merge - merging from [" << 
      mapFromName << "] onto [" << mapToName << "]." << std::endl << std::endl;

   dtAudio::AudioManager::Instantiate();

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

      std::vector<std::string> ignoreProps;
      if (!ignorePropsFile.empty())
      {
         ReadIgnoreProplist(ignorePropsFile, ignoreProps);
      }
      std::set<std::string> ignorePropsSet;
      ignorePropsSet.insert(ignoreProps.begin(), ignoreProps.end());

      MergeLibraries(mapFrom, mapTo);
      MergeProxies(mapFrom, mapTo, ignorePropsSet);
      MergeEvents(mapFrom, mapTo);

      dtDAL::Project::GetInstance().SaveMap(mapTo);

      std::cout << std::endl;
      std::cout << "Map Merge was a success!!! Successfully merged:" << std::endl << 
         "  from map [" << mapFromName << "] onto [" << mapToName << "]." << std::endl;
      std::cout << "  [" << mNumNamesChanged << "] Names Changed; [" << mNumEventsAdded << "] Events Added; [" << 
         mNumEventsChanged << "] Events Changed; [" << 
         mNumLibrariesAdded << "] Libs Added; " << std::endl;
      std::cout << "  [" << mNumActorsAdded << "] Actors Added; [" << 
         mNumActorsChanged << "] Actors Changed; [" << 
         mNumPropsChanged << "] Props Changed; " << std::endl;


      dtDAL::Project::GetInstance().CloseAllMaps(true);

      dtAudio::AudioManager::Destroy();
   }
   catch(const dtUtil::Exception& ex)
   {
      std::cout << "Map Merge failed. Error occurred merging from map [" <<
         mapFromName << "] onto [" << mapToName << "]." << std::endl;
      dtDAL::Project::GetInstance().CloseAllMaps(true);
      dtAudio::AudioManager::Destroy();
      LOG_ERROR(ex.ToString());
      return 1;
   }

}

