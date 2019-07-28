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
 * This file implements the base appender class.
 */

// self
//
#include    "snaplogger/appender.h"
#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"


// snapdev lib
//
#include    <snapdev/not_used.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


typedef std::map<std::string, appender_factory *>    appender_factory_t;

appender_factory_t      g_appender_types = appender_factory_t();


format::pointer_t       g_default_format = format::pointer_t();



}


appender::appender(std::string const & name)
    : f_name(name)
{
    guard g;

    if(g_default_format == nullptr)
    {
        g_default_format = std::make_shared<format>(
            //"${env:name=HOME:padding='-':align=center:exact_width=6} "
            "${date} ${time} ${hostname} ${progname}[${pid}]: ${severity}:"
            " ${message:escape} (in function \"${function}()\") (${basename}:${line})"
        );
    }

    f_format = g_default_format;
}


appender::~appender()
{
}


std::string const & appender::get_name() const
{
    return f_name;
}


bool appender::is_enabled() const
{
    return f_enabled;
}


void appender::set_enabled(bool status)
{
    guard g;

    f_enabled = status;
}


severity_t appender::get_severity() const
{
    return f_severity;
}


void appender::set_severity(severity_t severity_level)
{
    guard g;

    f_severity = severity_level;
}


void appender::set_config(advgetopt::getopt const & opts)
{
    guard g;

    // ENABLE
    //
    {
        std::string const specialized_enabled(f_name + "::enabled");
        if(opts.is_defined(specialized_enabled))
        {
            f_enabled = opts.get_string(specialized_enabled) != "false";
        }
        else if(opts.is_defined("enabled"))
        {
            f_enabled = opts.get_string("enabled") != "false";
        }
        else
        {
            f_enabled = true;
        }
    }

    // FORMAT
    //
    {
        std::string const specialized_format(f_name + "::format");
        if(opts.is_defined(specialized_format))
        {
            f_format = std::make_shared<format>(opts.get_string(specialized_format));
        }
        else if(opts.is_defined("format"))
        {
            f_format = std::make_shared<format>(opts.get_string("format"));
        }
    }

    // SEVERITY
    //
    f_severity = severity_t::SEVERITY_INFORMATION;
    std::string const specialized_severity(f_name + "::severity");
    if(opts.is_defined(specialized_severity))
    {
        std::string const severity_name(opts.get_string(specialized_severity));
        severity::pointer_t sev(snaplogger::get_severity(severity_name));
        if(sev != nullptr)
        {
            f_severity = sev->get_severity();
        }
        else
        {
            throw invalid_severity(
                          "severity level named \""
                        + severity_name
                        + "\" not found.");
        }
    }
    else if(opts.is_defined("severity"))
    {
        std::string const severity_name(opts.get_string("severity"));
        severity::pointer_t sev(snaplogger::get_severity(severity_name));
        if(sev != nullptr)
        {
            f_severity = sev->get_severity();
        }
        else
        {
            throw invalid_severity(
                          "severity level named \""
                        + severity_name
                        + "\" not found.");
        }
    }
}


format::pointer_t appender::set_format(format::pointer_t new_format)
{
    guard g;

    format::pointer_t old(f_format);
    f_format = new_format;
    return old;
}


void appender::send_message(message const & msg)
{
    if(msg.get_severity() < f_severity)
    {
        return;
    }

    std::string formatted_message(f_format->process_message(msg));
    if(formatted_message.empty())
    {
        return;
    }

    if(formatted_message.back() != '\n'
    && formatted_message.back() != '\r')
    {
        // TODO: add support to define line terminator (cr, nl, cr nl)
        //
        formatted_message += '\n';
    }

    process_message(msg, formatted_message);
}


void appender::process_message(message const & msg, std::string const & formatted_message)
{
    // the default is a "null appender" -- do nothing
    snap::NOTUSED(msg);
    snap::NOTUSED(formatted_message);
}





appender_factory::appender_factory(std::string const & type)
{
    if(g_appender_types.find(type) != g_appender_types.end())
    {
        throw duplicate_error(
                  "trying to register appender type \""
                + type
                + "\" twice won't work.");
    }

    g_appender_types[type] = this;
}


appender_factory::~appender_factory()
{
}



appender::pointer_t create_appender(std::string const & type, std::string const & name)
{
    auto it(g_appender_types.find(type));
    if(it != g_appender_types.end())
    {
        return it->second->create(name);
    }

    return appender::pointer_t();
}






safe_format::safe_format(appender::pointer_t a, format::pointer_t new_format)
    : f_appender(a)
    , f_old_format(a->set_format(new_format))
{
}


safe_format::~safe_format()
{
    snap::NOTUSED(f_appender->set_format(f_old_format));
}


} // snaplogger namespace
// vim: ts=4 sw=4 et
