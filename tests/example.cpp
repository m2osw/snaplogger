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
#include    <snaplogger/logger.h>
#include    <snaplogger/map_diagnostic.h>
#include    <snaplogger/message.h>


// C lib
//
#include    <unistd.h>



CATCH_TEST_CASE("example", "[example]")
{
    CATCH_START_SECTION("Simple logging")
    {
        snaplogger::set_diagnostic(snaplogger::DIAG_KEY_PROGNAME, "unittest");

        snaplogger::logger::pointer_t ptr(snaplogger::logger::get_instance());
        ptr->add_console_appender()->add_component(snaplogger::secure_component);
        ptr->add_syslog_appender("example");
        ptr->add_file_appender("my-file.log")->add_component(snaplogger::debug_component);
        //ptr->add_component_to_ignore(snaplogger::normal_component);
        //ptr->add_component_to_include(snaplogger::normal_component);

    	SNAP_LOG_ERROR << "Logging this error\n";
    	SNAP_LOG_WARNING << (isatty(fileno(stdout)) ? "" : "Hello world!");
    	SNAP_LOG_FATAL << "Saw empty one? " << 123 << std::endl;

        long value(123);
    	SNAP_LOG_INFORMATION
                //<< snaplogger::section(snaplogger::secure_component)
                << snaplogger::secure
                << snaplogger::section(snaplogger::debug_component)
                << "Saw empty one? "
                << value
                << std::endl;
    }
    CATCH_END_SECTION()
}


// vim: ts=4 sw=4 et
