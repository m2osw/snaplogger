/*
 * License:
 *    Copyright (c) 2006-2019  Made to Order Software Corp.  All Rights Reserved
 *
 *    https://snapwebsites.org/
 *    contact@m2osw.com
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Authors:
 *    Alexis Wilke   alexis@m2osw.com
 */

// self
//
#include    "main.h"


// snaplogger lib
//
#include    <snaplogger/buffer_appender.h>
#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
#include    <snaplogger/message.h>
#include    <snaplogger/severity.h>


// C lib
//
#include    <unistd.h>



CATCH_TEST_CASE("message_capture", "[message]")
{
    CATCH_START_SECTION("Message Buffering")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-logging");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        CATCH_REQUIRE(buffer->get_type() == "buffer");

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_ERROR << "Logging this error";
        CATCH_REQUIRE(buffer->str() == "Logging this error\n");
        buffer->str(std::string());

        // show that the "\n" does not get duplicated
        //
        SNAP_LOG_ERROR << "Error with newline\n";
        CATCH_REQUIRE(buffer->str() == "Error with newline\n");
        buffer->str(std::string());

        // show that the "\r\n" gets replaced by "\n"
        //
        SNAP_LOG_ERROR << "Error with CRLF\r\n";
        CATCH_REQUIRE(buffer->str() == "Error with CRLF\n");
        buffer->str(std::string());

        // severity too low, no change to buffer
        //
        SNAP_LOG_DEBUG << "Debug Message " << M_PI << " which does not make it at all...\n";
        CATCH_REQUIRE(buffer->str().empty());

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("JSON Buffering")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "json-logging");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("json-buffer"));

        CATCH_REQUIRE(buffer->get_type() == "buffer");

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "json-logger";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>(
                    "{\"version\":1,"
                    "\"message\":\"${message:escape='\\\r\n\t\"'}\"}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        SNAP_LOG_ERROR << "A JSON error message";
        CATCH_REQUIRE(buffer->str() == "{\"version\":1,\"message\":\"A JSON error message\"}\n");
        buffer->str(std::string());

        // show that the "\n" does not get duplicated
        //
        SNAP_LOG_ERROR << "See what happens with a \"quoted string\" within the message\n";
        CATCH_REQUIRE(buffer->str() == "{\"version\":1,\"message\":\"See what happens with a \\\"quoted string\\\" within the message\"}\n");
        buffer->str(std::string());

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("message_copy", "[message]")
{
    CATCH_START_SECTION("Copy Message")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-copying");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        CATCH_REQUIRE(buffer->get_type() == "buffer");

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        int const line_number = __LINE__;
        snaplogger::message::pointer_t msg(std::make_shared<snaplogger::message>
                        (::snaplogger::severity_t::SEVERITY_ERROR, __FILE__, __func__, line_number));

        CATCH_REQUIRE(msg->get_filename() == __FILE__);
        CATCH_REQUIRE(msg->get_function() == __func__);
        CATCH_REQUIRE(msg->get_line() == line_number);

        msg->set_filename("logger.cpp");
        msg->set_function("magical");
        msg->set_line(701);

        CATCH_REQUIRE(msg->get_filename() == "logger.cpp");
        CATCH_REQUIRE(msg->get_function() == "magical");
        CATCH_REQUIRE(msg->get_line() == 701);

        *msg << "Logging an error.";

        CATCH_REQUIRE(msg->str() == "Logging an error.");

        snaplogger::message::pointer_t copy(std::make_shared<snaplogger::message>(*msg, *msg));

        CATCH_REQUIRE(msg->str() == "Logging an error.");
        CATCH_REQUIRE(copy->str() == "Logging an error.");

        // no destructor called, the output is still empty
        //
        CATCH_REQUIRE(buffer->str().empty());

        copy.reset();

        // msg not lost
        //
        CATCH_REQUIRE(msg->str() == "Logging an error.");

        // destructor against copy does not trigger send_message()
        //
        CATCH_REQUIRE(buffer->str().empty());

        msg.reset();

        // now we get the message as expected!
        //
        // (note that internally we can skip receiving the message on the
        // original, but not as a client... we may want to have the ability
        // to cancel a message, though.)
        //
        CATCH_REQUIRE(buffer->str() == "Logging an error.\n");

        l->reset();
    }
    CATCH_END_SECTION()
}


CATCH_TEST_CASE("message_severity", "[message][severity]")
{
    CATCH_START_SECTION("Appender vs Message severity")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-copying");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        int const min_severity(static_cast<int>(snaplogger::severity_t::SEVERITY_MIN));
        int const max_severity(static_cast<int>(snaplogger::severity_t::SEVERITY_MAX));
        for(int i(min_severity); i <= max_severity; ++i)
        {
            buffer->set_severity(static_cast<snaplogger::severity_t>(i));
            for(int j(min_severity); j <= max_severity; ++j)
            {
                ::snaplogger::message(
                          static_cast<::snaplogger::severity_t>(j)
                        , __FILE__
                        , __func__
                        , __LINE__
                    ) << "The message itself";

                if(j >= i
                && i != static_cast<int>(snaplogger::severity_t::SEVERITY_OFF)
                && j != static_cast<int>(snaplogger::severity_t::SEVERITY_OFF))
                {
                    CATCH_REQUIRE(buffer->str() == "The message itself\n");
                }
                else
                {
                    CATCH_REQUIRE(buffer->str().empty());
                }
                buffer->str(std::string());
            }
        }

        l->reset();
    }
    CATCH_END_SECTION()

    CATCH_START_SECTION("Changing message severity (takes about 3.5min)")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "message-copying");

        snaplogger::logger::pointer_t l(snaplogger::logger::get_instance());
        snaplogger::buffer_appender::pointer_t buffer(std::make_shared<snaplogger::buffer_appender>("test-buffer"));

        advgetopt::options_environment opt_env;
        opt_env.f_project_name = "test-logger";
        advgetopt::getopt opts(opt_env);
        buffer->set_config(opts);

        snaplogger::format::pointer_t f(std::make_shared<snaplogger::format>("${message}"));
        buffer->set_format(f);

        l->add_appender(buffer);

        int const min_severity(static_cast<int>(snaplogger::severity_t::SEVERITY_MIN));
        int const max_severity(static_cast<int>(snaplogger::severity_t::SEVERITY_MAX));
        for(int i(min_severity); i <= max_severity; ++i)
        {
            buffer->set_severity(static_cast<snaplogger::severity_t>(i));
            for(int j(min_severity); j <= max_severity; ++j)
            {
                for(int k(min_severity); k <= max_severity; ++k)
                {
                    ::snaplogger::message::pointer_t msg(std::make_shared<::snaplogger::message>(
                              static_cast<::snaplogger::severity_t>(j)
                            , __FILE__
                            , __func__
                            , __LINE__
                        ));
                    *msg << "Start of message";
                    msg->set_severity(static_cast<::snaplogger::severity_t>(k));
                    *msg << " -- end of message";
                    msg.reset();
//std::cerr << "checking with " << i << ", " << j << ", " << k << "\n";

                    if(j >= i
                    && k >= i
                    && i != static_cast<int>(snaplogger::severity_t::SEVERITY_OFF)
                    && j != static_cast<int>(snaplogger::severity_t::SEVERITY_OFF))
                    //&& k != static_cast<int>(snaplogger::severity_t::SEVERITY_OFF))
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
                        CATCH_REQUIRE(buffer->str().empty());
                    }
                    buffer->str(std::string());
                }
            }
        }

        l->reset();
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
