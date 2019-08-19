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
#include    <snaplogger/message.h>
#include    <snaplogger/severity.h>
#include    <snaplogger/version.h>


// C lib
//
#include    <unistd.h>
#include    <netdb.h>
#include    <sys/param.h>





CATCH_TEST_CASE("appender", "[appender]")
{
    CATCH_START_SECTION("Create Appender")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "appender");

        snaplogger::appender::pointer_t unknown(snaplogger::create_appender("unknown", "test-buffer"));
        CATCH_REQUIRE(unknown == nullptr);

        snaplogger::appender::pointer_t buffer(snaplogger::create_appender("buffer", "test-buffer"));
        CATCH_REQUIRE(buffer != nullptr);

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${severity}: ${message}"));
        buffer->set_format(f);

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        l->add_appender(buffer);

        SNAP_LOG_FATAL << "Appender created by name" << SNAP_LOG_SEND;
        CATCH_REQUIRE(std::dynamic_pointer_cast<snaplogger::buffer_appender>(buffer)->str() == "fatal: Appender created by name\n");

        l->reset();
    }
    CATCH_END_SECTION()
}



// vim: ts=4 sw=4 et
