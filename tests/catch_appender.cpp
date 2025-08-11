// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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

// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/buffer_appender.h>
#include    <snaplogger/exception.h>
#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
#include    <snaplogger/message.h>
#include    <snaplogger/severity.h>
#include    <snaplogger/version.h>


// C
//
#include    <unistd.h>
#include    <netdb.h>
#include    <sys/param.h>





CATCH_TEST_CASE("appender", "[appender]")
{
    CATCH_START_SECTION("appender: create")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "appender");

        snaplogger::appender::pointer_t unknown(snaplogger::create_appender("unknown", "test-buffer"));
        CATCH_REQUIRE(unknown == nullptr);

        snaplogger::appender::pointer_t buffer(snaplogger::create_appender("buffer", "test-buffer"));
        CATCH_REQUIRE(buffer != nullptr);
        CATCH_REQUIRE(buffer->get_type() == "buffer");
        CATCH_REQUIRE(buffer->get_name() == "test-buffer");

        // the name cannot be changed because it is not "console" or "syslog"
        //
        CATCH_REQUIRE_THROWS_MATCHES(
                  buffer->set_name("not-available")
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                            "logger_error: the appender set_name() can only be used for the console & syslog appenders to rename them to your own appender name (and done internally only)."));

        char const * cargv[] =
        {
            "/usr/bin/daemon",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${severity}: ${message}"));
        buffer->set_format(f);
        CATCH_REQUIRE(buffer->get_format() == f);

        CATCH_REQUIRE(buffer->get_bytes_per_minute() == 0);
        CATCH_REQUIRE(buffer->get_bitrate_dropped_messages() == 0);

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        l->add_appender(buffer);

        CATCH_REQUIRE(buffer->is_enabled());

        SNAP_LOG_FATAL << "Appender created by name" << SNAP_LOG_SEND;
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::buffer_appender>(buffer)->str() == "fatal: Appender created by name\n");

        buffer->set_enabled(false);
        CATCH_REQUIRE_FALSE(buffer->is_enabled());

        SNAP_LOG_FATAL << "Another message when disabled does not make it" << SNAP_LOG_SEND;
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::buffer_appender>(buffer)->str() == "fatal: Appender created by name\n");

        CATCH_REQUIRE_FALSE(buffer->unique());

        // nothing happens by default
        //
        buffer->reopen();

        // change severity
        //
        CATCH_REQUIRE(buffer->get_severity() == snaplogger::severity_t::SEVERITY_INFORMATION);
        buffer->set_severity(snaplogger::severity_t::SEVERITY_ERROR);
        CATCH_REQUIRE(buffer->get_severity() == snaplogger::severity_t::SEVERITY_ERROR);

        buffer->set_enabled(true);
        CATCH_REQUIRE(buffer->is_enabled());

        SNAP_LOG_INFORMATION << "Severity prevents this message from going in" << SNAP_LOG_SEND;
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::buffer_appender>(buffer)->str() == "fatal: Appender created by name\n");

        buffer->reduce_severity(snaplogger::severity_t::SEVERITY_FATAL);
        CATCH_REQUIRE(buffer->get_severity() == snaplogger::severity_t::SEVERITY_ERROR);
        buffer->reduce_severity(snaplogger::severity_t::SEVERITY_INFORMATION);
        CATCH_REQUIRE(buffer->get_severity() == snaplogger::severity_t::SEVERITY_INFORMATION);

        snaplogger::format::pointer_t g(std::make_shared<snaplogger::format>("appender[${severity}]:${line}: ${message}"));
        std::shared_ptr<snaplogger::safe_format> safe(std::make_shared<snaplogger::safe_format>(buffer, g));
        CATCH_REQUIRE(buffer->get_format() == g);

        // the format includes the line number so we need to save that as it can
        // move each time we edit this file...
        //
        int const line = __LINE__;
        SNAP_LOG_TODO << "Complete the tests to 100%." << SNAP_LOG_SEND;

        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::buffer_appender>(buffer)->str()
                == "fatal: Appender created by name\n"
                   "appender[incomplete task]:" + std::to_string(line + 1) + ": Complete the tests to 100%.\n");

        safe.reset();
        CATCH_REQUIRE(buffer->get_format() == f);

        buffer->increase_severity(snaplogger::severity_t::SEVERITY_DEBUG);
        CATCH_REQUIRE(buffer->get_severity() == snaplogger::severity_t::SEVERITY_INFORMATION);
        buffer->increase_severity(snaplogger::severity_t::SEVERITY_MAJOR);
        CATCH_REQUIRE(buffer->get_severity() == snaplogger::severity_t::SEVERITY_MAJOR);

        snaplogger::appender::pointer_t other_buffer(snaplogger::create_appender("buffer", "other-buffer"));
        CATCH_REQUIRE(*other_buffer < *buffer);

        l->reset();
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
