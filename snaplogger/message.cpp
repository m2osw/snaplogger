/*
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

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
// 
#include    "snaplogger/logger.h"
#include    "snaplogger/message.h"


// C++ lib
//
#include    <iostream>


// C lib
//
#include    <sys/time.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


int null_buffer::overflow(int c)
{
    return c;
}





message::message(
          severity_t sev
        , char const * file
        , char const * func
        , int line)
    : f_severity(sev)
    , f_filename(file == nullptr ? std::string() : std::string(file))
    , f_funcname(func == nullptr ? std::string() : std::string(func))
    , f_line(line)
    , f_environment(create_environment())
{
    clock_gettime(CLOCK_REALTIME_COARSE, &f_timestamp);
}


message::~message()
{
    logger::get_instance()->log_message(*this);
}


void message::set_filename(std::string const & filename)
{
    f_filename = filename;
}


void message::set_function(std::string const & funcname)
{
    f_funcname = funcname;
}


void message::set_line(int line)
{
    f_line = line;
}


void message::set_recursive_message(bool state) const
{
    f_recursive_message = state;
}


severity_t message::get_severity() const
{
    return f_severity;
}


timespec const & message::get_timestamp() const
{
    return f_timestamp;
}


std::string const & message::get_filename() const
{
    return f_filename;
}


std::string const & message::get_function() const
{
    return f_funcname;
}


int message::get_line() const
{
    return f_line;
}


bool message::get_recursive_message() const
{
    return f_recursive_message;
}


environment::pointer_t message::get_environment() const
{
    return f_environment;
}





std::string message::get_message() const
{
    std::string s(str());
    if(s.back() == '\n')
    {
        s.pop_back();
    }
    if(s.back() == '\r')
    {
        s.pop_back();
    }
    return s;
}




} // snaplogger namespace
// vim: ts=4 sw=4 et
