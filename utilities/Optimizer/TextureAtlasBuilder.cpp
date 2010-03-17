/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
#include <stdlib.h>
#include <string.h>

#include <osgUtil/Optimizer>
#include "TextureAtlasBuilder.h"

#include <osg/ApplicationUsage>
#include <osg/Transform>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/OccluderNode>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/Texture>
#include <osg/PagedLOD>
#include <osg/ProxyNode>
#include <osg/ImageStream>
#include <osg/Timer>
#include <osg/TexMat>
#include <osg/io_utils>

#include <osgUtil/TransformAttributeFunctor>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Tessellator>
#include <osgUtil/Statistics>

#include <typeinfo>
#include <algorithm>
#include <numeric>
#include <sstream>

using namespace osgUtil;

static osg::ApplicationUsageProxy Optimizer_e0(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_OPTIMIZER \"<type> [<type>]\"","OFF | DEFAULT | FLATTEN_STATIC_TRANSFORMS | FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS | REMOVE_REDUNDANT_NODES | COMBINE_ADJACENT_LODS | SHARE_DUPLICATE_STATE | MERGE_GEOMETRY | MERGE_GEODES | SPATIALIZE_GROUPS  | COPY_SHARED_NODES  | TRISTRIP_GEOMETRY | OPTIMIZE_TEXTURE_SETTINGS | REMOVE_LOADED_PROXY_NODES | TESSELLATE_GEOMETRY | CHECK_GEOMETRY |  FLATTEN_BILLBOARDS | TEXTURE_ATLAS_BUILDER | STATIC_OBJECT_DETECTION");

static std::string getSimpleFileName ( const std::string& fileName )
{
    // Copied from osgDB/FileNameUtils.cpp so we aren't dependent on osgDB
    std::string::size_type slash1 = fileName.find_last_of('/');
    std::string::size_type slash2 = fileName.find_last_of('\\');
    if (slash1==std::string::npos) 
    {
        if (slash2==std::string::npos) return fileName;
        return std::string(fileName.begin()+slash2+1,fileName.end());
    }
    if (slash2==std::string::npos) return std::string(fileName.begin()+slash1+1,fileName.end());
    return std::string(fileName.begin()+(slash1>slash2?slash1:slash2)+1,fileName.end());
}

////////////////////////////////////////////////////////////////////////////
// TextureAtlasBuilder
////////////////////////////////////////////////////////////////////////////

TextureAtlasBuilder::TextureAtlasBuilder():
    _maximumAtlasWidth(4096),
    _maximumAtlasHeight(4096),
    _margin(8)
{
}

/**
 * \brief Resets this TextureAtlasBuilder, removing all atlases and sources.
 */
void TextureAtlasBuilder::reset()
{
    _sourceList.clear();
    _atlasList.clear();
}

/**
 * \brief Sets the maximum size allowed for any Atlas to be built.
 *
 * \param   width       maximum width of an Atlas
 * \param   height      maximum height of an Atlas
 *
 */
void TextureAtlasBuilder::setMaximumAtlasSize(unsigned int width, unsigned int height)
{
    _maximumAtlasWidth = width;
    _maximumAtlasHeight = height;
}


/**
 * \brief Sets the margin to allow around each texture image.
 *
 * \param   margin      number of pixels (scan lines) to add to
 *                      each edge of each texture image.
 *
 *  Leaving a margin between textures avoids bleeding between them
 *  at coarse MIP map levels.
 */
void TextureAtlasBuilder::setMargin(unsigned int margin)
{
    _margin = margin;
}

/**
 * \brief Adds a source image to the list to be included in this atlas.
 *
 * The list is maintained in sorted order, with smallest images (by height)
 *  first.
 */
void TextureAtlasBuilder::addSource(const osg::Image* image)
{
    if (!getSource(image))
    {
        osg::ref_ptr<Source> newSource ( new Source ( image ) );
        SourceList::iterator i;
        const std::string newFileName ( image->getName() );
        for ( i=_sourceList.begin();
              i<_sourceList.end();
              i++ )
        {
            if ( (*i)->_image.get() == image )
            {
                // We already have this image; don't duplicate
                osg::notify(osg::INFO) << "Not adding duplicate of " << image->getName() << std::endl;
                return;
            }
            else
            {
                const std::string thisFileName ( (*i)->_image->getName() );
                if ( thisFileName == newFileName )
                {
                    osg::notify(osg::INFO) << "Multiple Source objects reference same file " << newFileName << std::endl;
                }
                else if ( getSimpleFileName ( thisFileName ) == getSimpleFileName ( newFileName ) )
                {
                    osg::notify(osg::INFO) << "Source file base name " << newFileName << " matches " << thisFileName << std::endl;
                }
            }
            if ( ! ( (**i) < *newSource ) )
            {
                break;
            }
        }
        _sourceList.insert ( i, newSource );
    }
}

/**
 * \brief Adds the image from a source osg::Texture2D to the list to
 *        be included in this atlas.
 *
 * The list is maintained in sorted order, with smallest images (where
 *  size is defined as total number of pixels) first.
 */
void TextureAtlasBuilder::addSource(const osg::Texture2D* texture)
{
	if ( !getSource(texture) && texture->getImage() )
    {
        Source* newSource = new Source ( texture );
        const std::string newFileName ( texture->getImage()->getName() );
        SourceList::iterator i;
        for ( i=_sourceList.begin();
              i<_sourceList.end();
              i++ )
        {
            if ( (*i)->_image.get() == texture->getImage() )
            {
                // We already have this image; don't duplicate
                osg::notify(osg::INFO) << "Not adding duplicate of " << (*i)->_image->getName().c_str() << std::endl;
                return;
            }
            else
            {
                const std::string thisFileName ( (*i)->_image->getFileName() );
                if ( thisFileName == newFileName )
                {
                    osg::notify(osg::INFO) << "Multiple Source objects reference same file " << newFileName << std::endl;
                }
                else if ( getSimpleFileName ( thisFileName ) == getSimpleFileName ( newFileName ) )
                {
                    osg::notify(osg::INFO) << "Source file base name " << newFileName << " matches " << thisFileName << std::endl;
                }
            }
            if ( ! ( (**i) < *newSource ) )
            {
                break;
            }
        }
        _sourceList.insert ( i, newSource );
    }
}

/*
 * \brief Builds a series of atlases from the image list built by addSource().
 *
 * Each time we encounter an image that won't fit in the current
 * atlas, we create a new atlas and start to fill the new one.
 */
