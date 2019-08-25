/*
 * Copyright (c) 2006-2019  Made to Order Software Corp.  All Rights Reserved
 *
 * https://snapwebsites.org/project/snaplogger
 * contact@m2osw.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// self
//
#include    "main.h"


// snaplogger lib
//
#include    <snaplogger/buffer_appender.h>
#include    <snaplogger/exception.h>
#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
#include    <snaplogger/nested_diagnostic.h>
#include    <snaplogger/message.h>
#include    <snaplogger/severity.h>
#include    <snaplogger/version.h>


// C lib
//
#include    <unistd.h>



CATCH_TEST_CASE("diagnostic", "[message][diagnostic]")
{
    CATCH_START_SECTION("Map based and nested diagnostics")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "basic-format");

        // these two are not called in this test
        //
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROJECT_NAME, "test-logger");
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_VERSION, "5.32.1024");

        // test with our own diagnostics too
        //
        snaplogger::set_diagnostic("test_diag", "X-66-Q");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        opt_env.f_version = "5.32.1024";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${project_name} ${message} v${version}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_WARNING
            << "{${diagnostic:map=test_diag}}"
            << SNAP_LOG_SEND;

        CATCH_REQUIRE(buffer->str() == "test-logger {<test_diag=X-66-Q>} v5.32.1024"
                "\n");

        buffer->clear();

        {
            snaplogger::nested_diagnostic l1("level-I");

            SNAP_LOG_WARNING
                << "$${diagnostic:map=test_diag}$ & [${diagnostic:nested=10}]"
                << SNAP_LOG_SEND;

            CATCH_REQUIRE(buffer->str() ==
                    "test-logger $<test_diag=X-66-Q>$ & [{level-I}] v5.32.1024"
                    "\n");

            buffer->clear();

            {
                snaplogger::nested_diagnostic l2("sub-level-II");

                SNAP_LOG_WARNING
                    << "$${diagnostic:map=test_diag}$ & [${diagnostic:nested=10}]"
                    << SNAP_LOG_SEND;

                CATCH_REQUIRE(buffer->str() ==
                        "test-logger $<test_diag=X-66-Q>$ & [{level-I/sub-level-II}] v5.32.1024"
                        "\n");

                buffer->clear();

                {
                    snaplogger::nested_diagnostic l3("under-level-III");

                    SNAP_LOG_WARNING
                        << "$${diagnostic:map=test_diag}$ & [${diagnostic:nested=10}]"
                        << SNAP_LOG_SEND;

                    CATCH_REQUIRE(buffer->str() ==
                            "test-logger $<test_diag=X-66-Q>$ & [{level-I/sub-level-II/under-level-III}] v5.32.1024"
                            "\n");

                    buffer->clear();

                    SNAP_LOG_WARNING
                        << "$${diagnostic:map=test_diag}$ & [${diagnostic:nested=2}]"
                        << SNAP_LOG_SEND;

                    CATCH_REQUIRE(buffer->str() ==
                            "test-logger $<test_diag=X-66-Q>$ & [{.../sub-level-II/under-level-III}] v5.32.1024"
                            "\n");

                    buffer->clear();
                }
            }
        }

        l->reset();
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
