# - Try to find SnapLogger
#
# SNAPLOGGER_FOUND          - system has SnapLogger
# SNAPLOGGER_INCLUDE_DIRS   - the SnapLogger include directories
# SNAPLOGGER_LIBRARIES      - the libraries needed to use SnapLogger
# SNAPLOGGER_DEFINITIONS    - compiler switches required for using SnapLogger
# SNAPLOGGER_SHARE_CONF_DIR - directory where you install configuration files
#
# License:
#
# Copyright (c) 2011-2025  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/snaplogger
# contact@m2osw.com
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.


find_path(
    SNAPLOGGER_INCLUDE_DIR
        snaplogger/logger.h

    PATHS
        ENV SNAPLOGGER_INCLUDE_DIR
)

find_library(
    SNAPLOGGER_LIBRARY
        snaplogger

    PATHS
        ${SNAPLOGGER_LIBRARY_DIR}
        ENV SNAPLOGGER_LIBRARY
)

mark_as_advanced(
    SNAPLOGGER_INCLUDE_DIR
    SNAPLOGGER_LIBRARY
)

set(SNAPLOGGER_INCLUDE_DIRS ${SNAPLOGGER_INCLUDE_DIR})
set(SNAPLOGGER_LIBRARIES    ${SNAPLOGGER_LIBRARY})


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    SnapLogger
    REQUIRED_VARS
        SNAPLOGGER_INCLUDE_DIR
        SNAPLOGGER_LIBRARY
)

set(SNAPLOGGER_SHARE_CONF_DIR share/snaplogger/etc)

# vim: ts=4 sw=4 et