void TextureAtlasBuilder::buildAtlas()
{
    // assign the source to the atlas
    _atlasList.clear();
    reverse ( _sourceList.begin(), _sourceList.end() ); // Reverse list so largest textures will be inserted first.
    for(SourceList::iterator sitr = _sourceList.begin();
        sitr != _sourceList.end();
        ++sitr)
    {
        Source* source = sitr->get();
        if (source->suitableForAtlas(_maximumAtlasWidth,_maximumAtlasHeight,_margin))
        {
            bool addedSourceToAtlas = false;
            for(AtlasList::iterator aitr = _atlasList.begin();
                aitr != _atlasList.end() && !addedSourceToAtlas;
                ++aitr)
            {
                if ((*aitr)->doesSourceFit(source))
                {
                    addedSourceToAtlas = true;
                    (*aitr)->addSource(source);
                }
                else
                {
                    osg::notify(osg::INFO)<<"    source "<< getSimpleFileName(source->_image->getFileName()) <<" does not fit in atlas "<<aitr->get()<<std::endl;
                }
            }

            if (!addedSourceToAtlas)
            {
                const osg::Texture2D* sourceTexture ( source->_texture.get() );
                osg::notify(osg::INFO)<<">|< Creating new atlas for "<<source->_image->getFileName()<<std::endl;

                osg::ref_ptr<Atlas> atlas = new Atlas(_maximumAtlasWidth,_maximumAtlasHeight,_margin);
                if ( sourceTexture != NULL )
                {
                    atlas->_wrap_s = sourceTexture->getWrap(osg::Texture2D::WRAP_S);
                    atlas->_wrap_t = sourceTexture->getWrap(osg::Texture2D::WRAP_T);
                    if ( atlas->_wrap_s==osg::Texture2D::REPEAT ||
                         atlas->_wrap_s==osg::Texture2D::MIRROR )
                    {
                        atlas->_width = sourceTexture->getImage()->s();
                        osg::notify(osg::INFO) << "  periodic in S, width " << atlas->_width << std::endl;
                    }
                    else if (atlas->_wrap_t==osg::Texture2D::REPEAT ||
                             atlas->_wrap_t==osg::Texture2D::MIRROR)
                    {
                        atlas->_height = sourceTexture->getImage()->t();
                        osg::notify(osg::INFO) << "  periodic in T, height " << atlas->_height << std::endl;
                    }

                    atlas->_min_filter = sourceTexture->getFilter ( osg::Texture::MIN_FILTER );
                    atlas->_mag_filter = sourceTexture->getFilter ( osg::Texture::MAG_FILTER );
                    atlas->_max_anisotropy = sourceTexture->getMaxAnisotropy();
                    atlas->_int_format = sourceTexture->getInternalFormat();
                    atlas->_int_format_mode = sourceTexture->getInternalFormatMode();
                    atlas->_int_format_type = sourceTexture->getInternalFormatType();
                    atlas->_shadow_compare_func = sourceTexture->getShadowCompareFunc();
                    atlas->_shadow_texture_mode = sourceTexture->getShadowTextureMode();
                    atlas->_shadow_ambient = sourceTexture->getShadowAmbient();
                }
                _atlasList.push_back(atlas.get());

                atlas->addSource(source);
            }
        }
    }

    // build the atlases which are suitable for use, and discard the rest.
    AtlasList activeAtlasList;
    size_t nSourcesUsed = 0;
    for(AtlasList::iterator aitr = _atlasList.begin();
        aitr != _atlasList.end();
        ++aitr)
    {
        Atlas* atlas = aitr->get();

        if (atlas->_sourceList.size()==1)
        {
            // no point building an atlas with only one entry
            // so disconnect the source.
            Source* source = atlas->_sourceList[0].get();
            source->_atlas = 0;
            atlas->_sourceList.clear();
        }

        if (!(atlas->_sourceList.empty()))
        {
            std::stringstream ostr;
            ostr<<"atlas_"<<activeAtlasList.size()<<".png";
            atlas->_image->setFileName(ostr.str());
            std::string sWrapString ( " S clamped" );
            if ( atlas->_wrap_s==osg::Texture2D::REPEAT ||
                 atlas->_wrap_s==osg::Texture2D::MIRROR )
            {
                sWrapString = " S periodic";
            }
            std::string tWrapString ( " T clamped" );
            if ( atlas->_wrap_t==osg::Texture2D::REPEAT ||
                 atlas->_wrap_t==osg::Texture2D::MIRROR )
            {
                tWrapString = " T periodic";
            }
            std::string formatString;
            switch ( atlas->_image->getPixelFormat() )
            {
            case GL_COLOR_INDEX:
                formatString = " Color index mode";
                break;
            case GL_STENCIL_INDEX:
                formatString = " Stencil index mode";
                break;
            case GL_DEPTH_COMPONENT:
                formatString = " Depth component mode";
                break;
            case GL_RED:
                formatString = " Red mode";
                break;
            case GL_GREEN:
                formatString = " Blue mode";
                break;
            case GL_ALPHA:
                formatString = " Alpha mode";
                break;
            case GL_RGB:
                formatString = " RGB mode";
                break;
            case GL_RGBA:
                formatString = " GL_RGB mode with alpha";
                break;
            case GL_LUMINANCE:
                formatString = " Luminance mode";
                break;
            case GL_LUMINANCE_ALPHA:
                formatString = " Luminance mode with alpha";
                break;
            default:
				{
					std::stringstream msg;
					msg << " Unknown pixel format " << atlas->_image->getPixelFormat();
					formatString = msg.str();
				}
                break;
            }
            osg::notify(osg::NOTICE)<<"Atlas " << atlas << " named \"" << ostr.str() << "\""
                                    << " has " << atlas->_sourceList.size() << " textures: " << sWrapString + tWrapString
                                    << formatString << std::endl;

            nSourcesUsed += atlas->_sourceList.size();
            activeAtlasList.push_back(atlas);
            atlas->clampToNearestPowerOfTwoSize();
            atlas->copySources();

        }
    }

    // keep only the active atlases
    _atlasList.swap(activeAtlasList);
    osg::notify(osg::NOTICE)<<"Total of "<< _atlasList.size() << " atlases containing " << nSourcesUsed << " source textures." <<std::endl;

}

/*
 * \brief Fetches the Image for a selected Source within this Atlas.
 *
 * \param i      index within source list
 */
osg::Image* TextureAtlasBuilder::getImageAtlas(unsigned int i)
{
    Source* source = _sourceList[i].get();
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_image.get() : 0;
}

/*
 * \brief Fetches the Texture2D for a selected Source within this Atlas.
 *
 * \param i      index within source list
 */
osg::Texture2D* TextureAtlasBuilder::getTextureAtlas(unsigned int i)
{
    Source* source = _sourceList[i].get();
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_texture.get() : 0;
}

/*
 * \brief Fetches the matrix to transform from raw (s,t) of the
 *        specified Source to coordinates within the Atlas.
 */
osg::Matrix TextureAtlasBuilder::getTextureMatrix(unsigned int i)
{
    Source* source = _sourceList[i].get();
    return source ? source->computeTextureMatrix() : osg::Matrix();
}

/*
 * \brief Fetches the Image from within the Atlas corresponding to the
 *        given Image.
 */
osg::Image* TextureAtlasBuilder::getImageAtlas(const osg::Image* image)
{
    Source* source = getSource(image);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_image.get() : 0;
}

/*
 * \brief Fetches the Texture2D from within the Atlas corresponding to the
 *        given Image.
 */
osg::Texture2D* TextureAtlasBuilder::getTextureAtlas(const osg::Image* image)
{
    Source* source = getSource(image);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_texture.get() : 0;
}

/*
 * \brief Fetches the matrix to transform from raw (s,t) of the
 *        given Image to coordinates within the Atlas.
 */
osg::Matrix TextureAtlasBuilder::getTextureMatrix(const osg::Image* image)
{
    Source* source = getSource(image);
    return source ? source->computeTextureMatrix() : osg::Matrix();
}

/*
 * \brief Fetches the Image corresponding to the given Texture2D.
 *
 * \return    pointer to Image for the given Texture2D; NULL if texture
 *            hasn't been added to this TextureAtlasBuilder.
 */
osg::Image* TextureAtlasBuilder::getImageAtlas(const osg::Texture2D* texture)
{
    Source* source = getSource(texture);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_image.get() : 0;
}

/*
 * \brief Fetches the Texture2D corresponding to the given Texture2D.
 *
 * \return    pointer to Texture2D for the given Texture2D; NULL if texture
 *            hasn't been added to this TextureAtlasBuilder.
 */
osg::Texture2D* TextureAtlasBuilder::getTextureAtlas(const osg::Texture2D* texture)
{
    Source* source = getSource(texture);
    Atlas* atlas = source ? source->_atlas : 0;
    return atlas ? atlas->_texture.get() : 0;
}

