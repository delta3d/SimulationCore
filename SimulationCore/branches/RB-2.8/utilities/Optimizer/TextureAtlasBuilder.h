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

// #ifndef OSGUTIL_OPTIMIZER
// #define OSGUTIL_OPTIMIZER

#include <osg/NodeVisitor>
#include <osg/Matrix>
#include <osg/Geometry>
#include <osg/Transform>
#include <osg/Texture2D>

#include <osgUtil/Export>
#define osgutil_export

#include <set>

// namespace osgUtil {

// forward declare
namespace osgUtil
{
    class Optimizer;
}

/** Helper base class for implementing Optimizer techniques.*/
class osgutil_export BaseOptimizerVisitor : public osg::NodeVisitor
{
    public:

        BaseOptimizerVisitor(osgUtil::Optimizer* optimizer, unsigned int operation):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _optimizer(optimizer),
            _operationType(operation)
        {
            setNodeMaskOverride(0xffffffff);
        }
        virtual ~BaseOptimizerVisitor() {}

        inline bool isOperationPermissibleForObject(const osg::StateSet* object) const;
        inline bool isOperationPermissibleForObject(const osg::StateAttribute* object) const;
        inline bool isOperationPermissibleForObject(const osg::Drawable* object) const;
        inline bool isOperationPermissibleForObject(const osg::Node* object) const;

    protected:

        osgUtil::Optimizer*      _optimizer;
        unsigned int _operationType;
};

        enum OptimizationOptions
            {
                FLATTEN_STATIC_TRANSFORMS = (1 << 0),
                REMOVE_REDUNDANT_NODES =    (1 << 1),
                REMOVE_LOADED_PROXY_NODES = (1 << 2),
                COMBINE_ADJACENT_LODS =     (1 << 3),
                SHARE_DUPLICATE_STATE =     (1 << 4),
                MERGE_GEOMETRY =            (1 << 5),
                CHECK_GEOMETRY =            (1 << 6),
                SPATIALIZE_GROUPS =         (1 << 7),
                COPY_SHARED_NODES =         (1 << 8),
                TRISTRIP_GEOMETRY =         (1 << 9),
                TESSELLATE_GEOMETRY =       (1 << 10),
                OPTIMIZE_TEXTURE_SETTINGS = (1 << 11),
                MERGE_GEODES =              (1 << 12),
                FLATTEN_BILLBOARDS =        (1 << 13),
                TEXTURE_ATLAS_BUILDER =     (1 << 14),
                STATIC_OBJECT_DETECTION =   (1 << 15),
                FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS = (1 << 16),
                DEFAULT_OPTIMIZATIONS = FLATTEN_STATIC_TRANSFORMS |
                REMOVE_REDUNDANT_NODES |
                REMOVE_LOADED_PROXY_NODES |
                COMBINE_ADJACENT_LODS |
                SHARE_DUPLICATE_STATE |
                MERGE_GEOMETRY |
                CHECK_GEOMETRY |
                OPTIMIZE_TEXTURE_SETTINGS |
                STATIC_OBJECT_DETECTION,
                ALL_OPTIMIZATIONS = FLATTEN_STATIC_TRANSFORMS_DUPLICATING_SHARED_SUBGRAPHS |
                REMOVE_REDUNDANT_NODES |
                REMOVE_LOADED_PROXY_NODES |
                COMBINE_ADJACENT_LODS |
                SHARE_DUPLICATE_STATE |
                MERGE_GEODES |
                MERGE_GEOMETRY |
                CHECK_GEOMETRY |
                SPATIALIZE_GROUPS |
                COPY_SHARED_NODES |
                TRISTRIP_GEOMETRY |
                OPTIMIZE_TEXTURE_SETTINGS |
                TEXTURE_ATLAS_BUILDER |
                STATIC_OBJECT_DETECTION
            };


/**
 * TextureAtlasBuilder creates a set of textures/images which each contain multiple images.
 * Texture Atlases are used to make it possible to use much wider batching of data.
 */
