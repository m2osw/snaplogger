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
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    "snaplogger/logger.h"

#include    "snaplogger/console_appender.h"
#include    "snaplogger/exception.h"
#include    "snaplogger/file_appender.h"
#include    "snaplogger/guard.h"
#include    "snaplogger/private_logger.h"
#include    "snaplogger/syslog_appender.h"


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



bool                        g_first_instance = true;
logger::pointer_t *         g_instance = nullptr;


struct auto_delete_logger
{
    ~auto_delete_logger()
    {
        logger::pointer_t * ptr(nullptr);
        {
            guard g;
            swap(ptr, g_instance);
        }
        if(ptr != nullptr)
        {
            (*ptr)->shutdown();
            delete ptr;
        }
    }
};

auto_delete_logger          g_logger_deleter = auto_delete_logger();



}
// no name namespace



logger::logger()
{
}


logger::~logger()
{
}


logger::pointer_t logger::get_instance()
{
    guard g;

    if(g_instance == nullptr)
    {
        if(!g_first_instance)
        {
            throw duplicate_error("preventing an attempt of re-creating the snap logger.");
        }

        g_first_instance = false;

        // note that we create a `private_logger` object
        //
        g_instance = new logger::pointer_t();
        g_instance->reset(new private_logger());
    }

    return *g_instance;
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
    f_lowest_severity = severity_t::SEVERITY_OFF;
}


void logger::shutdown()
{
}


bool logger::is_configured() const
{
    guard g;

    return !f_appenders.empty();
}


bool logger::has_appender(std::string const & type) const
{
    return std::find_if(
          f_appenders.begin()
        , f_appenders.end()
        , [&type](auto a)
        {
            return type == a->get_type();
        }) != f_appenders.end();
}


appender::pointer_t logger::get_appender(std::string const & name) const
{
    auto it(std::find_if(
          f_appenders.begin()
        , f_appenders.end()
        , [&name](auto a)
        {
            return name == a->get_name();
        }));
    if(it == f_appenders.end())
    {
        return appender::pointer_t();
    }

    return *it;
}


void logger::set_config(advgetopt::getopt const & params)
{
    if(params.is_defined("asynchronous"))
    {
        set_asynchronous(params.get_string("asynchronous") == "true");
    }

    auto const & sections(params.get_option(advgetopt::CONFIGURATION_SECTIONS));
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
                    add_appender(a);
                }
                // else -- this may be a section which does not represent an appender
            }
        }
    }

    guard g;

    for(auto a : f_appenders)
    {
        a->set_config(params);
    }
}


void logger::reopen()
{
    guard g;

    for(auto a : f_appenders)
    {
        a->reopen();
    }
}


void logger::add_appender(appender::pointer_t a)
{
    guard g;

    if(a->unique())
    {
        std::string const type(a->get_type());
        auto it(std::find_if(
                  f_appenders.begin()
                , f_appenders.end()
                , [&type](auto app)
                {
                    return type == app->get_type();
                }));
        if(it != f_appenders.end())
        {
            throw duplicate_error(
                          "an appender of type \""
                        + type
                        + "\" can only be added once.");
        }
    }

    f_appenders.push_back(a);

    severity_changed(a->get_severity());
}


void logger::add_config(std::string const & config_filename)
{
    advgetopt::options_environment opt_env;

    char const * configuration_files[] =
    {
          config_filename.c_str()
        , nullptr
    };

    opt_env.f_project_name = "snaplogger";
    opt_env.f_environment_variable_name = "SNAPLOGGER";
    opt_env.f_configuration_files = configuration_files;
    opt_env.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS;

    advgetopt::getopt opts(opt_env);

    opts.parse_configuration_files();
    opts.parse_environment_variable();

    set_config(opts);
}


appender::pointer_t logger::add_console_appender()
{
    appender::pointer_t a(std::make_shared<console_appender>("console"));

    advgetopt::options_environment opt_env;
    opt_env.f_project_name = "snaplogger";
    advgetopt::getopt opts(opt_env);
    a->set_config(opts);

    guard g;

    add_appender(a);

    return a;
}


appender::pointer_t logger::add_syslog_appender(std::string const & identity)
{
    appender::pointer_t a(std::make_shared<syslog_appender>("syslog"));

    advgetopt::option options[] =
    {
        advgetopt::define_option(
              advgetopt::Name("syslog::identity")
            , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
        ),
        advgetopt::end_options()
    };

    advgetopt::options_environment opt_env;
    opt_env.f_project_name = "snaplogger";
    opt_env.f_options = options;
    advgetopt::getopt opts(opt_env);
    if(!identity.empty())
    {
        opts.get_option("syslog::identity")->set_value(0, identity);
    }
    a->set_config(opts);

    guard g;

    add_appender(a);

    return a;
}


