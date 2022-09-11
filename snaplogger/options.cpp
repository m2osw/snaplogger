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
 * \brief Handle logger specific command line and other options.
 *
 * The logger supports a few options to override configuration files
 * and tweak settings from the command line. Since the user is in
 * control of the environment variable, we do not offer that option
 * here.
 */


// self
//
#include    "snaplogger/options.h"

#include    "snaplogger/private_logger.h"
#include    "snaplogger/map_diagnostic.h"
#include    "snaplogger/version.h"


// boost lib
//
#include    <boost/algorithm/string/replace.hpp>


// advgetopt lib
//
#include    <advgetopt/exception.h>


// cppthread lib
//
#include    <cppthread/log.h>


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{


namespace
{



/** \brief The command line options that the logger adds.
 *
 * This variable holds the list of options that the logger adds when you
 * call the add_logger_options() function. These allows us to have the
 * same command line options in all the tools we develop with the snaplogger.
 *
 * The options are:
 *
 * * no-log
 * * log-file
 * * log-config
 * * syslog
 * * console
 * * logger-show-banner
 * * logger-hide-banner
 * * log-config-path
 * * debug
 * * trace
 * * list-severities
 * * log-severity
 * * force-severity
 * * log-component
 * * logger-version
 * * logger-configuration-filenames
 * * logger-plugin-paths
 */
advgetopt::option const g_options[] =
{
    // PLUGINS
    //
    advgetopt::define_option(
          advgetopt::Name("logger-plugin-paths")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("one or more paths separated by colons (:) to snaplogger plugins.")
    ),

    // DIRECT SELECT
    //
    advgetopt::define_option(
          advgetopt::Name("no-log")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("do not log anything.")
    ),
    advgetopt::define_option(
          advgetopt::Name("log-file")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("log messages to this specific log file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("log-config")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("only load this specific configuration file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("syslog")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("send the logs to syslog only, the argument, if specified, is the name to use as the identity.")
    ),
    advgetopt::define_option(
          advgetopt::Name("console")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("print the logs out to the console.")
    ),
    advgetopt::define_option(
          advgetopt::Name("logger-show-banner")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("show a banner on startup with the program name and version.")
    ),
    advgetopt::define_option(
          advgetopt::Name("logger-hide-banner")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("do not show the banner (--logger-show-banner has priority if specified).")
    ),

    // ALTERNATIVE CONFIG FILES
    //
    advgetopt::define_option(
          advgetopt::Name("log-config-path")
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("the path to the configuration folders.")
    ),

    // SEVERITY
    //
    advgetopt::define_option(
          advgetopt::Name("debug")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("change the logger severity level of each appender to DEBUG.")
    ),
    advgetopt::define_option(
          advgetopt::Name("trace")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("change the logger severity level of each appender to TRACE.")
    ),
    advgetopt::define_option(
          advgetopt::Name("log-severity")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("reduce the severity level of each appender to the specified level unless it is already lower.")
    ),
    advgetopt::define_option(
          advgetopt::Name("force-severity")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("change the logger severity level of each appender to the specified level.")
    ),

    // FILTERS
    //
    advgetopt::define_option(
          advgetopt::Name("log-component")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_MULTIPLE
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("filter logs by component, use ! in front of a name to prevent those logs.")
    ),

    // LIBEXCEPT EXTENSION
    //
    advgetopt::define_option(
          advgetopt::Name("except-stack-collect")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::DefaultValue("yes")
        , advgetopt::Help("what to collect from the stack on an exception: no, yes or simple, complete.")
    ),

    // COMMANDS
    //
    advgetopt::define_option(
          advgetopt::Name("list-appenders")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("show the list of available log appenders.")
    ),
    advgetopt::define_option(
          advgetopt::Name("list-severities")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("show the list of available log severities.")
    ),
    advgetopt::define_option(
          advgetopt::Name("logger-version")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("show the version of the logger library.")
    ),
    advgetopt::define_option(
          advgetopt::Name("logger-configuration-filenames")
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("show the list of configuration filenames that would be loaded with the current options.")
    ),

    // END
    //
    advgetopt::end_options()
};




}
// no name namespace


/** \brief Add the logger command line options.
 *
 * This function automatically adds the logger specific command line options
 * to the \p opts object.
 *
 * The options allow for some logger specific tweaking in all your
 * applications without having to do that on a per application basis.
 * It also makes use of some command line options which you should not
 * overload.
 *
 * On top of the command line option additions, the function automatically
 * adds up to four new diagnostics:
 *
 * * DIAG_KEY_VERSION -- the version of the tool
 * * DIAG_KEY_BUILD_DATE -- the build date of the tool
 * * DIAG_KEY_BUILD_TIME -- the build time of the tool
 * * DIAG_KEY_PROJECT_NAME -- the name of the project
 *
 * The project name is always added. The other parameters are added only
 * if found in the \p opts environment variables.
 *
 * \param[in,out] opts  The options to tweak with the logger options.
 */
void add_logger_options(advgetopt::getopt & opts)
{
    auto env(opts.get_options_environment());
    if(env.f_version != nullptr)
    {
        set_diagnostic(DIAG_KEY_VERSION, env.f_version);
    }
    if(env.f_build_date != nullptr)
    {
        set_diagnostic(DIAG_KEY_BUILD_DATE, env.f_build_date);
    }
    if(env.f_build_time != nullptr)
    {
        set_diagnostic(DIAG_KEY_BUILD_TIME, env.f_build_time);
    }
    set_diagnostic(DIAG_KEY_PROJECT_NAME, opts.get_project_name());

    opts.parse_options_info(g_options, true);
}




/** \brief Process the logger options.
 *
 * This function is expected to be called before you move forward with
 * the other work you want to do in your tool. It allows for processing
 * any and all the logger command line options.
 *
 * To use this command, you first want to call the add_logger_options()
 * function, process the arguments, and finally call this function as
 * follow:
 *
 * \code
 *     my_app::my_app(int argc, char * argv[])
 *         : f_opt(g_options_environment)
 *     {
 *         snaplogger::add_logger_options(f_opt);
 *         f_opt.finish_parsing(argc, argv);
 *         if(!snaplogger::process_logger_options(f_opt, "/etc/my-app/logger"))
 *         {
 *             // exit on any error
 *             throw advgetopt::getopt_exit("logger options generated an error.", 0);
 *         }
 *
 *         ...
 *     }
 * \endcode
 *
 * \exception advgetopt::getopt_exit
 * Some commands emit this exception which means that the process is done.
 * It is expected to quickly and silently exit after catching this
 * exception.
 *
 * \todo
 * See whether we can change the cppthread::log to a SNAP_LOG_... when
 * sending errors.
 *
 * \param[in] opts  The option definitions and values.
 * \param[in] config_path  A path where the logger configuration files are
 * to be searched. The logger also searches under `/usr/share/snaplogger/etc`
 * prior and `~/.config/<tool-name>/logger` after.
 * \param[in] out  The output stream to use in case the command is to generate
 * output to the user. Defaults to `std::cout`.
 * \param[in] show_banner  Whether to show the banner. For CLI and GUI tools,
 * you may want to hide this banner. For daemons, it is customary to leave
 * the banner as it just goes to a log file.
 *
 * \return true of success, false when an error occurs. You are expected to
 * stop your process immediately when this function returns false, but this
 * is not mandatory.
 */
bool process_logger_options(advgetopt::getopt & opts
                          , std::string const & config_path
                          , std::basic_ostream<char> & out
                          , bool show_banner)
{
    constexpr int const OPTION_NO_LOG           = 0x001;
    constexpr int const OPTION_LOG_FILE         = 0x002;
    constexpr int const OPTION_LOG_CONFIG       = 0x004;
    constexpr int const OPTION_SYSLOG           = 0x008;
    constexpr int const OPTION_CONSOLE          = 0x010;

    constexpr int const OPTION_TRACE_SEVERITY   = 0x020;
    constexpr int const OPTION_DEBUG_SEVERITY   = 0x040;
    constexpr int const OPTION_LOG_SEVERITY     = 0x080;
    constexpr int const OPTION_FORCE_SEVERITY   = 0x100;

    bool result(true);

    set_diagnostic(DIAG_KEY_PROGNAME, opts.get_program_name());

    // DYNAMIC INITIALIZATION
    //
    std::string plugin_paths(logger::default_plugin_paths());
    if(opts.is_defined("logger-plugin-paths"))
    {
        plugin_paths = opts.get_string("logger-plugin-paths");
    }
    logger::get_instance()->load_plugins(plugin_paths);

    // COMMANDS
    //
    if(opts.is_defined("logger-version"))
    {
        out << snaplogger::get_version_string() << std::endl;
        throw advgetopt::getopt_exit("--logger-version command processed.", 0);
    }
    if(opts.is_defined("list-appenders"))
    {
        out << "List of available appenders:\n";
        appender_factory_t const list(get_private_logger()->appender_factory_list());
        for(auto const & it : list)
        {
            out << " . " << it.first << " (" << it.second->get_type() << ")\n";
        }
        out << std::endl;
        throw advgetopt::getopt_exit("--list-appenders command processed.", 0);
    }
    if(opts.is_defined("list-severities"))
    {
        out << "List of the snaplogger known severities:\n";

        severity_by_severity_t const list(get_severities_by_severity());
        for(auto const & it : list)
        {
            string_vector_t const names(it.second->get_all_names());
            char const * sep = " . ";
            for(auto const & s : names)
            {
                out << sep << s;
                sep = ", ";
            }

            out << " [" << static_cast<int>(it.first);
            if(it.second->is_system())
            {
                out << "/system";
            }
            if(!it.second->get_styles().empty())
            {
                out << "/styles";
            }
            out << ']';

            //std::string const d(it.second->get_description());
            //if(!d.empty())
            //{
            //    out << " -- " << d;
            //}

            out << '\n';
        }
        out << std::endl;
        throw advgetopt::getopt_exit("--list-severities command processed.", 0);
    }

    // LOG CONFIG
    //
    int log_config(0);
    if(opts.is_defined("no-log"))
    {
        log_config |= OPTION_NO_LOG;
    }
    if(opts.is_defined("log-file"))
    {
        log_config |= OPTION_LOG_FILE;
    }
    if(opts.is_defined("log-config"))
    {
        log_config |= OPTION_LOG_CONFIG;
    }
    if(opts.is_defined("syslog"))
    {
        log_config |= OPTION_SYSLOG;
    }
    if(opts.is_defined("console"))
    {
        log_config |= OPTION_CONSOLE;
    }
    if(opts.is_defined("logger-show-banner"))
    {
        show_banner = true;
    }
    else if(opts.is_defined("logger-hide-banner"))
    {
        show_banner = false;
    }

    bool const show_logger_configuration_files(opts.is_defined("logger-configuration-filenames"));
    switch(log_config)
    {
    case 0:
        // defaults apply as normal

        {
            advgetopt::options_environment opt_env;

            std::string user_config("~/.config/");
            user_config += opts.get_project_name();
            user_config += "/logger";
            char const * config_dirs[] =
            {
                  "/usr/share/snaplogger/etc"
                , config_path.c_str()
                , user_config.c_str()
                , nullptr
            };

            if(opts.is_defined("log-config-path"))
            {
                config_dirs[0] = opts.get_string("log-config-path").c_str();
            }

            std::string const keep_project_name(opts.get_project_name());
            opt_env.f_project_name = keep_project_name.c_str();
            std::string const keep_group_name(opts.get_group_name());
            opt_env.f_group_name = keep_group_name.c_str();
            opt_env.f_environment_variable_name = "SNAPLOGGER";
            //opt_env.f_configuration_files = nullptr;
            opt_env.f_configuration_filename = "snaplogger.conf";
            opt_env.f_configuration_directories = config_dirs;
            opt_env.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_DYNAMIC_PARAMETERS;

            advgetopt::getopt system_opts(opt_env);

            if(show_logger_configuration_files)
            {
                advgetopt::string_list_t list(system_opts.get_configuration_filenames(false, false));
                out << "Logger common configuration filenames:" << std::endl;
                for(auto n : list)
                {
                    out << " . " << n << "\n";
                }
            }

            // load the system configuration file first
            //
            system_opts.parse_configuration_files();
            if(opts.get_program_fullname().empty())
            {
                // process environment variable now if no user filename
                // is going to be loaded
                //
                system_opts.parse_environment_variable();
            }
            logger::get_instance()->set_config(system_opts);

            if(!opts.get_program_fullname().empty())
            {
                // if we have a valid program name (non-empty) then try
                // to load these configuration files
                //
                std::string filename(opts.get_program_name());
                boost::replace_all(filename, "_", "-");
                filename += ".conf";
                opt_env.f_configuration_filename = filename.c_str();
                advgetopt::getopt config_opts(opt_env);

                if(show_logger_configuration_files)
                {
                    advgetopt::string_list_t list(config_opts.get_configuration_filenames(false, false));
                    out << "Logger application configuration filenames:" << std::endl;
                    for(auto n : list)
                    {
                        out << " . " << n << "\n";
                    }
                }

                config_opts.parse_configuration_files();
                config_opts.parse_environment_variable();
                logger::get_instance()->set_config(config_opts);
            }
        }
        break;

    case OPTION_NO_LOG:
        // do nothing
        break;

    case OPTION_LOG_FILE:
        configure_file(opts.get_string("log-file"));
        break;

    case OPTION_LOG_CONFIG:
        configure_config(opts.get_string("log-config"));
        break;

    case OPTION_SYSLOG:
        configure_syslog(opts.get_string("syslog"));
        break;

    case OPTION_CONSOLE:
        configure_console();
        break;

    default:
        cppthread::log << cppthread::log_level_t::error
                       << "only one of --no-log, --log-file, --log-config, --syslog, --console can be used on your command line."
                       << cppthread::end;
        result = false;
        break;

    }

    if(show_logger_configuration_files)
    {
        if(log_config != 0)
        {
            if(log_config == OPTION_LOG_CONFIG)
            {
                out << "Logger application configuration filename:" << std::endl
                    << " . " << opts.get_string("log-config") << std::endl;
            }
            else
            {
                out << "No logger application configuration filenames available with the current command line options." << std::endl;
            }
        }
        throw advgetopt::getopt_exit("--logger-configuration-filenames command processed.", 0);
    }

    // SEVERITY
    //
    int severity_selection(0);
    if(opts.is_defined("trace"))
    {
        severity_selection |= OPTION_TRACE_SEVERITY;
    }
    if(opts.is_defined("debug"))
    {
        severity_selection |= OPTION_DEBUG_SEVERITY;
    }
    if(opts.is_defined("log-severity"))
    {
        severity_selection |= OPTION_LOG_SEVERITY;
    }
    if(opts.is_defined("force-severity"))
    {
        severity_selection |= OPTION_FORCE_SEVERITY;
    }

    switch(severity_selection)
    {
    case 0:
        // keep as is
        break;

    case OPTION_TRACE_SEVERITY:
        logger::get_instance()->reduce_severity(severity_t::SEVERITY_TRACE);
        configure_console(true);
        break;

    case OPTION_DEBUG_SEVERITY:
        logger::get_instance()->reduce_severity(severity_t::SEVERITY_DEBUG);
        configure_console(true);
        break;

    case OPTION_LOG_SEVERITY:
        {
            std::string const severity_name(opts.get_string("log-severity"));
            severity::pointer_t sev(get_severity(severity_name));
            if(sev == nullptr)
            {
                cppthread::log << cppthread::log_level_t::error
                               << "unknown severity level \""
                               << severity_name
                               << "\"; please check your spelling."
                               << cppthread::end;
                result = false;
            }
            else
            {
                logger::get_instance()->reduce_severity(sev->get_severity());
            }
        }
        break;

    case OPTION_FORCE_SEVERITY:
        {
            std::string const severity_name(opts.get_string("force-severity"));
            severity::pointer_t sev(get_severity(severity_name));
            if(sev == nullptr)
            {
                cppthread::log << cppthread::log_level_t::error
                               << "unknown severity level \""
                               << severity_name
                               << "\"; please check your spelling against the --list-severities."
                               << cppthread::end;
                result = false;
            }
            else
            {
                logger::get_instance()->set_severity(sev->get_severity());
            }
        }
        break;

    default:
        cppthread::log << cppthread::log_level_t::error
                       << "only one of --debug, --log-severity, --force-severity can be used on your command line."
                       << cppthread::end;
        return false;

    }

    // FILTERS
    //
    if(opts.is_defined("log-component"))
    {
        size_t const max(opts.size("log-component"));
        for(size_t idx(0); idx < max; ++idx)
        {
            std::string log_component(opts.get_string("log-component", idx));
            if(!log_component.empty())
            {
                if(log_component[0] == '!')
                {
                    log_component = log_component.substr(1);
                    if(!log_component.empty())
                    {
                        component::pointer_t comp(get_component(log_component));
                        logger::get_instance()->add_component_to_ignore(comp);
                    }
                }
                else
                {
                    component::pointer_t comp(get_component(log_component));
                    logger::get_instance()->add_component_to_include(comp);
                }
            }
        }
    }

    // LIBEXCEPT EXTENSION
    //
    {
        // used in conjunction with SNAP_LOG_SEND_WITH_STACK_TRACE(e)
        // the default is "yes" (see libexcept/exception.cpp)
        //
        std::string const & collect(opts.get_string("except-stack-collect"));
        if(collect == "no")
        {
            // stack trace will be empty
            //
            libexcept::set_collect_stack(libexcept::collect_stack_t::COLLECT_STACK_NO);
        }
        else if(collect == "yes" || collect == "simple")
        {
            // names are not demangled and line numbers are not searched
            //
            libexcept::set_collect_stack(libexcept::collect_stack_t::COLLECT_STACK_YES);
        }
        else if(collect == "complete")
        {
            // frames include demangled names with line numbers
            //
            libexcept::set_collect_stack(libexcept::collect_stack_t::COLLECT_STACK_COMPLETE);
        }
        else
        {
            cppthread::log << cppthread::log_level_t::error
                           << "unknown type of stack collection \""
                           << collect
                           << "\"; try one of: \"no\", \"yes\", \"simple\", or \"complete\"."
                           << cppthread::end;
            return false;
        }
    }

    if(show_banner)
    {
        SNAP_LOG_INFO
                << section(g_normal_component)
                << section(g_self_component)
                << section(g_banner_component)
                << "--------------------------------------------------"
                << SNAP_LOG_SEND;
        SNAP_LOG_INFO
                << section(g_normal_component)
                << section(g_self_component)
                << section(g_banner_component)
                << opts.get_project_name()
                << " v"
                << opts.get_options_environment().f_version
                << " started."
                << SNAP_LOG_SEND;
    }

    return result;
}


} // snaplogger namespace
// vim: ts=4 sw=4 et
