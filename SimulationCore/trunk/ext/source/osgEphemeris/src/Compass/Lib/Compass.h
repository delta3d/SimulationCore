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
#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Vec4>

#include <osg/NodeVisitor>
#include <osg/ClipPlane>
#include <osg/Viewport>


class Compass : public osg::Projection
{
    public:
        Compass();
        Compass( osg::Viewport *vp);

        Compass(const Compass& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

        META_Node(osgCompass, Compass);

        void setViewport( osg::Viewport *vp );
        const osg::Viewport *getViewport() const;

    protected:

        virtual void traverse(osg::NodeVisitor&);
        //static const osg::Vec4 color;

    private:
        osg::ref_ptr<osg::MatrixTransform> _tx;
        osg::ref_ptr<osg::MatrixTransform> _ltx;
        osg::ref_ptr<osg::ClipPlane> _clipPlane;

        osg::ref_ptr<osg::Geode> _makeGeode();
        osg::ref_ptr<osg::Geode> _lineGeode();

        osg::ref_ptr<osg::Viewport> _viewport;

        bool _initialized;
        void _init();

};
