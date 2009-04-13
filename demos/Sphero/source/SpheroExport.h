/* -*-c++-*-
 * Delta3D Open Source Game and Simulation Engine
 * Copyright (C) 2008, Alion Science and Technology
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
 * David Guthrie
 */
#ifndef SPHERO_EXPORT_H
#define SPHERO_EXPORT_H

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#  ifdef SPHERO_LIBRARY
#    define SPHERO_EXPORT __declspec(dllexport)
#  else
#	  define SPHERO_EXPORT __declspec(dllimport)
#	endif
#else
#   ifdef SPHERO_LIBRARY
//#      define SPHERO_EXPORT __attribute__ ((visibility("default")))
#      define SPHERO_EXPORT
#   else
#      define SPHERO_EXPORT
#   endif 
#endif


#endif //SPHERO_EXPORT_H