/*
 * \brief Fetches the matrix to transform from raw (s,t) of the
 *        given Texture2D to coordinates within the Atlas.
 */
osg::Matrix TextureAtlasBuilder::getTextureMatrix(const osg::Texture2D* texture)
{
    Source* source = getSource(texture);
    return source ? source->computeTextureMatrix() : osg::Matrix();
}

/*
 * \brief Finds the Source corresponding to the given Image.
 *
 * \return      pointer to the Source, or NULL if the given Image hasn't been
 *              added to this TextureAtlasBuilder.
 */
TextureAtlasBuilder::Source* TextureAtlasBuilder::getSource(const osg::Image* image)
{
    for(SourceList::iterator itr = _sourceList.begin();
        itr != _sourceList.end();
        ++itr)
    {
        if ((*itr)->_image == image) return itr->get();
    }
    return 0;
}

/*
 * \brief Finds the Source corresponding to the given Texture2D.
 *
 * \return      pointer to the Source, or NULL if the given Texture2D hasn't
 *              been added to this TextureAtlasBuilder.
 */
TextureAtlasBuilder::Source* TextureAtlasBuilder::getSource(const osg::Texture2D* texture)
{
    for(SourceList::iterator itr = _sourceList.begin();
        itr != _sourceList.end();
        ++itr)
    {
        if ((*itr)->_texture == texture) return itr->get();
    }
    return 0;
}

/**
 * \brief Determines whether this Source can be put into an atlas.
 *
 * There are several reasons why a source texture may not be suitable for insertion in an atlas:
 * - It might be so big that it just won't fit in a maximum-sized atlas
 * - It might use a compressed pixel format, which we don't support in texture atlases.
 * - Its pixels might be a strange size, with a number of bits not divisible by 8.
 * - It may be tiled or mirrored, which we can't do if it's in the middle of an atlas.
 * - It may be sourced from a pbuffer, which isn't supported.
 *
 * The function Atlas::doesSourceFit() performs a similar check, but for a
 * particular Atlas.
 *
 * @param  maximumAtlasWidth    maximum width for any atlas
 * @param  maximumAtlasHeight   maximum height for any atlas
 * @param  margin               number of pixels (or scan lines) to allow between textures
 *                              to keep them from bleeding together in interpolation
 *
 * @return true                 unless one of the disqualifying conditions above is true.
 */
bool TextureAtlasBuilder::Source::suitableForAtlas(unsigned int maximumAtlasWidth, unsigned int maximumAtlasHeight, unsigned int margin)
{
    if (!_image) return false;
    const std::string& imageFileName ( _image->getFileName() );

    switch(_image->getPixelFormat())
    {
        case(GL_COMPRESSED_ALPHA_ARB):
        case(GL_COMPRESSED_INTENSITY_ARB):
        case(GL_COMPRESSED_LUMINANCE_ALPHA_ARB):
        case(GL_COMPRESSED_LUMINANCE_ARB):
        case(GL_COMPRESSED_RGBA_ARB):
        case(GL_COMPRESSED_RGB_ARB):
        case(GL_COMPRESSED_RGB_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT3_EXT):
        case(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT):
            // can't handle compressed textures inside an atlas
        osg::notify(osg::INFO) << "Image \"" << imageFileName <<
            "\" uses a compressed pixel format, so it can't go into a texture atlas." << std::endl;
            return false;
        default:
            break;
    }

    if ((_image->getPixelSizeInBits() % 8) != 0)
    {
        // pixel size not byte aligned so report as not suitable to prevent other atlas code from having problems with byte boundaries.
        osg::notify(osg::INFO) << "Image \"" << imageFileName << "\" has pixels with "
                               << _image->getPixelSizeInBits() << " bits; we can only deal with multiples of 8." << std::endl;
        return false;
    }

    if (_texture.valid())
    {

        if (_texture->getReadPBuffer()!=0)
        {
            // pbuffer textures not suitable
            osg::notify(osg::INFO) << "Image \"" << imageFileName << "\" can't go into an atlas because it comes from a pbuffer." << std::endl;
            return false;
        }

        bool tex_wrap_s = _texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
            _texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;
        bool tex_wrap_t = _texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
            _texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

        // Calculate size of padded image
        //size_t width  = _image->s()+margin*2;
        //size_t height = _image->t()+margin*2;

        // size too big?
        if ( _image->s()+margin*2 > maximumAtlasWidth || _image->t()+margin*2 > maximumAtlasHeight )
        {
            osg::notify(osg::INFO) << "Image \"" << imageFileName << "\" is " << _image->s() << "x" << _image->t() << ": too big to fit into an atlas: max is "
                                   << maximumAtlasWidth << " by " << maximumAtlasHeight << std::endl;
            return false;
        }

        if ( tex_wrap_s && tex_wrap_t )
        {
            // can't support repeating textures in texture atlas
            osg::notify(osg::INFO) << "Image \"" << imageFileName << "\" can't go into an atlas because it is periodic." << std::endl;
            return false;
        }
    }

    osg::notify(osg::INFO) << "Image \"" << imageFileName << "\" is eligible for atlas." << std::endl;
    return true;
}

/**
 * \brief Calculates the matrix to transform from raw (s,t) of this
 *        Source to coordinates within the Atlas.
 */
osg::Matrix TextureAtlasBuilder::Source::computeTextureMatrix() const
{
    if (!_atlas) return osg::Matrix();
    if (!_image) return osg::Matrix();
    if (!(_atlas->_image)) return osg::Matrix();

    return osg::Matrix::scale(float(_image->s())/float(_atlas->_image->s()), float(_image->t())/float(_atlas->_image->t()), 1.0)*
           osg::Matrix::translate(float(_x)/float(_atlas->_image->s()), float(_y)/float(_atlas->_image->t()), 0.0);
}

/**
 * \brief Checks whether the given Source can be added to this Atlas.
 *
 * \param       Source to add
 *
 * \return      true if successful; false if the Source cannot be added
 */
