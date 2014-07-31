

file(GLOB DRIVER_DEMO_DATA_FILES ${CMAKE_SOURCE_DIR}/demos/DriverDemo/*.xml ${CMAKE_SOURCE_DIR}/demos/DriverDemo/*.dtproj)

file(GLOB NET_DEMO_DATA_FILES ${CMAKE_SOURCE_DIR}/demos/NetDemo/*.xml ${CMAKE_SOURCE_DIR}/demos/NetDemo/*.dtproj)

file(GLOB TEST_WATER_DATA_FILES ${CMAKE_SOURCE_DIR}/demos/TestWater/*.xml ${CMAKE_SOURCE_DIR}/demos/TestWater/*.dtproj)

file(GLOB SIMCORE_TEST_DATA_FILES ${CMAKE_SOURCE_DIR}/tests/*.xml ${CMAKE_SOURCE_DIR}/tests/*.dtproj )

INSTALL (
   FILES        ${DRIVER_DEMO_DATA_FILES}
   DESTINATION  demos/DriverDemo
   COMPONENT simcore-demos
)

INSTALL (
   FILES        ${NET_DEMO_DATA_FILES}
   DESTINATION  demos/NetDemo
   COMPONENT simcore-demos
)

INSTALL (
   FILES        ${TEST_WATER_DATA_FILES}
   DESTINATION  demos/TestWater
   COMPONENT simcore-demos
)

INSTALL (
   FILES        ${SIMCORE_TEST_DATA_FILES}
   DESTINATION  tests
   COMPONENT simcore
)

INSTALL (
   DIRECTORY    ${CMAKE_SOURCE_DIR}/ProjectAssets_Shared
   DESTINATION  ./
   COMPONENT    simcore
   PATTERN .svn EXCLUDE
   PATTERN  *~  EXCLUDE
)

INSTALL (
   DIRECTORY    ${CMAKE_SOURCE_DIR}/demos/ProjectAssets_Demos
   DESTINATION  demos
   COMPONENT    simcore-demos
   PATTERN .svn EXCLUDE
   PATTERN  *~  EXCLUDE
)

file(GLOB CMAKE_MODULE_FILES "${CMAKE_SOURCE_DIR}/CMakeModules/*.cmake" "${CMAKE_SOURCE_DIR}/CMakeModules/*.cmake.in")

if("${CMAKE_SYSTEM}" MATCHES "Linux")
   INSTALL (
      FILES    ${CMAKE_MODULE_FILES}
      DESTINATION  share/cmake/Modules
      COMPONENT    simcore
   )
else ()
   INSTALL (
      FILES    ${CMAKE_MODULE_FILES}
      DESTINATION  CMakeModules
      COMPONENT    simcore
   )
endif()
