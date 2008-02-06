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
#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <osg/Group>
#include <osg/StateSet>
#include <osg/Image>
#include <osg/Texture2D>
#include <osg/TexGenNode>
#include <osg/TexGen>
#include <osg/Vec3>
#include <osg/Quat>
#include <osg/MatrixTransform>
#include <osg/ref_ptr>

#include <osgUtil/SceneView>
#include <SimCore/Export.h>

//class SIMCORE_EXPORT Projector : public osg::TexGenNode
//{
//public:
//   Projector(unsigned int unit) ;
//   Projector( unsigned int unit, double fov );
//   Projector( unsigned int unit, double hfov, double vfov );
//
//   void setFOV( double fov );
//   void setFOV( double hfov, double vfov );
//};


//static void printMat( double *m )
//{
//    for( int i = 0; i < 16; i++ )
//    {
//        if( !(i%4) ) printf("\n\t" );
//        printf( " %8.4lf", m[i] );
//    }
//    puts( "" );
//}

class SIMCORE_EXPORT Projector : public osg::StateSet
{
	public:
		Projector( osg::Texture2D *texture, unsigned int textureUnit=0 );
		void useTextureUnit( unsigned int unit );
		void setPositionAndAttitude( const osg::Vec3 &position, const osg::Quat &orientation );
		void setPositionAndAttitude( const osg::Matrix &matrix );
		void setFOV( float fov );
		void setFOV( float hfov, float vfov );

		void on(); 
		void off();
		bool isOn() { return _ison; }

		class UpdateCallback : public osg::NodeCallback
		{
			public:
				UpdateCallback(Projector &pj, osgUtil::SceneView *sceneview): 
						_pj(pj), _sceneview(sceneview)
				{}

        		virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
				{
					osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
					if( tx != NULL )
					{
			    		osg::Matrix L, matrix;
			    		tx->computeLocalToWorldMatrix( L, nv);
                        /*
                        printf( "\033[0;0H\n" );
                        printMat( M.ptr() );
                        */
                        osg::Matrixd C = _sceneview->getViewMatrix();
                        /*
                        puts( "\n" );
                        printMat( C.ptr() );
                        */
						osg::Matrix Ri( (C)(0,0), (C)(1,0), (C)(2,0), 0,
									    (C)(0,1), (C)(1,1), (C)(2,1), 0,
									    (C)(0,2), (C)(1,2), (C)(2,2), 0,
							   		     0, 0, 0, 1 );
						//matrix =  Ri * M * C;
                  matrix =  osg::Matrix::rotate( /*M_PI*/-osg::PI/2.0, 1, 0, 0 ) * L * C;
						_pj.setPositionAndAttitude( matrix );
					}
            		traverse(node,nv);
				}
			private:
				Projector &_pj;
				osg::ref_ptr<osgUtil::SceneView> _sceneview;
		};

	protected:
      virtual ~Projector(){}
	private:

	    class privateTexGen : public osg::TexGen
	    {
    	    public:
			    privateTexGen() {}

        		void setMatrix(const osg::Matrix& matrix) { _matrix = matrix; }
        		virtual void apply(osg::State& state) const
        		{
    	    		glPushMatrix();
#ifdef OSG_USE_DOUBLE_MATRICES
    	    		glLoadMatrixd(_matrix.ptr());
#else
    	    		glLoadMatrixf((float*)(_matrix.ptr()));
#endif
    	    		osg::TexGen::apply(state);
    	    		glPopMatrix();
        		}
    		private:
        		osg::Matrix _matrix;
		};

		osg::ref_ptr<osg::Texture2D> _texture;
		osg::ref_ptr<privateTexGen>  _texgen;
		unsigned int                 _textureUnit;
		osg::Matrix					 _matrix;
		bool _ison;

};
#endif
