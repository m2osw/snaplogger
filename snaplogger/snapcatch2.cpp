// Copyright (c) 2013-2025  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Test extensions helper function implementation.
 *
 * Implementation of various helper functions.
 */

// snaplogger
//
#include    <snaplogger/console_appender.h>
#include    <snaplogger/format.h>
#include    <snaplogger/logger.h>
#include    <snaplogger/snapcatch2.hpp>



namespace snaplogger
{



void setup_catch2_nested_diagnostics()
{
    logger::pointer_t l(logger::get_instance());

    l->add_console_appender();
    l->set_severity(snaplogger::severity_t::SEVERITY_ALL);

    appender::pointer_t console(l->get_appender("console"));
    if(console != nullptr)
    {
        std::string const & existing_format(console->get_format()->get_format());
        format::pointer_t new_format(std::make_shared<format>(existing_format + " ${diagnostic:nested}"));
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
