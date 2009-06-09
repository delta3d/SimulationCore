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
//						     PHYSX SDK TRAINING PROGRAMS
//							         USER STREAM
//
//						   Originally Written by Bob Schade, 10-15-05
// ===============================================================================

#ifndef _MODIFIED_STREAM_H_
#define _MODIFIED_STREAM_H_

#include "NxPhysics.h"
#include "NxStream.h"
#include <cstdio>
#include <SimCore/Export.h>

#ifdef AGEIA_PHYSICS

namespace SimCore
{
   /////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT MUserStream : public NxStream
   {
   public:
                                                                      MUserStream(const char* filename, bool load);
           virtual						~MUserStream();

           virtual		NxU8			readByte()								const;
           virtual		NxU16			readWord()								const;
           virtual		NxU32			readDword()								const;
           virtual		float			readFloat()								const;
           virtual		double		readDouble()							const;
           virtual		void			readBuffer(void* buffer, NxU32 size)	const;

           virtual		NxStream&		storeByte(NxU8 b);
           virtual		NxStream&		storeWord(NxU16 w);
           virtual		NxStream&		storeDword(NxU32 d);
           virtual		NxStream&		storeFloat(NxReal f);
           virtual		NxStream&		storeDouble(NxF64 f);
           virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size);

                FILE*			fp;
   };

   /////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT MMemoryWriteBuffer : public NxStream
   {
           public:
                                                                   MMemoryWriteBuffer();
           virtual					~MMemoryWriteBuffer();

           virtual		NxU8			readByte()								const	{ NX_ASSERT(0);	return 0;	}
           virtual		NxU16			readWord()								const	{ NX_ASSERT(0);	return 0;	}
           virtual		NxU32			readDword()								const	{ NX_ASSERT(0);	return 0;	}
           virtual		float			readFloat()								const	{ NX_ASSERT(0);	return 0.0f;}
           virtual		double		readDouble()							const	{ NX_ASSERT(0);	return 0.0;	}
           virtual		void			readBuffer(void* buffer, NxU32 size)	const	{ NX_ASSERT(0);				}

           virtual		NxStream&		storeByte(NxU8 b);
           virtual		NxStream&		storeWord(NxU16 w);
           virtual		NxStream&		storeDword(NxU32 d);
           virtual		NxStream&		storeFloat(NxReal f);
           virtual		NxStream&		storeDouble(NxF64 f);
           virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size);

           NxU32			currentSize;
           NxU32			maxSize;
           NxU8*			data;
   };

   /////////////////////////////////////////////////////////////////////
   class SIMCORE_EXPORT MMemoryReadBuffer : public NxStream
   {
           public:
                                                                   MMemoryReadBuffer(const NxU8* data);
           virtual					~MMemoryReadBuffer();

           virtual		NxU8			readByte()								const;
           virtual		NxU16			readWord()								const;
           virtual		NxU32			readDword()								const;
           virtual		float			readFloat()								const;
           virtual		double		readDouble()							const;
           virtual		void			readBuffer(void* buffer, NxU32 size)	const;

           virtual		NxStream&		storeByte(NxU8 b)							{ NX_ASSERT(0);	return *this;}
           virtual		NxStream&		storeWord(NxU16 w)						{ NX_ASSERT(0);	return *this;}
           virtual		NxStream&		storeDword(NxU32 d)						{ NX_ASSERT(0);	return *this;}
           virtual		NxStream&		storeFloat(NxReal f)						{ NX_ASSERT(0);	return *this;}
           virtual		NxStream&		storeDouble(NxF64 f)						{ NX_ASSERT(0);	return *this;}
           virtual		NxStream&		storeBuffer(const void* buffer, NxU32 size)	{ NX_ASSERT(0);	return *this;}

           mutable		const NxU8*		buffer;
   };
}

#endif //AGEIA_PHYSICS

#endif  // STREAM_H
