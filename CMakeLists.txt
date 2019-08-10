#
# Description:
#       A logger for our Snap! projects. This was part of the libsnapwebsites
#       library but we wanted to have access to this logger from any projects
#       (well, nearly any, some are _below_ this one... such as advgetopt.)
#
# License:
#       Copyright (c) 2013-2019 Made to Order Software Corp.
#
#       https://snapwebsites.org/projects/libutf8
#       contact@m2osw.com
#
#       This program is free software; you can redistribute it and/or modify
#       it under the terms of the GNU General Public License as published by
#       the Free Software Foundation; either version 2 of the License, or
#       (at your option) any later version.
#
#       This program is distributed in the hope that it will be useful,
#       but WITHOUT ANY WARRANTY; without even the implied warranty of
#       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#       GNU General Public License for more details.
#
#       You should have received a copy of the GNU General Public License
#       along with this program; if not, write to the Free Software
#       Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#


cmake_minimum_required(VERSION 2.8.4)

project(snaplogger_library)

find_package(SnapCMakeModules REQUIRED)
find_package(AdvGetOpt        REQUIRED)
find_package(CppThread        REQUIRED)
find_package(LibExcept        REQUIRED)
find_package(LibUtf8          REQUIRED)
find_package(SnapDev          REQUIRED)
find_package(SnapDoxygen              )

SnapGetVersion(SNAPLOGGER ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(snaplogger)
add_subdirectory(tests     )
add_subdirectory(doc       )
add_subdirectory(cmake     )
add_subdirectory(tools     )

# vim: ts=4 sw=4 et
