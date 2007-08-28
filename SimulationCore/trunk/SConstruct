# SConstruct - The construction file for the DVTE project
import glob
import os
import re
import sys
import string
import time
import platform
import delta3d
import macosx
import gch
import qt4
import SCons.Util
from SCons.Script.SConscript import SConsEnvironment

# set this to true to enable build emailing
email = 'false'

# fill this in with interested parties
peopleToEmail = []

# replcae with your local outgoing SMTP server
smtpServer = 'mysmtp@server.com'

SetOption('implicit_cache', 1)

# this is who the build email will come from
fromAddress = 'myaddress@company.com'

############################

# Function to calculate current time for logging
def CurrentTime() :
   now = time.localtime()
   display = time.strftime('%A, %B %d %Y, %X', now)
   return display

########################

# Function to email the result of the build
def EmailResults() :
	file = open('BuildLog.txt', 'r')
	buffer = MIMEText(file.read())
	buffer['Subject'] = 'Editor Build Log'
	file.close()

	s = smtplib.SMTP()
	s.connect(smtpServer) 
	s.sendmail(fromAddress, peopleToEmail, buffer.as_string())
	s.close()

########################

def CheckForBoost() :

   boostHeader = os.path.join('boost','python.hpp')
   
   if OS == 'windows' :
      boostLib = 'boost_python-vc71-mt.lib'
   elif OS == 'darwin' : 
      # This only has a hard-coded version since Boost.Python does not output
      # with the same file naming convention as other platform (as of 1.33.1)
      boostLib = 'libboost_python-1_33_1.dylib'
   else :
      boostLib = 'libboost_python-gcc-mt.so'
      
   foundLib = 0
   for dir in env['LIBPATH'] :
      if os.path.isfile( os.path.join( dir, boostLib ) ) :
         foundLib = 1
         break

   foundHeader = 0
   for dir in env['CPPPATH'] :
      if os.path.isfile( os.path.join( dir, boostHeader ) ) :
         foundHeader = 1
         break

   # If we don't find the lib name for Boost.Python 1.33 onward,
   # try the old library name instead...
   #if OS == 'linux' and foundLib == 0 :
   #   boostLib = 'libboost_python.so'

   #   for dir in env['LIBPATH'] :
   #      if os.path.isfile( os.path.join( dir, boostLib ) ) :
   #         foundLib = 1
   #         break

   return foundHeader != 0 and foundLib != 0

###########
# Options #
###########

# Open the error log
errorLog = open('BuildLog.txt', 'w')

print 'Build started: ' + CurrentTime() + ''
errorLog.write('Build started: ' + CurrentTime() + '\n\n')

print 'Platform: ' + sys.platform 

EnsureSConsVersion(0,96)
#SourceSignatures('timestamp')

# detemine the OS
if sys.platform == 'win32' :
   OS = 'windows'	
elif sys.platform == 'linux2' or sys.platform == 'linux-i386' :
   OS = 'linux'
elif sys.platform == 'darwin' :
   OS = 'darwin'
else :
   print 'Build Failed: Unsupported platform'
   errorLog.write('Build Failed: Unsupported platform: ' + sys.platform)
   errorLog.close()
   env.Exit(-1) 

tools = ["default", "gch"]

if os.environ.has_key('QTDIR') :
   tools.append('qt4') 

if OS == 'darwin':
   tools.append(macosx.TOOL_BUNDLE)

tools.append(delta3d.TOOL_BUNDLE)


# Some basic command line options
optCache = 'options.cache'
opts = Options(optCache)
opts.AddOptions(EnumOption('mode','Build as either debug or release','debug',allowed_values = ('debug','release'),map = {},ignorecase = 1), 
		PackageOption('rti', 'RTI installation directory', 'no'), PackageOption('ageia', 'Ageia PhysX', 'no'))

def CommaConverter( value ) :
   return value.split(',')

opts.Add( 'cpppath', 'Additional include directories (comma delimited)', converter=CommaConverter  )
opts.Add( 'libpath', 'Additional library directories (comma delimited)', converter=CommaConverter  )

if OS == 'windows' :
   opts.Add( PathOption( 'prefix', 'Directory to install under', os.getcwd() ) )
