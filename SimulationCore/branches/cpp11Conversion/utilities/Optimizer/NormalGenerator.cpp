#include "NormalGenerator.h"

#include <osg/Notify>
#include <osg/io_utils>
#include <iostream>
#include <cmath>

namespace LevelCompiler
{

NormalGenerator::NormalGenerator()
:    std::enable_shared_from_this(),
    N_(new osg::Vec3Array)
{
}

NormalGenerator::NormalGenerator(const NormalGenerator &copy, const osg::CopyOp &copyop)
:    std::enable_shared_from_this(copy),
    N_(static_cast<osg::Vec3Array *>(copyop(copy.N_.get())))
{
}

bool NormalGenerator::ValidateNormals(osg::Geometry* geo)
{
   const osg::Array* vx = geo->getVertexArray();
   const osg::Array* nx = geo->getNormalArray();

   if(vx != nullptr && nx != nullptr)
   {
      bool validNormals = false;
      
      if(vx->getNumElements() == nx->getNumElements())
      { 
         validNormals = verify(nx);
      }

      if(!validNormals)
      {
         std::cout << "Error, found invalid normals for geometry with name '" << geo->getName() << "'." << std::endl;          
      }

      return validNormals;
   }

   return false;
}

void NormalGenerator::Generate(osg::Geometry* geo)
{
    // check to see if vertex attributes indices exists, if so expand them to remove them
    if (geo->suitableForOptimization())
    {
        // removing coord indices so we don't have to deal with them in the binormal code.
        osg::notify(osg::INFO)<<"NormalGenerator::generate(Geometry*,int): Removing attribute indices"<<std::endl;
        geo->copyToAndOptimize(*geo);
    }

    const osg::Array* vx = geo->getVertexArray();
    //const osg::Array* nx = geo->getNormalArray();

    if (!vx) return;

    unsigned int vertex_count = vx->getNumElements();
    if (geo->getVertexIndices() == nullptr) 
    {
        N_->assign(vertex_count, osg::Vec3());
    }
    else 
    {
        unsigned int index_count = geo->getVertexIndices()->getNumElements();
        N_->assign(index_count, osg::Vec3());
        indices_ = new osg::UIntArray();
        unsigned int i;
        for (i=0;i<index_count;i++) 
        {
            indices_->push_back(i);
        }
    }

    unsigned int i; // VC6 doesn't like for-scoped variables

    for (unsigned int pri = 0; pri < geo->getNumPrimitiveSets(); ++pri) 
    {
        osg::PrimitiveSet *pset = geo->getPrimitiveSet(pri);

        unsigned int N = pset->getNumIndices();

        switch (pset->getMode()) 
        {

            case osg::PrimitiveSet::TRIANGLES:
                for (i=0; i<N; i+=3) 
                {
                    compute(pset, vx, i, i+1, i+2);
                }
                break;

            case osg::PrimitiveSet::QUADS:
                for (i=0; i<N; i+=4) 
                {
                    compute(pset, vx, i, i+1, i+2);
                    compute(pset, vx, i+2, i+3, i);
                }
                break;

            case osg::PrimitiveSet::TRIANGLE_STRIP:
                if (pset->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType) 
                {
                    osg::DrawArrayLengths *dal = static_cast<osg::DrawArrayLengths *>(pset);
                    unsigned int j = 0;
                    for (osg::DrawArrayLengths::const_iterator pi=dal->begin(); pi!=dal->end(); ++pi) 
                    {
                        unsigned int iN = static_cast<unsigned int>(*pi-2);
                        for (i=0; i<iN; ++i, ++j) 
                        {
                            if ((i%2) == 0) 
                            {
                                compute(pset, vx, j, j+1, j+2);
                            }
                            else 
                            {
                                compute(pset, vx, j+1, j, j+2);
                            }
                        }
                        j += 2;
                    }
                } 
                else 
                {
                    for (i=0; i<N-2; ++i) 
                    {
                        if ((i%2) == 0) 
                        {
                            compute(pset, vx, i, i+1, i+2);                            
                        }
                        else 
                        {
                            compute(pset, vx, i+1, i, i+2);
                        }
                    }
                }
                break;

            case osg::PrimitiveSet::TRIANGLE_FAN:
                if (pset->getType() == osg::PrimitiveSet::DrawArrayLengthsPrimitiveType) 
                {
                    osg::DrawArrayLengths *dal = static_cast<osg::DrawArrayLengths *>(pset);
                    unsigned int j = 0;
                    for (osg::DrawArrayLengths::const_iterator pi=dal->begin(); pi!=dal->end(); ++pi) 
                    {
                        unsigned int iN = static_cast<unsigned int>(*pi-2);
                        for (i=0; i<iN; ++i) 
                        {
                            compute(pset, vx, 0, j+1, j+2);
                        }
                        j += 2;
                    }
                } 
                else 
                {
                    for (i=0; i<N-2; ++i) 
                    {
                        compute(pset, vx, 0, i+1, i+2);
                    }
                }
                break;

            case osg::PrimitiveSet::POINTS:
            case osg::PrimitiveSet::LINES:
            case osg::PrimitiveSet::LINE_STRIP:
            case osg::PrimitiveSet::LINE_LOOP:
                break;

            default: osg::notify(osg::WARN) << "Warning: NormalGenerator: unknown primitive mode " << pset->getMode() << "\n";
        }
    }

    /* TO-DO: if indexed, compress the attributes to have only one
     * version of each (different indices for each one?) */

    //now normalize each normal
   int numNormals = N_->size();
   for(int vertexCount = 0; vertexCount < numNormals; ++vertexCount)
   {
      (*N_)[vertexCount].normalize();
   }
}

void NormalGenerator::compute(osg::PrimitiveSet *pset, const osg::Array* vx, int iA, int iB, int iC)
{
    iA = pset->index(iA);
    iB = pset->index(iB);
    iC = pset->index(iC);

    osg::Vec3 P1;
    osg::Vec3 P2;
    osg::Vec3 P3;

    int i; // VC6 doesn't like for-scoped variables

    switch (vx->getType())
    {
    case osg::Array::Vec2ArrayType:
        for (i=0; i<2; ++i) 
        {
            P1.ptr()[i] = static_cast<const osg::Vec2Array&>(*vx)[iA].ptr()[i];
            P2.ptr()[i] = static_cast<const osg::Vec2Array&>(*vx)[iB].ptr()[i];
            P3.ptr()[i] = static_cast<const osg::Vec2Array&>(*vx)[iC].ptr()[i];
        }
        break;

    case osg::Array::Vec3ArrayType:
        P1 = static_cast<const osg::Vec3Array&>(*vx)[iA];
        P2 = static_cast<const osg::Vec3Array&>(*vx)[iB];
        P3 = static_cast<const osg::Vec3Array&>(*vx)[iC];
        break;

    //case osg::Array::Vec4ArrayType:
        //for (i=0; i<3; ++i) 
        //{
        //    P1.ptr()[i] = static_cast<const osg::Vec3Array&>(*vx)[iA].ptr()[i];
        //    P2.ptr()[i] = static_cast<const osg::Vec3Array&>(*vx)[iB].ptr()[i];
        //    P3.ptr()[i] = static_cast<const osg::Vec3Array&>(*vx)[iC].ptr()[i];
        //}
        //break;

    default:
        osg::notify(osg::WARN) << "Warning: NormalGenerator: vertex array must be Vec2Array or Vec3Array" << std::endl;
    }

   
     osg::Vec3 face_normal = (P2 - P1) ^ (P3 - P1);

     face_normal.normalize();

     //accumulate all the normals to average them
     (*N_)[iA] += face_normal;
     (*N_)[iB] += face_normal;
     (*N_)[iC] += face_normal;
    
}

bool NormalGenerator::verify(const osg::Array* pArray)
{
   const osg::Vec3Array* normArray = dynamic_cast<const osg::Vec3Array*>(pArray);
   if(normArray != nullptr)
   {
      unsigned count = normArray->size();
      
      for(unsigned i = normArray->size(); i < count; ++i)
      {
         const osg::Vec3& norm = (*normArray)[i];
         if(!verify(norm))
         {
            return false;
         }
      }
   
      return true;
   }

   return false;
}


template <typename VecType>
bool IsFiniteVec(const VecType value)
{
   for (size_t i = 0; i < VecType::num_components; ++i)
   {
#if defined (WIN32) || defined (_WIN32) || defined (__WIN32__)
      if (!_finite(value[i]))
      {
         return false;
      }
#else
      if (!std::isfinite(value[i]))
      {
         return false;
      }
#endif
   }

   return true;
}

bool NormalGenerator::verify( const osg::Vec3& norm )
{
   bool result = IsFiniteVec(norm);
   if(result)
   {
      float length = norm.length();
      if(length > 1.00001f || length < 0.99999f)
      {
         result = false;
      }
   }
   return result;
}

}//namespace LevelCompiler
