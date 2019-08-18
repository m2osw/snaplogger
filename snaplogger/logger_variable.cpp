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
 * \brief Implementation of the various logger variable support.
 *
 * This file implements the few logger variables which retrieves their
 * value from the message variable.
 */


// self
//
#include    "snaplogger/exception.h"
#include    "snaplogger/format.h"
#include    "snaplogger/map_diagnostic.h"
#include    "snaplogger/nested_diagnostic.h"
#include    "snaplogger/variable.h"


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



DEFINE_LOGGER_VARIABLE(severity)
{
    severity_t sev(msg.get_severity());
    severity::pointer_t severity(get_severity(msg, sev));
    if(severity == nullptr)
    {
        // not found, write value directly
        //
        value += std::to_string(static_cast<int>(sev));
        variable::process_value(msg, value);
        return;
    }

    auto params(get_params());
    if(!params.empty())
    {
        if(params[0]->get_name() == "format")
        {
            auto v(params[0]->get_value());
            if(v != "alpha")
            {
                if(v == "number")
                {
                    value += std::to_string(static_cast<int>(sev));
                    variable::process_value(msg, value);
                    return;
                }
                throw invalid_variable(
                              "the ${severity:format=alpha|number} variable cannot be set to \""
                            + v
                            + "\".");
            }
        }
    }

    value += severity->get_description();

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(message)
{
    if(msg.get_recursive_message())
    {
        // do nothing if we find a ${message} inside the message itself
        return;
    }

    std::string const m(msg.get_message());
    if(m.find("${") == std::string::npos)
    {
        value += m;
    }
    else
    {
        msg.set_recursive_message(true);
        format f(m);
        value += f.process_message(msg);
        msg.set_recursive_message(false);
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(project_name)
{
    // when the advgetopt is properly connected to the logger, then the
    // logger will save the program name in its diagnostic map
    //
    map_diagnostics_t map(get_map_diagnostics(msg));
    auto it(map.find(DIAG_KEY_PROJECT_NAME));
    if(it != map.end())
    {
        value += it->second;
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(progname)
{
    // when the advgetopt is properly connected to the logger, then the
    // logger will save the program name in its diagnostic map
    //
    map_diagnostics_t map(get_map_diagnostics(msg));
    auto it(map.find(DIAG_KEY_PROGNAME));
    if(it != map.end())
    {
        value += it->second;
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(version)
{
    // when the advgetopt is properly connected to the logger, then the
    // logger will save the program name in its diagnostic map
    //
    map_diagnostics_t map(get_map_diagnostics(msg));
    auto it(map.find(DIAG_KEY_VERSION));
    if(it != map.end())
    {
        value += it->second;
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(build_date)
{
    // when the advgetopt is properly connected to the logger, then the
    // logger will save the program name in its diagnostic map
    //
    map_diagnostics_t map(get_map_diagnostics(msg));
    auto it(map.find(DIAG_KEY_BUILD_DATE));
    if(it != map.end())
    {
        value += it->second;
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(build_time)
{
    // when the advgetopt is properly connected to the logger, then the
    // logger will save the program name in its diagnostic map
    //
    map_diagnostics_t map(get_map_diagnostics(msg));
    auto it(map.find(DIAG_KEY_BUILD_TIME));
    if(it != map.end())
    {
        value += it->second;
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(filename)
{
    value += msg.get_filename();

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(basename)
{
    std::string const filename(msg.get_filename());
    std::string::size_type const pos(filename.rfind('/'));
    if(pos == std::string::npos)
    {
        value += filename;
    }
    else
    {
        value += filename.substr(pos + 1);
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(path)
{
    std::string const filename(msg.get_filename());
    std::string::size_type const pos(filename.rfind('/'));
    if(pos != std::string::npos)
    {
        value += filename.substr(0, pos);
    }

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(function)
{
    value += msg.get_function();

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(line)
{
    value += std::to_string(msg.get_line());

    variable::process_value(msg, value);
}



DEFINE_LOGGER_VARIABLE(diagnostic)
{
    constexpr int FLAG_NESTED = 0x01;
    constexpr int FLAG_MAP    = 0x02;

    std::int64_t nested_depth(-1);
    std::string key;
    int flags(0);

    for(auto p : get_params())
    {
        if(p->get_name() == "nested")
        {
            nested_depth = p->get_integer();
            flags |= FLAG_NESTED;
        }
        else if(p->get_name() == "map")
        {
            key = p->get_value();
            flags |= FLAG_MAP;
        }
    }

    if(flags == 0
    || (flags & FLAG_NESTED) != 0)
    {
        char sep('{');
        string_vector_t nested(get_nested_diagnostics(msg));
        size_t idx(0);
        if((flags & FLAG_NESTED) != 0
        && static_cast<ssize_t>(nested.size()) > nested_depth)
        {
            idx = nested.size() - nested_depth;
            value += sep;
            value += "...";
            sep = '/';
        }

        for(; idx < nested.size(); ++idx)
        {
            value += sep;
            sep = '/';
            value += nested[idx];
        }
        value += "}";
    }

    if(flags == 0
    || (flags & FLAG_MAP) != 0)
    {
        auto const diagnostics(get_map_diagnostics(msg));
        if(!diagnostics.empty())
        {
            if(key.empty())
            {
                char sep('<');
                for(auto d : diagnostics)
                {
                    value += sep;
                    sep = ':';
                    value += d.first;
                    value += '=';
                    value += d.second;
                }
                value += '>';
            }
            else
            {
                auto it(diagnostics.find(key));
                if(it != diagnostics.end())
                {
                    value += '<';
                    value += key;
                    value += '=';
                    value += it->second;
                    value += '>';
                }
            }
        }
    }

    variable::process_value(msg, value);
}


}
// no name namespace


} // snaplogger namespace
// vim: ts=4 sw=4 et
