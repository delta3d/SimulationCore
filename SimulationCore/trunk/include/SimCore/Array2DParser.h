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
 * @author Chris Rodgers
 */
#ifndef FLOAT_ARRAY_2D_H
#define FLOAT_ARRAY_2D_H

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <vector>
#include <dtUtil/datastream.h>

namespace SimCore
{
   /////////////////////////////////////////////////////////////////////////////
   // FLOAT ARRAY 2D CODE
   /////////////////////////////////////////////////////////////////////////////
   template<typename T>
   class Array2DParser
   {
   public:
      typedef T value_type;
      typedef typename std::vector<T> VectorType;

      Array2DParser()
      : mColumns(0)
      , mLittleEndian(false)
      {
      }

      virtual ~Array2DParser() {}

      void SetLittleEndianStorage(bool littleEndian) { mLittleEndian = littleEndian; }
      bool GetLittleEndianStorage() const { return mLittleEndian; }

      void SetValue(T value, unsigned indexRow, unsigned indexCol)
      {
         unsigned actualIndex = (indexRow * mColumns) + indexCol;
         if (indexCol >= mColumns )
         {
            return;
         }

         if (actualIndex >= mData.size())
         {
            size_t remainder = (actualIndex + 1) % mColumns;
            size_t rows = (actualIndex + 1) / mColumns;
            if (remainder > 0)
            {
               rows += 1;
            }

            mData.resize(rows * mColumns);
         }

         mData[actualIndex] = value;
      }

      T GetValue(unsigned indexRow, unsigned indexCol) const
      {
         unsigned actualIndex = (indexRow * mColumns) + indexCol;
         return mData[actualIndex];
      }

      void SetColumns(size_t columns) { mColumns = columns; }
      size_t GetColumns() const { return mColumns; };

      size_t GetRows() const
      {
         if (mColumns == 0)
         {
            return 0;
         }

         size_t remainder = mData.size() % mColumns;
         size_t rows = mData.size() / mColumns;
         if (remainder > 0)
         {
            rows += 1;
         }

         return rows;
      };

      VectorType& GetData() { return mData; }
      const VectorType& GetData() const { return mData; }

      size_t Encode(char* buffer, const size_t maxSize) const
      {
         typedef typename VectorType::const_iterator ConstIter;
         dtUtil::DataStream ds(buffer, maxSize, false);
         ds.SetForceLittleEndian(mLittleEndian);
         ds.ClearBuffer();
         short cols = mColumns;
         short rows = GetRows();
         ds << cols;
         ds << rows;

         ConstIter i, iend;
         i = mData.begin();
         iend = mData.end();
         for (; i != iend; ++i)
         {
            ds << *i;
         }
         return ds.GetBufferSize();
      }

      void Decode(const char* buffer, size_t bufferSize)
      {
         dtUtil::DataStream ds(const_cast<char*>(buffer), bufferSize, false);
         ds.SetForceLittleEndian(mLittleEndian);
         short cols = 0, rows = 0;
         ds >> cols;
         ds >> rows;

         mColumns = cols;

         // apparently, if it has 0 rows, but 1 column, it really means it has 1 row.
         if (cols > 0 && rows == 0)
         {
            rows = 1;
         }

         size_t length = cols*rows;
         mData.clear();
         mData.reserve(length);
         for (size_t i = 0; i < length; ++i)
         {
            T f;
            ds >> f;
            mData.push_back(f);
         }
      }

      size_t GetEncodedSize() const
      {
         return (2 * sizeof(short)) + (mData.size() * sizeof(T));
      }

      void ClearData() { mData.clear(); }

   protected:

   private:
      size_t mColumns;
      bool mLittleEndian;
      VectorType mData;
   };

}

#endif
