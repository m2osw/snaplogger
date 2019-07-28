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
#include    "snaplogger/severity.h"


// advgetopt lib
//
#include    <advgetopt/advgetopt.h>


// C++ lib
//
#include    <streambuf>


// C lib
//
#include    <sys/time.h>



namespace snaplogger
{


class component;


class null_buffer
    : public std::streambuf
{
public:
    virtual int         overflow(int c) override;
};


// the message class is final because the destructor does tricks which
// would not work right if derived further
//
// also we do not offer a shared pointer because we expect the message
// object to be created and immediately destroyed from the stack
//
class message final
    : public std::stringstream
{
public:
                        message(
                                  severity_t sev = severity_t::SEVERITY_INFORMATION
                                , char const * file = nullptr
                                , char const * func = nullptr
                                , int line = -1);
    virtual             ~message();

    void                set_severity(severity_t severity);
    void                set_filename(std::string const & filename);
    void                set_function(std::string const & funcname);
    void                set_line(int line);
    void                add_component(component const & c);

    severity_t          get_severity() const;
    timespec const &    get_timestamp() const;
    std::string const & get_filename() const;
    std::string const & get_function() const;
    int                 get_line() const;
    std::string         get_message() const;

private:
    timespec            f_timestamp = timespec();
    severity_t          f_severity = severity_t::SEVERITY_INFORMATION;
    std::string         f_filename = std::string();
    std::string         f_funcname = std::string();
    int                 f_line = 0;
};





#define    SNAP_LOG_FATAL               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_FATAL,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_EMERG               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_EMERGENCY,         __FILE__, __func__, __LINE__)
#define    SNAP_LOG_EMERGENCY           ::snaplogger::message(::snaplogger::severity_t::SEVERITY_EMERGENCY,         __FILE__, __func__, __LINE__)
#define    SNAP_LOG_ALERT               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_ALERT,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_CRIT                ::snaplogger::message(::snaplogger::severity_t::SEVERITY_CRITICAL,          __FILE__, __func__, __LINE__)
#define    SNAP_LOG_CRITICAL            ::snaplogger::message(::snaplogger::severity_t::SEVERITY_CRITICAL,          __FILE__, __func__, __LINE__)
#define    SNAP_LOG_ERR                 ::snaplogger::message(::snaplogger::severity_t::SEVERITY_ERROR,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_ERROR               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_ERROR,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_RECOVERABLE_ERROR   ::snaplogger::message(::snaplogger::severity_t::SEVERITY_RECOVERABLE_ERROR, __FILE__, __func__, __LINE__)
#define    SNAP_LOG_MAJOR               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_MAJOR,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_WARN                ::snaplogger::message(::snaplogger::severity_t::SEVERITY_WARNING,           __FILE__, __func__, __LINE__)
#define    SNAP_LOG_WARNING             ::snaplogger::message(::snaplogger::severity_t::SEVERITY_WARNING,           __FILE__, __func__, __LINE__)
#define    SNAP_LOG_DEPRECATED          ::snaplogger::message(::snaplogger::severity_t::SEVERITY_DEPRECATED,        __FILE__, __func__, __LINE__)
#define    SNAP_LOG_MINOR               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_MINOR,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_IMPORTANT           ::snaplogger::message(::snaplogger::severity_t::SEVERITY_IMPORTANT,         __FILE__, __func__, __LINE__)
#define    SNAP_LOG_INFO                ::snaplogger::message(::snaplogger::severity_t::SEVERITY_INFORMATION,       __FILE__, __func__, __LINE__)
#define    SNAP_LOG_INFORMATION         ::snaplogger::message(::snaplogger::severity_t::SEVERITY_INFORMATION,       __FILE__, __func__, __LINE__)
#define    SNAP_LOG_UNIMPORTANT         ::snaplogger::message(::snaplogger::severity_t::SEVERITY_UNIMPORTANT,       __FILE__, __func__, __LINE__)
#define    SNAP_LOG_NOTICE              ::snaplogger::message(::snaplogger::severity_t::SEVERITY_NOTICE,            __FILE__, __func__, __LINE__)
#define    SNAP_LOG_DEBUG               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_DEBUG,             __FILE__, __func__, __LINE__)
#define    SNAP_LOG_TRACE               ::snaplogger::message(::snaplogger::severity_t::SEVERITY_TRACE,             __FILE__, __func__, __LINE__)



} // snaplogger namespace
// vim: ts=4 sw=4 et
