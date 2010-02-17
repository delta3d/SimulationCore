IF(WIN32)
    CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
ELSE(WIN32)
    IF(APPLE)
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6 FATAL_ERROR)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning: A critical CMake bug exists in 2.4.6 and below. Trying to build Universal Binaries will result in a compile error that seems unrelated. Either avoid building Universal Binaries by changing the CMAKE_OSX_ARCHITECTURES field to list only your architecture, or upgrade to the current CVS version of CMake or a newer stable version if it exists.")
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ELSE(APPLE) 
        CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0 FATAL_ERROR)
    ENDIF(APPLE) 
ENDIF(WIN32)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW) 
endif(COMMAND cmake_policy)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0005 OLD)
endif(COMMAND cmake_policy)

PROJECT(SimulationCore)

################################################################################
# User Options
OPTION(BUILD_HLA "Enables the building of HLA related projects (e.g., # SimCoreHLA)" ON)
OPTION(BUILD_DEMOS "Enables building SimCore Demos in the demos directory" OFF)
OPTION(USE_PHYSX "Enable the building of Simulation Core with PhysX support" OFF)
OPTION(USE_dtPhysics "Enable the building of Simulation core with dtPhysics" OFF)
OPTION(AUTO_RUN_TESTS "Automatically runs the unit tests upon successful compile and link" ON)

if (BUILD_HLA)
   OPTION(USE_RTIS "Use RTI-s for the RTI implementation" ON)
   OPTION(USE_CERTI "Use CERTI for the RTI implementation" OFF)
endif (BUILD_HLA)

IF (WIN32)
   OPTION(BUILD_WITH_MP "Enables the /MP multi-processor compiler option for Visual Studio 2005 and above" OFF)
   MARK_AS_ADVANCED(BUILD_WITH_MP)
   IF(BUILD_WITH_MP)
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
   ENDIF(BUILD_WITH_MP)
   SET(CMAKE_DEBUG_POSTFIX  "d")
ENDIF (WIN32)

# We want to build SONAMES shared librariess
SET(DELTA32_SONAMES TRUE)

# We have some custom .cmake scripts not in the official distribution.
# Maybe this can be used override existing behavior if needed?
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;/usr/local/share/cmake/Modules;${CMAKE_MODULE_PATH}")

# Find OpenGL
FIND_PACKAGE(OpenGL REQUIRED)

# Add macro functions 
INCLUDE(UtilityMacros)

