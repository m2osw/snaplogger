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
#include    <snaplogger/exception.h>
#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
#include    <snaplogger/message.h>
#include    <snaplogger/severity.h>
#include    <snaplogger/version.h>


// C lib
//
#include    <unistd.h>
#include    <netdb.h>
#include    <sys/param.h>





CATCH_TEST_CASE("variable_param", "[variable][param][error]")
{
    CATCH_START_SECTION("variable: Param Name is Mandatory")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  new snaplogger::param(std::string())
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: a parameter must have a non-empty name."));
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_variable", "[variable][param]")
{
    CATCH_START_SECTION("variable: get_type() to use padding as integer or string (hostname)")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-logging");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${hostname:padding=3:align=right:min_width=30} ${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        char host[HOST_NAME_MAX + 2 + 30];
        CATCH_REQUIRE(gethostname(host, HOST_NAME_MAX + 1) == 0);
        host[HOST_NAME_MAX + 1] = '\0';
        std::string aligned(host);
        while(aligned.length() < 30)
        {
            aligned = "3" + aligned;
        }

        SNAP_LOG_ERROR << "Check the param::get_type()" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == aligned + " Check the param::get_type()\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"t\":align=center:min_width=30} ${message}");
        buffer->set_format(f);

        aligned = host;
        std::string::size_type const low((30 - aligned.length()) / 2 + aligned.length());
        while(aligned.length() < low)
        {
            aligned = "t" + aligned;
        }
        while(aligned.length() < 30)
        {
            aligned = aligned + "t";
        }

        SNAP_LOG_ERROR << "Try again with a string" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == aligned + " Try again with a string\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=center:max_width=30}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "This message will have a maximum width of 30 chars" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " ge will have a maximum width o\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=right:max_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "This message will have a maximum width of 25 chars" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " maximum width of 25 chars\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=center:min_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "minimum width 25" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " ttttminimum width 25ttttt\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=left:min_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "minimum width 25" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " minimum width 25ttttttttt\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=left:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "First we get this message cut to the specified width." << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " First we get this message\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=center:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "First we get this message cut to the specified width." << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " his message cut to the sp\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"t\":align=right:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "First we get this message cut to the specified width." << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " t to the specified width.\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"x\":align=left:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Small Message" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " Small Messagexxxxxxxxxxxx\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"x\":align=center:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Small Message" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " xxxxxxSmall Messagexxxxxx\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"x\":align=center:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Small Message (even)" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " xxSmall Message (even)xxx\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:padding=\"x\":align=right:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Small Message" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " xxxxxxxxxxxxSmall Message\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:prepend=\"(P) \":padding=\"z\":align=right:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Small Message" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " zzzzzzzz(P) Small Message\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:append=\" (A)\":padding=\"z\":align=right:exact_width=25}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Small Message" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " zzzzzzzzSmall Message (A)\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"q\":align=101:min_width=30} ${message}");
        buffer->set_format(f);

        snaplogger::message::pointer_t msg(std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR));
        *msg << "The align=101 parameter is the wrong type";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the ${...:align=<value>} parameter must be a valid string (not an integer)."));

        // this is important here because we want to make sure that the
        // `message` destructor works as expected (i.e. it does not call
        // std::terminate() because of the throw as the align=101 is
        // invalid)
        //
        msg.reset();

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"t\":align=justify:min_width=30} ${message}");
        buffer->set_format(f);

        msg = std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR);
        *msg << "Try align=\"justify\" which has to fail.";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the ${...:align=left|center|right} was expected, got \"justify\"."));

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"q\":align=left:min_width=wide} ${message}");
        buffer->set_format(f);

        msg = std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR);
        *msg << "The min_width=wide parameter is the wrong type";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the ${...:min_width=<value>} parameter must be a valid integer."));

        // this is important here because we want to make sure that the
        // `message` destructor works as expected (i.e. it does not call
        // std::terminate() because of the throw as the align=101 is
        // invalid)
        //
        msg.reset();

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=99:align=left:min_width=wide} ${message}");
        buffer->set_format(f);

        msg = std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR);
        *msg << "The padding=... accepts a number between 0 and 9 inclusive";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the ${...:padding=<value>} when set to a number must be one digit ('0' to '9'), not \"99\"."));

        // this is important here because we want to make sure that the
        // `message` destructor works as expected (i.e. it does not call
        // std::terminate() because of the throw as the align=101 is
        // invalid)
        //
        msg.reset();

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding='abc':align=left:min_width=wide} ${message}");
        buffer->set_format(f);

        msg = std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR);
        *msg << "The padding=... accepts one character";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the ${...:padding=' '} must be exactly one character, not \"abc\"."));

        // this is important here because we want to make sure that the
        // `message` destructor works as expected (i.e. it does not call
        // std::terminate() because of the throw as the align=101 is
        // invalid)
        //
        msg.reset();

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: escape")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "escape");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${hostname} ${message:escape}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        char host[HOST_NAME_MAX + 2 + 30];
        CATCH_REQUIRE(gethostname(host, HOST_NAME_MAX + 1) == 0);
        host[HOST_NAME_MAX + 1] = '\0';

        SNAP_LOG_ERROR << "Default escape for newline (\n), carriage return (\r), and tab (\t)" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " Default escape for newline (\\n), carriage return (\\r), and tab (\\t)\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:escape=\"[]\"}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Try again [with a string]" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " Try again \\[with a string\\]\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname} ${message:escape=\"\a\b\f\n\r\t\v\x1f\xC2\x88\xC2\x97\"}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "Escape all \a\b\f\n\r\t\v\x1f\xC2\x88\xC2\x97 types" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == std::string(host) + " Escape all \\a\\b\\f\\n\\r\\t\\v^_@H@W types\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: caps")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "caps");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message:caps}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_ERROR << "this message words will get their FIRST-LETTER capitalized." << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "This Message Words Will Get Their First-Letter Capitalized.\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: lower/upper")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "case");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message:lower}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_ERROR << "This message words will get their FIRST-LETTER capitalized." << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "this message words will get their first-letter capitalized.\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${message:upper}");
        buffer->set_format(f);

        SNAP_LOG_ERROR << "This message words will get their FIRST-LETTER capitalized." << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == "THIS MESSAGE WORDS WILL GET THEIR FIRST-LETTER CAPITALIZED.\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: default align value")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-variable-default-param");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${hostname:max_width=3} ${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        char host[HOST_NAME_MAX + 2 + 30];
        CATCH_REQUIRE(gethostname(host, HOST_NAME_MAX + 1) == 0);
        host[HOST_NAME_MAX + 1] = '\0';
        std::string aligned(host);
        aligned = aligned.substr(0, 3);
        while(aligned.length() < 3)
        {
            aligned = " " + aligned;
        }

        SNAP_LOG_ERROR << "<- first three letters of hostname" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == aligned + " <- first three letters of hostname\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: systemd severity")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-variable-systemd-severity");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${severity:format=systemd} ${message} (${severity:format=alpha})"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_EMERGENCY << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;
        SNAP_LOG_ALERT << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;
        SNAP_LOG_CRIT << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;
        SNAP_LOG_ERROR << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;
        SNAP_LOG_WARNING << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;
        SNAP_LOG_MINOR << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;        // a.k.a. a NOTICE for syslog
        SNAP_LOG_INFO << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;
        SNAP_LOG_DEBUG << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() == "<0> <- severity tag for systemd/syslog (emergency)\n"
                                       "<1> <- severity tag for systemd/syslog (alert)\n"
                                       "<2> <- severity tag for systemd/syslog (critical)\n"
                                       "<3> <- severity tag for systemd/syslog (error)\n"
                                       "<4> <- severity tag for systemd/syslog (warning)\n"
                                       "<5> <- severity tag for systemd/syslog (minor)\n"
                                       "<6> <- severity tag for systemd/syslog (information)\n");

        l->set_severity(::snaplogger::severity_t::SEVERITY_ALL);
        SNAP_LOG_DEBUG << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() == "<0> <- severity tag for systemd/syslog (emergency)\n"
                                       "<1> <- severity tag for systemd/syslog (alert)\n"
                                       "<2> <- severity tag for systemd/syslog (critical)\n"
                                       "<3> <- severity tag for systemd/syslog (error)\n"
                                       "<4> <- severity tag for systemd/syslog (warning)\n"
                                       "<5> <- severity tag for systemd/syslog (minor)\n"
                                       "<6> <- severity tag for systemd/syslog (information)\n"
                                       "<7> <- severity tag for systemd/syslog (debug)\n");
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: systemd severity with an invalid format")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-variable-systemd-severity-invalid-format");

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

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${severity:format=invalid} ${message} (${severity:format=alpha})"));
        buffer->set_format(f);

        l->add_appender(buffer);

        CATCH_REQUIRE_THROWS_MATCHES(
                  SNAP_LOG_MAJOR << "<- severity tag for systemd/syslog" << SNAP_LOG_SEND
                , snaplogger::invalid_variable
                , Catch::Matchers::ExceptionMessage(
                      "logger_error: the ${severity:format=alpha|number|systemd}"
                      " variable cannot be set to \"invalid\"."));

        CATCH_REQUIRE(buffer->str() == "");
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("duplicate_factory", "[variable][factory]")
{
    CATCH_START_SECTION("variable: attempt dynamically creating a factory which already exists")
    {
        class fake_variable_factory final
            : public snaplogger::variable_factory
        {
        public:
            fake_variable_factory()
                : variable_factory("version")
            {
            }

            virtual snaplogger::variable::pointer_t create_variable() override final
            {
                // we can't even register this one so returning an empty
                // pointer is perfectly safe here
                //
                return snaplogger::variable::pointer_t();
            }
        };

        CATCH_REQUIRE_THROWS_MATCHES(
                  snaplogger::register_variable_factory(std::make_shared<fake_variable_factory>())
                , snaplogger::duplicate_error
                , Catch::Matchers::ExceptionMessage("logger_error: trying to add two variable factories of type \"version\"."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: attempt creating a variable with a non-existant type")
    {
        CATCH_REQUIRE(snaplogger::get_variable("fake") == nullptr);
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("variable: attempt creating a function factory with an existing name")
    {
        class fake_function final
            : public snaplogger::function
        {
        public:
            fake_function()
                : function("padding")
            {
            }

            virtual void apply(
                  ::snaplogger::message const & msg
                , ::snaplogger::function_data & d
                , ::snaplogger::param::pointer_t const & p) override
            {
                snapdev::NOT_USED(msg, d, p);
            }
        };

        CATCH_REQUIRE_THROWS_MATCHES(
                  snaplogger::register_function(std::make_shared<fake_function>())
                , snaplogger::duplicate_error
                , Catch::Matchers::ExceptionMessage("logger_error: trying to add two functions named \"padding\"."));
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
