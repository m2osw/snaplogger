// Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#pragma once

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */


// C++
//
#include    <map>
#include    <string>
#include    <vector>


// C
//
#include    <sys/stat.h>



namespace snaplogger
{


typedef std::map<std::string, std::string>      string_map_t;
typedef std::map<std::string, std::u32string>   u8u32string_map_t;
typedef std::vector<std::string>                string_vector_t;


bool        is_rotational(std::string const & filename);
bool        is_rotational(struct stat & s);



} // snaplogger namespace
// vim: ts=4 sw=4 et
