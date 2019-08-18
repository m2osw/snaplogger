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

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
// 
#include    "snaplogger/message.h"

#include    "snaplogger/exception.h"
#include    "snaplogger/logger.h"


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
    : f_logger(logger::get_instance())
    , f_severity(sev)
    , f_filename(file == nullptr ? std::string() : std::string(file))
    , f_funcname(func == nullptr ? std::string() : std::string(func))
    , f_line(line)
    , f_environment(create_environment())
{
    clock_gettime(CLOCK_REALTIME_COARSE, &f_timestamp);

    if(f_severity < f_logger->get_lowest_severity()
    || f_severity == severity_t::SEVERITY_OFF)
    {
        f_null.reset(new null_buffer);
        std::ostream & ref = *this;
        f_saved_buffer = ref.rdbuf(f_null.get());
    }
}


message::message(std::basic_stringstream<char> const & m, message const & msg)
    : f_logger(msg.f_logger)
    , f_timestamp(msg.f_timestamp)
    , f_severity(msg.f_severity)
    , f_filename(msg.f_filename)
    , f_funcname(msg.f_funcname)
    , f_line(msg.f_line)
    , f_recursive_message(msg.f_recursive_message)
    , f_environment(msg.f_environment)
    , f_components(msg.f_components)
    , f_null(null_buffer::pointer_t())
    , f_saved_buffer(nullptr)
    , f_copy(true)
{
    *this << m.rdbuf();
}


message::~message()
{
    if(f_saved_buffer != nullptr)
    {
        std::ostream & ref = *this;
        ref.rdbuf(f_saved_buffer);
    }
}


void message::set_severity(severity_t severity)
{
    f_severity = severity;
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


void message::add_component(component::pointer_t c)
{
    if(c != nullptr)
    {
        f_components.insert(c);
    }
}


std::shared_ptr<logger> message::get_logger() const
{
    return f_logger;
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


component::set_t const & message::get_components() const
{
    return f_components;
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


void send_message(std::basic_ostream<char> & out)
{
    message * msg(dynamic_cast<message *>(&out));
    if(msg == nullptr)
    {
        throw not_a_message("the 'out' parameter to the send_message() function is expected to be a snaplogger::message object.");
    }

    logger::get_instance()->log_message(*msg);
}




} // snaplogger namespace
// vim: ts=4 sw=4 et
