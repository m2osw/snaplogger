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
 * \brief Handle logger specific command line and other options.
 *
 * The logger supports a few options to override configuration files
 * and tweak settings from the command line. Since the user is in
 * control of the environment variable, we do not offer that option
 * here.
 */


// self
//
#include    "snaplogger/logger.h"
#include    "snaplogger/options.h"


// advgetopt lib
//
#include    "advgetopt/log.h"



namespace snaplogger
{


namespace
{



advgetopt::option const g_options[] =
{
    // DIRECT SELECT
    //
    advgetopt::define_option(
          advgetopt::Name("no-log")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("do not log anything.")
    ),
    advgetopt::define_option(
          advgetopt::Name("log-file")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("log data to this specific log files")
    ),
    advgetopt::define_option(
          advgetopt::Name("log-config")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("only load this specific configuration file.")
    ),
    advgetopt::define_option(
          advgetopt::Name("syslog")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("send the logs to syslog only, the argument, if specified, is the name to use as the identity.")
    ),
    advgetopt::define_option(
          advgetopt::Name("console")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("print out the time and date when %p was built and exit.")
    ),

    // ALTERNATIVE CONFIG FILES
    //
    advgetopt::define_option(
          advgetopt::Name("log-config-path")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("the path to the configuration folders.")
    ),

    // SEVERITY
    //
    advgetopt::define_option(
          advgetopt::Name("debug")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("change the severity level of each appender to DEBUG.")
    ),
    advgetopt::define_option(
          advgetopt::Name("log-severity")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                            , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("change the severity level of each appender to the specified level.")
    ),

    // FILTERS
    //
    advgetopt::define_option(
          advgetopt::Name("log-component")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("filter logs by component, use ! in front of a name to prevent those logs.")
    ),
    advgetopt::define_option(
          advgetopt::Name("filter-diagnotics")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("filter logs by diagnostics, use ! in front of the name to prevent those logs.")
    ),
    advgetopt::define_option(
          advgetopt::Name("path-to-option-definitions")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("print out the path to the option definitons.")
    ),

    // END
    //
    advgetopt::end_options()
};




}
// no name namespace


void add_logger_options(advgetopt::getopt & opts)
{
    opts.parse_options_info(g_options, true);
}


constexpr int const OPTION_NO_LOG       = 0x01;
constexpr int const OPTION_LOG_FILE     = 0x02;
constexpr int const OPTION_LOG_CONFIG   = 0x04;
constexpr int const OPTION_SYSLOG       = 0x08;
constexpr int const OPTION_CONSOLE      = 0x10;

void process_logger_options(advgetopt::getopt & opts
                          , std::string const & project_name
                          , std::string const & progname
                          , std::string const & config_path)
{
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

    switch(log_config)
    {
    case 0:
        // defaults apply as normal

        {
            advgetopt::options_environment opt_env;

            char const * config_dirs[] =
            {
                  config_path.c_str()
                , nullptr
            };

            if(opts.is_defined("log-config-path"))
            {
                config_dirs[0] = opts.get_string("log-config-path").c_str();
            }

            opt_env.f_project_name = project_name.c_str();
            opt_env.f_environment_variable_name = "SNAPLOGGER";
            //opt_env.f_configuration_files = nullptr;
            opt_env.f_configuration_filename = "snaplogger.conf";
            opt_env.f_configuration_directories = config_dirs;
            opt_env.f_environment_flags = 0;

            advgetopt::getopt config_opts(opt_env);

            // load the system configuration file first
            //
            config_opts.parse_configuration_files();

            if(!progname.empty())
            {
                opt_env.f_configuration_filename = progname.c_str();
                config_opts.parse_configuration_files();
            }

            config_opts.parse_environment_variable();

            logger::get_instance()->set_config(config_opts);
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
        advgetopt::log << advgetopt::log_level_t::error
                       << "only one of --no-log, --log-file, --log-config, --syslog, --console can be used simultaneously."
                       << advgetopt::end;
        return;

    }


//
//    // SEVERITY
//    //
//    advgetopt::define_option(
//          advgetopt::Name("debug")
//        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
//        , advgetopt::Help("change the severity level of each appender to DEBUG.")
//    ),
//    advgetopt::define_option(
//          advgetopt::Name("log-severity")
//        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
//                            , advgetopt::GETOPT_FLAG_REQUIRED>())
//        , advgetopt::Help("change the severity level of each appender to the specified level.")
//    ),
//
//    // FILTERS
//    //
//    advgetopt::define_option(
//          advgetopt::Name("log-component")
//        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
//        , advgetopt::Help("filter logs by component, use ! in front of a name to prevent those logs.")
//    ),
//    advgetopt::define_option(
//          advgetopt::Name("filter-diagnotics")
//        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
//        , advgetopt::Help("filter logs by diagnostics, use ! in front of the name to prevent those logs.")
//    ),
//    advgetopt::define_option(
//          advgetopt::Name("path-to-option-definitions")
//        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
//        , advgetopt::Help("print out the path to the option definitons.")
//    ),

}


} // snaplogger namespace
// vim: ts=4 sw=4 et
