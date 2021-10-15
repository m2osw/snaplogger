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
    // The asynchronous flag can cause problems unless the programmer
    // specifically planned for it so we do not allow it in configuration
    // file at the moment. Later we may have two flags. If both are true
    // then we will allow asynchronous logging.
    //
    //if(params.is_defined("asynchronous"))
    //{
    //    set_asynchronous(params.get_string("asynchronous") == "true");
    //}

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
            // the console is a pretty special type because it can't be
            // added twice but it may get added early because an error
            // occurs and forces initialization of the logger "too soon"
            //
            if(type == "console")
            {
                if(a->get_name() != "console"
                && (*it)->get_name() == "console")
                {
                    (*it)->set_name(a->get_name());
                }
                return;
            }
            if(type == "syslog")
            {
                if(a->get_name() != "syslog"
                && (*it)->get_name() == "syslog")
                {
                    (*it)->set_name(a->get_name());
                }
                return;
            }
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
    opt_env.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_AUTO_DONE;
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
    opt_env.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_AUTO_DONE;
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
    opt_env.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_AUTO_DONE;
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
        auto const min(std::min_element(f_appenders.begin(), f_appenders.end()));
        f_lowest_severity = (*min)->get_severity();
    }
}


severity_t logger::get_default_severity() const
{
    private_logger const * l(dynamic_cast<private_logger const *>(this));
    severity::pointer_t sev(l->get_default_severity());
    return sev == nullptr ? severity_t::SEVERITY_DEFAULT : sev->get_severity();
}


bool logger::set_default_severity(severity_t severity_level)
{
    private_logger * l(dynamic_cast<private_logger *>(this));
    if(severity_level == severity_t::SEVERITY_ALL)
    {
        // reset to default
        l->set_default_severity(severity::pointer_t());
    }
    else
    {
        severity::pointer_t sev(l->get_severity(severity_level));
        if(sev == nullptr)
        {
            return false;
        }
        l->set_default_severity(sev);
    }
    return true;
}


void logger::add_component_to_include(component::pointer_t comp)
{
    guard g;

    f_components_to_include.insert(comp);
}


void logger::remove_component_to_include(component::pointer_t comp)
{
    guard g;

    f_components_to_include.erase(comp);
}


void logger::add_component_to_ignore(component::pointer_t comp)
{
    guard g;

    f_components_to_ignore.insert(comp);
}


void logger::remove_component_to_ignore(component::pointer_t comp)
{
    guard g;

    f_components_to_ignore.erase(comp);
}


void logger::add_default_field(std::string const & name, std::string const & value)
{
    if(!name.empty())
    {
        if(name[0] == '_')
        {
            throw invalid_parameter(
                  "field name \""
                + name
                + "\" is a system name (whether reserved or already defined) and as such is read-only."
                  " Do not start your field names with an underscore (_).");
        }
        if(name == "id")
        {
            throw invalid_parameter(
                  "field name \"id\" is automatically set by the message class,"
                  " it cannot be set as a default field.");
        }

        guard g;

        f_default_fields[name] = value;
    }
}


std::string logger::get_default_field(std::string const & name) const
{
    guard g;

    auto it(f_default_fields.find(name));
    if(it != f_default_fields.end())
    {
        return it->second;
    }
    return std::string();
}


field_map_t logger::get_default_fields() const
{
    guard g;

    return f_default_fields;
}


void logger::remove_default_field(std::string const & name)
{
    guard g;

    auto it(f_default_fields.find(name));
    if(it != f_default_fields.end())
    {
        f_default_fields.erase(it);
    }
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

        ++f_severity_stats[static_cast<std::size_t>(msg.get_severity())];

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


/** \brief Return statistics about log severities.
 *
 * This function returns the statistics counting each message sent per
 * severity.
 *
 * If you enabled the asynchronous functionality of the snaplogger,
 * then this statistics may not reflect the current state as the
 * logger thread may still not have processed all the messages.
 *
 * \note
 * The severity_stats_t type is a vector that includes all possible
 * severity levels (0 to 255), including severity levels that are not
 * currently declared. It is done that way so the access is as fast
 * as possible when we want to increment one of the stats. Using a
 * map would have a much greater impact on the process_message()
 * function.
 *
 * \return a copy of the severity statistics at the time of the call.
 */
severity_stats_t logger::get_severity_stats() const
{
    guard g;

    return f_severity_stats;
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