IF(APPLE)
    FIND_LIBRARY(CARBON_LIBRARY Carbon)
    FIND_LIBRARY(APPSERVICES_LIBRARY ApplicationServices)
    FIND_LIBRARY(IOKIT_LIBRARY IOKit)
    FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
    FIND_LIBRARY(AGL_LIBRARY AGL)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
 
    SET(CMAKE_CXX_FLAGS "-pipe -mmacosx-version-min=10.4")
    SET(CMAKE_C_FLAGS "-pipe -mmacosx-version-min=10.4")
    SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,-single_module -compatibility_version 2 -current_version 2" )
    SET(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-2")
ELSE (APPLE)
   #We only want X11 if we are not running on OSX, but still with a unix-like environment 
   IF(UNIX)
       FIND_PACKAGE(X11)
       FIND_LIBRARY(XXF86VM_LIBRARY Xxf86vm)
       SET(${X11_LIBRARIES} "${X11_LIBRARIES} ${XXF86VM_LIBRARY}")
   ENDIF(UNIX)

ENDIF(APPLE)

IF(UNIX)
     # Some Unicies need explicit linkage to the Math library or the build fails.
    FIND_LIBRARY(MATH_LIBRARY m)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-non-virtual-dtor")
ENDIF (UNIX)
        
IF(WIN32)
    ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    #needed for net plugin
    SET (SimulationCore_SOCKET_LIBS wsock32)
    ADD_DEFINITIONS(-DNOMINMAX)
########################################################################################################
# the foolowing options are MSVC specific,
# the first SimulationCore_MSVC_VERSIONED_DLL activate a custom build-time layout that should allow to run examples and application 
# fron bin folder without requiring installation step.
# it also prepend "osg${OPENSCENEGRAPH_SOVERSION}-" to only .dll files, leaving .lib files untouched in lib
# it also use a hack to get rid of Debug and Release folder in MSVC projects 
# all the .dll and .pdb are in bin and all the .lib and .exp are in lib
# 
# the second option disable incremental linking in debug build , that is enabled by default by CMake
##########################################################################################################

    IF(MSVC)
        SET(CMAKE_MODULE_LINKER_FLAGS "/NODEFAULTLIB:python25_d;libc.lib;licmt;libcd;libcmtd")
        SET(CMAKE_SHARED_LINKER_FLAGS "/NODEFAULTLIB:python25_d;libc.lib;licmt;libcd;libcmtd")
        SET(CMAKE_EXE_LINKER_FLAGS "/NODEFAULTLIB:python25_d;libc.lib;licmt;libcd;libcmtd")
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning:  disabling versioned options 2.4.6 exibits inconsintencies in .pdb naming, at least under MSVC, suggested upgrading at least to 2.4.7")
            SET(SimulationCore_MSVC_VERSIONED_DLL OFF)
            SET(SimulationCore_MSVC_DEBUG_INCREMENTAL_LINK ON)
        ELSE(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            OPTION(SimulationCore_MSVC_VERSIONED_DLL "Set to ON to build OpenSceneGraph with versioned dll names" ON)
            MARK_AS_ADVANCED(SimulationCore_MSVC_VERSIONED_DLL)
            OPTION(SimulationCore_MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build OpenSceneGraph without incremental linking in debug (release is off by default)" ON)
            MARK_AS_ADVANCED(SimulationCore_MSVC_DEBUG_INCREMENTAL_LINK)
            IF(NOT SimulationCore_MSVC_DEBUG_INCREMENTAL_LINK)
                SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
                SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
                SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            ENDIF(NOT SimulationCore_MSVC_DEBUG_INCREMENTAL_LINK)
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ENDIF(MSVC)
ENDIF(WIN32)


################################################################################
# 3rd Party Dependency Stuff
# Common to all platforms:

FIND_PACKAGE(SimulationCore REQUIRED)

IF (USE_PHYSX)
   MESSAGE("Using Nvidia PhysX for the Physics Engine.")
   FIND_PACKAGE(DTPhysX REQUIRED)

   #IF (DTPHYSX_FOUND)
      #look for module loaders in dtPhysX CMakeModules
      SET(CMAKE_MODULE_PATH "${DTPHYSX_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

      FIND_PACKAGE(PhysX REQUIRED)

      INCLUDE_DIRECTORIES( ${PHYSX_INCLUDES} ${DTPHYSX_INCLUDE_DIR} )
      LINK_DIRECTORIES( ${DTPHYSX_LIB_DIR} )

      ADD_DEFINITIONS(-DAGEIA_PHYSICS)
   #ENDIF (DTPHYSX_FOUND)
   
ELSE (USE_PHYSX)
   if (USE_dtPhysics)
      MESSAGE("Using the dtPhysics Physics Abstraction Layer (ODE, Bullet, PhysX).")
      FIND_PACKAGE(DTPhysics REQUIRED)
      IF (DTPHYSICS_FOUND)
         #look for module loaders in dtPhysX CMakeModules
         SET(CMAKE_MODULE_PATH "${DTPHYSICS_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

         FIND_PACKAGE(Pal REQUIRED)
 
         INCLUDE_DIRECTORIES( ${PAL_INCLUDE_DIR} ${DTPHYSICS_INCLUDE_DIR} )
         LINK_DIRECTORIES( ${DTPHYSICS_LIB_DIR} )
       ENDIF (DTPHYSICS_FOUND)
   else (USE_dtPhysics)
      MESSAGE("You must pick a physics engine.  Set USE_dtPhysics or USE_PHYSX to on")
   endif (USE_dtPhysics)
ENDIF (USE_PHYSX)

FIND_PACKAGE(Delta3D REQUIRED)

IF (DELTA3D_FOUND)
   #look for module loaders in delta3d CMakeModules
   SET(CMAKE_MODULE_PATH "${DELTA_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

   IF(NOT WIN32)
      FIND_PACKAGE(Curses REQUIRED)
   ENDIF(NOT WIN32)

   FIND_PACKAGE(OpenAL REQUIRED)

   FIND_PACKAGE(OpenThreads REQUIRED)
   FIND_PACKAGE(OSG REQUIRED)

   FIND_PACKAGE(Cal3d REQUIRED)
   FIND_PACKAGE(CEGUI REQUIRED)
   FIND_PACKAGE(CPPUNIT REQUIRED)
   FIND_PACKAGE(ODE REQUIRED)
   FIND_PACKAGE(XercesC REQUIRED)
   FIND_PACKAGE(HAWKNL REQUIRED)
   FIND_PACKAGE(GNE REQUIRED)
   FIND_PACKAGE(Boost)


   IF (NOT WIN32 AND NOT APPLE)
     FIND_PACKAGE(UUID REQUIRED) 
   ENDIF (NOT WIN32 AND NOT APPLE)

   SET(QT_USE_QTOPENGL "true")
   FIND_PACKAGE(Qt4)

   FIND_PACKAGE(GDAL)
   FIND_PACKAGE(DIS)

   if (BUILD_HLA)
      if (USE_RTIS)
         MESSAGE("RTI-s selected for HLA")
         FIND_PACKAGE(RTI-s) #for dtHLAGM
      else (USE_RTIS)
         if (USE_CERTI)
            MESSAGE("CERTI selected for HLA")
            FIND_PACKAGE(RTICERTI REQUIRED)
         else (USE_CERTI)
            MESSAGE("No HLA RTI selected, PICK CERTI or RTI-s")
         endif (USE_CERTI)
      endif (USE_RTIS)
   endif (BUILD_HLA)

ENDIF (DELTA3D_FOUND)

# Make the headers visible to everything
INCLUDE_DIRECTORIES(
    ${DELTA3D_INCLUDE_DIR}
    ${SIMULATIONCORE_INCLUDE_DIR}
    ${OSGEPHEMERIS_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${OPENTHREADS_INCLUDE_DIR}
    ${OSG_INCLUDE_DIR}
    ${CAL3D_INCLUDE_DIR}
    ${CEGUI_INCLUDE_DIR}
    ${ODE_INCLUDE_DIR}
    ${GNE_INCLUDE_DIR}
    ${HAWKNL_INCLUDE_DIR}
    ${XERCES_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${PYTHON_INCLUDE_PATH}  
    ${Boost_INCLUDE_DIR}
)

IF (APPLE)
   INCLUDE_DIRECTORIES(/usr/include/malloc)
ENDIF (APPLE)

LINK_DIRECTORIES( ${DELTA3D_LIB_DIR} )

IF (DELTA3D_EXT_DIR)
  LINK_DIRECTORIES( ${DELTA3D_EXT_DIR}/lib )
ENDIF (DELTA3D_EXT_DIR)
################################################################################
# Create bin and lib directories if required

IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib )
ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")


################################################################################
# Installation stuff



IF(UNIX AND NOT WIN32 AND NOT APPLE)
  IF(CMAKE_SIZEOF_VOID_P MATCHES "8")
      SET(LIB_POSTFIX "64" CACHE STRING "suffix for 32/64 dir placement")
      MARK_AS_ADVANCED(LIB_POSTFIX)
  ENDIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
ENDIF(UNIX AND NOT WIN32 AND NOT APPLE)
IF(NOT DEFINED LIB_POSTFIX)
    SET(LIB_POSTFIX "")
ENDIF(NOT DEFINED LIB_POSTFIX)
    
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})

SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})


# Expose CMAKE_INCLUDE_PATH and CMAKE_LIBARY_PATH to the GUI so users
# may set these values without needing to manipulate the environment.
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} CACHE STRING "You may add additional search paths here. Use ; to separate multiple paths.")
# We are proposing that a new variable called CMAKE_PREFIX_PATH be introduced
# to CMake to compliment CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH.
# A formal feature request has been submited to CMake, Bug #4947.
# It is intended for those users who have common prefixes for their INCLUDE
# and LIBRARY locations. So if users have headers in /usr/local/include
# and libraries in /usr/local/lib, the common prefix is /usr/local.
# It should also cover the case where headers and libraries are
# in the same directory.
# Our proposal expects that FIND_* commands will automatically search for
# CMAKE_PREFIX_PATH right after CMAKE_INCLUDE_PATH or CMAKE_LIBRARY_PATH.
# Obviously, since CMake does not currently support this, we must write
# our Find*.cmake modules to explicitly support this. Otherwise, this variable
# will have no impact.
# This is unofficial so this may be removed or changed at anytime.
SET(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} CACHE STRING "(EXPERIMENTAL) You may add additional search paths here. Use ; to separate multiple paths.")


