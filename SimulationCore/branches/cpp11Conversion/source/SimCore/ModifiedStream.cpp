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

// ===============================================================================
//                     PHYSX SDK TRAINING PROGRAMS
//                            USER STREAM
//
//                Originally Written by Bob Schade, 10-15-05
// ===============================================================================
#include <prefix/SimCorePrefix.h>
#ifdef AGEIA_PHYSICS
#include <SimCore/ModifiedStream.h>

namespace SimCore
{
   ///////////////////////////////////////////////////////////////////
   MUserStream::MUserStream(const char* filename, bool load) : fp(nullptr)
   {
      fp = fopen(filename, load ? "rb" : "wb");
   }

   ///////////////////////////////////////////////////////////////////
   MUserStream::~MUserStream()
   {
      if(fp)	fclose(fp);
   }

   ///////////////////////////////////////////////////////////////////
   //                      Load API                                 //
   ///////////////////////////////////////////////////////////////////

   ///////////////////////////////////////////////////////////////////
   NxU8 MUserStream::readByte() const
   {
      NxU8 b;
      size_t r = fread(&b, sizeof(NxU8), 1, fp);
      NX_ASSERT(r);
      return b;
   }

   ///////////////////////////////////////////////////////////////////
   NxU16 MUserStream::readWord() const
   {
      NxU16 w;
      size_t r = fread(&w, sizeof(NxU16), 1, fp);
      NX_ASSERT(r);
      return w;
   }

   ///////////////////////////////////////////////////////////////////
   NxU32 MUserStream::readDword() const
   {
      NxU32 d;
      size_t r = fread(&d, sizeof(NxU32), 1, fp);
      NX_ASSERT(r);
      return d;
   }

   ///////////////////////////////////////////////////////////////////
   float MUserStream::readFloat() const
   {
      NxReal f;
      size_t r = fread(&f, sizeof(NxReal), 1, fp);
      NX_ASSERT(r);
      return f;
   }

   ///////////////////////////////////////////////////////////////////
   double MUserStream::readDouble() const
   {
      NxF64 f;
      size_t r = fread(&f, sizeof(NxF64), 1, fp);
      NX_ASSERT(r);
      return f;
   }

   ///////////////////////////////////////////////////////////////////
   //                      Save API                                 //
   ///////////////////////////////////////////////////////////////////


   ///////////////////////////////////////////////////////////////////
   void MUserStream::readBuffer(void* buffer, NxU32 size)	const
   {
      size_t w = fread(buffer, size, 1, fp);
      NX_ASSERT(w);
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MUserStream::storeByte(NxU8 b)
   {
      size_t w = fwrite(&b, sizeof(NxU8), 1, fp);
      NX_ASSERT(w);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MUserStream::storeWord(NxU16 w)
   {
      size_t ww = fwrite(&w, sizeof(NxU16), 1, fp);
      NX_ASSERT(ww);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MUserStream::storeDword(NxU32 d)
   {
      size_t w = fwrite(&d, sizeof(NxU32), 1, fp);
      NX_ASSERT(w);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MUserStream::storeFloat(NxReal f)
   {
      size_t w = fwrite(&f, sizeof(NxReal), 1, fp);
      NX_ASSERT(w);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MUserStream::storeDouble(NxF64 f)
   {
      size_t w = fwrite(&f, sizeof(NxF64), 1, fp);
      NX_ASSERT(w);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MUserStream::storeBuffer(const void* buffer, NxU32 size)
   {
      size_t w = fwrite(buffer, size, 1, fp);
      NX_ASSERT(w);
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   MMemoryWriteBuffer::MMemoryWriteBuffer() : currentSize(0), maxSize(0), data(nullptr)
   {
   }

   ///////////////////////////////////////////////////////////////////
   MMemoryWriteBuffer::~MMemoryWriteBuffer()
   {
      NX_FREE(data);
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MMemoryWriteBuffer::storeByte(NxU8 b)
   {
      storeBuffer(&b, sizeof(NxU8));
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MMemoryWriteBuffer::storeWord(NxU16 w)
   {
      storeBuffer(&w, sizeof(NxU16));
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MMemoryWriteBuffer::storeDword(NxU32 d)
   {
      storeBuffer(&d, sizeof(NxU32));
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MMemoryWriteBuffer::storeFloat(NxReal f)
   {
      storeBuffer(&f, sizeof(NxReal));
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MMemoryWriteBuffer::storeDouble(NxF64 f)
   {
      storeBuffer(&f, sizeof(NxF64));
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   NxStream& MMemoryWriteBuffer::storeBuffer(const void* buffer, NxU32 size)
   {
      NxU32 expectedSize = currentSize + size;
      if(expectedSize > maxSize)
      {
         maxSize = expectedSize + 4096;

         NxU8* newData = (NxU8*)NX_ALLOC(maxSize);
         if(data)
         {
            memcpy(newData, data, currentSize);
            NX_FREE(data);
         }
         data = newData;
      }
      memcpy(data+currentSize, buffer, size);
      currentSize += size;
      return *this;
   }

   ///////////////////////////////////////////////////////////////////
   MMemoryReadBuffer::MMemoryReadBuffer(const NxU8* data) : buffer(data)
   {
   }

   ///////////////////////////////////////////////////////////////////
   MMemoryReadBuffer::~MMemoryReadBuffer()
   {
      // We don't own the data => no delete
   }

   ///////////////////////////////////////////////////////////////////
   NxU8 MMemoryReadBuffer::readByte() const
   {
      NxU8 b;
      memcpy(&b, buffer, sizeof(NxU8));
      buffer += sizeof(NxU8);
      return b;
   }

   ///////////////////////////////////////////////////////////////////
   NxU16 MMemoryReadBuffer::readWord() const
   {
      NxU16 w;
      memcpy(&w, buffer, sizeof(NxU16));
      buffer += sizeof(NxU16);
      return w;
   }

   ///////////////////////////////////////////////////////////////////
   NxU32 MMemoryReadBuffer::readDword() const
   {
      NxU32 d;
      memcpy(&d, buffer, sizeof(NxU32));
      buffer += sizeof(NxU32);
      return d;
   }

   ///////////////////////////////////////////////////////////////////
   float MMemoryReadBuffer::readFloat() const
   {
      float f;
      memcpy(&f, buffer, sizeof(float));
      buffer += sizeof(float);
      return f;
   }

   ///////////////////////////////////////////////////////////////////
   double MMemoryReadBuffer::readDouble() const
   {
      double f;
      memcpy(&f, buffer, sizeof(double));
      buffer += sizeof(double);
      return f;
   }

   ///////////////////////////////////////////////////////////////////
   void MMemoryReadBuffer::readBuffer(void* dest, NxU32 size) const
   {
      memcpy(dest, buffer, size);
      buffer += size;
   }
}
#endif
