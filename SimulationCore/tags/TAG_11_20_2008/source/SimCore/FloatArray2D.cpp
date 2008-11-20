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

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <prefix/SimCorePrefix-src.h>
#include <osg/Endian>
#include <SimCore/FloatArray2D.h>



namespace SimCore
{
   /////////////////////////////////////////////////////////////////////////////
   // FLOAT ARRAY 2D CODE
   /////////////////////////////////////////////////////////////////////////////
   FloatArray2D::FloatArray2D()
      : mRowSize(0)
   {
   }

   /////////////////////////////////////////////////////////////////////////////
   FloatArray2D::~FloatArray2D()
   {
      ClearData();
   }

   /////////////////////////////////////////////////////////////////////////////
   void FloatArray2D::SetValue( float value, int index )
   {
      if( index < int(mData.size()) )
      {
         mData[index] = value;
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   float FloatArray2D::GetValue( int index ) const
   {
      if( index < int(mData.size()) )
      {
         return mData[index];
      }
      return 0.0f;
   }

   /////////////////////////////////////////////////////////////////////////////
   void FloatArray2D::SetRowSize( int rowSize )
   {
      // TODO: Adjust the the array to fit the new row size if the new
      // row size conflicts with the current array length.

      mRowSize = rowSize;
   }

   /////////////////////////////////////////////////////////////////////////////
   int FloatArray2D::GetRowSize() const
   {
      return mRowSize;
   }

   /////////////////////////////////////////////////////////////////////////////
   std::vector<float>& FloatArray2D::GetData()
   {
      return mData;
   }

   /////////////////////////////////////////////////////////////////////////////
   const std::vector<float>& FloatArray2D::GetData() const
   {
      return mData;
   }

   /////////////////////////////////////////////////////////////////////////////
   void FloatArray2D::Encode( char* buffer ) const
   {
      unsigned short cols = (unsigned short)(mRowSize);
      unsigned short rows = cols == 0 ? 0 : (unsigned short)(mData.size()/size_t(cols));

      bool isLittleEndian = osg::getCpuByteOrder() == osg::LittleEndian;
      if( isLittleEndian )
      {
         osg::swapBytes2((char*)(&cols));
         osg::swapBytes2((char*)(&rows));
      }

      size_t offset = 0;
      memcpy( &buffer[0], &cols, 2 );
      offset += 2;
      memcpy( &buffer[offset], &rows, 2 );
      offset += 2;

      float curValue = 0.0f;
      std::vector<float>::const_iterator curElement = mData.begin();
      std::vector<float>::const_iterator endElementArray = mData.end();
      for( ; curElement != endElementArray; ++curElement, offset += 4 )
      {
         curValue = *curElement;
         if( isLittleEndian )
         {
            osg::swapBytes4((char*)(&curValue));
         }
         memcpy( &buffer[offset], &curValue, 4 );
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   void FloatArray2D::Decode( const char* buffer )
   {
      unsigned short cols = *(unsigned short*)(&buffer[0]);
      size_t offset = 2;
      unsigned short rows = *(unsigned short*)(&buffer[offset]);
      offset += 2;

      bool isLittleEndian = osg::getCpuByteOrder() == osg::LittleEndian;
      if( isLittleEndian )
      {
         osg::swapBytes2((char*)(&cols));
         osg::swapBytes2((char*)(&rows));
      }

      size_t dataLength = cols * rows;
      mData.clear();
      mData.reserve( dataLength );
      mRowSize = cols;

      float curValue = 0.0f;
      for( size_t i = 0; i < dataLength; ++i, offset += 4 )
      {
         memcpy( &curValue, &buffer[offset], 4 );

         if( isLittleEndian )
         {
            osg::swapBytes4((char*)(&curValue));
         }

         mData.push_back(curValue);
      }
   }

   /////////////////////////////////////////////////////////////////////////////
   int FloatArray2D::GetEncodeSize() const
   {
      return 2 + 2 + int(mData.size() * 4);
   }

   /////////////////////////////////////////////////////////////////////////////
   void FloatArray2D::ClearData()
   {
      mRowSize = 0;
      mData.clear();
   }

}