ADD_SUBDIRECTORY(ext/source)
ADD_SUBDIRECTORY(source)
ADD_SUBDIRECTORY(tests)
ADD_SUBDIRECTORY(utilities)

if (BUILD_DEMOS)
   ADD_SUBDIRECTORY(demos)
endif (BUILD_DEMOS)

# This is for an advanced option to give aggressive warnings 
# under different compilers. If yours is not implemented, this option
# will not be made available.
IF(CMAKE_COMPILER_IS_GNUCXX)
    # To be complete, we might also do GNUCC flags, 
    # but everything here is C++ code.
    # -Wshadow and -Woverloaded-virtual are also interesting flags, but OSG
    # returns too many hits.
    # FYI, if we do implement GNUCC, then -Wmissing-prototypes in another 
    # interesting C-specific flag.
    # Also, there is a bug in gcc 4.0. Under C++, -pedantic will create
    # errors instead of warnings for certain issues, including superfluous
    # semicolons and commas, and the use of long long. -fpermissive seems
    # to be the workaround. 
    SET(SimulationCore_AGGRESSIVE_WARNING_FLAGS "-Wall -Wparentheses -Wformat=2 -Wno-long-long -Wno-import -pedantic -Wreturn-type -Wmissing-braces -Wunknown-pragmas -Wunused -fpermissive")
