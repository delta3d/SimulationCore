#ifndef MAIN_H_
#define MAIN_H_

#include <dtABC/application.h>
#include <dtGUI/ceuidrawable.h>
#include <dtGame/messagefactory.h>
#include <dtGame/basemessages.h>

dtABC::Application& GetGlobalApplication();
dtGUI::CEUIDrawable& GetGlobalCEGUIDrawable();

#endif /*MAIN_H_*/
