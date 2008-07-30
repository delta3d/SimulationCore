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

void printImage( osg::Image *image, const std::string &name )
{
    printf( "unsigned int osgEphemeris::MoonModel::%sWidth  = %u;\n", name.c_str(), image->s() );
    printf( "unsigned int osgEphemeris::MoonModel::%sHeight = %u;\n", name.c_str(), image->t() );
    printf( "unsigned char osgEphemeris::MoonModel::%sData[] = {\n", name.c_str() );
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
    if( argc < 3 )
        return 1;

    std::string moonImageFile = std::string(argv[1]);
    std::string moonNormalImageFile = std::string(argv[2]);

    if( moonImageFile.empty() || moonNormalImageFile.empty() )
        return 1;

    Producer::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;
    rs->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
    rs->realize();

    printf( "\n#include <osgEphemeris/MoonModel>\n\n" );

    osg::ref_ptr<osg::Image> image = osgDB::readImageFile( moonImageFile );

    printf( "unsigned int osgEphemeris::MoonModel::_moonImageInternalTextureFormat = 0x%x;\n", image->getInternalTextureFormat() );
    printf( "unsigned int osgEphemeris::MoonModel::_moonImagePixelFormat           = 0x%x;\n", image->getPixelFormat() );
    printf( "\n" );


    printImage( image.get(), "_moonImageHiLod" );

    image->scaleImage( 256, 128, 1 );
    printImage( image.get(), "_moonImageLoLod" );

    image = osgDB::readImageFile( moonNormalImageFile );
    printf( "\n" );
    printf( "unsigned int osgEphemeris::MoonModel::_moonNormalImageInternalTextureFormat = 0x%x;\n", image->getInternalTextureFormat() );
    printf( "unsigned int osgEphemeris::MoonModel::_moonNormalImagePixelFormat           = 0x%x;\n", image->getPixelFormat() );
    printf( "\n" );
    printImage( image.get(), "_moonNormalImageHiLod" );

    image->scaleImage( 256, 128, 1 );
    printImage( image.get(), "_moonNormalImageLoLod" );

    return 0;
}
