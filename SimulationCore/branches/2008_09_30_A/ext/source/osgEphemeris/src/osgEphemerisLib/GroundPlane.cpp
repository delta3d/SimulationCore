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
#include <osgEphemeris/GroundPlane>

using namespace osgEphemeris;

GroundPlane::GroundPlane( double radius )
{
    osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;

    coords->push_back( osg::Vec3( -radius, -radius, -1000.0 ));
    coords->push_back( osg::Vec3(  radius, -radius, -1000.0 ));
    coords->push_back( osg::Vec3(  radius,  radius, -1000.0 ));
    coords->push_back( osg::Vec3( -radius,  radius, -1000.0 ));

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back( osg::Vec3(0,0,1));

    osg::ref_ptr<osg::Vec4Array> colors  = new osg::Vec4Array;
    colors->push_back( osg::Vec4( 0.0, 0.0, 0.0, 1.0 ));

    osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
    tcoords->push_back( osg::Vec2( 0.0, 0.0 ));
    tcoords->push_back( osg::Vec2( 1.0, 0.0 ));
    tcoords->push_back( osg::Vec2( 1.0, 1.0 ));
    tcoords->push_back( osg::Vec2( 0.0, 1.0 ));


    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray( coords.get() );
    geometry->setTexCoordArray( 0, tcoords.get() );

    geometry->setNormalArray( normals.get() );
    geometry->setNormalBinding( osg::Geometry::BIND_OVERALL );

    geometry->setColorArray( colors.get() );
    geometry->setColorBinding( osg::Geometry::BIND_OVERALL );

    geometry->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, coords->size() ));

    addDrawable( geometry.get() );
}
