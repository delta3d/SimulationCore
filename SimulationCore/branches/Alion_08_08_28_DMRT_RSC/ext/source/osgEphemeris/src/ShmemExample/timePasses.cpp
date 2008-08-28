/* -*-c++-*- OpenSceneGraph - Ephemeris Model Copyright (C) 2005 Don Burns
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
#include <time.h>
#include <unistd.h>
#include <osgEphemeris/EphemerisData>

int main( int argc, char **argv )
{
    time_t seconds = 0;

    // Attached to the default shared memory segment
    osgEphemeris::EphemerisData *data = new(osgEphemeris::EphemerisData::getDefaultShmemFileName())osgEphemeris::EphemerisData;

    for( ;; )
    {
         seconds += 60;
         struct tm *_tm = localtime(&seconds);

         data->dateTime.setYear( _tm->tm_year + 1900 );
         data->dateTime.setMonth( _tm->tm_mon + 1 );
         data->dateTime.setDayOfMonth( _tm->tm_mday + 1 );
         data->dateTime.setHour( _tm->tm_hour );
         data->dateTime.setMinute( _tm->tm_min );
         data->dateTime.setSecond( _tm->tm_sec );

         // Sleep 16 milliseconds
         usleep(16667);
    }

    return 0;
}

