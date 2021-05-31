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
#include    "snaplogger/syslog_appender.h"


// C++ lib
//
#include    <iostream>


// C lib
//
#include    <syslog.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{


APPENDER_FACTORY(syslog);


struct facility_by_name
{
    char const * const  f_name = nullptr;
    int const           f_facility = LOG_USER;
};


constexpr facility_by_name g_facility_by_name[] =
{
    { "auth",       LOG_AUTH     },
    { "authpriv",   LOG_AUTHPRIV },
    { "cron",       LOG_CRON     },
    { "daemon",     LOG_DAEMON   },
    { "ftp",        LOG_FTP      },
    { "kern",       LOG_KERN     },
    { "local0",     LOG_LOCAL0   },
    { "local1",     LOG_LOCAL1   },
    { "local2",     LOG_LOCAL2   },
    { "local3",     LOG_LOCAL3   },
    { "local4",     LOG_LOCAL4   },
    { "local5",     LOG_LOCAL5   },
    { "local6",     LOG_LOCAL6   },
    { "local7",     LOG_LOCAL7   },
    { "lpr",        LOG_LPR      },
    { "mail",       LOG_MAIL     },
    { "news",       LOG_NEWS     },
    { "syslog",     LOG_SYSLOG   },
    { "user",       LOG_USER     },
    { "uucp",       LOG_UUCP     }
};




}
// no name namespace



syslog_appender::syslog_appender(std::string const name)
    : appender(name, "syslog")
{
}


syslog_appender::~syslog_appender()
{
}


bool syslog_appender::unique() const
{
    return true;
}


void syslog_appender::set_config(advgetopt::getopt const & opts)
{
    closelog();

    appender::set_config(opts);

    // FACILITY
    //
    int facility(LOG_USER);
    std::string const facility_field(get_name() + "::facility");
    if(opts.is_defined(facility_field))
    {
        std::string const facility_name(opts.get_string(facility_field));
        int i(0);
        int j(sizeof(g_facility_by_name) / sizeof(g_facility_by_name[0]));
        while(i < j)
        {
            int const p(i + (j - i) / 2);
            int const r(facility_name.compare(g_facility_by_name[p].f_name));
            if(r == 0)
            {
                facility = g_facility_by_name[p].f_facility;
                break;
            }
            if(r < 0)
            {
                i = p + 1;
            }
            else // if(r > 0)
            {
                j = p;
            }
        }
    }

    // IDENTITY
    //
    std::string const identity_field(get_name() + "::identity");
    if(opts.is_defined(identity_field))
    {
        f_identity = opts.get_string(identity_field);
    }

    // FALLBACK TO CONSOLE
    //
    int options(LOG_NDELAY | LOG_PID);
    std::string const fallback_to_console_field(get_name() + "::fallback_to_console");
    if(opts.is_defined(fallback_to_console_field))
    {
        if(opts.get_string(fallback_to_console_field) == "true")
        {
            options |= LOG_CONS;
        }
    }

    // open the log immediately
    //
    openlog(f_identity.c_str(), options, facility);
}


void syslog_appender::process_message(message const & msg, std::string const & formatted_message)
{
    int priority(LOG_INFO);
    severity_t const sev(msg.get_severity());
    if(sev <= severity_t::SEVERITY_DEBUG)
    {
        priority = LOG_DEBUG;
    }
    else if(sev <= severity_t::SEVERITY_INFORMATION)
    {
        priority = LOG_INFO;
    }
    else if(sev <= severity_t::SEVERITY_MINOR)
    {
        priority = LOG_NOTICE;
    }
    else if(sev <= severity_t::SEVERITY_MAJOR)
    {
        priority = LOG_WARNING;
    }
    else if(sev <= severity_t::SEVERITY_ERROR)
    {
        priority = LOG_ERR;
    }
    else if(sev <= severity_t::SEVERITY_CRITICAL)
    {
        priority = LOG_CRIT;
    }
    else if(sev <= severity_t::SEVERITY_ALERT)
    {
        priority = LOG_ALERT;
    }
    else
    {
        priority = LOG_EMERG;
    }

    syslog(priority, "%s", formatted_message.c_str());
}





} // snaplogger namespace
// vim: ts=4 sw=4 et
