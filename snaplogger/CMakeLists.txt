#
# Description:
#       A simple to use C++ library to log messages.
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

##
## snaplogger library
##
project(snaplogger)

# Put the version in the header file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

add_library( ${PROJECT_NAME} SHARED
    #log.cpp -- original

    appender.cpp
    buffer_appender.cpp
    component.cpp
    console_appender.cpp
    date_variable.cpp
    environment_variable.cpp
    file_appender.cpp
    format.cpp
    guard.cpp
    logger.cpp
    logger_variable.cpp
    map_diagnostic.cpp
    message.cpp
    nested_diagnostic.cpp
    severity.cpp
    syslog_appender.cpp
    system_functions.cpp
    system_variable.cpp
    user_variable.cpp
    variable.cpp
    version.cpp
)

include_directories(
    ${ADVGETOPT_INCLUDE_DIRS}
    ${LIBEXCEPT_INCLUDE_DIRS}
    ${LIBUFTF8_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${ADVGETOPT_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
    ${LIBUFTF8_LIBRARIES}
    ${LOG4CPLUS_LIBRARIES}
)

set_target_properties( ${PROJECT_NAME} PROPERTIES
    VERSION
        ${SNAPLOGGER_VERSION_MAJOR}.${SNAPLOGGER_VERSION_MINOR}

    SOVERSION
        ${SNAPLOGGER_VERSION_MAJOR}
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    LIBRARY DESTINATION
        lib

    ARCHIVE DESTINATION
        lib
)

install(
    FILES
        log.h
        exception.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.h

    DESTINATION
        include/snaplogger
)


# vim: ts=4 sw=4 et
