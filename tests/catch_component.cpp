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

// this test checks components
//
#include    <snaplogger/component.h>


// self
//
#include    "catch_main.h"


// snaplogger
//
#include    <snaplogger/buffer_appender.h>
#include    <snaplogger/exception.h>
#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/message.h>
#include    <snaplogger/options.h>






CATCH_TEST_CASE("component", "[component]")
{
    CATCH_START_SECTION("component: Write component to stream")
    {
        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_cppthread_component);
            CATCH_REQUIRE(ss.str() == "(section:cppthread)");
        }

        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_debug_component);
            CATCH_REQUIRE(ss.str() == "(section:debug)");
        }

        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_normal_component);
            CATCH_REQUIRE(ss.str() == "(section:normal)");
        }

        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_secure_component);
            CATCH_REQUIRE(ss.str() == "(section:secure)");
        }

        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_self_component);
            CATCH_REQUIRE(ss.str() == "(section:self)");
        }

        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_banner_component);
            CATCH_REQUIRE(ss.str() == "(section:banner)");
        }

        {
            std::stringstream ss;
            ss << snaplogger::section(snaplogger::g_not_implemented_component);
            CATCH_REQUIRE(ss.str() == "(section:not_implemented)");
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("component: Make sure creating component generates unique entries")
    {
        // this worked from the start since the private logger instance
        // uses a map to store the components
        //
        struct name_ptr
        {
            typedef std::vector<name_ptr>   vector_t;

            std::string     f_name = std::string();
            snaplogger::component::pointer_t
                            f_component = snaplogger::component::pointer_t();
        };
        name_ptr::vector_t names =
        {
            { "component1", },
            { "component2", },
            { "component3", },
            { "component4", },
            { "component5", },
            { "component6", },
            { "component7", },
            { "component8", },
            { "component9", },
            { "component10", },
        };

        // create the components
        //
        for(auto & p : names)
        {
            p.f_component = snaplogger::get_component(p.f_name);
        }

        // verify the component pointers
        //
        for(auto & p : names)
        {
            CATCH_REQUIRE(p.f_component == snaplogger::get_component(p.f_name));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("component: Make sure creating component generates unique entries")
    {
        // this worked from the start since the private logger instance
        // uses a map to store the components
        //
        struct name_ptr
        {
            typedef std::vector<name_ptr>   vector_t;

            std::string     f_name = std::string();
            snaplogger::component::pointer_t
                            f_component = snaplogger::component::pointer_t();
        };
        name_ptr::vector_t names =
        {
            { "component1", },
            { "component2", },
            { "component3", },
            { "component4", },
            { "component5", },
            { "Component6", },
            { "component7", },
            { "component8", },
            { "component9", },
            { "component10", },
        };

        // create the components
        //
        for(auto & p : names)
        {
            p.f_component = snaplogger::get_component(p.f_name);
        }

        // verify the component pointers
        //
        for(auto & p : names)
        {
            CATCH_REQUIRE(p.f_component == snaplogger::get_component(p.f_name));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("component: Send a component via the macros and << operator")
    {
        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());

        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));
        buffer->add_component(snaplogger::g_debug_component);
        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>(
                                        "${message} (${severity:format=number}) -- ${components}"));
        buffer->set_format(f);
        l->add_appender(buffer);

        char const * cargv[] =
        {
            "/usr/bin/daemon",
            "--log-severity",
            "noisy",
            nullptr
        };
        int const argc(sizeof(cargv) / sizeof(cargv[0]) - 1);
        char ** argv = const_cast<char **>(cargv);

        advgetopt::options_environment environment_options;
        environment_options.f_project_name = "test-logger";
        environment_options.f_environment_flags = advgetopt::GETOPT_ENVIRONMENT_FLAG_SYSTEM_PARAMETERS;
        environment_options.f_version = "3.4";
        advgetopt::getopt opts(environment_options);
        opts.parse_program_name(argv);
        snaplogger::add_logger_options(opts);
        opts.parse_arguments(argc, argv, advgetopt::option_source_t::SOURCE_COMMAND_LINE);

        CATCH_REQUIRE(snaplogger::process_logger_options(opts, "/etc/my-app/logger"));

        buffer->set_config(opts);

        SNAP_LOG_NOISY
            << "We got a component!"
            << snaplogger::section(snaplogger::g_debug_component)
            << SNAP_LOG_SEND;

        CATCH_REQUIRE_FALSE(buffer->empty());

        std::string const severity1(std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_INFORMATION)));
        std::string const severity2(std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_NOISY)));
        std::string const expected("-------------------------------------------------- (" + severity1 + ") -- [normal,self,banner]\n"
                      "test-logger v3.4 started. (" + severity1 + ") -- [normal,self,banner]\n"
                      "We got a component! (" + severity2 + ") -- [debug]\n");
        CATCH_REQUIRE(buffer->str() == expected);

        buffer->clear();

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("component_errors", "[component][error]")
{
    CATCH_START_SECTION("component: component name cannot start with a digit")
    {
        for(int d(0); d < 10; ++d)
        {
            std::string bad_name(std::to_string(d) + "name");
            CATCH_REQUIRE_THROWS_MATCHES(
                      snaplogger::get_component(bad_name)
                    , snaplogger::invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                              "logger_error: a component name cannot start with a digits ("
                            + bad_name
                            + ")."));
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("component: component name cannot include certain characters")
    {
        for(int count(0); count < 10; ++count)
        {
            std::string bad_name;
            int const max(rand() & 31 + 3);
            bool bad_added(false);
            char bad_char('\0');
            for(int idx(0); idx < max || !bad_added; ++idx)
            {
                char c(rand() % 95 + ' ');
                while(idx == 0 && c >= '0' && c <= '9')
                {
                    c = rand() % 95 + ' ';
                }
                bad_name += c;
                if((c < '0' || c > '9')
                && (c < 'A' || c > 'Z')
                && (c < 'a' || c > 'z')
                && c != '-'
                && c != '_'
                && !bad_added)
                {
                    bad_added = true;
                    bad_char = c;
                }
            }
            CATCH_REQUIRE_THROWS_MATCHES(
                      snaplogger::get_component(bad_name)
                    , snaplogger::invalid_parameter
                    , Catch::Matchers::ExceptionMessage(
                              std::string("logger_error: a component name cannot include a '")
                            + bad_char
                            + "' character ("
                            + bad_name
                            + ")."));
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