class osgutil_export TextureAtlasBuilder
{
 public:
    TextureAtlasBuilder();
    virtual ~TextureAtlasBuilder() {}

    void reset();

    void setMaximumAtlasSize(unsigned int width, unsigned int height);

    unsigned int getMaximumAtlasWidth() const { return _maximumAtlasWidth; }
    unsigned int getMaximumAtlasHeight() const { return _maximumAtlasHeight; }

    void setMargin(unsigned int margin);
    unsigned int getMargin() const { return _margin; }

    void addSource(const osg::Image* image);
    void addSource(const osg::Texture2D* texture);

    unsigned int getNumSources() const { return _sourceList.size(); }
    const osg::Image* getSourceImage(unsigned int i) { return _sourceList[i]->_image.get(); }
    const osg::Texture2D* getSourceTexture(unsigned int i) { return _sourceList[i]->_texture.get(); }

    void buildAtlas();

    osg::Image* getImageAtlas(unsigned int i);
    osg::Texture2D* getTextureAtlas(unsigned int i);
    osg::Matrix getTextureMatrix(unsigned int i);

    osg::Image* getImageAtlas(const osg::Image* image);
    osg::Texture2D* getTextureAtlas(const osg::Image* image);
    osg::Matrix getTextureMatrix(const osg::Image* image);

    osg::Image* getImageAtlas(const osg::Texture2D* textue);
    osg::Texture2D* getTextureAtlas(const osg::Texture2D* texture);
    osg::Matrix getTextureMatrix(const osg::Texture2D* texture);

 protected:

    unsigned int _maximumAtlasWidth;
    unsigned int _maximumAtlasHeight;
    unsigned int _margin;

    /**
     * Represents a rectangular area within a texture atlas.
     *
     * Free space within an atlas can be managed as a list,
     * vector, or set of Rectangles.
     */
    class Rectangle : public osg::Referenced
    {
    public:
        Rectangle():
            _minX(0), _minY(0), _maxX(0), _maxY(0) {}

        Rectangle( int x0, int y0, int x1, int y1 ):
            _minX(x0), _minY(y0), _maxX(x1), _maxY(y1) {}

        Rectangle( const osg::Image* image, int x0, int y0 ):
            _minX(x0), _minY(y0), _maxX(x0+image->s()-1), _maxY(y0+image->t()-1) {}

        unsigned int width()  const { return _maxX - _minX + 1; }
        unsigned int height() const { return _maxY - _minY + 1; }
        unsigned int x()      const { return _minX; }
        unsigned int y()      const { return _minY; }
        void setWidth ( unsigned int w )  { _maxX = _minX + w -1; }
        void setHeight ( unsigned int h ) { _maxY = _minY + h -1; }

        bool canFit ( const osg::Image* image, unsigned int margin=0 ) const
        { return ( image->s()+2*margin <= width() && image->t()+2*margin <= height() ); }

        bool operator< ( const Rectangle& other ) const
        {
            return ( width() * height() < other.width() * other.height() ); // Sort with smallest first
        }

    protected:
        unsigned int _minX;          //!< Index of first pixel occupied by the Rectangle
        unsigned int _minY;          //!< Index of last pixel occupied by the Rectangle; the next should start with _minX = this->_maxX + 1.
        unsigned int _maxX;          //!< Index of first scan line occupied by the Rectangle
        unsigned int _maxY;          //!< Index of last scan line occupied by the Rectangle; the next should start with _minY = this->_maxY + 1.
    };

    class RectangleList : public std::vector< osg::ref_ptr<Rectangle> >
    {
    public:
        RectangleList() : std::vector< osg::ref_ptr<Rectangle> >() {}

