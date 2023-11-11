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




CATCH_TEST_CASE("severity", "[severity]")
{
    CATCH_START_SECTION("severity: Create Severity")
    {
        snaplogger::severity_t const err_plus_one(static_cast<snaplogger::severity_t>(static_cast<int>(snaplogger::severity_t::SEVERITY_ERROR) + 1));

        // user severity by name
        {
            snaplogger::severity::pointer_t s(std::make_shared<snaplogger::severity>(err_plus_one, "error"));

            CATCH_REQUIRE_THROWS_MATCHES(
                      snaplogger::add_severity(s)
                    , snaplogger::duplicate_error
                    , Catch::Matchers::ExceptionMessage(
                              "logger_error: a system severity (error) cannot be replaced (same name)."));
        }

        // system severity by name
        {
            snaplogger::severity::pointer_t s(std::make_shared<snaplogger::severity>(err_plus_one, "error", true));

            CATCH_REQUIRE_THROWS_MATCHES(
                      snaplogger::add_severity(s)
                    , snaplogger::duplicate_error
                    , Catch::Matchers::ExceptionMessage(
                              "logger_error: a system severity (error) cannot be replaced (same name)."));
        }

        // user severity by severity
        {
            snaplogger::severity::pointer_t s(std::make_shared<snaplogger::severity>(snaplogger::severity_t::SEVERITY_ERROR, "bad-error"));

            CATCH_REQUIRE_THROWS_MATCHES(
                      snaplogger::add_severity(s)
                    , snaplogger::duplicate_error
                    , Catch::Matchers::ExceptionMessage(
                                "logger_error: a system severity ("
                              + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_ERROR))
                              + ") cannot be replaced (same severity level: "
                              + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_ERROR))
                              + ")."));
        }

        // user severity by severity
        {
            snaplogger::severity::pointer_t s(std::make_shared<snaplogger::severity>(snaplogger::severity_t::SEVERITY_ERROR, "bad-error", true));

            CATCH_REQUIRE_THROWS_MATCHES(
                      snaplogger::add_severity(s)
                    , snaplogger::duplicate_error
                    , Catch::Matchers::ExceptionMessage(
                                    "logger_error: a system severity ("
                                  + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_ERROR))
                                  + ") cannot be replaced (same severity level: "
                                  + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_ERROR))
                                  + ")."));
        }

        // actually create a valid severity
        {
            CATCH_REQUIRE(snaplogger::get_severity("bad-error") == nullptr);
            CATCH_REQUIRE(snaplogger::get_severity("big-error") == nullptr);

            snaplogger::severity_t const level(static_cast<snaplogger::severity_t>(205));
            snaplogger::severity::pointer_t s(std::make_shared<snaplogger::severity>(level, "bad-error"));

            CATCH_REQUIRE(s->get_severity() == level);
            CATCH_REQUIRE(s->get_name() == "bad-error");

            CATCH_REQUIRE(s->get_all_names().size() == 1);
            CATCH_REQUIRE(s->get_all_names()[0] == "bad-error");

            CATCH_REQUIRE(snaplogger::get_severity("bad-error") == nullptr);
            CATCH_REQUIRE(snaplogger::get_severity("big-error") == nullptr);

            snaplogger::add_severity(s);

            CATCH_REQUIRE(snaplogger::get_severity("bad-error") == s);
            CATCH_REQUIRE(snaplogger::get_severity("big-error") == nullptr);

            s->add_alias("big-error");

            CATCH_REQUIRE(s->get_all_names().size() == 2);
            CATCH_REQUIRE(s->get_all_names()[0] == "bad-error");
            CATCH_REQUIRE(s->get_all_names()[1] == "big-error");

            CATCH_REQUIRE(s->get_description() == "bad-error");

            s->set_description("bad error");
            CATCH_REQUIRE(s->get_description() == "bad error");

            s->set_description(std::string());
            CATCH_REQUIRE(s->get_description() == "bad-error");

            CATCH_REQUIRE(snaplogger::get_severity("bad-error") == s);
            CATCH_REQUIRE(snaplogger::get_severity("big-error") == s);
            CATCH_REQUIRE(snaplogger::get_severity(level) == s);

            s->set_styles("orange");
            CATCH_REQUIRE(s->get_styles() == "orange");

            snaplogger::severity_t const level_plus_one(static_cast<snaplogger::severity_t>(static_cast<int>(level) + 1));
            CATCH_REQUIRE(snaplogger::get_severity(level_plus_one) == nullptr);

            snaplogger::message msg(::snaplogger::severity_t::SEVERITY_ERROR, __FILE__, __func__, __LINE__);
            CATCH_REQUIRE(snaplogger::get_severity(msg, "bad-error") == s);
            CATCH_REQUIRE(snaplogger::get_severity(msg, "big-error") == s);
        }

        // verify that the "<name>"_sev syntax works as expected
        {
            snaplogger::severity_t const level(static_cast<snaplogger::severity_t>(25));
            snaplogger::severity::pointer_t s(std::make_shared<snaplogger::severity>(level, "remark"));

            snaplogger::add_severity(s);

            CATCH_REQUIRE(s->get_severity() == level);
            CATCH_REQUIRE(s->get_name() == "remark");

#if SNAPDEV_CHECK_GCC_VERSION(7, 5, 0)
            snaplogger::severity::pointer_t r("remark"_sev);
            CATCH_REQUIRE(r == s);

            CATCH_REQUIRE(r->get_severity() == level);
            CATCH_REQUIRE(r->get_name() == "remark");
#endif
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("severity: Print Severity")
    {
        struct level_and_name_t
        {
            ::snaplogger::severity_t    f_level = ::snaplogger::severity_t::SEVERITY_ERROR;
            std::string                 f_name = std::string();
        };

        std::vector<level_and_name_t> level_and_name =
        {
            { ::snaplogger::severity_t::SEVERITY_ALL,               "all" },
            { ::snaplogger::severity_t::SEVERITY_TRACE,             "trace" },
            { ::snaplogger::severity_t::SEVERITY_DEBUG,             "debug" },
            { ::snaplogger::severity_t::SEVERITY_NOTICE,            "notice" },
            { ::snaplogger::severity_t::SEVERITY_UNIMPORTANT,       "unimportant" },
            { ::snaplogger::severity_t::SEVERITY_VERBOSE,           "verbose" },
            { ::snaplogger::severity_t::SEVERITY_INFORMATION,       "information" },
            { ::snaplogger::severity_t::SEVERITY_IMPORTANT,         "important" },
            { ::snaplogger::severity_t::SEVERITY_MINOR,             "minor" },
            { ::snaplogger::severity_t::SEVERITY_DEPRECATED,        "deprecated" },
            { ::snaplogger::severity_t::SEVERITY_WARNING,           "warning" },
            { ::snaplogger::severity_t::SEVERITY_MAJOR,             "major" },
            { ::snaplogger::severity_t::SEVERITY_RECOVERABLE_ERROR, "recoverable-error" },
            { ::snaplogger::severity_t::SEVERITY_ERROR,             "error" },
            { ::snaplogger::severity_t::SEVERITY_CRITICAL,          "critical" },
            { ::snaplogger::severity_t::SEVERITY_ALERT,             "alert" },
            { ::snaplogger::severity_t::SEVERITY_EMERGENCY,         "emergency" },
            { ::snaplogger::severity_t::SEVERITY_FATAL,             "fatal" },
            { ::snaplogger::severity_t::SEVERITY_OFF,               "off" },
        };

        for(auto const & ln : level_and_name)
        {
            std::stringstream buffer;
            buffer << ln.f_level;
            CATCH_REQUIRE(buffer.str() == ln.f_name);
        }

        {
            std::stringstream buffer;
            buffer << static_cast<::snaplogger::severity_t>(254);
            CATCH_REQUIRE(buffer.str() == "(unknown severity: 254)");
        }
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