bool TextureAtlasBuilder::Atlas::doesSourceFit(Source* source)
{
    // does the source have a valid image?
    const osg::Image* sourceImage = source->_image.get();
    if (!sourceImage) return false;

    // does pixel format match?
    if (_image.valid())
    {
        if (_image->getPixelFormat() != sourceImage->getPixelFormat()) return false;
        if (_image->getDataType() != sourceImage->getDataType()) return false;
    }

    const osg::Texture2D* sourceTexture = source->_texture.get();
    if ( !sourceTexture )
    {
        // no space for the texture
        osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() ) <<" is rejected by doesSourceFit(): no source texture"<<std::endl;
        return false;
    }
    else
    {
        if (sourceTexture->getReadPBuffer()!=0)
        {
            // pbuffer textures not suitable
            osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() ) <<" is rejected by doesSourceFit(): source is a Pbuffer"<<std::endl;
            return false;
        }

        if ( _wrap_s != sourceTexture->getWrap(osg::Texture2D::WRAP_S) )
        {
            // Can't insert a source if wrap modes differ.
            osg::notify(osg::DEBUG_INFO)<< "Source "<< getSimpleFileName ( source->_image->getFileName() )
                                        << " is rejected by doesSourceFit(): S wrap modes differ"<<std::endl;
            return false;
        }

        if ( _wrap_t != sourceTexture->getWrap(osg::Texture2D::WRAP_T) )
        {
            // Can't insert a source if wrap modes differ.
            osg::notify(osg::DEBUG_INFO)<< "Source "<< getSimpleFileName ( source->_image->getFileName() )
                                        << " is rejected by doesSourceFit(): T wrap modes differ"<<std::endl;
            return false;
        }

        bool tex_wrap_s = sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
                          sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;
        bool tex_wrap_t = sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
                          sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

        // Calculate size of padded image
        size_t sourceWidth  = sourceImage->s()+_margin*2;
        size_t sourceHeight = sourceImage->t()+_margin*2;


        if ( tex_wrap_s )
        {
            sourceWidth = sourceImage->s();
            if ( sourceWidth != _width )
            {
                osg::notify(osg::DEBUG_INFO)<< "Source "<<getSimpleFileName ( source->_image->getFileName() )
                                            << " rejected by doesSourceFit(): image width is " << _image->s()
                                            << ", but atlas width is " << _width <<std::endl;
                return false;
            }
        }

        if ( tex_wrap_t )
        {
            sourceHeight = sourceImage->t();
            if ( sourceHeight != _height )
            {
                osg::notify(osg::DEBUG_INFO)<< "Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            << " rejected by doesSourceFit(): image height is " << _image->t()
                                            << ", but atlas height is " << _height <<std::endl;
                return false;
            }
        }

        if (_texture.valid())
        {

            bool sourceUsesBorder = sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::CLAMP_TO_BORDER ||
                                    sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::CLAMP_TO_BORDER;

            bool atlasUsesBorder = sourceTexture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::CLAMP_TO_BORDER ||
                                   sourceTexture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::CLAMP_TO_BORDER;

            if (sourceUsesBorder!=atlasUsesBorder)
            {
                // border wrapping does not match
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): border differs from atlas"<<std::endl;
                return false;
            }

            if (sourceUsesBorder)
            {
                // border colours don't match
                if (_texture->getBorderColor() != sourceTexture->getBorderColor())
                {
                    osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                                <<" is rejected by doesSourceFit(): border differs from atlas"<<std::endl;
                    return false;
                }
            }
            if (_texture->getFilter(osg::Texture2D::MIN_FILTER) != sourceTexture->getFilter(osg::Texture2D::MIN_FILTER))
            {
                // inconsitent min filters
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): min filter differs from atlas"<<std::endl;
                return false;
            }

            if (_texture->getFilter(osg::Texture2D::MAG_FILTER) != sourceTexture->getFilter(osg::Texture2D::MAG_FILTER))
            {
                // inconsitent mag filters
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): mag filter differs from atlas"<<std::endl;
                return false;
            }

            if (_texture->getMaxAnisotropy() != sourceTexture->getMaxAnisotropy())
            {
                // anisotropy different.
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): anisotropy differs from atlas"<<std::endl;
                return false;
            }

            if (_texture->getInternalFormat() != sourceTexture->getInternalFormat())
            {
                // internal formats inconistent
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): internal format "
                                            << sourceTexture->getInternalFormat() << " differs from atlas format "
                                            << _texture->getInternalFormat() <<std::endl;
                return false;
            }

            if (_texture->getShadowCompareFunc() != sourceTexture->getShadowCompareFunc())
            {
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): shadow function differs from atlas"<<std::endl;
                // shadow functions inconsitent
                return false;
            }

            if (_texture->getShadowTextureMode() != sourceTexture->getShadowTextureMode())
            {
                // shadow texture mode inconsitent
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): shadow texture mode differs from atlas"<<std::endl;
                return false;
            }

            if (_texture->getShadowAmbient() != sourceTexture->getShadowAmbient())
            {
                // shadow ambient inconsitent
                osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() )
                                            <<" is rejected by doesSourceFit(): shadow ambient differs from atlas"<<std::endl;
                return false;
            }
        }

        if ( freeSpace.canFit ( sourceWidth, sourceHeight ) )
        {
            osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() ) <<" fits in atlas" << std::endl;
            return true;
        }
    }

    // no space for the texture
    osg::notify(osg::DEBUG_INFO)<<"Source "<< getSimpleFileName ( source->_image->getFileName() ) <<" is rejected by doesSourceFit(): too large for remaining space"<<std::endl;
    return false;
}

std::string TextureAtlasBuilder::Atlas::toString() const
{
    if ( _image == NULL || _image->getFileName().empty() )
    {
        std::stringstream result;
        result << this;
        return result.str();
    }
    else
    {
        return _image->getFileName();
    }
}

/**
 * \brief Adds a new Source to this Atlas.
 *
 * \param       Source to add
 *
 * \return      true if successful; false if, according to doesSourceFit(),
 *              the Source cannot be added to this Atlas
 */
bool TextureAtlasBuilder::Atlas::addSource(Source* source)
{
    // double check source is compatible
    if (!doesSourceFit(source))
    {
        osg::notify(osg::INFO)<<"source "<< getSimpleFileName ( source->_image->getFileName() ) <<" does not fit in atlas "<< this->toString() <<std::endl;
        return false;
    }

    const osg::Image* sourceImage = source->_image.get();
    const osg::Texture2D* sourceTexture = source->_texture.get();

    size_t sMargin = _margin;
    size_t tMargin = _margin;

    size_t sourceWidth  = sourceImage->s() + 2*sMargin;
    size_t sourceHeight = sourceImage->t() + 2*tMargin;

    std::string periodString ( "not periodic" );

    // Fitting differs depending on wrap mode.
    if ( _wrap_s==osg::Texture::REPEAT ||
         _wrap_s==osg::Texture::MIRROR )
    {
        sMargin = 0;
        sourceWidth = sourceImage->s();
        _maximumAtlasWidth = _width = sourceWidth;
        _height += sourceImage->t() + 2 * tMargin;
        periodString = "periodic in S ";
    }

    if ( _wrap_t==osg::Texture::REPEAT ||
         _wrap_t==osg::Texture::MIRROR )
    {
        tMargin = 0;
        _width += sourceImage->s() + 2 * tMargin;
        sourceHeight = sourceImage->t();
        _maximumAtlasHeight = _height = sourceHeight;
        periodString = "periodic in T";
    }

    if (!_image)
    {        // need to create an image of the same pixel format to store the atlas in
        _image = new osg::Image;
        _image->setInternalTextureFormat ( sourceImage->getInternalTextureFormat() );
        _image->setPixelFormat ( sourceImage->getPixelFormat() );
        _image->setDataType ( sourceImage->getDataType() );
//         _image->setImage ( _maximumAtlasWidth, _maximumAtlasHeight, 1,
//                            sourceImage->getInternalTextureFormat(),
//                            sourceImage->getPixelFormat(),
//                            sourceImage->getDataType(),
//                            static_cast<unsigned char*>( malloc ( _width * _height * (sourceImage->getPixelSizeInBits()+7) / 8 ) ),
// 						   osg::Image::USE_NEW_DELETE,
//                            1 );
        freeSpace.reset ( _maximumAtlasWidth, _maximumAtlasHeight ); // Start with a rectangle of correct size
    }

    osg::ref_ptr<Rectangle> imagePlace = freeSpace.getSpace ( sourceWidth, sourceHeight );

    if (!_texture && sourceTexture)
    {
        _texture = new osg::Texture2D(_image.get());
        _texture->setTextureSize ( _maximumAtlasWidth, _maximumAtlasHeight );
        _texture->setWrap(osg::Texture2D::WRAP_S, sourceTexture->getWrap(osg::Texture2D::WRAP_S));
        _texture->setWrap(osg::Texture2D::WRAP_T, sourceTexture->getWrap(osg::Texture2D::WRAP_T));

        _texture->setBorderColor(sourceTexture->getBorderColor());
        _texture->setBorderWidth(0);

        _texture->setFilter(osg::Texture2D::MIN_FILTER, sourceTexture->getFilter(osg::Texture2D::MIN_FILTER));
        _texture->setFilter(osg::Texture2D::MAG_FILTER, sourceTexture->getFilter(osg::Texture2D::MAG_FILTER));

        _texture->setMaxAnisotropy(sourceTexture->getMaxAnisotropy());

        _texture->setInternalFormat(sourceTexture->getInternalFormat());

        _texture->setShadowCompareFunc(sourceTexture->getShadowCompareFunc());
        _texture->setShadowTextureMode(sourceTexture->getShadowTextureMode());
        _texture->setShadowAmbient(sourceTexture->getShadowAmbient());

    }

    osg::notify(osg::INFO)<<"  Source "<< getSimpleFileName ( source->_image->getFileName() )
                          << " size (" << sourceImage->s() << "," << sourceImage->t()
						  << ") fits at "<<imagePlace->x()<<","<<imagePlace->y()
                          <<" in atlas " << this->toString() << ", " << periodString << std::endl;
    _sourceList.push_back(source);
    source->_x = imagePlace->x() + sMargin;
    source->_y = imagePlace->y() + tMargin;
    source->_atlas = this;
    _width  = std::max ( unsigned(_width),  unsigned(imagePlace->x() + sourceWidth) );
    _height = std::max ( unsigned(_height), unsigned(imagePlace->y() + sourceHeight) );
    return true;
}