        /** Creates a RectangleList with a single member of specified size and
         * minimum coordinates (0,0).
         *
         * \param width         width of area
         * \param height        height of area
         */
        RectangleList ( unsigned int width, unsigned int height ) : std::vector< osg::ref_ptr<Rectangle> >()
        {
            push_back ( new Rectangle ( 0, 0, width-1, height-1 ) );
        }

        virtual ~RectangleList() {
            for ( std::vector< osg::ref_ptr<Rectangle> >::iterator i = begin();
                  i < end();
                  i++ )
            {
                *i = NULL;
            }
        }

        bool canFit ( unsigned int width, unsigned int height );
        void addRectangle ( unsigned int x0, unsigned int y0, unsigned int width, unsigned int height );
        osg::ref_ptr<Rectangle> getSpace ( unsigned int width, unsigned int height );
        osg::ref_ptr<Rectangle> getSpace ( const osg::Image* image, unsigned int margin=0 );

        /**
         * Changes the shape covered by a RectangleList.
         *
         * The RectangleList is presumed to be in its initial state:
         * at most one rectangle.
         *
         * \param width         width of area to be covered after reset
         * \param height        height of area to be covered after reset
         */
        void reset ( unsigned int width, unsigned int height )
        {
            if ( size() > 1 )
            {
                throw std::exception();
            }
            if ( !empty() )
            {
                pop_back();
            }
            push_back ( new Rectangle ( 0, 0, width-1, height-1 ) );
        }
    };

    // forward declare
    class Atlas;

    class Source : public osg::Referenced
    {
    public:
        Source():
            _x(0),_y(0),_atlas(0),_isAtlas(false) {}

        Source(const osg::Image* image):
            _x(0),_y(0),_atlas(0),_image(image),_isAtlas(false) {}

        Source(const osg::Texture2D* texture):
            _x(0),_y(0),_atlas(0),_texture(texture),_isAtlas(false) { if (texture) _image = texture->getImage(); }

        unsigned int _x;
        unsigned int _y;
        Atlas* _atlas;
        bool   _isAtlas;

        osg::ref_ptr<const osg::Image> _image;
        osg::ref_ptr<const osg::Texture2D> _texture;

        bool suitableForAtlas(unsigned int maximumAtlasWidth, unsigned int maximumAtlasHeight, unsigned int margin);
        osg::Matrix computeTextureMatrix() const;

        bool operator< ( const Source& other ) const
        {
            return ( _image->t() < other._image->t() ); // Sort by height to pack rows better
        }

    protected:

        virtual ~Source() {}
    };

    typedef std::vector< osg::ref_ptr<Source> > SourceList;

    class Atlas : public osg::Referenced
    {
    public:
        Atlas(unsigned int width, unsigned height, unsigned margin):
            _maximumAtlasWidth(width),
            _maximumAtlasHeight(height),
            _margin(margin),
            _x(0),
            _y(0),
            _width(0),
            _height(0),
			_wrap_s(osg::Texture::CLAMP),
            _wrap_t(osg::Texture::CLAMP),
            freeSpace(width,height){}

        unsigned int _maximumAtlasWidth;
        unsigned int _maximumAtlasHeight;
        unsigned int _margin;

        osg::ref_ptr<osg::Texture2D> _texture;
        osg::ref_ptr<osg::Image> _image;

        SourceList _sourceList;

        RectangleList freeSpace;
        unsigned int _x;
        unsigned int _y;
        unsigned int _width;
        unsigned int _height;
        osg::Texture::WrapMode _wrap_s;
        osg::Texture::WrapMode _wrap_t;
        osg::Texture::FilterMode _min_filter;
        osg::Texture::FilterMode _mag_filter;
        osg::Texture::InternalFormatMode _int_format_mode;
        osg::Texture::InternalFormatType _int_format_type;
        int _int_format;
        bool _shadow_comparison;
        osg::Texture::ShadowCompareFunc _shadow_compare_func;
        osg::Texture::ShadowTextureMode _shadow_texture_mode;
        float _shadow_ambient;
        float _max_anisotropy;

