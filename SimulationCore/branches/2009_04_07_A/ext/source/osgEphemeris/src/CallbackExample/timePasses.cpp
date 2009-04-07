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
#include <osgEphemeris/EphemerisUpdateCallback>
#include <osgEphemeris/EphemerisData>

class TimePassesCallback : public osgEphemeris::EphemerisUpdateCallback
{
    public:
        TimePassesCallback(): EphemerisUpdateCallback( "TimePassesCallback" ),
             _seconds(0)
        {}

        void operator()( osgEphemeris::EphemerisData *data )
        {
             _seconds += 60;
             struct tm *_tm = localtime(&_seconds);

             data->dateTime.setYear( _tm->tm_year + 1900 );
             data->dateTime.setMonth( _tm->tm_mon + 1 );
             data->dateTime.setDayOfMonth( _tm->tm_mday + 1 );
             data->dateTime.setHour( _tm->tm_hour );
             data->dateTime.setMinute( _tm->tm_min );
             data->dateTime.setSecond( _tm->tm_sec );
        }

    private:
        time_t _seconds;
};

osgEphemeris::EphemerisUpdateCallbackProxy<TimePassesCallback> _timePassesCallbackProxy;