/**
 * \brief Forces the size of this Atlas to the minimum power of 2
 *        (in each direction) that will contain all of its Source entries.
 */
void TextureAtlasBuilder::Atlas::clampToNearestPowerOfTwoSize()
{
    unsigned int w = 1;
    while (w<_width) w *= 2;

    unsigned int h = 1;
    while (h<_height) h *= 2;

    osg::notify(osg::NOTICE)<<"  Clamping "<<_width<<", "<<_height<<" to "<<w<<","<<h<<std::endl;

    _width = w;
    _height = h;
}

/**
 * \brief Allocates storage for this Atlas and copies the Image from
 *        each Source into that area.
 */
void TextureAtlasBuilder::Atlas::copySources()
{
    osg::notify(osg::INFO) << "Atlas \"" << _image->getFileName() << "\" allocated to "<<_width << "," << _height << std::endl;

	_image->allocateImage( _width,_height,1,
                           _image->getPixelFormat(),_image->getDataType(),
                           _image->getPacking());

    _texture->setImage ( _image );

    {
        // clear memory
        unsigned int size = _image->getTotalSizeInBytes();
        unsigned char* str = _image->data();
        for(unsigned int i=0; i<size; ++i) *(str++) = 0;
    }

    for(SourceList::iterator itr = _sourceList.begin();
        itr !=_sourceList.end();
        ++itr)
    {
        Source* source = itr->get();
        Atlas* atlas = source->_atlas;

        if (atlas)
        {
            osg::notify(osg::INFO)<<"Copying image "<< getSimpleFileName ( source->_image->getFileName() ) <<" to "<<source->_x<<", "<<source->_y<<std::endl;
            osg::notify(osg::INFO)<<"        image size "<<source->_image->s()<<","<<source->_image->t()<<std::endl;

            const osg::Image* sourceImage = source->_image.get();
            osg::Image* atlasImage = atlas->_image.get();

            unsigned int rowSize = sourceImage->getRowSizeInBytes();
            unsigned int pixelSizeInBits = sourceImage->getPixelSizeInBits();
            unsigned int pixelSizeInBytes = pixelSizeInBits/8;
            unsigned int marginSizeInBytes = pixelSizeInBytes*_margin;

            unsigned int x = source->_x;
            unsigned int y = source->_y;

            int t;
            for(t=0; t<sourceImage->t(); ++t, ++y)
            {
                unsigned char* destPtr = atlasImage->data(x, y);
                const unsigned char* sourcePtr = sourceImage->data(0, t);
                for(unsigned int i=0; i<rowSize; i++)
                {
                    *(destPtr++) = *(sourcePtr++);
                }
            }

            unsigned int m;

            if ( !_wrap_t )
            {
                // copy top row margin
                y = source->_y + sourceImage->t();
                for(m=0; m<_margin; ++m, ++y)
                {
                    unsigned char* destPtr = atlasImage->data(x, y);
                    const unsigned char* sourcePtr = sourceImage->data(0, sourceImage->t()-1);
                    for(unsigned int i=0; i<rowSize; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }

                }



                // copy bottom row margin
                y = source->_y-1;
                for(m=0; m<_margin; ++m, --y)
                {
                    unsigned char* destPtr = atlasImage->data(x, y);
                    const unsigned char* sourcePtr = sourceImage->data(0, 0);
                    for(unsigned int i=0; i<rowSize; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }

                }
            }

            // copy left column margin
            if ( !_wrap_s )
            {
                y = source->_y;
                for(t=0; t<sourceImage->t(); ++t, ++y)
                {
                    x = source->_x-1;
                    for(m=0; m<_margin; ++m, --x)
                    {
                        unsigned char* destPtr = atlasImage->data(x, y);
                        const unsigned char* sourcePtr = sourceImage->data(0, t);
                        for(unsigned int i=0; i<pixelSizeInBytes; i++)
                        {
                            *(destPtr++) = *(sourcePtr++);
                        }
                    }
                }

                // copy right column margin
                y = source->_y;
                for(t=0; t<sourceImage->t(); ++t, ++y)
                {
                    x = source->_x + sourceImage->s();
                    for(m=0; m<_margin; ++m, ++x)
                    {
                        unsigned char* destPtr = atlasImage->data(x, y);
                        const unsigned char* sourcePtr = sourceImage->data(sourceImage->s()-1, t);
                        for(unsigned int i=0; i<pixelSizeInBytes; i++)
                        {
                            *(destPtr++) = *(sourcePtr++);
                        }
                    }
                }
            }

            // copy top left corner margin
            if ( !_wrap_t && !_wrap_s )
            {
                y = source->_y + sourceImage->t();
                for(m=0; m<_margin; ++m, ++y)
                {
                    unsigned char* destPtr = atlasImage->data(source->_x - _margin, y);
                    unsigned char* sourcePtr = atlasImage->data(source->_x - _margin, y-1); // copy from row below
                    for(unsigned int i=0; i<marginSizeInBytes; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }
                }

                // copy top right corner margin
                y = source->_y + sourceImage->t();
                for(m=0; m<_margin; ++m, ++y)
                {
                    unsigned char* destPtr = atlasImage->data(source->_x + sourceImage->s(), y);
                    unsigned char* sourcePtr = atlasImage->data(source->_x + sourceImage->s(), y-1); // copy from row below
                    for(unsigned int i=0; i<marginSizeInBytes; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }
                }

                // copy bottom left corner margin
                y = source->_y - 1;
                for(m=0; m<_margin; ++m, --y)
                {
                    unsigned char* destPtr = atlasImage->data(source->_x - _margin, y);
                    unsigned char* sourcePtr = atlasImage->data(source->_x - _margin, y+1); // copy from row below
                    for(unsigned int i=0; i<marginSizeInBytes; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }
                }

                // copy bottom right corner margin
                y = source->_y - 1;
                for(m=0; m<_margin; ++m, --y)
                {
                    unsigned char* destPtr = atlasImage->data(source->_x + sourceImage->s(), y);
                    unsigned char* sourcePtr = atlasImage->data(source->_x + sourceImage->s(), y+1); // copy from row below
                    for(unsigned int i=0; i<marginSizeInBytes; i++)
                    {
                        *(destPtr++) = *(sourcePtr++);
                    }
                }
            }

        }
    }
}

