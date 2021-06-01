// Copyright (c) 2013-2021  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
// 
#include    "snaplogger/message.h"

#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
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


namespace
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
constexpr char const * const g_system_field_names[] =
{
    [static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_MESSAGE      )] = "_message",
    [static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_TIMESTAMP    )] = "_timestamp",
    [static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_SEVERITY     )] = "_severity",
    [static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_FILENAME     )] = "_filename",
    [static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_FUNCTION_NAME)] = "_function_name",
    [static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_LINE         )] = "_line",
};
#pragma GCC diagnostic pop


int         g_message_id = 0;


int get_next_id()
{
    guard g;

    ++g_message_id;
    if(g_message_id == 0)
    {
        // never use 0 as the id; of course, it's very unlikely that this happens
        g_message_id = 1;
    }
    return g_message_id;
}



}
// no name namespace




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

    add_field("id", std::to_string(get_next_id()));

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
    , f_fields(msg.f_fields)
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


severity_t message::default_severity()
{
    return logger::get_instance()->get_default_severity();
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


bool message::can_add_component(component::pointer_t c) const
{
    if(c != nullptr)
    {
        return !c->is_mutually_exclusive(f_components);
    }

    return false;
}


void message::add_component(component::pointer_t c)
{
    if(c != nullptr)
    {
        if(!can_add_component(c))
        {
            throw duplicate_error(
                  "component \""
                + c->get_name()
                + "\" cannot be added to this message as it is mutually exclusive with one or more of the other components that were added to this message.");
        }

        f_components.insert(c);
    }
}


void message::add_field(std::string const & name, std::string const & value)
{
    if(!name.empty())
    {
        if(name[0] == '_')
        {
            throw invalid_parameter(
                  "field name \""
                + name
                + "\" is a system name (whether reserved or already defined) and as such is read-only."
                  " Do not start your field names with an underscore.");
        }

        f_fields[name] = value;
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


void message::set_precise_time()
{
    clock_gettime(CLOCK_REALTIME, &f_timestamp);
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


bool message::has_component(component::pointer_t c) const
{
    return f_components.find(c) != f_components.end();
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

    if(!s.empty()
    && s.back() == '\n')
    {
        s.pop_back();
    }

    if(!s.empty()
    && s.back() == '\r')
    {
        s.pop_back();
    }

    return s;
}


char const * message::get_system_field_name(system_field_t field)
{
    std::size_t const idx(static_cast<std::size_t>(field));
    if(idx >= static_cast<std::size_t>(system_field_t::SYSTEM_FIELD_max))
    {
        return "_unknown";
    }
    return g_system_field_names[idx];
}


system_field_t message::get_system_field_from_name(std::string const & name)
{
    if(name.length() >= 2
    && name[0] == '_')
    {
        switch(name[1])
        {
        case 'f':
            if(name == "_filename")
            {
                return system_field_t::SYSTEM_FIELD_FILENAME;
            }
            if(name == "_function_name")
            {
                return system_field_t::SYSTEM_FIELD_FUNCTION_NAME;
            }
            break;

        case 'l':
            if(name == "_line")
            {
                return system_field_t::SYSTEM_FIELD_LINE;
            }
            break;

        case 'm':
            if(name == "_message")
            {
                return system_field_t::SYSTEM_FIELD_MESSAGE;
            }
            break;

        case 's':
            if(name == "_severity")
            {
                return system_field_t::SYSTEM_FIELD_SEVERITY;
            }
            break;

        case 't':
            if(name == "_timestamp")
            {
                return system_field_t::SYSTEM_FIELD_TIMESTAMP;
            }
            break;

        }
    }

    return system_field_t::SYSTEM_FIELD_UNDEFINED;
}


std::string message::get_field(std::string const & name) const
{
    if(!name.empty()
    && name[0] == '_')
    {
        switch(get_system_field_from_name(name))
        {
        case system_field_t::SYSTEM_FIELD_MESSAGE:
            return get_message();

        case system_field_t::SYSTEM_FIELD_TIMESTAMP:
            // TODO: offer ways to get the date & time converted to strings
            {
                std::string timestamp(std::to_string(f_timestamp.tv_sec));
                if(f_timestamp.tv_nsec != 0)
                {
                    std::string nsec(std::to_string(f_timestamp.tv_nsec));
                    while(nsec.length() < 9)
                    {
                        nsec = '0' + nsec;
                    }
                    while(nsec.back() == '0')
                    {
                        nsec.pop_back();
                    }
                    timestamp += '.';
                    timestamp += nsec;
                }
                return timestamp;
            }

        case system_field_t::SYSTEM_FIELD_SEVERITY:
            {
                severity::pointer_t sev(snaplogger::get_severity(f_severity));
                return sev == nullptr ? "<unknown>" : sev->get_name();
            }

        case system_field_t::SYSTEM_FIELD_FILENAME:
            return f_filename;

        case system_field_t::SYSTEM_FIELD_FUNCTION_NAME:
            return f_funcname;

        case system_field_t::SYSTEM_FIELD_LINE:
            return std::to_string(f_line);

        default:
            return std::string();

        }
    }

    auto it(f_fields.find(name));
    if(it == f_fields.end())
    {
        return std::string();
    }
    return it->second;
}


message::field_map_t message::get_fields() const
{
    return f_fields;
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
