// Copyright (c) 2013-2022  Made to Order Software Corp.  All Rights Reserved
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
 * This file implements the base appender class.
 */


// self
//
#include    "snaplogger/appender.h"

#include    "snaplogger/exception.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/private_logger.h"


// snapdev
//
#include    <snapdev/empty_set_intersection.h>
#include    <snapdev/not_used.h>


// C++
//
#include    <iostream>


// C lib
//
#include    <math.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



// if we want to be able to reference such we need to create it TBD
// (and it should probably be in the null_appender.cpp file instead)
//APPENDER_FACTORY(null);


}


appender::appender(std::string const & name, std::string const & type)
    : f_type(type)
    , f_name(name)
    , f_normal_component(get_component(COMPONENT_NORMAL))
{
    guard g;

    f_format = get_private_logger()->get_default_format();
}


appender::~appender()
{
}


std::string const & appender::get_type() const
{
    // we do not need to guard this one because we set it up on creation
    // and it can't be modified later
    //
    return f_type;
}


void appender::set_name(std::string const & name)
{
    guard g;

    if(f_name != "console"
    && f_name != "syslog")
    {
        throw invalid_parameter(
                  "the appender set_name() can only be used for the console & syslog appenders to rename them to your own appender name (and done internally only).");
    }

    f_name = name;
}


std::string const & appender::get_name() const
{
    guard g;

    return f_name;
}


bool appender::is_enabled() const
{
    guard g;

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
    logger::get_instance()->severity_changed(severity_level);
}


void appender::reduce_severity(severity_t severity_level)
{
    guard g;

    if(severity_level < f_severity)
    {
        set_severity(severity_level);
    }
}


