// Copyright (c) 2006-2023  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// snaplogger lib
//
#include    <snaplogger/buffer_appender.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
#include    <snaplogger/message.h>


// C lib
//
#include    <unistd.h>



CATCH_TEST_CASE("example", "[example]")
{
    CATCH_START_SECTION("asynchronous: Simple logging")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "async-unittest");
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_VERSION, "1.0");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        char const * cargv[] =
        {
            "/usr/bin/daemon",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "async-unittest";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${progname}: ${severity}: ${message} (${version})"));
        buffer->set_format(f);

        l->add_appender(buffer);

        l->set_asynchronous(true);

        // the cppthread library generates messages "enter" and "exit"
        // we need to prevent those because we can't easily compare
        // the results
        //
        l->add_component_to_ignore(snaplogger::g_cppthread_component);

        //l->add_console_appender()->add_component(snaplogger::g_secure_component);
        //l->add_component_to_ignore(snaplogger::g_normal_component);
        //l->add_component_to_include(snaplogger::g_normal_component);

    	SNAP_LOG_WARNING
            << "Sent through thread..."
            << SNAP_LOG_SEND;

        // this call blocks until the thread stopped and joined
        //
        l->set_asynchronous(false);

        // TODO: add the ${tid} as one of the message parameter and a way
        //       to retrieve the tid of the async. thread
        //
        CATCH_REQUIRE(buffer->str() == "async-unittest: warning: Sent through thread... (1.0)\n");
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
