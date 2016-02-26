#include <prefix/SimCorePrefix.h>
#ifndef MAIN_H_
#define MAIN_H_

#include <CEGUI/CEGUIVersion.h>

#include <dtABC/application.h>
#include <dtCore/system.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>
#include <dtGame/gamemanager.h>
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
#include <dtGUI/ceuidrawable.h>
#else
#include <dtGUI/gui.h>
#endif

dtABC::Application& GetGlobalApplication();
#if CEGUI_VERSION_MAJOR >= 0 && CEGUI_VERSION_MINOR < 7
dtGUI::CEUIDrawable& GetGlobalCEGUIDrawable();
#else
dtGUI::GUI& GetGlobalGUI();
#endif

#endif /*MAIN_H_*/
