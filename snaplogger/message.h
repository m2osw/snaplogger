/*
 * Copyright (c) 2013-2021  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Handle the message generator.
 *
 * This file declares the base message class which is derived from an
 * std::stringstram. This allows you to use our logger with `<<`
 * to send anything that the `<<` operator understands to the logs.
 */


// self
//
#include    "snaplogger/component.h"
#include    "snaplogger/environment.h"
#include    "snaplogger/severity.h"


// advgetopt lib
//
#include    <advgetopt/advgetopt.h>


// C++ lib
//
#include    <sstream>
#include    <streambuf>


// C lib
//
#include    <sys/time.h>



namespace snaplogger
{


class logger;



class null_buffer
    : public std::streambuf
{
public:
    typedef std::unique_ptr<null_buffer>    pointer_t;

    virtual int         overflow(int c) override;
};


// the message class is final because the destructor does tricks which
// would not work right if derived further
//
// also we do not offer a shared pointer because we expect the message
// object to be created and immediately destroyed from the stack
//
class message final
    : public std::basic_stringstream<char>
{
public:
    typedef std::shared_ptr<message>            pointer_t;
    typedef std::map<std::string, std::string>  field_map_t;

                                message(
                                          severity_t sev = default_severity()
                                        , char const * file = nullptr
                                        , char const * func = nullptr
                                        , int line = -1);
                                message(std::basic_stringstream<char> const & m, message const & msg);
                                message(message const & rhs) = delete;
    virtual                     ~message();

    message &                   operator = (message const & rhs) = delete;

    static severity_t           default_severity();

    void                        set_severity(severity_t severity);
    void                        set_filename(std::string const & filename);
    void                        set_function(std::string const & funcname);
    void                        set_line(int line);
    void                        set_recursive_message(bool state) const;
    void                        add_component(component::pointer_t c);
    void                        add_field(std::string const & name, std::string const & value);

    std::shared_ptr<logger>     get_logger() const;
    severity_t                  get_severity() const;
    void                        set_precise_time();
    timespec const &            get_timestamp() const;
    std::string const &         get_filename() const;
    std::string const &         get_function() const;
    int                         get_line() const;
    bool                        get_recursive_message() const;
    component::set_t const &    get_components() const;
    environment::pointer_t      get_environment() const;
    std::string                 get_message() const;
    std::string                 get_field(std::string const & name) const;
    field_map_t                 get_fields() const;

private:
    std::shared_ptr<logger>     f_logger = std::shared_ptr<logger>(); // make sure it does not go away under our feet
    timespec                    f_timestamp = timespec();
    severity_t                  f_severity = severity_t::SEVERITY_INFORMATION;
    std::string                 f_filename = std::string();
    std::string                 f_funcname = std::string();
    int                         f_line = 0;
    mutable bool                f_recursive_message = false;
    environment::pointer_t      f_environment = environment::pointer_t();
    component::set_t            f_components = component::set_t();
    field_map_t                 f_fields = field_map_t();
    null_buffer::pointer_t      f_null = null_buffer::pointer_t();
    std::streambuf *            f_saved_buffer = nullptr;
    bool                        f_copy = false;
};


template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
operator << (std::basic_ostream<CharT, Traits> & os, section_ptr sec)
{
    message * m(dynamic_cast<message *>(&os));
    if(m == nullptr)
    {
        os << "(section:"
           << sec.f_component->get_name()
           << ")";
    }
    else
    {
        m->add_component(sec.f_component);
    }
    return os;
}


template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
secure(std::basic_ostream<CharT, Traits> & os)
{
    message * m(dynamic_cast<message *>(&os));
    if(m == nullptr)
    {
        os << "(section:secure)";
    }
    else
    {
        m->add_component(g_secure_component);
    }
    return os;
}


template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
precise_time(std::basic_ostream<CharT, Traits> & os)
{
    message * m(dynamic_cast<message *>(&os));
    if(m == nullptr)
    {
        m->set_precise_time();
    }
    return os;
}



struct field_t
{
    std::string             f_name;
    std::string             f_value;
};

inline field_t field(std::string const & name, std::string const & value)
{
    return { name, value };
}

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
operator << (std::basic_ostream<CharT, Traits> & os, field_t const & f)
{
    message * m(dynamic_cast<message *>(&os));
    if(m != nullptr)
    {
        m->add_field(f.f_name, f.f_value);
    }
    return os;
}



void send_message(std::basic_ostream<char> & msg);


// TODO: with C++20, change the __FILE__, __LINE__, __func__ with
//       std::location::current()
//
#define SNAP_LOG_FATAL              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_FATAL,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_EMERG              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_EMERGENCY,         __FILE__, __func__, __LINE__)
#define SNAP_LOG_EMERGENCY          ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_EMERGENCY,         __FILE__, __func__, __LINE__)
#define SNAP_LOG_ALERT              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_ALERT,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_CRIT               ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_CRITICAL,          __FILE__, __func__, __LINE__)
#define SNAP_LOG_CRITICAL           ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_CRITICAL,          __FILE__, __func__, __LINE__)
#define SNAP_LOG_ERR                ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_ERROR,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_ERROR              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_ERROR,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_RECOVERABLE_ERROR  ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_RECOVERABLE_ERROR, __FILE__, __func__, __LINE__)
#define SNAP_LOG_MAJOR              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_MAJOR,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_WARN               ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_WARNING,           __FILE__, __func__, __LINE__)
#define SNAP_LOG_WARNING            ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_WARNING,           __FILE__, __func__, __LINE__)
#define SNAP_LOG_DEPRECATED         ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_DEPRECATED,        __FILE__, __func__, __LINE__)
#define SNAP_LOG_MINOR              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_MINOR,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_IMPORTANT          ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_IMPORTANT,         __FILE__, __func__, __LINE__)
#define SNAP_LOG_INFO               ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_INFORMATION,       __FILE__, __func__, __LINE__)
#define SNAP_LOG_INFORMATION        ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_INFORMATION,       __FILE__, __func__, __LINE__)
#define SNAP_LOG_VERBOSE            ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_VERBOSE,           __FILE__, __func__, __LINE__)
#define SNAP_LOG_UNIMPORTANT        ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_UNIMPORTANT,       __FILE__, __func__, __LINE__)
#define SNAP_LOG_NOTICE             ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_NOTICE,            __FILE__, __func__, __LINE__)
#define SNAP_LOG_DEBUG              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_DEBUG,             __FILE__, __func__, __LINE__)
#define SNAP_LOG_TRACE              ::snaplogger::send_message((::snaplogger::message(::snaplogger::severity_t::SEVERITY_TRACE,             __FILE__, __func__, __LINE__)

#define SNAP_LOG_DEFAULT            ::snaplogger::send_message((::snaplogger::message(::snaplogger::message::default_severity(),            __FILE__, __func__, __LINE__)

#define SNAP_LOG_FIELD(name, value) ::snaplogger::field((name), (value))

// The (( are in the opening macros
//
#define SNAP_LOG_SEND               ""))
#define SNAP_LOG_SEND_SECURELY      ::snaplogger::secure))



} // snaplogger namespace
// vim: ts=4 sw=4 et
