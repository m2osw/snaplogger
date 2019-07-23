# - Try to find SnapLogger
#
# Once done this will define
#
# SNAPLOGGER_FOUND        - System has SnapLogger
# SNAPLOGGER_INCLUDE_DIR  - The SnapLogger include directories
# SNAPLOGGER_LIBRARY      - The libraries needed to use SnapLogger (none)
# SNAPLOGGER_DEFINITIONS  - Compiler switches required for using SnapLogger (none)

find_path( SNAPLOGGER_INCLUDE_DIR snaplogger/log.h
		   PATHS $ENV{SNAPLOGGER_INCLUDE_DIR}
		 )
find_library( SNAPLOGGER_LIBRARY snaplogger
		   PATHS $ENV{SNAPLOGGER_LIBRARY}
		 )
mark_as_advanced( SNAPLOGGER_INCLUDE_DIR SNAPLOGGER_LIBRARY )

set( SNAPLOGGER_INCLUDE_DIRS ${SNAPLOGGER_INCLUDE_DIR} )
set( SNAPLOGGER_LIBRARIES    ${SNAPLOGGER_LIBRARY}     )

include( FindPackageHandleStandardArgs )
# handle the QUIETLY and REQUIRED arguments and set SNAPLOGGER_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args( SnapLogger DEFAULT_MSG SNAPLOGGER_INCLUDE_DIR SNAPLOGGER_LIBRARY )
