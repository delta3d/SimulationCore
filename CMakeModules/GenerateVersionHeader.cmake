# Thanks to Guillaume Jacquenot for the base of this script.

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)

TODAY(BUILD_DATE)

# the FindSubversion.cmake module is part of the standard distribution
include(FindSubversion)
# extract working copy information for SOURCE_DIR into MY_XXX variables
Subversion_WC_INFO(${SOURCE_DIR} SIMCORE_SVN)
# write a file with the SVNVERSION define
file(WRITE SimCoreGeneratedVersion.h.txt "
#ifndef DELTA_SIMCOREVERSION
#define DELTA_SIMCOREVERSION

#define SIMCORE_SVN_REVISION \"${SIMCORE_SVN_WC_REVISION}\"
#define SIMCORE_SVN_DATE \"${SIMCORE_SVN_WC_LAST_CHANGED_DATE}\"
#define SIMCORE_BUILD_DATE \"${BUILD_DATE}\"

#endif
")

# copy the file to the final header only if the version changes
# reduces needless rebuilds
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        SimCoreGeneratedVersion.h.txt ${CMAKE_CURRENT_SOURCE_DIR}/SimCoreGeneratedVersion.h)