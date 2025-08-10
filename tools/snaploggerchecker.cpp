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


/** \file
 * \brief Logger command line tool.
 *
 * This tool is mainly used to verify the logger configuration files.
 * These are fairly simple but it is still much better to verify
 * before you run your software (that way you can verify at compile
 * time instead of later when you may already have shipped your
 * software).
 *
 * \code
 *     snaploggerchecker \
 *         --verify \
 *         --config <filename> \
 *         ... (not implemented yet) ...
 * \endcode
 *
 * The tool can also log messages using any available configuration
 * file. This is practical to log messages from scripts or other
 * languages which do not support the snaplogger library.
 *
 * Here is an example:
 *
 * \code
 *     snaploggerchecker \
 *        --log \
 *        --console \
 *        --message "this sends a log to appenders" \
 *        --severity debug \
 *        --filename "my-script.sh" \
 *        --function "foo_bar()" \
 *        --line 123 \
 *        --column 34 \
 *        --trace
 * \endcode
 *
 * The \c --log means you want to send a log message.
 *
 * Here we show \c --console which sets up an appender to send the log to
 * the console (std::cerr).
 *
 * The \c --message defines the message to be logged.
 *
 * The \c --severity option is used to define the name or level number of
 * the severity to use to send that log message. Note that will be checked
 * against the logger severity level. If lower then the log will not happen.
 *
 * The \c --trace option is to lower the logger severity level so that way
 * the message gets through.
 *
 * To help with determining some of the parameters, you can use the list
 * commands:
 *
 * \li \c --list-appenders -- list known appenders
 * \li \c --list-components -- list available components
 * \li \c --list-formats -- list formats used to generate the string to output
 * \li \c --list-severities -- list all the available severity levels
 */


// snaplogger
//
#include    <snaplogger/exception.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/options.h>
#include    <snaplogger/version.h>


// advgetopt
//
#include    <advgetopt/exception.h>


// libexcept
//
#include    <libexcept/file_inheritance.h>


// snapdev
//
#include    <snapdev/stringize.h>


