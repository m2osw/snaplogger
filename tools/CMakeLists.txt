# Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved.
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
## build the snap logger checker tool
##
project(snaploggerchecker)

include_directories(
    ${LIBEXCEPT_INCLUDE_DIRS}
    ${SNAPDEV_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    snaploggerchecker.cpp
)

target_link_libraries(${PROJECT_NAME}
    snaplogger
    ${ADVGETOPT_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    COMPONENT
        runtime
)


##
## build the shredding tool
##
project(shredlog)

include_directories(
    ${LIBEXCEPT_INCLUDE_DIRS}
    ${SNAPDEV_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    shredlog.cpp
)

target_link_libraries(${PROJECT_NAME}
    snaplogger
    ${ADVGETOPT_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    COMPONENT
        runtime
)


##
## build the is-hdd tool
##
project(is-hdd)

include_directories(
    ${LIBEXCEPT_INCLUDE_DIRS}
    ${SNAPDEV_INCLUDE_DIRS}
)

add_executable(${PROJECT_NAME}
    is_hdd.cpp
)

target_link_libraries(${PROJECT_NAME}
    snaplogger
    ${ADVGETOPT_LIBRARIES}
    ${LIBEXCEPT_LIBRARIES}
)

install(
    TARGETS
        ${PROJECT_NAME}

    RUNTIME DESTINATION
        bin

    COMPONENT
        runtime
)


# vim: ts=4 sw=4 et nocindent