/**
 * \brief Determines whether a region of specified size is available
 *        within this RectangleList.
 *
 * \return      true if the space is available
 */
bool TextureAtlasBuilder::RectangleList::canFit ( unsigned int width, unsigned int height )
{
    RectangleList::const_iterator limit ( end() );
    for ( RectangleList::const_iterator i=begin(); i!= limit; i++ )
    {
        if ( (*i)->width() >= width && (*i)->height() >= height )
        {
            return true;
        }
    }
    return false;
}

/**
 * \brief Adds a specified Rectangle to this RectangleList.
 *
 * \param x0            minimum x coordinate of the Rectangle
 * \param y0            minimum y coordinate of the Rectangle
 * \param width         width of the Rectangle
 * \param height        height of the Rectangle
 */
void TextureAtlasBuilder::RectangleList::addRectangle ( unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1 )
{
    osg::ref_ptr<Rectangle> newRect ( new Rectangle ( y0, x0, y1, x1 ) );
    std::vector< osg::ref_ptr<Rectangle> >::iterator insertPoint ( lower_bound ( begin(),
                                                                                 end(),
                                                                                 newRect ) );
    insert ( insertPoint, newRect );
}

/**
 * \brief Finds a Rectangle at least as large as the specified area.
 *
 * \param width         width of area requested
 * \param height        height of area requested
 *
 * \retval              a Rectangle no smaller than (but possibly larger than)
 *                      the specified area
 * \retval              NULL if this RectangleList contains no Rectangle big enough
 */
osg::ref_ptr<TextureAtlasBuilder::Rectangle> TextureAtlasBuilder::RectangleList::getSpace ( unsigned int width, unsigned int height )
{
    osg::ref_ptr<Rectangle> result = NULL;
    for ( std::vector< osg::ref_ptr<Rectangle> >::iterator i = begin();
          i < end();
          i++ )
    {
        if ( (*i)->width() >= width && (*i)->height() >= height )
        {
            // Found a rectangle that's big enough
            result = *i;
            erase ( i );
            if ( result->height() > height )
            {
                unsigned int x0 = result->x();
                unsigned int x1 = x0 + result->width()-1;
                unsigned int y0 = result->y() + height;
                unsigned int y1 = result->y() + result->height()-1;
                push_back ( new Rectangle ( x0, y0, x1, y1 ) );
                if ( result->width() > width )
                {
                    x0 = result->x() + width;
                    x1 = result->width()-1;
                    y0 = result->y();
                    y1 = y0 + height-1;
                    push_back ( new Rectangle ( x0, y0, x1, y1 ) );
                }
            }
            else if ( result->width() > width )
            {
                unsigned int x0 = result->x() + width;
                unsigned int x1 = result->x() + result->width()-1;
                unsigned int y0 = result->y();
                unsigned int y1 = y0 + result->height()-1;
                push_back ( new Rectangle ( x0, y0, x1, y1 ) );
            }
            result->setWidth ( width );
            result->setHeight ( height );
            break;
        }
    }
    return result;
}

/**
 * \brief Finds a Rectangle at least as large as the given image plus the specified margin.
 *
 * \param image         an osg::Image
 * \param margin        the margin to add to all four sides of the image
 *
 * \retval              a Rectangle at least big enough to hold the given Image with
 *                      the specified margin
 * \retval              NULL if this RectangleList contains no Rectangle big enough
 */
osg::ref_ptr<TextureAtlasBuilder::Rectangle> TextureAtlasBuilder::RectangleList::getSpace ( const osg::Image* image, unsigned int margin )
{
    osg::ref_ptr<Rectangle> result = NULL;
    for ( std::vector< osg::ref_ptr<Rectangle> >::iterator i = begin();
          i < end();
          i++ )
    {
        if ( (*i)->canFit ( image, margin ) )
        {
            // Found a rectangle that's big enough
            result = *i;
            erase ( i );
            break;
        }
    }
    return result;
}


/** Empties the visitor and makes it ready for next traversal.*/
void TextureAtlasVisitor::reset()
{
    _statesetMap.clear();
    _statesetStack.clear();
    _textures.clear();
    _builder.reset();
}

/**
 * \brief Adds a StateSet to the current stack.
 *
 * The stack holds pointers to all the StateSets currently active;
 * it is updated during traversal.
 */
bool TextureAtlasVisitor::pushStateSet(osg::StateSet* stateset)
{
    osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();

    // if no textures, ignore this StateSet
    if (tal.empty()) return false;

    bool pushStateState = false;

    // if already in stateset list ignore
    if (_statesetMap.count(stateset)>0)
    {
        pushStateState = true;
    }
    else
    {
        bool containsTexture2D = false;
        for(unsigned int unit=0; unit<tal.size(); ++unit)
        {
            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(unit,osg::StateAttribute::TEXTURE));
            if (texture2D)
            {
                containsTexture2D = true;
                _textures.insert(std::pair<osg::Texture2D*,Extent2D>(texture2D,Extent2D()));
            }
        }

        if (containsTexture2D)
        {
            _statesetMap[stateset];
            pushStateState = true;
        }
    }

    if (pushStateState)
    {
        _statesetStack.push_back(stateset);
    }


    return pushStateState;
}

void TextureAtlasVisitor::popStateSet()
{
    _statesetStack.pop_back();
}

/**
 * \brief Processes the given Node.
 *
 * This method handles all nodes except Geode. If node has a StateSet,
 * we push it on the stack before traversing its children; otherwise,
 * we ignore it.
 */
void TextureAtlasVisitor::apply(osg::Node& node)
{
    bool pushedStateState = false;

    osg::StateSet* ss = node.getStateSet();
    if (ss && ss->getDataVariance()==osg::Object::STATIC)
    {
        if (isOperationPermissibleForObject(&node) &&
            isOperationPermissibleForObject(ss))
        {
            pushedStateState = pushStateSet(ss);
        }
    }

    traverse(node);

    if (pushedStateState) popStateSet();
}

/**
 * \brief Processes the given Geode.
 *
 * If geode has its own StateSet, we push it on the stack.  We then
 * loop through all Drawables of the Geode, adding the Drawable to the
 * _statesetMap of each StateSet that affects it.
 */
void TextureAtlasVisitor::apply(osg::Geode& geode)
{
    if (!isOperationPermissibleForObject(&geode)) return;

    osg::StateSet* ss = geode.getStateSet();


    bool pushedGeodeStateState = false;

    if (ss && ss->getDataVariance()==osg::Object::STATIC)
    {
        if (isOperationPermissibleForObject(ss))
        {
            pushedGeodeStateState = pushStateSet(ss);
        }
    }

    for(unsigned int i=0;i<geode.getNumDrawables();++i)
    {

        osg::Drawable* drawable = geode.getDrawable(i);
        if (drawable)
        {
            bool pushedDrawableStateState = false;

            ss = drawable->getStateSet();
            if (ss && ss->getDataVariance()==osg::Object::STATIC)
            {
                if (isOperationPermissibleForObject(drawable) &&
                    isOperationPermissibleForObject(ss))
                {
                    pushedDrawableStateState = pushStateSet(ss);
                }
            }

            if (!_statesetStack.empty())
            {
                for(StateSetStack::iterator ssitr = _statesetStack.begin();
                    ssitr != _statesetStack.end();
                    ++ssitr)
                {
                    _statesetMap[*ssitr].insert(drawable);
                }
            }

            if (pushedDrawableStateState) popStateSet();
        }

    }

    if (pushedGeodeStateState) popStateSet();
}

