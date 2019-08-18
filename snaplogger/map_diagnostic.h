/*
 * Copyright (c) 2013-2019  Made to Order Software Corp.  All Rights Reserved
 *
 * https://snapwebsites.org/project/snaplogger
 * contact@m2osw.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#pragma once

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    "snaplogger/message.h"

// C++ lib
//
#include    <map>


// C lib
//
#include    <sys/time.h>



namespace snaplogger
{


constexpr char const        DIAG_KEY_PROJECT_NAME[] = "project_name";
constexpr char const        DIAG_KEY_PROGNAME[]     = "progname";
constexpr char const        DIAG_KEY_VERSION[]      = "version";
constexpr char const        DIAG_KEY_BUILD_DATE[]   = "build_date";
constexpr char const        DIAG_KEY_BUILD_TIME[]   = "build_time";


typedef std::map<std::string, std::string>        map_diagnostics_t;

void                set_diagnostic(std::string const & key, std::string const & diagnostic);
void                unset_diagnostic(std::string const & key);

map_diagnostics_t   get_map_diagnostics();
map_diagnostics_t   get_map_diagnostics(message const & msg);


} // snaplogger namespace
// vim: ts=4 sw=4 et
