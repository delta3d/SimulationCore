/* -*-c++-*- OpenSceneGraph - Ephemeris Model Copyright (C) 2005 Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <iostream>
#include <string>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "../Lib/Compass.h"

class ItIs {
    public:
        ItIs() { puts("It is what it is" ); }
};

static ItIs itIs;

bool Compass_readLocalData(osg::Object& obj, osgDB::Input& fr);
bool Compass_writeLocalData(const osg::Object& obj, osgDB::Output& fw);

osgDB::RegisterDotOsgWrapperProxy g_CompassProxy (
     new Compass,
         "Compass",
         "Compass",
          &Compass_readLocalData,
          &Compass_writeLocalData,
          osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool Compass_readLocalData(osg::Object& obj, osgDB::Input& fr)
{
    bool itAdvanced = false;
    Compass &compass = static_cast<Compass &>(obj);

    if( fr[0].matchWord("viewport")) 
    {
        ++fr;
        int x = atoi( fr[0].getStr() );
        ++fr;
        int y = atoi( fr[0].getStr() );
        ++fr;
        int width = atoi( fr[0].getStr() );
        ++fr;
        int height = atoi( fr[0].getStr() );

        compass.setViewport( new osg::Viewport( x, y, width, height ));

        itAdvanced = true;
    }

    return itAdvanced;
}

bool Compass_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const Compass &compass = static_cast<const Compass &>(obj);

    const osg::Viewport *vp = compass.getViewport();
    fw.indent() << " viewport " << vp->x() << " " << vp->y() << " " << vp->width() << " " << vp->height() << std::endl;
         

    return true;
}
