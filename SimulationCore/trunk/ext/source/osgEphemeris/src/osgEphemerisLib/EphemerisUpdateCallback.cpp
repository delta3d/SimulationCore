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
#include <osgEphemeris/EphemerisUpdateCallback>

using namespace osgEphemeris;


EphemerisUpdateCallbackRegistry::EphemerisUpdateCallbackRegistry()
{
}

EphemerisUpdateCallbackRegistry *EphemerisUpdateCallbackRegistry::instance()
{
    static EphemerisUpdateCallbackRegistry *s = new EphemerisUpdateCallbackRegistry;
    return s;
}

//void UpdateCallbackRegistry::registerUpdateCallback( const std::string &name, UpdateCallback * callback )
void EphemerisUpdateCallbackRegistry::registerUpdateCallback( EphemerisUpdateCallback * callback )
{
        _callbacks[callback->getName()] = callback;
}

EphemerisUpdateCallback *EphemerisUpdateCallbackRegistry::getUpdateCallback( const std::string &name )
{
    std::map<std::string, osg::ref_ptr<EphemerisUpdateCallback> >::iterator p;
    if( (p = _callbacks.find(name)) == _callbacks.end() )
        return 0L;
    return (*p).second.get();
}

