/* -*-c++-*-
 * SimulationCore
 * Copyright 2010, Alion Science and Technology
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
 *
 * David Guthrie
 */
#ifndef STEALTHQTPREFIX_h__
#define STEALTHQTPREFIX_h__

  #ifdef SIMCORE_USE_PCH
    #ifdef _MSC_VER
      #include <QtGui/QDialog>
      #include <StealthViewer/GMApp/ViewWindowConfigObject.h>
      #include <StealthViewer/GMApp/PreferencesVisibilityConfigObject.h>
    #else //_MSC_VER
      #include <prefix/dtgameprefix.h>
   #endif  //_MSC_VER
 #endif //SIMCORE_USE_PCH

#endif // STEALTHQTPREFIX_h__