appender::pointer_t logger::add_file_appender(std::string const & filename)
{
    file_appender::pointer_t a(std::make_shared<file_appender>("file"));

    advgetopt::option options[] =
    {
        advgetopt::define_option(
              advgetopt::Name("file::filename")
            , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_REQUIRED>())
        ),
        advgetopt::end_options()
    };

    advgetopt::options_environment opt_env;
    opt_env.f_project_name = "snaplogger";
    opt_env.f_options = options;
    advgetopt::getopt opts(opt_env);
    if(!filename.empty())
    {
        opts.get_option("file::filename")->set_value(0, filename);
    }
    a->set_config(opts);

    guard g;

    add_appender(a);

    return a;
}


severity_t logger::get_lowest_severity() const
{
    guard g;

    if(f_appenders.empty())
    {
        // we do not know the level yet, we do not have the appenders
        // yet... so accept anything at this point
        //
        return severity_t::SEVERITY_ALL;
    }

    return f_lowest_severity;
}


void logger::set_severity(severity_t severity_level)
{
    guard g;

    f_lowest_severity = severity_level;
    for(auto a : f_appenders)
    {
        a->set_severity(severity_level);
    }
}


void logger::reduce_severity(severity_t severity_level)
{
    for(auto a : f_appenders)
    {
        a->reduce_severity(severity_level);
    }
}


void logger::severity_changed(severity_t severity_level)
{
    guard g;

    if(severity_level < f_lowest_severity)
    {
        f_lowest_severity = severity_level;
    }
    else if(severity_level > f_lowest_severity)
    {
        // if the severity level grew we have to search for the new lowest;
        // this happens very rarely while running, it's likely to happen
        // up to once per appender on initialization.
        //
        auto minmax(std::minmax_element(f_appenders.begin(), f_appenders.end()));
        f_lowest_severity = (*minmax.first)->get_severity();
    }
}


void logger::add_component_to_include(component::pointer_t comp)
{
    guard g;

    f_components_to_include.insert(comp);
}


void logger::add_component_to_ignore(component::pointer_t comp)
{
    guard g;

    f_components_to_ignore.insert(comp);
}


bool logger::is_asynchronous() const
{
    guard g;

    return f_asynchronous;
}


void logger::set_asynchronous(bool status)
{
    status = status != false;

    bool do_delete(false);
    {
        guard g;

        if(f_asynchronous != status)
        {
            f_asynchronous = status;
            if(!f_asynchronous)
            {
                do_delete = true;
            }
        }
    }

    if(do_delete)
    {
        private_logger * l(dynamic_cast<private_logger *>(this));
        l->delete_thread();
    }
}


void logger::log_message(message const & msg)
{
    if(const_cast<message &>(msg).tellp() != 0)
    {
        bool asynchronous(false);
        {
            guard g;

            if(f_asynchronous)
            {
                message::pointer_t m(std::make_shared<message>(msg, msg));
                private_logger * l(dynamic_cast<private_logger *>(this));
                l->send_message_to_thread(m);
                asynchronous = true;
            }
        }

        if(!asynchronous)
        {
            process_message(msg);
        }
    }

    if(f_fatal_severity != severity_t::SEVERITY_OFF
    && msg.get_severity() >= f_fatal_severity)
    {
        call_fatal_error_callback();
        throw fatal_error("A fatal error occurred.");
    }
}


void logger::process_message(message const & msg)
{
    appender::vector_t appenders;

    {
        guard g;

        bool include(f_components_to_include.empty());
        component::set_t const & components(msg.get_components());
        if(components.empty())
        {
            if(f_components_to_ignore.find(f_normal_component) != f_components_to_ignore.end())
            {
                return;
            }
            if(!include)
            {
                if(f_components_to_include.find(f_normal_component) != f_components_to_include.end())
                {
                    include = true;
                }
            }
        }
        else
        {
            for(auto c : components)
            {
                if(f_components_to_ignore.find(c) != f_components_to_ignore.end())
                {
                    return;
                }
                if(!include)
                {
                    if(f_components_to_include.find(c) != f_components_to_include.end())
                    {
                        include = true;
                    }
                }
            }
        }
        if(!include)
        {
            return;
        }

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


void logger::set_fatal_error_callback(std::function<void(void)> & f)
{
    f_fatal_error_callback = f;
}


void logger::call_fatal_error_callback()
{
    if(f_fatal_error_callback != nullptr)
    {
        f_fatal_error_callback();
    }
}


bool is_configured()
{
    guard g;

    if(g_instance == nullptr)
    {
        return false;
    }

    return (*g_instance)->is_configured();
}


bool has_appender(std::string const & type)
{
    guard g;

    if(g_instance == nullptr)
    {
        return false;
    }

    return (*g_instance)->has_appender(type);
}


void reopen()
{
    guard g;

    if(g_instance == nullptr)
    {
        return;
    }

    (*g_instance)->reopen();
}


bool configure_console(bool force)
{
    bool result(!is_configured() || (force && !has_appender("console")));
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
