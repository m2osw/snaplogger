// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
//
// https://snapwebsites.org/project/snaplogger
// contact@m2osw.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// snaplogger
//
#include    <snaplogger/utils.h>
#include    <snaplogger/options.h>
#include    <snaplogger/version.h>


// advgetopt
//
#include    <advgetopt/exception.h>
#include    <advgetopt/options.h>
#include    <advgetopt/utils.h>


// snapdev
//
#include    <snapdev/stringize.h>


// C
//
#include    <string.h>


// last include
//
#include    <snapdev/poison.h>





/** \file
 * \brief Check whether a file is considered rotational or not.
 *
 * This tool is used to determine whether a file is on an HDD (exit with 0)
 * or not (exit with 1).
 *
 * On an error, exit with 2.
 */

namespace
{


advgetopt::option const g_options[] =
{
    // OPTIONS
    //
    advgetopt::define_option(
          advgetopt::Name("verbose")
        , advgetopt::ShortName('v')
        , advgetopt::Flags(advgetopt::standalone_command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("write to stdout on success showing defining the selection: \"rotational\" or \"solid state\".")
    ),

    // FILENAME/DIRECTORY
    //
    advgetopt::define_option(
          advgetopt::Name("filename")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_NONE
                    , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
    ),

    // END
    //
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
    .f_project_name = "snaplogger",
    .f_group_name = nullptr,
    .f_options = g_options,
    .f_options_files_directory = nullptr,
    .f_environment_variable_name = "IS_HDD",
    .f_environment_variable_intro = nullptr,
    .f_section_variables_name = nullptr,
    .f_configuration_files = nullptr,
    .f_configuration_filename = nullptr,
    .f_configuration_directories = nullptr,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [--<opt>] <filename>\n"
                     "where --<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = SNAPLOGGER_VERSION_STRING,
    .f_license = "GNU GPL v2",
    .f_copyright = "Copyright (c) 2013-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    .f_build_date = UTC_BUILD_DATE,
    .f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};
#pragma GCC diagnostic pop







class tool
{
public:
                                    tool(int argc, char * argv[]);

    int                             execute();

private:
    advgetopt::getopt               f_opt;
    bool                            f_verbose = false;
};



tool::tool(int argc, char * argv[])
    : f_opt(g_options_environment)
{
    snaplogger::add_logger_options(f_opt);
    f_opt.finish_parsing(argc, argv);
    if(!snaplogger::process_logger_options(
                  f_opt
                , "/etc/snaplogger/logger"
                , std::cout
                , false))
    {
        // exit on any error
        //
        throw advgetopt::getopt_exit("logger options generated an error.", 0);
    }

    f_verbose = f_opt.is_defined("verbose");
}


int tool::execute()
{
    if(f_opt.size("filename") == 0)
    {
        std::cerr << "error: a file or directory name is required.\n";
        return 2;
    }

    if(snaplogger::is_rotational(f_opt.get_string("filename")))
    {
        // it is rotational
        //
        if(f_verbose)
        {
            std::cout << "rotational\n";
        }
        return 0;
    }

    // error?
    //
    if(errno != 0)
    {
        int const e(errno);
        std::cerr
            << "error: an error occurred while checking the file or the disk on which it is defined: "
            << e
            << ", "
            << strerror(e)
            << '\n';
        return 2;
    }

    // not rotational
    //
    if(f_verbose)
    {
        std::cout << "solid state\n";
    }
    return 1;
}



}
// no name namespace



int main(int argc, char * argv[])
{
    try
    {
        tool t(argc, argv);
        return t.execute();
    }
    catch(advgetopt::getopt_exit const &)
    {
        return 0;
    }
    catch(std::exception const & e)
    {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}



// vim: ts=4 sw=4 et
