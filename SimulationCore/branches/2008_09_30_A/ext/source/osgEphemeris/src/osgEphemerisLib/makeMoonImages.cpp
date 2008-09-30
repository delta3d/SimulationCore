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
#include <string>
#include <osgDB/ReadFile>
#include <stdio.h>
#include <osg/Image>


int main(int argc, char **argv )
{
    if( argc < 3 )
        return 1;

    std::string moonImageFile = std::string(argv[1]);
    std::string moonNormalImageFile = std::string(argv[2]);

    if( moonImageFile.empty() || moonNormalImageFile.empty() )
        return 1;
    /*
        static unsigned int _moonImageLoLodWidth, _moonImageLoLodHeight;
        static unsigned char _moonImageLoLodData[];
        static unsigned int _moonImageHiLodWidth, _moonImageHiLodHeight;
        static unsigned char _moonImageHiLodData[];

        static unsigned int _moonNormalImageLoLodWidth, _moonNormalImageLoLodHeight;
        static unsigned char _moonNormalImageLoLodData[];
        static unsigned int _moonNormalImageHiLodWidth, _moonNormalImageHiLodHeight;
        static unsigned char _moonNormalImageHiLodData[];
        */


    osg::ref_ptr<osg::Image> moonImage = osgDB::readImageFile( moonImageFile );

    printf( "\n#include <Ephemeris/MoonModel>\n\n" );
    printf( "osgEphemeris::MoonModel::_moonImageHiLodWidth  = %u;\n", moonImage->s() );
    printf( "osgEphemeris::MoonModel::_moonImageHiLodHeight = %u;\n", moonImage->t() );

    //unsigned char *ptr = moonImage->data();

    return 0;
}
