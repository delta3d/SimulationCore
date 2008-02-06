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
#ifdef DELTA_PCH

#ifndef DVTE_PREFIX
#define DVTE_PREFIX

#define NO_DT_WIN_PCH_HEADER
#include <prefix/dtgameprefix-src.h>
#undef NO_DT_WIN_PCH_HEADER

#include <SimCore/Actors/Platform.h>
//#include <SimCore/Actors/Human.h>

#ifdef AGEIA_PHYSICS
#include <NxAgeiaCharacterHelper.h>
#include <NxAgeiaPrimitivePhysicsHelper.h>
//#include <Stream.h>
#endif

//#ifdef None
//#undef None
//#endif
//#include <CEGUI.h>

#endif

#endif
