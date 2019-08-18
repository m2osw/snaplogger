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





CATCH_TEST_CASE("variable_param", "[variable][param]")
{
    CATCH_START_SECTION("Param Name is Mandatory")
    {
        CATCH_REQUIRE_THROWS_MATCHES(
                  new snaplogger::param(std::string())
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "a parameter must have a non-empty name."));
    }
    CATCH_END_SECTION()
}



CATCH_TEST_CASE("system_variable", "[variable][param]")
{
    CATCH_START_SECTION("get_type() to use padding as integer or string (hostname)")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-logging");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
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

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"t\":align=right:min_width=30} ${message}");
        buffer->set_format(f);

        aligned = host;
        while(aligned.length() < 30)
        {
            aligned = "t" + aligned;
        }

        SNAP_LOG_ERROR << "Try again with a string" << SNAP_LOG_SEND;
        CATCH_REQUIRE(buffer->str() == aligned + " Try again with a string\n");

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"q\":align=101:min_width=30} ${message}");
        buffer->set_format(f);

        snaplogger::message::pointer_t msg(std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR, __FILE__, __func__, __LINE__));
        *msg << "The align=101 parameter is the wrong type";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "the ${...:align=<value>} parameter must be a valid string (not an integer)."));

        // this is important here because we want to make sure that the
        // `message` destructor works as expected (i.e. it does not call
        // std::terminate() because of the throw as the align=101 is
        // invalid)
        //
        msg.reset();

        buffer->clear();

        f = std::make_shared<snaplogger::format>("${hostname:padding=\"q\":align=left:min_width=wide} ${message}");
        buffer->set_format(f);

        msg = std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR, __FILE__, __func__, __LINE__);
        *msg << "The min_width=wide parameter is the wrong type";
        CATCH_REQUIRE_THROWS_MATCHES(
                  l->log_message(*msg)
                , snaplogger::invalid_parameter
                , Catch::Matchers::ExceptionMessage(
                          "the ${...:min_width=<value>} parameter must be a valid integer."));

        // this is important here because we want to make sure that the
        // `message` destructor works as expected (i.e. it does not call
        // std::terminate() because of the throw as the align=101 is
        // invalid)
        //
        msg.reset();

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("default align value")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-variable-default-param");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
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
}



// vim: ts=4 sw=4 et
