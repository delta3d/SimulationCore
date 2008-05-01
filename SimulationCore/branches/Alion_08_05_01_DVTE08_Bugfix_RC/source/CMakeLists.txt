
ADD_SUBDIRECTORY(SimCore)

# Can't build the Stealth Viewer without HLA support at the moment.
IF (BUILD_HLA)
    ADD_SUBDIRECTORY(SimCore/HLA)
    ADD_SUBDIRECTORY(StealthViewer/GMApp)
    ADD_SUBDIRECTORY(StealthViewer/Qt)
    ADD_SUBDIRECTORY(StealthViewer)
ENDIF (BUILD_HLA)