// last include
//
#include    <snapdev/poison.h>




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
          advgetopt::Name("log")
        , advgetopt::ShortName('l')
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("send a log to a set of appenders (make sure to use --trace if you want to send logs with any severity).")
    ),
    advgetopt::define_option(
          advgetopt::Name("verify")
        , advgetopt::Flags(advgetopt::standalone_command_flags<advgetopt::GETOPT_FLAG_GROUP_COMMANDS>())
        , advgetopt::Help("verify the snaplogger configuration files (this is the default if no command is specified).")
    ),

    // OPTIONS
    advgetopt::define_option(
          advgetopt::Name("column")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("column number the log is being generated from.")
    ),
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
          advgetopt::Name("severity")
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("the log severity (info, warn, error, etc.)")
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
        , advgetopt::ShortName('m')
        , advgetopt::Help("the message to log.")
    ),
    advgetopt::define_option(
          advgetopt::Name("output")
        , advgetopt::ShortName('o')
        , advgetopt::Flags(advgetopt::command_flags<advgetopt::GETOPT_FLAG_GROUP_OPTIONS
                                                  , advgetopt::GETOPT_FLAG_REQUIRED>())
        , advgetopt::Help("name of the output file (to save the compiled configuration files).")
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
        , advgetopt::ShortName('v')
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
advgetopt::options_environment const g_options_environment =
{
    .f_project_name = "snaploggerchecker",
    .f_group_name = "snaplogger",
    .f_options = g_command_line_options,
    .f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_PROCESS_SYSTEM_PARAMETERS,
    .f_help_header = "Usage: %p [--<opt>] <config-name> ...\n"
                     "where --<opt> is one or more of:",
    .f_help_footer = "%c",
    .f_version = SNAPLOGGER_VERSION_STRING,
    .f_license = "GNU GPL v3",
    .f_copyright = "Copyright (c) 2013-"
                   SNAPDEV_STRINGIZE(UTC_BUILD_YEAR)
                   " by Made to Order Software Corporation -- All Rights Reserved",
    .f_groups = g_group_descriptions
};
#pragma GCC diagnostic pop






class tool
{
public:
                                    tool(int argc, char * argv[]);

    int                             run();

private:
    typedef std::function<int()>    command_t;

    int                             determine_command();

    int                             list_components();
    int                             list_formats();
    int                             list_types();
    int                             log();
    int                             verify();

    advgetopt::getopt               f_opts;
    command_t                       f_command = nullptr;
};



tool::tool(int argc, char * argv[])
    : f_opts(g_options_environment)
{
    snaplogger::logger::get_instance()->add_default_field("tool", "snaploggerchecker");
    snaplogger::add_logger_options(f_opts);

    f_opts.finish_parsing(argc, argv);

    if(!snaplogger::process_logger_options(
              f_opts
            , "/etc/snaplogger/logger"
            , std::cout
            , !isatty(fileno(stdin))))
    {
        // exit on any error
        throw advgetopt::getopt_exit("logger options generated an error.", 0);
    }
}


int tool::run()
{
    int r;

    r = determine_command();
    if(r != 0)
    {
        return r;
    }

    f_command();

    return 0;
}


int tool::determine_command()
{
    struct command_name_pointer_t
    {
        char const *    f_name = nullptr;
        command_t       f_command = nullptr;
    };

    command_name_pointer_t const command_list[] =
    {
        { "list-components", std::bind(&tool::list_components, this) },
        { "list-formats",    std::bind(&tool::list_formats,    this) },
        { "log",             std::bind(&tool::log,             this) },
        { "verify",          std::bind(&tool::verify,          this) },
    };

    std::string first;
    for(auto const & c : command_list)
    {
        if(f_opts.is_defined(c.f_name))
        {
            if(f_command != nullptr)
            {
                SNAP_LOG_ERROR
                    << "you can only specify one command on the command line; found \""
                    << first
                    << "\" and \""
                    << c.f_name
                    << "\"."
                    << SNAP_LOG_SEND;
                return 1;
            }

            first = c.f_name;
            f_command = c.f_command;
        }
    }

    // if not specified, default to --verify
    //
    if(f_command == nullptr)
    {
        f_command = std::bind(&tool::verify, this);
    }

    return 0;
}


int tool::list_components()
{
    snaplogger::component::map_t const components(snaplogger::logger::get_instance()->get_component_list());

    std::cout << "Available components:\n";
    for(auto const & c : components)
    {
        std::cout << "  . " << c.first << '\n';
    }
    std::cout << std::endl;

    return 0;
}


int tool::list_formats()
{
    snaplogger::appender::vector_t const appenders(snaplogger::logger::get_instance()->get_appenders());
    for(auto const & a : appenders)
    {
        snaplogger::format::pointer_t f(a->get_format());

        std::cout
            << a->get_name()
            << ": "
            << f->get_format()
            << '\n';
    }
    std::cout << std::endl;

    return 0;
}


int tool::log()
{
    // get severity
    //
    snaplogger::severity_t sev(snaplogger::severity_t::SEVERITY_ERROR);
    if(f_opts.is_defined("severity"))
    {
        std::string const severity_name(f_opts.get_string("severity"));
        if(!severity_name.empty())
        {
            snaplogger::severity::pointer_t severity(snaplogger::get_severity(severity_name));
            if(severity == nullptr)
            {
                SNAP_LOG_RECOVERABLE_ERROR
                    << "the severity name \""
                    << severity_name
                    << "\" is not defined."
                    << SNAP_LOG_SEND;
            }
            else
            {
                sev = severity->get_severity();
            }
        }
    }

    // get the actual message
    //
    std::string message_string(f_opts.get_string("message"));
    if(message_string.empty())
    {
        SNAP_LOG_RECOVERABLE_ERROR
            << "the --message (-m) option is mandatory with the --log command."
            << SNAP_LOG_SEND;
        message_string = "default message";
    }

    snaplogger::message msg(sev);

    if(f_opts.is_defined("filename"))
    {
        msg.set_filename(f_opts.get_string("filename"));
    }
    if(f_opts.is_defined("function"))
    {
        msg.set_function(f_opts.get_string("function"));
    }
    if(f_opts.is_defined("line"))
    {
        msg.set_line(f_opts.get_long("line"));
    }
    if(f_opts.is_defined("column"))
    {
        msg.set_column(f_opts.get_long("column"));
    }

    msg << message_string;

    snaplogger::logger::pointer_t lg(snaplogger::logger::get_instance());

    lg->log_message(msg);

    return 0;
}


int tool::verify()
{
    SNAP_LOG_FATAL
        << snaplogger::section(snaplogger::g_normal_component)
        << snaplogger::section(snaplogger::g_not_implemented_component)
        << "The --verify command is not yet implemented."
        << SNAP_LOG_SEND;
    return 0;
}



}
// no name namespace





int main(int argc, char * argv[])
{
    libexcept::verify_inherited_files();
    libexcept::collect_stack_trace();

    try
    {
        tool t(argc, argv);
        return t.run();
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
