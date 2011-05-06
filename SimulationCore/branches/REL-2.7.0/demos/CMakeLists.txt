
if (USE_PHYSX)
   MESSAGE("The SimCore Demos do not support dtAgeiaPhysX - bypassing.")
else (USE_PHYSX)
   ADD_SUBDIRECTORY(NetDemo)
   ADD_SUBDIRECTORY(DriverDemo)
endif (USE_PHYSX)
