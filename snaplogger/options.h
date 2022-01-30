// Copyright (c) 2013-2022  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#pragma once

/** \file
 * \brief Handle logger specific command line and other options.
 *
 * The logger supports a few options to override configuration files
 * and tweak settings from the command line. Since the user is in
 * control of the environment variable, we do not offer that option
 * here.
 */


// advgetopt lib
//
#include    <advgetopt/advgetopt.h>


// C++ lib
//
#include    <iostream>



namespace snaplogger
{



void            add_logger_options(advgetopt::getopt & opts);
bool            process_logger_options(advgetopt::getopt & opts
                                     , std::string const & config_path = std::string("/etc/snaplogger/logger")
                                     , std::basic_ostream<char> & out = std::cout
                                     , bool show_banner = true);



} // snaplogger namespace
// vim: ts=4 sw=4 et
