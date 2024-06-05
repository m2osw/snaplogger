// Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Severity levels for your log messages.
 *
 * The severity implementation loads the severity configuration file
 * and generates a set of severity levels that one can attach to
 * log messages.
 */

// self
//
#include    <snaplogger/utils.h>



// snapdev
//
#include    <snapdev/version.h>


// C++
//
#include    <memory>


// C
//
#include    <sys/time.h>



namespace snaplogger
{


class message;


enum class severity_t
{
    SEVERITY_ALL                   = 0,
    SEVERITY_TRACE                 = 10,
    SEVERITY_NOISY                 = 20,
    SEVERITY_DEBUG                 = 30,
    SEVERITY_NOTICE                = 40,
    SEVERITY_UNIMPORTANT           = 50,
    SEVERITY_VERBOSE               = 60,
    SEVERITY_CONFIGURATION         = 70,
    SEVERITY_CONFIGURATION_WARNING = 80,
    SEVERITY_INFORMATION           = 90,
    SEVERITY_IMPORTANT             = 100,
    SEVERITY_MINOR                 = 110,
    SEVERITY_TODO                  = 120,
    SEVERITY_DEPRECATED            = 130,
    SEVERITY_WARNING               = 140,
    SEVERITY_MAJOR                 = 150,
    SEVERITY_RECOVERABLE_ERROR     = 160,
    SEVERITY_ERROR                 = 170,
    SEVERITY_NOISY_ERROR           = 180,  // from here above is usually the threshold for errors to report to the admin
    SEVERITY_SEVERE                = 190,
    SEVERITY_EXCEPTION             = 200,
    SEVERITY_CRITICAL              = 210,
    SEVERITY_ALERT                 = 220,
    SEVERITY_EMERGENCY             = 230,
    SEVERITY_FATAL                 = 250,
    SEVERITY_OFF                   = 255,

    SEVERITY_DEFAULT = SEVERITY_INFORMATION,        // WARNING: can dynamically be changed using the severity.ini file
    SEVERITY_MIN = SEVERITY_ALL,
    SEVERITY_MAX = SEVERITY_OFF
};


typedef std::vector<severity_t>     severity_array_t;




class severity
{
public:
    typedef std::shared_ptr<severity>           pointer_t;

                        severity(severity_t sev, std::string const & name, bool system = false);

    severity_t          get_severity() const;
    bool                is_system() const;
    void                mark_as_registered();
    bool                is_registered() const;

    std::string         get_name() const;
    void                add_alias(std::string const & name);
    string_vector_t     get_all_names() const;

    void                set_description(std::string const & description);
    std::string         get_description() const;

    void                set_styles(std::string const & styles);
    std::string         get_styles() const;

private:
    severity_t const    f_severity;
    string_vector_t     f_names = string_vector_t();
    bool const          f_system;
    bool                f_registered = false;
    std::string         f_description = std::string();
    std::string         f_styles = std::string();
};

typedef std::map<severity_t, severity::pointer_t>   severity_by_severity_t;
typedef std::map<std::string, severity::pointer_t>  severity_by_name_t;

void                    add_severity(severity::pointer_t sev);
severity::pointer_t     get_severity(std::string const & name);
severity::pointer_t     get_severity(message const & msg, std::string const & name);
severity::pointer_t     get_severity(severity_t sev);
severity::pointer_t     get_severity(message const & msg, severity_t sev);
severity_by_name_t      get_severities_by_name();
severity_by_severity_t  get_severities_by_severity();

template<typename CharT, typename Traits>
inline std::basic_ostream<CharT, Traits> &
operator << (std::basic_ostream<CharT, Traits> & os, severity_t sev)
{
    severity::pointer_t s(get_severity(sev));
    if(s == nullptr)
    {
        os << "(unknown severity: "
           << static_cast<int>(sev)
           << ")";
    }
    else
    {
        os << s->get_name();
    }
    return os;
}



} // snaplogger namespace


// outside of namespace so it can be used right up
//
#if SNAPDEV_CHECK_GCC_VERSION(7, 5, 0)
snaplogger::severity::pointer_t     operator ""_sev (char const * name, unsigned long size);
#endif


// vim: ts=4 sw=4 et
