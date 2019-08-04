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
 * \brief Severity levels for your log messages.
 *
 * The severity implementation loads the severity configuration file
 * and generates a set of severity levels that one can attach to
 * log messages.
 */

// self
//
#include    "snaplogger/utils.h"


// C++ lib
//
#include    <memory>


// C lib
//
#include    <sys/time.h>



namespace snaplogger
{


class message;


enum class severity_t
{
    SEVERITY_ALL                = 0,
    SEVERITY_TRACE              = 10,
    SEVERITY_DEBUG              = 20,
    SEVERITY_NOTICE             = 30,
    SEVERITY_UNIMPORTANT        = 40,
    SEVERITY_INFORMATION        = 50,
    SEVERITY_IMPORTANT          = 60,
    SEVERITY_MINOR              = 70,
    SEVERITY_DEPRECATED         = 80,
    SEVERITY_WARNING            = 100,
    SEVERITY_MAJOR              = 150,
    SEVERITY_RECOVERABLE_ERROR  = 190,
    SEVERITY_ERROR              = 200,
    SEVERITY_CRITICAL           = 210,
    SEVERITY_ALERT              = 220,
    SEVERITY_EMERGENCY          = 230,
    SEVERITY_FATAL              = 250,
    SEVERITY_OFF                = 255,

    SEVERITY_DEFAULT = SEVERITY_INFORMATION,
    SEVERITY_MIN = SEVERITY_ALL,
    SEVERITY_MAX = SEVERITY_OFF
};




class severity
{
public:
    typedef std::shared_ptr<severity>           pointer_t;

                        severity(severity_t sev, std::string const & name, bool system = false);

    severity_t          get_severity() const;
    bool                is_system() const;

    std::string         get_name() const;
    string_vector_t     get_all_names() const;

    void                add_alias(std::string const & name);
    string_vector_t     get_aliases() const;

    void                set_description(std::string const & description);
    std::string         get_description() const;

    void                set_styles(std::string const & color);
    std::string         get_styles() const;

private:
    severity_t const    f_severity;
    string_vector_t     f_names = string_vector_t();
    bool const          f_system;
    std::string         f_description = std::string();
    std::string         f_styles = std::string();
};


void                    add_severity(severity::pointer_t sev);
severity::pointer_t     get_severity(std::string const & name);
severity::pointer_t     get_severity(message const & msg, std::string const & name);
severity::pointer_t     get_severity(severity_t sev);
severity::pointer_t     get_severity(message const & msg, severity_t sev);




} // snaplogger namespace
// vim: ts=4 sw=4 et