elif OS == 'linux':

   defaultLibDir = 'lib'
   if platform.architecture()[0] == '64bit' :
      defaultLibDir = 'lib64'
   
   opts.Add( BoolOption( 'pch', 'use a prefix/precompiled header', True ) )
   opts.Add( PathOption( 'prefix', 'Directory to install under', '/usr/local' ) )
   opts.Add( PathOption( 'libdir', 'Directory to install libraries under (if set, this will override the normal prefix option)', '/usr/local/'+defaultLibDir ) )
   opts.Add( PathOption( 'bindir', 'Directory to install executables under (if set, this will override the normal prefix option)', '/usr/local/bin' ) )
   opts.Add( PathOption( 'includedir', 'Directory to install headers under (if set, this will override the normal prefix option)', '/usr/local/include' ) )
else :
   opts.Add( PathOption( 'prefix', 'Directory to install under', '.' ) )

env = Environment( options = opts, tools = tools, toolpath = "." )

env['OS'] = OS

env['boost']=False

if(env.get('ageia') != 0) :
   env['AGEIA_INC'] = os.environ.get('AGEIA_INC')
   env['AGEIA_LIB'] = os.environ.get('AGEIA_LIB')
   env.Append(CPPPATH = env['AGEIA_INC']) 
   env.Append(LIBPATH = env['AGEIA_LIB']) 

# Locate and add the Delta3D directories
delta_inc = os.environ.get('DELTA_INC')
delta_lib = os.environ.get('DELTA_LIB')

if OS == 'windows' :
	splitChar = ';'
else :
	splitChar = ':'
   
delta_inc = delta_inc.split(splitChar)
delta_lib = delta_lib.split(splitChar)


env.Append(CPPPATH = ['#include'] + delta_inc)
env.Append(CPPPATH = ['#ext/include/' + sys.platform])
env.Append(LIBPATH = ['#lib', '#ext/lib/' + sys.platform] + delta_lib)

conf = env.CreateConf(errorLog)

env['additionalLibsOrder'] = [ 'osgGA', 'osgProducer', 'osgEphemeris' ]

if env.get('ageia') != 0 :
   env['additionalLibsOrder'] += [ 'NxCooking', 'NxCharacter', 'NxExtensions', 'PhysXLoader' ]
	

if OS == 'windows' and env['mode'] == 'debug' :
   env['dtLibs']['SimViewerCore'] = 'SimViewerCored'
   env['dtLibs']['StealthGMApp'] = 'StealthGMAppd'
   env['dtLibs']['StealthQt'] = 'StealthQtd'
   env['extLibs']['osgGA'] = 'osgGAd'
   env['extLibs']['osgProducer'] = 'osgProducerd'
   env['extLibs']['osgEphemeris'] = 'osgEphemerisd'
   if env.get('ageia') != 0 :
      env['dtLibs']['dtAgeiaPhysX'] = 'dtAgeiaPhysXd'
      env['extLibs']['NxCooking'] = 'NxCooking'
      env['extLibs']['NxCharacter'] = 'NxCharacter'
      env['extLibs']['NxExtensions'] = 'NxExtensions'
      env['extLibs']['PhysXLoader'] = 'PhysXLoader'
else :
   env['dtLibs']['SimViewerCore'] = 'SimViewerCore'
   env['dtLibs']['StealthGMApp'] = 'StealthGMApp'
   env['dtLibs']['StealthQt'] = 'StealthQt'
   env['extLibs']['osgGA'] = 'osgGA'
   env['extLibs']['osgProducer'] = 'osgProducer'
   env['extLibs']['osgEphemeris'] = 'osgEphemeris'
   if env.get('ageia') != 0 :
      env['dtLibs']['dtAgeiaPhysX'] = 'dtAgeiaPhysX'
      env['extLibs']['NxCooking'] = 'NxCooking'
      env['extLibs']['NxCharacter'] = 'NxCharacter'
      env['extLibs']['NxExtensions'] = 'NxExtensions'
      env['extLibs']['PhysXLoader'] = 'PhysXLoader'

env.CompilerConf(conf, errorLog)

def CommaConverter( value ) :
   return value.split(',')


Help( opts.GenerateHelpText( env ) )
opts.Save( 'delta3d.conf', env )


if env.get('mode') == 'debug':
   mode = 'debug'
elif env.get('mode') == 'release':
   mode = 'release'

# Store the file signatures
env.SConsignFile()

# Copy in the external environment variables
env.ENV = os.environ

launchDir = env.GetLaunchDir()
   

# Export the variables other files will need
Export('env')

# Build the SConscript files
modules = ['source']

for module in modules :
   SConscript(dirs = [module], duplicate = 0)
	
Default(['bin', 'lib', 'tests'])
if OS == 'darwin':
   Default(['Applications'])
