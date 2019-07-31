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
 * This file declares the base appender class.
 */

// self
//
#include    "snaplogger/console_appender.h"
#include    "snaplogger/file_appender.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/logger.h"
#include    "snaplogger/syslog_appender.h"


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{


logger::pointer_t           g_instance = logger::pointer_t();


}
// no name namespace



logger::logger()
{
}


logger::pointer_t logger::get_instance()
{
    guard g;

    if(g_instance == nullptr)
    {
        g_instance = std::make_shared<logger>();
    }

    return g_instance;
}


/** \brief Reset the logger to its startup state.
 *
 * This function resets the logger to non-asynchronous and no appenders.
 *
 * This is mainly used in our unit tests so we do not have to run the
 * tests one at a time. It should nearly never be useful in your environment
 * except if you do a fork() and wanted the child to have its own special
 * log environment.
 */
void logger::reset()
{
    guard g;

    set_asynchronous(false);
    f_appenders.clear();
}


bool logger::is_configured() const
{
    guard g;

    return !f_appenders.empty();
}


void logger::set_config(advgetopt::getopt const & params)
{
    if(params.is_defined("asynchronous"))
    {
        set_asynchronous(params.get_string("asynchronous") == "true");
    }

    std::string const name(advgetopt::CONFIGURATION_SECTIONS);
    auto const & sections(params.get_option(name));
    if(sections != nullptr)
    {
        size_t const max(sections->size());
        for(size_t idx(0); idx < max; ++idx)
        {
            std::string const section_name(sections->get_value(idx));
            std::string const section_type(section_name + "::type");
            std::string type;
            if(params.is_defined(section_type))
            {
                type = params.get_string(section_type);
            }
            else
            {
                // try with the name of the section if no type is defined
                //
                type = section_name;
            }
            if(!type.empty())
            {
                appender::pointer_t a(create_appender(type, section_name));
                if(a != nullptr)
                {
                    f_appenders.push_back(a);
                }
                // else -- this may be a section which does not represent an appender
            }
        }
    }

    for(auto a : f_appenders)
    {
        a->set_config(params);
    }
}


void logger::add_config(std::string const & config_filename)
{
    advgetopt::options_environment opt_env;

    char const * configuration_files[] =
    {
          config_filename.c_str()
        , nullptr
    };

    opt_env.f_project_name = "logger";
    opt_env.f_environment_variable_name = "SNAPLOGGER";
    opt_env.f_configuration_files = configuration_files;
    opt_env.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS;

    advgetopt::getopt opts(opt_env);

    opts.parse_configuration_files();
    opts.parse_environment_variable();

    set_config(opts);
}


void logger::add_console_appender()
{
    appender::pointer_t a(std::make_shared<console_appender>("console"));

    guard g;

    f_appenders.push_back(a);
}


void logger::add_syslog_appender(std::string const & identity)
{
    appender::pointer_t a(std::make_shared<syslog_appender>("syslog"));

    advgetopt::option options[] =
    {
        advgetopt::define_option(
              advgetopt::Name("syslog::identity")
            , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
            , advgetopt::DefaultValue(identity.c_str())
        ),
        advgetopt::end_options()
    };

    advgetopt::options_environment opt_env;
    opt_env.f_project_name = "logger";
    if(!identity.empty())
    {
        opt_env.f_options = options;
    }
    advgetopt::getopt opts(opt_env);
    a->set_config(opts);

    guard g;

    f_appenders.push_back(a);
}


void logger::add_file_appender(std::string const & filename)
{
    file_appender::pointer_t a(std::make_shared<file_appender>("file"));
    a->set_filename(filename);

    guard g;

    f_appenders.push_back(a);
}


bool logger::is_asynchronous() const
{
    guard g;

    return f_asynchronous;
}


void logger::set_asynchronous(bool status)
{
    status = status != false;

    guard g;

    if(f_asynchronous != status)
    {
        f_asynchronous = status;

        if(f_asynchronous)
        {
            // TODO: create thread
        }
        else
        {
            // TODO: kill thread
        }
    }
}


void logger::log_message(message const & msg)
{
    if(const_cast<message &>(msg).tellp() == 0)
    {
        // no message, ignore
        //
        return;
    }

    appender::vector_t appenders;
    {
        guard g;

        if(f_appenders.empty())
        {
            if(isatty(fileno(stdout)))
            {
                add_console_appender();
            }
            else
            {
                add_syslog_appender(std::string());
            }
        }

        appenders = f_appenders;
    }

    for(auto a : appenders)
    {
        a->send_message(msg);
    }
}


bool is_configured()
{
    if(g_instance == nullptr)
    {
        return false;
    }

    return g_instance->is_configured();
}


bool configure_console()
{
    bool result(!is_configured());
    if(result)
    {
        logger::get_instance()->add_console_appender();
    }

    return result;
}


bool configure_syslog(std::string const & identity)
{
    bool result(!is_configured());
    if(result)
    {
        logger::get_instance()->add_syslog_appender(identity);
    }

    return result;
}


bool configure_file(std::string const & filename)
{
    bool result(!is_configured());
    if(result)
    {
        logger::get_instance()->add_file_appender(filename);
    }

    return result;
}


bool configure_config(std::string const & config_filename)
{
    bool result(!is_configured());
    if(result)
    {
        logger::get_instance()->add_config(config_filename);
    }

    return result;
}




} // snaplogger namespace
// vim: ts=4 sw=4 et
