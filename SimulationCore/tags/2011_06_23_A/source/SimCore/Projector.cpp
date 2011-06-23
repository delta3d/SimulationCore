/* -*-c++-*-
* Simulation Core
* Copyright 2007-2008, Alion Science and Technology
*
* This library is free software; you can redistribute it and/or modify it under
* the terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; either version 2.1 of the License, or (at your option)
* any later version.
*
* This library is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
* details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
* This software was developed by Alion Science and Technology Corporation under
* circumstances in which the U. S. Government may have rights in the software.
*/
#include <prefix/SimCorePrefix.h>
#include <stdio.h>
#include <math.h>

#include <osg/NodeCallback>

#include <SimCore/Projector.h>

using namespace osg;

//Projector::Projector(unsigned int unit) 
//{
//   setTextureUnit( unit );
//}
//
//Projector::Projector( unsigned int unit, double fov )
//{
//   setTextureUnit( unit );
//   setFOV( fov );
//}
//
//Projector::Projector( unsigned int unit, double hfov, double vfov )
//{
//   setTextureUnit( unit );
//   setFOV( hfov, vfov );
//}
//
//void Projector::setFOV( double fov )
//{
//   setFOV(fov,fov);
//}
//
//void Projector::setFOV( double hfov, double vfov )
//{
//   osg::TexGen *texGen = getTexGen();
//   texGen->setMode( osg::TexGen::EYE_LINEAR );
//
//   double left   = -tan(hfov*0.5);
//   double right  =  tan(hfov*0.5);
//   double bottom = -tan(vfov*0.5);
//   double top    =  tan(vfov*0.5);
//   osg::Matrix P;
//   //P.makeFrustum( left, right, bottom, top, 1.0, 100.0 );
//   P.makePerspective( 90.0, 1.0, 30.0, 300.0 );
//
//
//   osg::Matrix C(
//      0.5, 0, 0, 0,
//      0, 0.5, 0, 0,
//      0, 0, 0.5, 0,
//      0.5, 0.5, 0.5, 1
//      );
//
//   osg::Matrix PC = P * C;
//   texGen->setPlanesFromMatrix( PC );
//}



Projector::Projector( Texture2D *texture, unsigned int textureUnit )
{
	_texture = texture;
	_textureUnit = textureUnit;
	_matrix.makeIdentity();
	_texgen  = new privateTexGen;
   _texgen->setMode(osg::TexGen::OBJECT_LINEAR);
   _texgen->setMatrix(osg::Matrix::identity());
   setFOV(/*M_PI*/osg::PI/4.0);
	on();
}

void Projector::on()
{
	setTextureAttributeAndModes( _textureUnit, _texture.get() );
    setTextureAttribute(_textureUnit,_texgen.get());
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
	_ison = true;
}

void Projector::off()
{
    setTextureMode( _textureUnit, GL_TEXTURE_2D, osg::StateAttribute::OFF );
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_S,osg::StateAttribute::OFF);
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_T,osg::StateAttribute::OFF);
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_R,osg::StateAttribute::OFF);
    setTextureMode(_textureUnit,GL_TEXTURE_GEN_Q,osg::StateAttribute::OFF);
	_ison = false;
}


void Projector::useTextureUnit( unsigned int unit )
{
	if( unit != _textureUnit )
	{
		off();
		_textureUnit = unit;
		on();
	}
}

void Projector::setPositionAndAttitude( const osg::Vec3 &position, const osg::Quat &orientation )
{
	Matrix m;
	orientation.get(m);
	_matrix = m * osg::Matrix::translate( position );
	_texgen->setMatrix(_matrix);
}

void Projector::setPositionAndAttitude( const osg::Matrix &matrix )
{
	_matrix = matrix;
	_texgen->setMatrix(_matrix);
}

void Projector::setFOV( float fov )
{
	setFOV(fov,fov);
}

void Projector::setFOV( float hfov, float vfov )
{
    osg::Matrix P;
    float left   = -tan(hfov/2.0);
    float right  =  tan(hfov/2.0);
    float bottom = -tan(vfov/2.0);
    float top    = tan(vfov/2.0);
    P.makeFrustum( left, right, bottom, top, 1.0, 100.0 );

    osg::Matrix C(
		0.5, 0, 0, 0,
		0, 0.5, 0, 0,
		0, 0, 0.5, 0,
		0.5, 0.5, 0.5, 1
    );

	osg::Matrix PC = P * C;
    _texgen->setPlane(osg::TexGen::S,osg::Plane(PC(0,0),PC(1,0),PC(2,0),PC(3,0)));
    _texgen->setPlane(osg::TexGen::T,osg::Plane(PC(0,1),PC(1,1),PC(2,1),PC(3,1)));
    _texgen->setPlane(osg::TexGen::R,osg::Plane(PC(0,2),PC(1,2),PC(2,2),PC(3,2)));
    _texgen->setPlane(osg::TexGen::Q,osg::Plane(PC(0,3),PC(1,3),PC(2,3),PC(3,3)));
}
