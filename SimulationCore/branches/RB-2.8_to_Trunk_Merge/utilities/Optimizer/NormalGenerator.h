/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or .45
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSGUTIL_NormalGenerator_
#define OSGUTIL_NormalGenerator_

#include <osgUtil/Export>

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Array>
#include <osg/Geometry>

namespace LevelCompiler
{

class NormalGenerator: public osg::Referenced 
{
public:
    NormalGenerator();
    NormalGenerator(const NormalGenerator& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

    void Generate(osg::Geometry* geo);
    bool ValidateNormals(osg::Geometry* geo);

    inline osg::Vec3Array* getNormalArray()                { return N_.get(); }
    inline const osg::Vec3Array* getNormalArray() const    { return N_.get(); }
    inline void setNormalArray(osg::Vec3Array* array)      { N_ = array; }

    inline osg::IndexArray* getIndices() { return indices_.get(); }

protected:

    virtual ~NormalGenerator() {}
    NormalGenerator &operator=(const NormalGenerator &) { return *this; }

    void compute(osg::PrimitiveSet* pset,
                 const osg::Array* vx,
                 int iA, int iB, int iC);

    bool verify(const osg::Array* normArray);
    bool verify(const osg::Vec3& norm);

    osg::ref_ptr<osg::Vec3Array> N_;
    osg::ref_ptr<osg::UIntArray> indices_;
};

}

#endif
