// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Test extensions helper function implementation.
 *
 * Implementation of various helper functions.
 */

// snaplogger
//
#include    <snaplogger/console_appender.h>
#include    <snaplogger/format.h>
#include    <snaplogger/private_logger.h>
#include    <snaplogger/snapcatch2.hpp>



namespace snaplogger
{



/** \brief Add a nested diagnostic.
 *
 * This function changes the console appender format to include a
 * nested diagnostic with the section name.
 *
 * This helps greatly when debugging tests since each log message
 * automatically includes the name of the current section. Some
 * tests create threads or processes that would otherwise not
 * properly indicate which test section started those tasks.
 *
 * Further, if the console uses the default format, then it gets
 * updated to remove the max_width=1000 parameter on the message
 * itself. This makes it a lot easier to debug when we end up
 * with really long messages. Since these are for our unit tests
 * only, we can manage the length (opposed to really long messages
 * generated in production).
 */
void setup_catch2_nested_diagnostics()
{
    logger::pointer_t l(logger::get_instance());

    l->add_console_appender();
    l->set_severity(snaplogger::severity_t::SEVERITY_ALL);

    appender::pointer_t console(l->get_appender("console"));
    if(console != nullptr)
    {
        private_logger::pointer_t p(std::dynamic_pointer_cast<private_logger>(l));
        std::string const & default_format(p->get_default_format()->get_format());
        std::string const & existing_format(console->get_format()->get_format());
        format::pointer_t new_format;
        if(default_format == existing_format)
        {
            // this is used in tests which we control and we also want the
            // whole errors logged so we can actually make sure we get it
            // right (and it can help us debug our tests more easily)
            //
            new_format = std::make_shared<format>(
                "${date} ${time} ${hostname}"
                " ${progname}[${pid}/${tid}]: ${severity}:"
                " ${message:escape}" // <-- removed the ":max_width=1000" parameter
                " (in function \"${function}()\")"
                " (${basename}:${line})"
                " ${diagnostic:nested}");
        }
        else
        {
            new_format = std::make_shared<format>(existing_format + " ${diagnostic:nested}");
        }
        console->set_format(new_format);

        console_appender::pointer_t c(std::dynamic_pointer_cast<console_appender>(console));
        if(c != nullptr)
        {
            c->set_force_style(); // always show colors (so it works inside less ...)
        }
    }
    else
    {
        std::cerr << "warning: \"console\" appender not found.\n";
    }
}



} // namespace snaplogger
// vim: ts=4 sw=4 et