bool appender::operator < (appender const & rhs) const
{
    guard g;

    return f_severity < rhs.f_severity;
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

    // BITRATE
    //
    {
        // the input is considered to be in mbps
        //
        std::string bitrate("0");
        std::string const specialized_bitrate(f_name + "::bitrate");
        if(opts.is_defined(specialized_bitrate))
        {
            bitrate = opts.get_string(specialized_bitrate);
        }
        else if(opts.is_defined("bitrate"))
        {
            bitrate = opts.get_string("bitrate");
        }
        char * end(nullptr);
        errno = 0;
        double rate(strtod(bitrate.c_str(), &end));
        if(rate < 0.0
        || end == nullptr
        || *end != '\0'
        || errno == ERANGE)
        {
            rate = 0.0;
        }
        if(rate > 0.0)
        {
            // transform the rate to bytes per minute
            //
            rate = rate * (60.0 * 1'000'000.0 / 8.0);
        }
        f_bytes_per_minute = static_cast<decltype(f_bytes_per_minute)>(floor(rate));
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
            set_severity(sev->get_severity());
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
            set_severity(sev->get_severity());
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
        add_component(f_normal_component);
    }
    else
    {
        advgetopt::string_list_t component_names;
        advgetopt::split_string(comp, component_names, {","});
        for(auto name : component_names)
        {
            add_component(get_component(name));
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

    // NO REPEAT
    //
    {
        std::string no_repeat(f_name + "::no-repeat");
        if(!opts.is_defined(no_repeat))
        {
            if(opts.is_defined("no-repeat"))
            {
                no_repeat = "no-repeat";
            }
            else
            {
                no_repeat.clear();
            }
        }
        if(!no_repeat.empty())
        {
            std::string const value(opts.get_string(no_repeat));
            if(value != "off")
            {
                if(value == "max"
                || value == "maximum")
                {
                    f_no_repeat_size = NO_REPEAT_MAXIMUM;
                }
                else if(value == "default")
                {
                    f_no_repeat_size = NO_REPEAT_DEFAULT;
                }
                else
                {
                    f_no_repeat_size = opts.get_long(no_repeat, 0, 0, NO_REPEAT_MAXIMUM);
                }
            }
        }
    }
}


void appender::reopen()
{
}


void appender::add_component(component::pointer_t comp)
{
    guard g;

    f_components.insert(comp);
}


format::pointer_t appender::set_format(format::pointer_t new_format)
{
    guard g;

    format::pointer_t old(f_format);
    f_format = new_format;
    return old;
}


long appender::get_bytes_per_minute() const
{
    return f_bytes_per_minute;
}


/** \brief Return the number of dropped message due to bitrate restrictions.
 *
 * It is possible to set the number of bits per second that an appender
 * will accept. Anything over that amount will be dropped.
 *
 * The logger converts the bits per second amount to a bytes per minute.
 * Each time a message gets sent, the number of bytes in that message
 * string is added to a counter. If that counter reaches a number of bytes
 * per minute larger than the allowed bytes per minutes, then the messages
 * get dropped until 1 minute elapses.
 *
 * This function returns any message that was sent and was dropped because
 * the bytes per minute limit was reached.
 *
 * \note
 * This counter doesn't get reset so reading it always returns a grand
 * total of all the messages that were dropped so far. This counter is
 * per appender.
 *
 * \return The number of messages that were dropped.
 */
std::size_t appender::get_bytes_dropped_messages() const
{
    return f_bytes_dropped_messages;
}


void appender::send_message(message const & msg)
{
    guard g;

    if(!is_enabled()
    || msg.get_severity() < f_severity)
    {
        return;
    }

    component::set_t const & components(msg.get_components());
    if(components.empty())
    {
        // user did not supply any component in 'msg', check for
        // the normal component
        //
        if(!f_components.empty()
        && f_components.find(f_normal_component) == f_components.end())
        {
            return;
        }
    }
    else
    {
        if(snapdev::empty_set_intersection(f_components, components))
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

    // TBD: should we use the time of the message rather than 'now'?
    //
    if(f_bytes_per_minute != 0)
    {
        time_t const now(time(0));
        if(f_bytes_minute - now >= 60)
        {
            f_bytes_minute = now;
            f_bytes_received = 0;
        }
        else if(f_bytes_received + static_cast<long>(formatted_message.length())
                                        >= f_bytes_per_minute)
        {
            // overflow
            //
            // IMPORTANT NOTE: this algorithm may kick out a very long log
            // message and then accept a smaller one which still fits in the
            // `f_bytes_per_minute` bitrate
            //
            ++f_bytes_dropped_messages;
            return;
        }
        f_bytes_received += formatted_message.length();
    }

    if(f_no_repeat_size > NO_REPEAT_OFF)
    {
        std::string const non_changing_message(f_format->process_message(msg, true));
        auto it(std::find(f_last_messages.rbegin(), f_last_messages.rend(), non_changing_message));
        if(it != f_last_messages.rend())
        {
            // TODO: look into a way to count said messages and print out
            //       the total number or something of the sort...
            //       (maybe we store those messages in a buffer and once
            //       we are to replace a message, that's when we forward
            //       it and that can include the count? also that extra
            //       write can be based on time and/or count)
            //
            f_last_messages.erase(std::next(it).base());    // erase() expects an iterator, not a reverse iterator
        }
        f_last_messages.push_back(non_changing_message);
        if(f_last_messages.size() > f_no_repeat_size)
        {
            f_last_messages.pop_front();
        }
    }

    process_message(msg, formatted_message);
}


void appender::process_message(message const & msg, std::string const & formatted_message)
{
    // the default is a "null appender" -- do nothing
    snapdev::NOT_USED(msg, formatted_message);
}





appender_factory::appender_factory(std::string const & type)
    : f_type(type)
{
    char const * appender_factory_debug(getenv("APPENDER_FACTORY_DEBUG"));
    if(appender_factory_debug != nullptr
    && *appender_factory_debug != '\0')
    {
        std::cerr << "appender_factor:debug: adding appender factory \"" << type << "\".\n";
    }
}


appender_factory::~appender_factory()
{
}


std::string const & appender_factory::get_type() const
{
    return f_type;
}




void register_appender_factory(appender_factory::pointer_t factory)
{
    get_private_logger()->register_appender_factory(factory);
}


appender::pointer_t create_appender(std::string const & type, std::string const & name)
{
    return get_private_logger()->create_appender(type, name);
}






safe_format::safe_format(appender::pointer_t a, format::pointer_t new_format)
    : f_appender(a)
    , f_old_format(a->set_format(new_format))
{
}


safe_format::~safe_format()
{
    snapdev::NOT_USED(f_appender->set_format(f_old_format));
}







} // snaplogger namespace
// vim: ts=4 sw=4 et
