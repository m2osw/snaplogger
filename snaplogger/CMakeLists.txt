# Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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


##
## snaplogger library
##
project(snaplogger)

# Put the version in the header file
#
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)

add_library(${PROJECT_NAME} SHARED
    appender.cpp
    buffer_appender.cpp
    component.cpp
    console_appender.cpp
    convert_ansi.cpp
    date_variable.cpp
    environment.cpp
    environment_variable.cpp
    file_appender.cpp
    format.cpp
    guard.cpp
    logger.cpp
    logger_variable.cpp
    map_diagnostic.cpp
    message.cpp
    nested_diagnostic.cpp
    options.cpp
    ordinal_indicator.cpp
    private_logger.cpp
    severity.cpp
    snapcatch2.cpp
    syslog_appender.cpp
    system_functions.cpp
    system_variable.cpp
    trace_diagnostic.cpp
    user_variable.cpp
    utils.cpp
    variable.cpp
    version.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${ADVGETOPT_INCLUDE_DIRS}
        ${CPPTHREAD_INCLUDE_DIRS}
        ${LIBEXCEPT_INCLUDE_DIRS}
        ${LIBUTF8_INCLUDE_DIRS}
        ${SERVERPLUGINS_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${ADVGETOPT_LIBRARIES}
    ${CPPTHREAD_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
    ${LIBUTF8_LIBRARIES}
    ${LOG4CPLUS_LIBRARIES}
    ${SERVERPLUGINS_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
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
        appender.h
        buffer_appender.h
        component.h
        console_appender.h
        environment.h
        exception.h
        file_appender.h
        format.h
        guard.h
        logger.h
        map_diagnostic.h
        message.h
        nested_diagnostic.h
        options.h
        severity.h
        snapcatch2.hpp
        syslog_appender.h
        trace_diagnostic.h
        utils.h
        variable.h
        ${CMAKE_CURRENT_BINARY_DIR}/version.h

    DESTINATION
        include/snaplogger
)


# vim: ts=4 sw=4 et