        bool doesSourceFit(Source* source);
        std::string toString() const;
        bool addSource(Source* source);
        void clampToNearestPowerOfTwoSize();
        void copySources();

    protected:

        virtual ~Atlas() {}
    };

    typedef std::vector< osg::ref_ptr<Atlas> > AtlasList;

    Source* getSource(const osg::Image* image);
    Source* getSource(const osg::Texture2D* texture);

    SourceList _sourceList;
    AtlasList _atlasList;
};


/**
 * Combines textures into one or more texture atlases.
 *
 * Switching between textures is an expensive hardware
 * operation, so there is much to be gained by combining many
 * textures into one large texture, called an atlas.  Each
 * Drawable is revised to use a small portion of the atlas
 * rather than its own texture, so more geometry can be
 * batched together and rendered without state changes.
 */
class osgutil_export TextureAtlasVisitor : public BaseOptimizerVisitor
{
 public:

    /// default to traversing all children.
    TextureAtlasVisitor(osgUtil::Optimizer* optimizer=0):
        BaseOptimizerVisitor(optimizer, TEXTURE_ATLAS_BUILDER) {}
    ~TextureAtlasVisitor() {}


    TextureAtlasBuilder& getTextureAtlasBuilder() { return _builder; }

    virtual void reset();

    virtual void apply(osg::Node& node);

    virtual void apply(osg::Geode& geode);

    void optimize();

 protected:

    class Extent2D
    {
    public:
        Extent2D() : minS(FLT_MAX), maxS(-FLT_MAX), minT(FLT_MAX), maxT(-FLT_MAX) {}

        void include ( float s, float t )
        {
            minS = std::min ( minS, s );
            maxS = std::max ( maxS, s );
            minT = std::min ( minT, t );
            maxT = std::max ( maxT, t );
        }

        void include ( osg::Vec2 st ) { include ( st.x(), st.y() ); }

        float minS, maxS;
        float minT, maxT;
    };

    typedef std::set<osg::Drawable*>  Drawables;
    typedef std::map<osg::StateSet*, Drawables>  StateSetMap;
    typedef std::map<osg::Texture2D*, Extent2D> TextureExtentMap;
    typedef std::set<osg::Texture2D*>  Textures;
    typedef std::vector<osg::StateSet*>  StateSetStack;

    bool pushStateSet(osg::StateSet* stateset);
    void popStateSet();
    void buildWrapLists();
    void minimizeWrapLists();
    void buildTextureAtlases();
    void remapTexturesToAtlases ( const Drawables& drawablesThatHaveMultipleTexturesOnOneUnit );

    TextureAtlasBuilder _builder;

    StateSetMap     _statesetMap;
    StateSetStack   _statesetStack;

    Textures _texturesThatRepeat;
    Textures _texturesThatRepeatAndSpanS;
    Textures _texturesThatRepeatAndSpanT;
    TextureExtentMap        _textures;

};

inline bool BaseOptimizerVisitor::isOperationPermissibleForObject(const osg::StateSet* object) const
{
    return _optimizer ? _optimizer->isOperationPermissibleForObject(object,_operationType) :  true; 
}

inline bool BaseOptimizerVisitor::isOperationPermissibleForObject(const osg::StateAttribute* object) const
{
    return _optimizer ? _optimizer->isOperationPermissibleForObject(object,_operationType) :  true; 
}

inline bool BaseOptimizerVisitor::isOperationPermissibleForObject(const osg::Drawable* object) const
{
    return _optimizer ? _optimizer->isOperationPermissibleForObject(object,_operationType) :  true; 
}

inline bool BaseOptimizerVisitor::isOperationPermissibleForObject(const osg::Node* object) const
{
    return _optimizer ? _optimizer->isOperationPermissibleForObject(object,_operationType) :  true; 
}

// }

// #endif

// Local Variables: **
// mode: C++ **
// c-basic-offset: 4 **
// indent-tabs-mode: nil **
// End: **
