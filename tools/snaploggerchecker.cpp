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
// 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// snaplogger lib
//
#include <snaplogger/exception.h>
#include <snaplogger/logger.h>
#include <snaplogger/options.h>
#include <snaplogger/version.h>


// advgetopt lib
//
#include <advgetopt/exception.h>


// snapdev lib
//
#include <snapdev/not_reached.h>
#include <snapdev/not_used.h>


// boost lib
//
#include <boost/preprocessor/stringize.hpp>


// last include
//
#include <snapdev/poison.h>




/** \file
 * \brief Logger command line tool.
 *
 * This tool is mainly used to verify the logger configuration files.
 * These are fairly simple but it is still much better to verify
 * before you run your software (that way you can verify at compile
 * time instead of later when you may already have shipped your
 * software.)
 *
 * The tool can also log messages using any available configuration
 * file. This is practical to log messages from script or other
 * languages which do not support the snaplogger.
 */

namespace
{



advgetopt::option const g_command_line_options[] =
{
    // COMMANDS
    advgetopt::define_option(
          advgetopt::Name("list-components")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("list of available log components.")
    ),
    advgetopt::define_option(
          advgetopt::Name("list-formats")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("list each appender output format.")
    ),
    advgetopt::define_option(
          advgetopt::Name("list-levels")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("list of available log levels.")
    ),
    advgetopt::define_option(
          advgetopt::Name("list-types")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("list of available log types.")
    ),
    advgetopt::define_option(
          advgetopt::Name("log")
        , advgetopt::ShortName('l')
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("verify the snaplogger configuration files.")
    ),
    advgetopt::define_option(
          advgetopt::Name("verify")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("verify the snaplogger configuration files.")
    ),

    // OPTIONS
    advgetopt::define_option(
          advgetopt::Name("config")
        , advgetopt::ShortName('c')
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("path to the configuration files.")
    ),
    advgetopt::define_option(
          advgetopt::Name("filename")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("name of the file the log is being generated from.")
    ),
    advgetopt::define_option(
          advgetopt::Name("function")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("name of the function the log is being generated from.")
    ),
    advgetopt::define_option(
          advgetopt::Name("level")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("the log level (info, warn, error, etc.)")
    ),
    advgetopt::define_option(
          advgetopt::Name("line")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("line number the log is being generated from.")
    ),
    advgetopt::define_option(
          advgetopt::Name("message")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("the message to log.")
    ),
    advgetopt::define_option(
          advgetopt::Name("output")
        , advgetopt::ShortName('o')
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("name of the output file (to save the compiled configuration files.)")
    ),
    advgetopt::define_option(
          advgetopt::Name("project")
        , advgetopt::ShortName('p')
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("name of the project to load the administrator modified configuration files.")
    ),
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("make the process verbose.")
    ),
    advgetopt::define_option(
          advgetopt::Name("--")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_NONE
                                                  , advgetopt::GETOPT_FLAG_MULTIPLE
                                                  , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
    ),

    // END
    advgetopt::end_options()
};



advgetopt::group_description const g_group_descriptions[] =
{
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_COMMANDS)
        , advgetopt::GroupName("command")
        , advgetopt::GroupDescription("Commands:")
    ),
    advgetopt::define_group(
          advgetopt::GroupNumber(advgetopt::GETOPT_FLAG_GROUP_OPTIONS)
        , advgetopt::GroupName("option")
        , advgetopt::GroupDescription("Options:")
    ),
    advgetopt::end_groups()
};




// until we have C++20, remove warnings this way
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "snapwebsites",
    .f_group_name = nullptr,
    .f_options = g_command_line_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = nullptr,
    .f_environment_variable_intro = nullptr,
    .f_section_variables_name = nullptr,
    .f_configuration_files = nullptr,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [--<opt>] <config-name> ...\n"
                     "where --<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = SNAPLOGGER_VERSION_STRING,
    .f_license = "GNU GPL v2",
    .f_copyright = "Copyright (c) 2013-"
                   BOOST_PP_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};
#pragma GCC diagnostic pop






class tool
{
public:
    int                             init(int argc, char * argv[]);

private:
    advgetopt::getopt::pointer_t    f_opt = advgetopt::getopt::pointer_t();
};



int tool::init(int argc, char * argv[])
{
    f_opt = std::make_shared<advgetopt::getopt>(g_options_environment);

    snaplogger::logger::get_instance()->add_default_field("tool", "snaploggerchecker");
    snaplogger::add_logger_options(*f_opt);

    f_opt->finish_parsing(argc, argv);

    if(!snaplogger::process_logger_options(*f_opt))
    {
        // exit on any error
        throw advgetopt::getopt_exit("logger options generated an error.", 0);
    }

    return 0;
}



}
// no name namespace





int main(int argc, char * argv[])
{
    try
    {
        tool t;
        if(t.init(argc, argv) != 0)
        {
            return 0;
        }

        return 0;
    }
    catch(advgetopt::getopt_exit const &)
    {
        return 0;
    }
    catch(std::exception const & e)
    {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;
    }
}


// vim: ts=4 sw=4 et
