#
# File:
#       CMakeLists.txt
#
# Description:
#       A logger for our Snap! projects. This was part of the libsnapwebsites
#       library but we wanted to have access to this logger from any projects
#       (well, nearly any, some are _below_ this one... such as advgetopt)
#       so as to not have to deal with log4cplus directly. The ultimate idea
#       is to actually get rid of log4cplus which is really big and has a lot
#       of unnecessary features.
#
# Documentation:
#       See the CMake documentation.
#
# License:
#       Copyright (c) 2013-2019 Made to Order Software Corp.
#
#       https://snapwebsites.org/projects/libutf8
#       contact@m2osw.com
#
#       Permission is hereby granted, free of charge, to any person obtaining a
#       copy of this software and associated documentation files (the
#       "Software"), to deal in the Software without restriction, including
#       without limitation the rights to use, copy, modify, merge, publish,
#       distribute, sublicense, and/or sell copies of the Software, and to
#       permit persons to whom the Software is furnished to do so, subject to
#       the following conditions:
#
#       The above copyright notice and this permission notice shall be included
#       in all copies or substantial portions of the Software.
#
#       THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#       OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#       MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#       IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
#       CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#       TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#       SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#


cmake_minimum_required(VERSION 2.8.4)

project(snaplogger_library)

find_package(SnapCMakeModules REQUIRED)
find_package(AdvGetOpt        REQUIRED)
find_package(LibExcept        REQUIRED)
find_package(LibUtf8          REQUIRED)
find_package(log4cplus        REQUIRED)
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
