
ADD_SUBDIRECTORY(SimCore)

# Can't build the Stealth Viewer without HLA support at the moment.
IF (BUILD_HLA)
   ADD_SUBDIRECTORY(SimCore/HLA)
   if (BUILD_STEALTH_VIEWER)
      ADD_SUBDIRECTORY(SimCoreWidgets)
      ADD_SUBDIRECTORY(StealthViewer/GMApp)
      ADD_SUBDIRECTORY(StealthViewer/Qt)
      ADD_SUBDIRECTORY(StealthViewer)
   endif (BUILD_STEALTH_VIEWER)
ENDIF (BUILD_HLA)