void TextureAtlasVisitor::buildWrapLists()
{
    // Loop through all StateSets
    StateSetMap::iterator sitr;
    for(sitr = _statesetMap.begin();
        sitr != _statesetMap.end();
        ++sitr)
    {
        osg::StateSet* stateset = sitr->first;
        Drawables& drawables = sitr->second;

        // Loop through all textures for this StateSet
        osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();
        for(unsigned int unit=0; unit<tal.size(); ++unit)
        {
            osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(unit,osg::StateAttribute::TEXTURE));
			if ( texture )
            {
                bool s_repeat = texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
                                texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;

                bool t_repeat = texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
                                texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

                if (s_repeat || t_repeat)
                {
                    _texturesThatRepeat.insert(texture);

                    Extent2D& range( _textures[texture] );

                    float s_eps;
                    float t_eps;
                    if ( texture->getImage() )
                    {
                        s_eps = 1.0f / (float)texture->getImage()->s(); // If texture spills over by less than one pixel, ignore it.

                        t_eps = 1.0f / (float)texture->getImage()->t(); // If texture spills over by less than one pixel, ignore it.
                    }
                    else
                    {
                        s_eps = 1.0f / (float)texture->getTextureWidth(); // If texture spills over by less than one pixel, ignore it.

                        t_eps = 1.0f / (float)texture->getTextureHeight(); // If texture spills over by less than one pixel, ignore it.
                    }

                    for(Drawables::iterator ditr = drawables.begin();
                        ditr != drawables.end();
                        ++ditr)
                    {
                        osg::Geometry* geom = (*ditr)->asGeometry();
                        osg::Vec2Array* texcoords = geom ? dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit)) : 0;
                        if (texcoords && !texcoords->empty())
                        {
                            for(osg::Vec2Array::iterator titr = texcoords->begin();
                                titr != texcoords->end();
                                ++titr)
                            {
                                osg::Vec2 tc = *titr;
                                range.include ( tc );
                            }
//                             if ( !s_outOfRange && !t_outOfRange )
//                             {
//                                 // We might have a drawable that crosses no texture boundaries, but isn't within the unit square.
//                                 float sOffset = floor ( range.minS + s_eps );
//                                 float tOffset = floor ( range.minT + t_eps );

//                                 for(osg::Vec2Array::iterator titr = texcoords->begin();
//                                     titr != texcoords->end() &&  ( !s_outOfRange || !t_outOfRange );
//                                     ++titr)
//                                 {
//                                     (*titr)[0] -= sOffset;
//                                     (*titr)[1] -= tOffset;
//                                 }
//                             }

                            float nWraps;
                            if ( range.maxS - range.minS > s_eps )
                            {
                                nWraps = floor ( range.minS + s_eps );
                                if ( floor ( range.maxS - s_eps ) > nWraps )
                                {
                                    _texturesThatRepeatAndSpanS.insert(texture); //!< \todo If range is within span, but not in (0,1), must adjust
                                }
                            }

                            if ( range.maxT - range.minT > t_eps )
                            {
                                nWraps = floor ( range.minT + t_eps );
                                if ( floor ( range.maxT - t_eps ) > nWraps )
                                {
                                    _texturesThatRepeatAndSpanT.insert(texture); //!< \todo If range is within span, but not in (0,1), must adjust
                                }
                            }
                        }
                        else
                        {
                            // if no texcoords then texgen must be
                            // used, so we must assume that texture is
                            // really repeating
                            _texturesThatRepeatAndSpanS.insert(texture);
                            _texturesThatRepeatAndSpanT.insert(texture);
                        }
                    }
                }
            }
        }
    }
}

/**
 * \brief Switches off texture wrap for textures that don't need it.
 *
 * If the mode of a texture is wrap or mirror, but the texture
 * coordinates don't actually cross an integer boundary, we can set
 * the mode to CLAMP mode, allowing that texture to be used in an
 * atlas.
 */
void TextureAtlasVisitor::minimizeWrapLists()
{
    Textures::iterator titr;
    for(titr = _texturesThatRepeat.begin();
        titr != _texturesThatRepeat.end();
        ++titr)
    {
        osg::Texture2D* texture = *titr;
        if ( _texturesThatRepeatAndSpanT.count(texture) == 0 )
        {
            // Safe to switch off T wrap
            texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::CLAMP);
            Extent2D& stRange ( _textures[texture] );
            if ( _texturesThatRepeatAndSpanS.count(texture) == 0 )
            {
                // Also safe to switch off S wrap
                texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP);
                osg::notify(osg::INFO) << "Clamp all for file " << getSimpleFileName ( texture->getImage()->getFileName() );
                osg::notify(osg::INFO) << "\tS\t" << stRange.minS << "\t" << stRange.maxS
                                       << "\tT\t" << stRange.minT << "\t" << stRange.maxT << std::endl;
            }
            else
            {
                osg::notify(osg::INFO) <<  "Clamp T   for file " << getSimpleFileName ( texture->getImage()->getFileName() );
                osg::notify(osg::INFO) << "\tS\t" << stRange.minS << "\t" << stRange.maxS
                                       << "\tT\t" << stRange.minT << "\t" << stRange.maxT << std::endl;
            }
        }
        else
        {
            if ( _texturesThatRepeatAndSpanS.count(texture) == 0 )
            {
                // Safe to switch off S wrap
                texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::CLAMP);
                Extent2D& stRange ( _textures[texture] );
                osg::notify(osg::INFO) << "Clamp S   for file " << getSimpleFileName ( texture->getImage()->getFileName() );
                osg::notify(osg::INFO) << "\tS\t" << stRange.minS << "\t" << stRange.maxS
                                       << "\tT\t" << stRange.minT << "\t" << stRange.maxT << std::endl;
            }
            else if ( texture->getWrap(osg::Texture2D::WRAP_S)!=osg::Texture::REPEAT &&
                      texture->getWrap(osg::Texture2D::WRAP_S)!=osg::Texture::MIRROR )
            {
                osg::notify(osg::WARN) << "Warning:: setting S wrap ON for texture file " << getSimpleFileName ( texture->getImage()->getFileName() ) << std::endl;
                texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture::REPEAT);
            }
            if ( texture->getWrap(osg::Texture2D::WRAP_T)!=osg::Texture::REPEAT &&
                 texture->getWrap(osg::Texture2D::WRAP_T)!=osg::Texture::MIRROR )
            {
                osg::notify(osg::WARN) << "Warning:: setting T wrap ON for texture file " << getSimpleFileName ( texture->getImage()->getFileName() ) << std::endl;
                texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture::REPEAT);
            }
        }
    }
}

/**
 * \brief Builds texture atlases from the sets built by buildWrapLists().
 *
 * buildWrapLists() fills three sets: _texturesThatRepeat,
 * _texturesThatRepeatAndSpanS, and _texturesThatRepeatAndSpanT. From
 * these, we build as many atlases as needed, since
 *
 * - textures with different channel content (RGB, RGBA, etc.) can't
 *   share an atlas
 * - textures with different compression modes can't share an atlas, and
 * - textures with different wrap modes can't share an atlas, their
 *   atlases will be structured differently, and they must match in
 *   the direction of periodicity in order to share an atlas.
 */