ELSE(CMAKE_COMPILER_IS_GNUCXX)
    IF(MSVC)
        # FIXME: What are good aggressive warning flags for Visual Studio?
        # And do we need to further subcase this for different versions of VS?
        # CMake variables: MSVC60, MSVC70, MSVC71, MSVC80, CMAKE_COMPILER_2005
        SET(SimulationCore_AGGRESSIVE_WARNING_FLAGS "/Wall /W4")
        

    ELSE(MSVC)
        # CMake lacks an elseif, so other non-gcc, non-VS compilers need
        # to be listed below. If unhandled, SimulationCore_AGGRESSIVE_WARNING_FLAGS should 
        # remain unset.
    ENDIF(MSVC)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

# This part is for the CMake menu option to toggle the warnings on/off.
# This will only be made available if we set values for SimulationCore_AGGRESSIVE_WARNING_FLAGS.
IF(SimulationCore_AGGRESSIVE_WARNING_FLAGS)
    OPTION(SimulationCore_USE_AGGRESSIVE_WARNINGS "Enable to activate aggressive warnings" OFF)
    MARK_AS_ADVANCED(SimulationCore_USE_AGGRESSIVE_WARNINGS)

    IF(SimulationCore_USE_AGGRESSIVE_WARNINGS)
        IF(NOT "${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
            SET(OLD_CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE INTERNAL "Old CXX flags")
            SET(OLD_CMAKE_CXX_FLAGS_WAS_SET 1 CACHE INTERNAL "Old CXX flags was set")
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SimulationCore_AGGRESSIVE_WARNING_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
        ENDIF(NOT "${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
    ELSE(SimulationCore_USE_AGGRESSIVE_WARNINGS)
        # FIXME: This will lose any changes made after OLD_CMAKE_CXX_FLAGS was 
        # set. The better way would be to parse the string and remove each
        # option explicitly.
        IF("${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
            SET(CMAKE_CXX_FLAGS "${OLD_CMAKE_CXX_FLAGS}" CACHE STRING "Flags used by the compiler during all build types." FORCE)
            SET(OLD_CMAKE_CXX_FLAGS_WAS_SET 0 CACHE INTERNAL "Old CXX flags was set")
        ENDIF("${OLD_CMAKE_CXX_FLAGS_WAS_SET}")
    ENDIF(SimulationCore_USE_AGGRESSIVE_WARNINGS)
ENDIF(SimulationCore_AGGRESSIVE_WARNING_FLAGS)

# Set defaults for Universal Binaries. We want 32-bit Intel/PPC on 10.4 
# and 32/64-bit Intel/PPC on >= 10.5. Anything <= 10.3 doesn't support.
#IF(APPLE)
#
#    # These are just defaults/recommendations, but how we want to build
#    # out of the box. But the user needs to be able to change these options.
#    # So we must only set the values the first time CMake is run, or we 
#    # will overwrite any changes the user sets.
#    # FORCE is used because the options are not reflected in the UI otherwise.
#    # Seems like a good place to add version specific compiler flags too.
#    IF(NOT SimulationCore_CONFIG_HAS_BEEN_RUN_BEFORE)
#        # This is really fragile, but CMake doesn't provide the OS system 
#        # version information we need. (Darwin versions can be changed 
#        # independently of OS X versions.)
#        # It does look like CMake handles the CMAKE_OSX_SYSROOT automatically.
#        IF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
#            SET(CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING "Build architectures for OSX" FORCE)
#            SET(CMAKE_OSX_ARCHITECTURES "ppc;i386;ppc64;x86_64" CACHE STRING "Build architectures for OSX" FORCE)
#            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.5 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types." FORCE)
#        ELSE(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
#            IF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
#                SET(CMAKE_OSX_ARCHITECTURES "i386" CACHE STRING "Build architectures for OSX" FORCE)
#                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4 -ftree-vectorize -fvisibility-inlines-hidden" CACHE STRING "Flags used by the compiler during all build types." FORCE)
#            ELSE(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
#                # No Universal Binary support
#                # Should break down further to set the -mmacosx-version-min,
#                # but the SDK detection is too unreliable here.
#            ENDIF(EXISTS /Developer/SDKs/MacOSX10.4u.sdk)
#        ENDIF(EXISTS /Developer/SDKs/MacOSX10.5.sdk)
#    ENDIF(NOT SimulationCore_CONFIG_HAS_BEEN_RUN_BEFORE)
#    
#    OPTION(SimulationCore_BUILD_APPLICATION_BUNDLES "Enable the building of applications and examples as OSX Bundles" OFF)
#    
#ENDIF(APPLE)


# This needs to be run very last so other parts of the scripts can take
# advantage of this.
#IF(NOT SimulationCore_CONFIG_HAS_BEEN_RUN_BEFORE)
#    SET(SimulationCore_CONFIG_HAS_BEEN_RUN_BEFORE 1 CACHE INTERNAL "Flag to track whether this is the first time running CMake or if CMake has been configured before")
#ENDIF(NOT SimulationCore_CONFIG_HAS_BEEN_RUN_BEFORE)


#-----------------------------------------------------------------------------
### uninstall target
#-----------------------------------------------------------------------------
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

