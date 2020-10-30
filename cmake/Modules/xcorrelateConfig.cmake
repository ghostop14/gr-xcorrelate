INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_XCORRELATE xcorrelate)

FIND_PATH(
    XCORRELATE_INCLUDE_DIRS
    NAMES xcorrelate/api.h
    HINTS $ENV{XCORRELATE_DIR}/include
        ${PC_XCORRELATE_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    XCORRELATE_LIBRARIES
    NAMES gnuradio-xcorrelate
    HINTS $ENV{XCORRELATE_DIR}/lib
        ${PC_XCORRELATE_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/xcorrelateTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XCORRELATE DEFAULT_MSG XCORRELATE_LIBRARIES XCORRELATE_INCLUDE_DIRS)
MARK_AS_ADVANCED(XCORRELATE_LIBRARIES XCORRELATE_INCLUDE_DIRS)
