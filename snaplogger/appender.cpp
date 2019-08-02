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

#include    <snapdev/empty_set_intersection.h>

// self
//
#include    "snaplogger/appender.h"
#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"


// snapdev lib
//
#include    <snapdev/not_used.h>


// C++ lib
//
#include    <iostream>


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


// if we want to be able to reference such we need to create it TBD
// (and it should probably be in the null_appender.cpp file instead)
//APPENDER_FACTORY(null);


}


appender::appender(std::string const & name, std::string const & type)
    : f_type(type)
    , f_name(name)
{
    guard g;

    if(g_default_format == nullptr)
    {
        g_default_format = std::make_shared<format>(
            //"${env:name=HOME:padding='-':align=center:exact_width=6} "
            "${date} ${time} ${hostname}"
            " ${progname}[${pid}]: ${severity}:"
            " ${message:escape:max_width=1000}"
            " (in function \"${function}()\")"
            " (${basename}:${line})"
        );
    }

    f_format = g_default_format;
}


appender::~appender()
{
}


std::string const & appender::get_type() const
{
    return f_type;
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


bool appender::unique() const
{
    return false;
}


severity_t appender::get_severity() const
{
    guard g;

    return f_severity;
}


void appender::set_severity(severity_t severity_level)
{
    guard g;

    f_severity = severity_level;
}


void appender::reduce_severity(severity_t severity_level)
{
    guard g;

    if(severity_level < f_severity)
    {
        set_severity(severity_level);
    }
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

    // COMPONENTS
    //
    std::string comp;
    std::string const components(f_name + "::components");
    if(opts.is_defined(components))
    {
        comp = opts.get_string(components);
    }
    else if(opts.is_defined("components"))
    {
        comp = opts.get_string("components");
    }
    if(comp.empty())
    {
        add_component(normal_component);
    }
    else
    {
        advgetopt::string_list_t component_names;
        advgetopt::split_string(comp, component_names, {","});
        for(auto name : component_names)
        {
            add_component(component::get_component(name));
        }
    }

    // FILTER
    //
    {
        std::string filter;
        std::string const specialized_filter(f_name + "::filter");
        if(opts.is_defined(specialized_filter))
        {
            filter = opts.get_string(specialized_filter);
        }
        else if(opts.is_defined("filter"))
        {
            filter = opts.get_string("filter");
        }
        if(!filter.empty())
        {
            std::regex_constants::syntax_option_type flags(std::regex::nosubs | std::regex::optimize);
            std::regex_constants::syntax_option_type type(std::regex::extended);
            if(filter[0] == '/')
            {
                std::string::size_type pos(filter.rfind('/'));
                if(pos == 0)
                {
                    throw invalid_variable(
                                  "invalid filter \""
                                + filter
                                + "\"; missing ending '/'.");
                }
                std::string const flag_list(filter.substr(pos + 1));
                filter = filter.substr(1, pos - 2);
                if(filter.empty())
                {
                    throw invalid_variable(
                                  "invalid filter \""
                                + filter
                                + "\"; the regular expression is empty.");
                }
                // TODO: for errors we would need to iterate using the libutf8
                //       (since we could have a Unicode character after the /)
                //
                // TODO: if two type flags are found, err too
                //
                int count(0);
                for(auto f : flag_list)
                {
                    switch(f)
                    {
                    case 'i':
                        flags |= std::regex::icase;
                        break;

                    case 'c':
                        flags |= std::regex::collate;
                        break;

                    case 'j':
                        type = std::regex::ECMAScript;
                        ++count;
                        break;

                    case 'b':
                        type = std::regex::basic;
                        ++count;
                        break;

                    case 'x':
                        type = std::regex::extended;
                        ++count;
                        break;

                    case 'a':
                        type = std::regex::awk;
                        ++count;
                        break;

                    case 'g':
                        type = std::regex::grep;
                        ++count;
                        break;

                    case 'e':
                        type = std::regex::egrep;
                        ++count;
                        break;

                    default:
                        throw invalid_variable(
                                      "in \""
                                    + filter
                                    + "\", found invalid flag '"
                                    + f
                                    + "'.");

                    }
                    if(count > 1)
                    {
                        throw invalid_variable(
                                      "found multiple types in \""
                                    + filter
                                    + "\".");
                    }
                }
            }
            f_filter = std::make_shared<std::regex>(filter, flags | type);
        }
    }
}


void appender::add_component(component::pointer_t comp)
{
    f_components.insert(comp);
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

    component::set_t const & components(msg.get_components());
    if(components.empty())
    {
        // user did not supply any component in 'msg', check for
        // the normal component
        //
        if(f_components.find(normal_component) == f_components.end())
        {
            return;
        }
    }
    else
    {
        if(snap::empty_set_intersection(f_components, components))
        {
            return;
        }
    }

    std::string formatted_message(f_format->process_message(msg));
    if(formatted_message.empty())
    {
        return;
    }

    if(f_filter != nullptr
    && !std::regex_match(formatted_message, *f_filter))
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