void TextureAtlasVisitor::buildTextureAtlases()
{
    int nSWrap = 0;
    int nTWrap = 0;
    int nSTWrap = 0;
    // add the textures as sources for the TextureAtlasBuilder
    TextureExtentMap::iterator allTexIt;
    for(allTexIt = _textures.begin();
        allTexIt != _textures.end();
        ++allTexIt)
    {
        osg::Texture2D* texture = allTexIt->first;

        bool s_repeat = texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
                        texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;

        bool t_repeat = texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
                        texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

        if (!s_repeat || !t_repeat)
        {
            // This will capture both nonperiodic textures and those
            // periodic in one direction only.
            _builder.addSource(allTexIt->first);
        }
        else
        {
            // Texture is periodic in both directions
            const std::string& fileName ( getSimpleFileName ( texture->getImage()->getFileName() ) );
            osg::notify(osg::INFO) << "Double periodic texture excluded from texture atlas: "
                                   << fileName << std::endl;
            nSTWrap++;
        }
    }
    osg::notify(osg::NOTICE) << "Textures not in atlases: " << nSWrap << " S " << nTWrap << " T " << nSTWrap << " both." << std::endl << std::endl;

    // build the atlases
    _builder.buildAtlas();
}

/**
 * \brief Remaps texture coordinates from original texture to texture
 *        atlas as applicable.
 *
 * After building the atlases, the original texture coordinates must
 * be revised to point to the relevant subarea of a texture atlas,
 * rather than spanning the entire original texture.
 */
void TextureAtlasVisitor::remapTexturesToAtlases ( const Drawables& drawablesThatHaveMultipleTexturesOnOneUnit )
{
    // Remap the textures in the StateSets
    for(StateSetMap::iterator sitr = _statesetMap.begin();
        sitr != _statesetMap.end();
        ++sitr)
    {
        osg::StateSet* stateset = sitr->first;
        osg::StateSet::TextureAttributeList& tal = stateset->getTextureAttributeList();
        for(unsigned int unit=0; unit<tal.size(); ++unit)
        {
            osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(stateset->getTextureAttribute(unit,osg::StateAttribute::TEXTURE));
            if (texture)
            {
//                bool s_repeat = texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::REPEAT ||
//                                texture->getWrap(osg::Texture2D::WRAP_S)==osg::Texture2D::MIRROR;
//
//                bool t_repeat = texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::REPEAT ||
//                                texture->getWrap(osg::Texture2D::WRAP_T)==osg::Texture2D::MIRROR;

                osg::Texture2D* newTexture = _builder.getTextureAtlas(texture);
                if (newTexture && newTexture!=texture)
                {
//                     if (s_repeat || t_repeat)
//                     {
//                         osg::notify(osg::NOTICE)<<"Warning!!! shouldn't get here"<<std::endl;
//                     }

                    stateset->setTextureAttribute(unit, newTexture);

                    Drawables& drawables = sitr->second;

                    osg::Matrix matrix = _builder.getTextureMatrix(texture);

                    // first check to see if all drawables are ok for applying texturematrix to.
                    bool canTexMatBeFlattenedToAllDrawables = true;
                    for(Drawables::iterator ditr = drawables.begin();
                        ditr != drawables.end() && canTexMatBeFlattenedToAllDrawables;
                        ++ditr)
                    {
                        osg::Geometry* geom = (*ditr)->asGeometry();
                        osg::Vec2Array* texcoords = geom ? dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit)) : 0;

                        if (!texcoords)
                        {
                            canTexMatBeFlattenedToAllDrawables = false;
                        }

                        if (drawablesThatHaveMultipleTexturesOnOneUnit.count(*ditr)!=0)
                        {
                            canTexMatBeFlattenedToAllDrawables = false;
                        }
                    }

                    if (canTexMatBeFlattenedToAllDrawables)
                    {
                        // osg::notify(osg::NOTICE)<<"All drawables can be flattened "<<drawables.size()<<std::endl;
                        for(Drawables::iterator ditr = drawables.begin();
                            ditr != drawables.end();
                            ++ditr)
                        {
                            osg::Geometry* geom = (*ditr)->asGeometry();
                            osg::Vec2Array* texcoords = geom ? dynamic_cast<osg::Vec2Array*>(geom->getTexCoordArray(unit)) : 0;
                            if (texcoords)
                            {
                                for(osg::Vec2Array::iterator titr = texcoords->begin();
                                    titr != texcoords->end();
                                    ++titr)
                                {
                                    osg::Vec2 tc = *titr;
                                    (*titr).set(tc[0]*matrix(0,0) + tc[1]*matrix(1,0) + matrix(3,0),
                                              tc[0]*matrix(0,1) + tc[1]*matrix(1,1) + matrix(3,1));
                                }
                            }
                            else
                            {
                                osg::notify(osg::NOTICE)<<"Error, TextureAtlasVisitor::optimize() shouldn't ever get here..."<<std::endl;
                            }
                        }
                    }
                    else
                    {
                        // osg::notify(osg::NOTICE)<<"Applying TexMat "<<drawables.size()<<std::endl;
                        stateset->setTextureAttribute(unit, new osg::TexMat(matrix));
                    }
                }
            }
        }

    }
}

void TextureAtlasVisitor::optimize()
{
    _builder.reset();
    _texturesThatRepeat.clear();
    _texturesThatRepeatAndSpanS.clear();
    _texturesThatRepeatAndSpanT.clear();

    if (_textures.size()<2)
    {
        // nothing to optimize
        return;
    }


    buildWrapLists();                        // Find which textures wrap and which don't.

    minimizeWrapLists();                     // If wrap flag is on, but coordinates don't cross a boundary, turn the flag off.

    buildTextureAtlases();

    // Deal with Drawables that have more than one texture mapped.

    typedef std::set<osg::StateSet*> StateSetSet;
    typedef std::map<osg::Drawable*, StateSetSet> DrawableStateSetMap;
    DrawableStateSetMap dssm;
    for(StateSetMap::iterator sitr = _statesetMap.begin();
        sitr != _statesetMap.end();
        ++sitr)
    {
        Drawables& drawables = sitr->second;
        for(Drawables::iterator ditr = drawables.begin();
            ditr != drawables.end();
            ++ditr)
        {
            dssm[(*ditr)->asGeometry()].insert(sitr->first);
        }
    }

    Drawables drawablesThatHaveMultipleTexturesOnOneUnit;
    for(DrawableStateSetMap::iterator ditr = dssm.begin();
        ditr != dssm.end();
        ++ditr)
    {
        osg::Drawable* drawable = ditr->first;
        StateSetSet& ssm = ditr->second;
        if (ssm.size()>1)
        {
            typedef std::map<unsigned int, Textures> UnitTextureMap;
            UnitTextureMap unitTextureMap;
            for(StateSetSet::iterator ssm_itr = ssm.begin();
                ssm_itr != ssm.end();
                ++ssm_itr)
            {
                osg::StateSet* ss = *ssm_itr;
                unsigned int numTextureUnits = ss->getTextureAttributeList().size();
                for(unsigned int unit=0; unit<numTextureUnits; ++unit)
                {
                    osg::Texture2D* texture = dynamic_cast<osg::Texture2D*>(ss->getTextureAttribute(unit, osg::StateAttribute::TEXTURE));
                    if (texture) unitTextureMap[unit].insert(texture);
                }
            }
            bool drawablesHasMultiTextureOnOneUnit = false;
            for(UnitTextureMap::iterator utm_itr = unitTextureMap.begin();
                utm_itr != unitTextureMap.end() && !drawablesHasMultiTextureOnOneUnit;
                ++utm_itr)
            {
                if (utm_itr->second.size()>1)
                {
                    drawablesHasMultiTextureOnOneUnit = true;
                }
            }
            if (drawablesHasMultiTextureOnOneUnit)
            {
                drawablesThatHaveMultipleTexturesOnOneUnit.insert(drawable);
            }

        }
    }

    remapTexturesToAtlases ( drawablesThatHaveMultipleTexturesOnOneUnit );
}
