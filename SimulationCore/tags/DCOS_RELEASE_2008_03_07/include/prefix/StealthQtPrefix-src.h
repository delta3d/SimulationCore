#ifdef DELTA_PCH

#ifndef STEALTHQT_PREFIX
#define STEALTHQT_PREFIX

#define NO_DT_WIN_PCH_HEADER
#include <prefix/SimCorePrefix-src.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QAction>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>


//remove useless macros in X11 headers
#ifdef None
#undef None
#endif

#endif
#endif
