// Copyright (c) 2025  Made to Order Software Corp.  All Rights Reserved
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
#include    <snaplogger/convert_ansi.h>
#include    <snaplogger/options.h>
#include    <snaplogger/version.h>


// advgetopt
//
#include    <advgetopt/exception.h>
#include    <advgetopt/options.h>
//#include    <advgetopt/utils.h>


// cppthread
//
//#include    <cppthread/log.h>


// snapdev
//
//#include    <snapdev/glob_to_list.h>
#include    <snapdev/stringize.h>


// last include
//
#include    <snapdev/poison.h>





/** \file
 * \brief Tool to convert ANSI output to plain text, HTML, or Markdown.
 *
 * This tool is useful to convert console output (a.k.a. including ANSI
 * or Escape codes) to plain text, HTML, or Markdown.
 *
 * HTML will include the colors and font styles. Markdown is currently
 * limited to bold and italic (although it supports HTML tags, the point
 * is to keep the out readable).
 */

namespace
{


advgetopt::option const g_options[] =
{
    // OPTIONS
    //
    advgetopt::define_option(
          advgetopt::Name("br")
        , advgetopt::ShortName('b')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("use <br/> along \\n characters (default).")
    ),
    advgetopt::define_option(
          advgetopt::Name("html")
        , advgetopt::ShortName('H')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("generate HTML (default).")
    ),
    advgetopt::define_option(
          advgetopt::Name("markdown")
        , advgetopt::ShortName('M')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("generate Markdown.")
    ),
    advgetopt::define_option(
          advgetopt::Name("no-br")
        , advgetopt::ShortName('B')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("do NOT use <br/> along \\n characters.")
    ),
    advgetopt::define_option(
          advgetopt::Name("optimize")
        , advgetopt::ShortName('O')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("use smaller HTML tag whenever possible.")
    ),
    advgetopt::define_option(
          advgetopt::Name("output")
        , advgetopt::ShortName('o')
        , advgetopt::Flags(advgetopt::all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                    , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::DefaultValue("-")
        , advgetopt::Help("filename to output to or '-' for stdout.")
    ),
    advgetopt::define_option(
          advgetopt::Name("output-style-tag")
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("first output the style tag, then the converted data (HTML only).")
    ),
    advgetopt::define_option(
          advgetopt::Name("text")
        , advgetopt::ShortName('T')
        , advgetopt::Flags(advgetopt::standalone_all_flags<
                      advgetopt::GETOPT_FLAG_GROUP_OPTIONS>())
        , advgetopt::Help("generate plain text.")
    ),

    // FILENAMES
    //
    advgetopt::define_option(
          advgetopt::Name("filename")
        , advgetopt::Flags(advgetopt::command_flags<
                      advgetopt::GETOPT_FLAG_GROUP_NONE
                    , advgetopt::GETOPT_FLAG_MULTIPLE
                    , advgetopt::GETOPT_FLAG_DEFAULT_OPTION>())
        , advgetopt::DefaultValue("-")
    ),

    // END
    //
    advgetopt::end_options()
};



char const * g_configuration_directories[] =
{
    "/etc/snaplogger",
    nullptr
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


advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "snaplogger",
    //.f_group_name = nullptr,
    .f_options = g_options,
    //.f_options_files_directory = nullptr,
    .f_environment_variable_name = "CONVERT_ANSI",
    //.f_environment_variable_intro = nullptr,
    //.f_section_variables_name = nullptr,
    //.f_configuration_files = nullptr,
    .f_configuration_filename = "convert-ansi.conf",
    .f_configuration_directories = g_configuration_directories,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [--<opt>] [<console-output.txt> | -]\n"
                     "where --<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = SNAPLOGGER_VERSION_STRING,
    .f_license = "GNU GPL v2",
    .f_copyright = "Copyright (c) 2025-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    //.f_build_date = UTC_BUILD_DATE,
    //.f_build_time = UTC_BUILD_TIME,
    .f_groups = g_group_descriptions
};







class converter
{
public:
                                    converter(int argc, char * argv[]);
    int                             run();

private:
    advgetopt::getopt               f_opt;
    snaplogger::convert_ansi::pointer_t
                                    f_converter = snaplogger::convert_ansi::pointer_t();
};



converter::converter(int argc, char * argv[])
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
        throw advgetopt::getopt_exit("logger options generated an error.", 1);
    }

    if((f_opt.is_defined("text")     ? 1 : 0)
     + (f_opt.is_defined("html")     ? 1 : 0)
     + (f_opt.is_defined("markdown") ? 1 : 0) >= 2)
    {
        throw advgetopt::getopt_exit("--text, --html, and --markdown are mutually exclusive.", 1);
    }
}


int converter::run()
{
    // create the converter
    //
    snaplogger::ansi_output_t type(snaplogger::ansi_output_t::ANSI_OUTPUT_HTML);
    if(f_opt.is_defined("text"))
    {
        type = snaplogger::ansi_output_t::ANSI_OUTPUT_PLAIN_TEXT;
    }
    else if(f_opt.is_defined("markdown"))
    {
        type = snaplogger::ansi_output_t::ANSI_OUTPUT_MARKDOWN;
    }
    f_converter = std::make_shared<snaplogger::convert_ansi>(type);

    if(f_opt.is_defined("optimize"))
    {
        f_converter->set_optimize();
    }

    if(f_opt.is_defined("br"))
    {
        f_converter->set_br();
    }

    // read input and save it in the converter
    //
    std::string const input_filename(f_opt.get_string("--"));
    if(input_filename == "-")
    {
        std::string line;
        while(std::getline(std::cin, line))
        {
            f_converter->write(line + '\n');
        }
    }
    else
    {
        std::ifstream in;
        in.open(input_filename);
        if(!in.is_open())
        {
            std::cerr
                << "error: could not open \""
                << input_filename
                << "\".\n";
            return 1;
        }
        std::string line;
        while(std::getline(in, line))
        {
            f_converter->write(line + '\n');
        }
    }

    std::string const result(f_converter->read());
    std::string styles(f_opt.is_defined("output-style-tag") ? f_converter->get_styles() : std::string());
    if(!styles.empty())
    {
        styles = "<style>\n" + styles + "</style>\n";
    }

    std::string const output_filename(f_opt.get_string("output"));
    if(output_filename == "-")
    {
        std::cout << styles << result;
    }
    else
    {
        std::ofstream out;
        out.open(input_filename);
        if(!out.is_open())
        {
            std::cerr
                << "error: could not open \""
                << output_filename
                << "\".\n";
            return 1;
        }
        out << styles << result;
    }

    return 0;
}



}
// no name namespace





int main(int argc, char * argv[])
{
    try
    {
        converter t(argc, argv);
        return t.run();
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
