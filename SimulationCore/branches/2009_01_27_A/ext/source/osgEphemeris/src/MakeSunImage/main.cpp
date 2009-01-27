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
#include <Producer/RenderSurface>
#include <string>
#include <osgDB/ReadFile>
#include <stdio.h>
#include <osg/Image>

void printImage( osg::Image *image )
{
    printf( "unsigned int osgEphemeris::SkyDome::_sunImageWidth  = %u;\n", image->s() );
    printf( "unsigned int osgEphemeris::SkyDome::_sunImageHeight = %u;\n", image->t() );
    printf( "unsigned char osgEphemeris::SkyDome::_sunImageData[] = {\n" );
    unsigned char *ptr = image->data();

    unsigned int is = image->getPixelSizeInBits()/8;
    unsigned int n = image->s() * image->t() * is;
    for( unsigned int i = 0; i < n; i++ )
    {
        if( !(i%16) )
            printf("\n\t" );

        unsigned char c = *(ptr++);
        printf(" 0x%02x,", c );
    }
    printf("\n};\n\n" );
}


int main(int argc, char **argv )
{
    if( argc < 2 )
        return 1;

    std::string sunImageFile = std::string(argv[1]);

    if( sunImageFile.empty() )
        return 1;

    Producer::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;
    rs->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
    rs->realize();

    printf( "\n#include <osgEphemeris/SkyDome>\n\n" );

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( sunImageFile );

    printf( "unsigned int osgEphemeris::SkyDome::_sunImageInternalTextureFormat = 0x%x;\n", image->getInternalTextureFormat() );
    printf( "unsigned int osgEphemeris::SkyDome::_sunImagePixelFormat           = 0x%x;\n", image->getPixelFormat() );
    printf( "\n" );

    printImage( image.get() );

    return 0;
}
