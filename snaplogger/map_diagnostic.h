/*
 * File:
 *    snaplogger/map_diagnostic.h -- logger for Snap! and others
 *
 * License:
 *    Copyright (c) 2013-2019  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/
 *    contact@m2osw.com
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *    Alexis Wilke   alexis@m2osw.com
 */
#pragma once

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// C++ lib
//
#include    <map>


// C lib
//
#include    <sys/time.h>



namespace snaplogger
{


constexpr char const        DIAG_KEY_PROGNAME[] = "progname";
constexpr char const        DIAG_KEY_VERSION[]  = "version";


typedef std::map<std::string, std::string>        map_diagnostics_t;

void                set_diagnostic(std::string const & key, std::string const & diagnostic);
void                unset_diagnostic(std::string const & key);

map_diagnostics_t   get_map_diagnostics();


} // snaplogger namespace
// vim: ts=4 sw=4 et
