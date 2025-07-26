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


// advgetopt
//
#include    <advgetopt/conf_file.h>
#include    <advgetopt/validator_integer.h>


// snapdev
//
#include    <snapdev/enum_class_math.h>


// C
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

            // duplicates are not allowed
            //
            CATCH_REQUIRE_THROWS_MATCHES(
                      s->add_alias("bad-error")
                    , snaplogger::duplicate_error
                    , Catch::Matchers::ExceptionMessage(
                              "logger_error: severity \""
                              "bad-error"
                              "\" already has an alias \""
                              "bad-error"
                              "\"."));

            snaplogger::add_severity(s);

            CATCH_REQUIRE(snaplogger::get_severity("bad-error") == s);
            CATCH_REQUIRE(snaplogger::get_severity("big-error") == nullptr);

            s->add_alias("big-error");

            // duplicates are not allowed
            //
            CATCH_REQUIRE_THROWS_MATCHES(
                      s->add_alias("big-error")
                    , snaplogger::duplicate_error
                    , Catch::Matchers::ExceptionMessage(
                              "logger_error: severity \""
                              "bad-error"
                              "\" already has an alias \""
                              "big-error"
                              "\"."));

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

            snaplogger::message msg(::snaplogger::severity_t::SEVERITY_ERROR);
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
            { ::snaplogger::severity_t::SEVERITY_ALL,                   "all" },
            { ::snaplogger::severity_t::SEVERITY_TRACE,                 "trace" },
            { ::snaplogger::severity_t::SEVERITY_NOISY,                 "noisy" },
            { ::snaplogger::severity_t::SEVERITY_DEBUG,                 "debug" },
            { ::snaplogger::severity_t::SEVERITY_NOTICE,                "notice" },
            { ::snaplogger::severity_t::SEVERITY_UNIMPORTANT,           "unimportant" },
            { ::snaplogger::severity_t::SEVERITY_VERBOSE,               "verbose" },
            { ::snaplogger::severity_t::SEVERITY_CONFIGURATION,         "configuration" },
            { ::snaplogger::severity_t::SEVERITY_CONFIGURATION_WARNING, "configuration-warning" },
            { ::snaplogger::severity_t::SEVERITY_INFORMATION,           "information" },
            { ::snaplogger::severity_t::SEVERITY_IMPORTANT,             "important" },
            { ::snaplogger::severity_t::SEVERITY_MINOR,                 "minor" },
            { ::snaplogger::severity_t::SEVERITY_DEPRECATED,            "deprecated" },
            { ::snaplogger::severity_t::SEVERITY_WARNING,               "warning" },
            { ::snaplogger::severity_t::SEVERITY_MAJOR,                 "major" },
            { ::snaplogger::severity_t::SEVERITY_RECOVERABLE_ERROR,     "recoverable-error" },
            { ::snaplogger::severity_t::SEVERITY_ERROR,                 "error" },
            { ::snaplogger::severity_t::SEVERITY_NOISY_ERROR,           "noisy-error" },
            { ::snaplogger::severity_t::SEVERITY_SEVERE,                "severe" },
            { ::snaplogger::severity_t::SEVERITY_EXCEPTION,             "exception" },
            { ::snaplogger::severity_t::SEVERITY_CRITICAL,              "critical" },
            { ::snaplogger::severity_t::SEVERITY_ALERT,                 "alert" },
            { ::snaplogger::severity_t::SEVERITY_EMERGENCY,             "emergency" },
            { ::snaplogger::severity_t::SEVERITY_FATAL,                 "fatal" },
            { ::snaplogger::severity_t::SEVERITY_OFF,                   "off" },
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

    CATCH_START_SECTION("severity: Severity by Level or Name")
    {
        snaplogger::severity_by_severity_t severities(snaplogger::get_severities_by_severity());
        snaplogger::severity_by_name_t names(snaplogger::get_severities_by_name());

        // this is not true, there are more names than severity levels
        //
        //CATCH_REQUIRE(severities.size() == names.size());

// this does not hold true, that is, the map is properly sorted, but for
// entries with an alias, the s->get_name() returns the base name, not the
// alias, and thus, the order may be skewed for all aliases
//
//        std::string previous_name;
//        for(auto const & s : names)
//        {
//            CATCH_REQUIRE(s.second->get_name() > previous_name);
//            previous_name = s.second->get_name();
//        }

        snaplogger::severity_t previous_severity(snaplogger::severity_t::SEVERITY_ALL - 1);
        for(auto const & s : severities)
        {
            CATCH_REQUIRE(s.second->get_severity() > previous_severity);
            previous_severity = s.second->get_severity();
        }
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("severity: severity.ini file matches")
    {
        // whenever I make an edit to the list of severity levels, I have to
        // keep the severity.ini up to date or the system won't be able to
        // load the file properly; this test verifies the one found in the
        // source tree against the severity levels defined in the header
        //
        std::string severity_ini_filename(SNAP_CATCH2_NAMESPACE::g_source_dir());
        severity_ini_filename += "/conf/severity.ini";
        advgetopt::conf_file_setup setup(severity_ini_filename);
        CATCH_REQUIRE(setup.is_valid());

        advgetopt::conf_file::pointer_t severity_ini(advgetopt::conf_file::get_conf_file(setup));
        CATCH_REQUIRE(severity_ini != nullptr);

        std::set<std::string> found;

        advgetopt::conf_file::sections_t sections(severity_ini->get_sections());
        for(auto const & s : sections)
        {
            snaplogger::severity::pointer_t severity(snaplogger::get_severity(s));

            // the default severity.ini file only defines system severities
            //
            CATCH_REQUIRE(severity->is_system());

            // make sure the entry includes a severity=... value
            //
            std::string const level_name(s + "::severity");
            CATCH_REQUIRE(severity_ini->has_parameter(level_name));

            std::string const level(severity_ini->get_parameter(level_name));

            // the level must be a valid integer
            //
            std::int64_t value(0);
            CATCH_REQUIRE(advgetopt::validator_integer::convert_string(level, value));

            // the .ini level must match the internal (library) level
            //
            CATCH_REQUIRE(severity->get_severity() == static_cast<snaplogger::severity_t>(value));

            found.insert(s);

            // severities may have aliases which needs to be added to `found`
            //
            std::string const aliases_name(s + "::aliases");
            if(severity_ini->has_parameter(aliases_name))
            {
                std::string const aliases(severity_ini->get_parameter(aliases_name));
                advgetopt::string_list_t names;
                advgetopt::split_string(aliases, names, {","});
                for(auto const & n : names)
                {
                    found.insert(n);
                }
            }
        }

        // further, make sure that all system severities defined in the
        // library are found in the .ini file
        //
        // with aliases, this is not 100% true, maybe we should check
        // with the by_severity() list instead...
        //
        for(auto const & s : snaplogger::get_severities_by_name())
        {
            if(s.second->is_system())
            {
                std::cout
                    << "--- verifying that \""
                    << s.first
                    << "\" severity is defined in "
                    <<severity_ini_filename
                    << "\n";
                CATCH_REQUIRE(found.contains(s.first));
            }
        }
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("severity_error", "[severity][error]")
{
    CATCH_START_SECTION("severity: too small")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  snaplogger::severity(snaplogger::severity_t::SEVERITY_MIN - 1, "TOO_SMALL")
                , snaplogger::invalid_severity
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the severity level cannot be "
                        + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_MIN - 1))
                        + ". The possible range is ["
                        + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_MIN))
                        + ".."
                        + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_MAX))
                        + "]."));
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("severity: too large")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  snaplogger::severity(snaplogger::severity_t::SEVERITY_MAX + 1, "TOO_LARGE")
                , snaplogger::invalid_severity
                , Catch::Matchers::ExceptionMessage(
                          "logger_error: the severity level cannot be "
                        + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_MAX + 1))
                        + ". The possible range is ["
                        + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_MIN))
                        + ".."
                        + std::to_string(static_cast<int>(snaplogger::severity_t::SEVERITY_MAX))
                        + "]."));
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
