// Copyright (c) 2006-2025  Made to Order Software Corp.  All Rights Reserved
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

// header being tested
//
#include    <snaplogger/message.h>


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
#include    <snaplogger/severity.h>
#include    <snaplogger/version.h>


// snapdev
//
#include    <snapdev/enum_class_math.h>


// advgetopt
//
#include    <advgetopt/exception.h>


// C
//
#include    <math.h>
#include    <unistd.h>


namespace
{


std::source_location get_other_loc()
{
    return std::source_location::current();
}



} // no name namespace


CATCH_TEST_CASE("not_a_message", "[message]")
{
    CATCH_START_SECTION("message: Call send_message() with wrong ostream")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  snaplogger::send_message(std::cout)
                , snaplogger::not_a_message
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the 'out' parameter to the send_message() function is expected to be a snaplogger::message object."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: Print snaplogger::secure to wrong ostream")
    {
        std::stringstream buffer;
        std::streambuf * old(std::cout.rdbuf(buffer.rdbuf()));
        std::cout << snaplogger::secure << std::endl;
        CATCH_REQUIRE(buffer.str() == "(section:secure)\n");
        std::cout.rdbuf(old);
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("message_capture", "[message]")
{
    CATCH_START_SECTION("message: Buffering")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-logging");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        CATCH_REQUIRE(buffer->get_type() == "buffer");

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

        l->add_appender(buffer);

        SNAP_LOG_ERROR << snaplogger::precise_time << "Logging this error" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "error: Logging this error\n");

        // test the other str() function too
        //
        buffer->str("Start: ");

        // show that the "\n" does not get duplicated
        //
        SNAP_LOG_ERROR << "Error with newline\n" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "Start: error: Error with newline\n");
        buffer->clear();

        // show that the "\r\n" gets replaced by "\n"
        //
        SNAP_LOG_ERROR << "Error with CRLF\r\n" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "error: Error with CRLF\n");
        buffer->clear();

        // severity too low, no change to buffer
        //
        SNAP_LOG_DEBUG << "Debug Message " << M_PI << " which does not make it at all...\n" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->empty());

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: JSON Buffering")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "json-logging");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("json-buffer"));

        CATCH_REQUIRE(buffer->get_type() == "buffer");

        char const * cargv[] =
        {
            "/usr/bin/daemon",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "json-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>(
                    "{\"version\":1,"
                    "\"message\":\"${message:escape='\\\r\n\t\"'}\"}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_ERROR
            << SNAP_LOG_FIELD(std::string("format"), std::string("json"))
            << "A JSON error message (format:${field:name=format})"
            << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "{\"version\":1,\"message\":\"A JSON error message (format:json)\"}\n");
        buffer->clear();

        // show that the "\n" does not get duplicated
        //
        SNAP_LOG_ERROR
            << "See what happens with a \"quoted string\" within the message (${fields})\n"
            << SNAP_LOG_FIELD(std::string("format"), std::string("json"))
            << SNAP_LOG_FIELD(std::string("language"), std::string("js"))
            << SNAP_LOG_SEND;
        std::string const expected(std::regex_replace(buffer->str(), std::regex("\\\\\"id\\\\\":\\\\\"[0-9]+\\\\\","), ""));
        CATCH_REQUIRE(expected == "{\"version\":1,\"message\":\"See what happens with a \\\"quoted string\\\" within the message ({\\\"format\\\":\\\"json\\\",\\\"language\\\":\\\"js\\\"})\"}\n");
        buffer->clear();

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("message_copy", "[message]")
{
    CATCH_START_SECTION("message: Copy")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-copying");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        CATCH_REQUIRE(buffer->get_type() == "buffer");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        std::source_location const loc(std::source_location::current());
        snaplogger::message::pointer_t msg(std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR, loc));

        CATCH_REQUIRE(msg->get_filename() == loc.file_name());
        CATCH_REQUIRE(msg->get_function() == loc.function_name());
        CATCH_REQUIRE(msg->get_line() == loc.line());
        CATCH_REQUIRE(msg->get_column() == loc.column());

        // using a standard location
        //
        std::source_location const other_loc(get_other_loc());

        CATCH_REQUIRE(msg->get_filename() == loc.file_name());
        CATCH_REQUIRE(msg->get_function() == loc.function_name());
        CATCH_REQUIRE(msg->get_line() == loc.line());
        CATCH_REQUIRE(msg->get_column() == loc.column());

        // direct
        //
        msg->set_filename("we-are-under-control.cpp");
        msg->set_function("testing_set_function");
        msg->set_line(123);
        msg->set_column(64);

        CATCH_REQUIRE(msg->get_filename() == "we-are-under-control.cpp");
        CATCH_REQUIRE(msg->get_function() == "testing_set_function");
        CATCH_REQUIRE(msg->get_line() == 123);
        CATCH_REQUIRE(msg->get_column() == 64);

        *msg << "Logging an error.";

        CATCH_REQUIRE(msg->str() == "Logging an error.");

        snaplogger::message::pointer_t copy(std::make_shared<snaplogger::message>(*msg, *msg));

        CATCH_REQUIRE(msg->str() == "Logging an error.");
        CATCH_REQUIRE(copy->str() == "Logging an error.");

        // no destructor called, the output is still empty
        //
        CATCH_REQUIRE(buffer->empty());

        copy.reset();

        // msg not lost
        //
        CATCH_REQUIRE(msg->str() == "Logging an error.");

        // destructor against copy does not trigger send_message()
        //
        CATCH_REQUIRE(buffer->empty());

        snaplogger::send_message(*msg);

        // now we get the message as expected!
        //
        // (note that internally we can skip receiving the message on the
        // original, but not as a client... we may want to have the ability
        // to cancel a message, though.)
        //
        CATCH_REQUIRE(buffer->str() == "Logging an error.\n");

        msg.reset();

        CATCH_REQUIRE(buffer->str() == "Logging an error.\n");

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("message_severity", "[message][severity]")
{
    CATCH_START_SECTION("message: Appender vs Message severity")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-severity");

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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        // ++<enum> works here because of this header:
        //    <snapdev/enum_class_math.h>
        //
        for(snaplogger::severity_t i(snaplogger::severity_t::SEVERITY_MIN); i <= snaplogger::severity_t::SEVERITY_MAX; ++i)
        {
            buffer->set_severity(i);
            for(snaplogger::severity_t j(snaplogger::severity_t::SEVERITY_MIN); j <= snaplogger::severity_t::SEVERITY_MAX; ++j)
            {
                snaplogger::send_message(*::snaplogger::create_message(j)
                                                << "The message itself");

                if(j >= i
                && i != snaplogger::severity_t::SEVERITY_OFF
                && j != snaplogger::severity_t::SEVERITY_OFF)
                {
                    CATCH_REQUIRE(buffer->str() == "The message itself\n");
                }
                else
                {
                    CATCH_REQUIRE(buffer->empty());
                }
                buffer->clear();
            }
        }

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: Changing message severity (takes about 3.5min)")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-copying");

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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        // <enum> += n works here because of this header:
        //    <snapdev/enum_class_math.h>
        //
        for(snaplogger::severity_t i(snaplogger::severity_t::SEVERITY_MIN); i <= snaplogger::severity_t::SEVERITY_MAX; i += 1 + (rand() & 15))
        {
            buffer->set_severity(i);
            for(snaplogger::severity_t j(snaplogger::severity_t::SEVERITY_MIN); j <= snaplogger::severity_t::SEVERITY_MAX; j += 1 + (rand() & 15))
            {
                for(snaplogger::severity_t k(snaplogger::severity_t::SEVERITY_MIN); k <= snaplogger::severity_t::SEVERITY_MAX; k += 1 + (rand() & 15))
                {
                    ::snaplogger::message::pointer_t msg(std::make_shared<::snaplogger::message>(j));
                    *msg << "Start of message";
                    msg->set_severity(k);
                    *msg << " -- end of message";
                    snaplogger::send_message(*msg);
//std::cerr << "checking with " << i << ", " << j << ", " << k << "\n";

                    if(j >= i
                    && k >= i
                    && i != snaplogger::severity_t::SEVERITY_OFF
                    && j != snaplogger::severity_t::SEVERITY_OFF)
                    //&& k != snaplogger::severity_t::SEVERITY_OFF)
                    {
                        if(j >= i)
                        {
                            CATCH_REQUIRE(buffer->str() == "Start of message -- end of message\n");
                        }
                        else
                        {
                            CATCH_REQUIRE(buffer->str() == "Start of message\n");
                        }
                    }
                    else
                    {
                        CATCH_REQUIRE(buffer->empty());
                    }
                    buffer->clear();
                }
            }
        }

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("message_format", "[message][format]")
{
    CATCH_START_SECTION("message: Recursivity")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "basic-format");

        // these two are not called in this test
        //
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROJECT_NAME, "test-logger");
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_VERSION, "5.32.1024");

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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_version = "5.32.1024";
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${project_name} ${message} v${version}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_WARNING
            << "Message Project Name = ${project_name} and Version = ${version} -- uses \"recursive\""
            << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() ==
                "test-logger Message Project Name = test-logger and"
                " Version = 5.32.1024 -- uses \"recursive\" v5.32.1024"
                "\n");

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: ${message} itself is not recursive")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "prevent-infinite-loop");

        // these two are not called in this test
        //
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROJECT_NAME, "test-logger");
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_VERSION, "5.32.1024");

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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_version = "5.32.1024";
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${project_name} ${message} v${version}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_WARNING
            << "Message ${message} says: Project Name = ${project_name} and Version = ${version} -- uses \"recursive\""
            << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() ==
                "test-logger Message  says: Project Name = test-logger and"
                " Version = 5.32.1024 -- uses \"recursive\" v5.32.1024"
                "\n");

        buffer->clear();

        snaplogger::unset_diagnostic(snaplogger::DIAG_KEY_VERSION);

        SNAP_LOG_WARNING
            << "Removed the version: ${message} says: Project Name = ${project_name} and Version = ${version} -- uses \"recursive\""
            << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() ==
                "test-logger Removed the version:  says: Project Name = test-logger and"
                " Version =  -- uses \"recursive\" v"
                "\n");

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: ${pid} uses the get_environment() function")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "get-environment");

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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_WARNING
            << "Test PID = ${pid} == ${pid:running}"
            << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() ==
                  "Test PID = " + std::to_string(getpid())
                + " == " + std::to_string(getpid())
                + "\n");

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: Verify year")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "get-environment");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        CATCH_REQUIRE(l->get_appender("test-buffer") == nullptr);

        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        CATCH_REQUIRE(l->get_appender("test-buffer") == buffer);

        // we create a message so we can check the timestamp in our test
        //
        snaplogger::message::pointer_t msg(std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR));
        *msg << "Created message on YYYY = ${date:year}, MM = ${date:month}, DD = ${date:day}";

        timespec const stamp(msg->get_timestamp());

        snaplogger::send_message(*msg);

        tm t;
        gmtime_r(&stamp.tv_sec, &t);
        char year[16];
        char month[16];
        char day[16];
        strftime(year,  16, "%Y", &t);
        strftime(month, 16, "%m", &t);
        strftime(day,   16, "%d", &t);

        CATCH_REQUIRE(buffer->str() ==
                  std::string("Created message on YYYY = ")
                + year
                + ", MM = "
                + std::to_string(std::atoi(month))  // remove the leading '0' if necessary
                + ", DD = "
                + std::to_string(std::atoi(day))    // remove the leading '0' if necessary
                + "\n");

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("message_component_filter", "[message][component]")
{
    CATCH_START_SECTION("message: Filter Message with Component")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "component-filter");

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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message} (${severity:format=number})"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_WARNING
            << snaplogger::secure       // mark as a secure message
            << "This message is secure but not the buffer"
            << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->empty());

        SNAP_LOG_WARNING
            << "Test number: "
            << 2
            << " with buffer still unsecure..."
            << SNAP_LOG_SEND_SECURELY;  // mark at the end

        CATCH_REQUIRE(buffer->empty());

        // mark the buffer as a secure buffer now
        //
        buffer->add_component(snaplogger::g_secure_component);

        SNAP_LOG_WARNING
            << snaplogger::secure       // mark as a secure message
            << "This message is secure and so is the buffer"
            << SNAP_LOG_SEND;

        // TODO: get the WARNING severity level dynamically
        std::string const expected1("This message is secure and so is the buffer ("
                + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_WARNING))
                + ")\n");
        CATCH_REQUIRE(buffer->str() == expected1);

        buffer->clear();

        SNAP_LOG_WARNING
            << "Test number: "
            << 4
            << " with secure buffer...\r\n"
            << SNAP_LOG_SEND_SECURELY;  // mark at the end

        // TODO: get the WARNING severity level dynamically
        std::string const expected2("Test number: 4 with secure buffer... ("
                + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_WARNING))
                + ")\n");
        CATCH_REQUIRE(buffer->str() == expected2);

        l->reset();
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("message_exception", "[message][exception]")
{
    CATCH_START_SECTION("message: Use '... << exception << ...'")
    {
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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>(
                "${message} (${severity:format=number}) exit code: ${field:name=exception_exit_code}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        advgetopt::getopt_exit const error("testing an exception -> logging", 123);

        SNAP_LOG_WARNING
            << "We got an exception! ["
            << error
            << "]"
            << SNAP_LOG_SEND;

        CATCH_REQUIRE_FALSE(buffer->empty());

        std::string const expected("We got an exception! [getopt_exception: testing an exception -> logging] ("
                                        + std::to_string(static_cast<int>(::snaplogger::severity_t::SEVERITY_WARNING))
                                        + ") exit code: 123\n");
        CATCH_REQUIRE(buffer->str() == expected);

        buffer->clear();

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("message: Use '... << stringstream << ...'")
    {
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
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message} (${severity:format=number})"));
        buffer->set_format(f);

        l->add_appender(buffer);

        std::stringstream ss;
        ss << "testing that we can also \"send\" the content of a string stream";

        SNAP_LOG_WARNING
            << "We got an exception! ["
            << ss
            << "]"
            << SNAP_LOG_SEND;

        CATCH_REQUIRE_FALSE(buffer->empty());

        std::string const expected("We got an exception! [testing that we can also \"send\" the content of a string stream] ("
                                        + std::to_string(static_cast<int>(::snaplogger::severity_t::SEVERITY_WARNING))
                                        + ")\n");
        CATCH_REQUIRE(buffer->str() == expected);

        buffer->clear();

        l->reset();
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
