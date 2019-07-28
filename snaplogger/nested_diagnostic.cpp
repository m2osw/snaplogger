/*
 * License:
 *    Copyright (c) 2013-2019  Made to Order Software Corp.  All Rights Reserved
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

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */


// self
//
#include    "snaplogger/guard.h"
#include    "snaplogger/message.h"
#include    "snaplogger/nested_diagnostic.h"


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



namespace
{


string_vector_t       g_diagnostics = string_vector_t();



}
// no name namespace



nested_diagnostic::nested_diagnostic(std::string const & diagnostic, bool emit_enter_exit_event)
    : f_emit_enter_exit_event(emit_enter_exit_event)
{
    {
        guard g;

        g_diagnostics.push_back(diagnostic);
    }

    if(f_emit_enter_exit_event)
    {
        SNAP_LOG_UNIMPORTANT << "entering nested diagnostic";
    }
}


nested_diagnostic::~nested_diagnostic()
{
    if(f_emit_enter_exit_event)
    {
        SNAP_LOG_UNIMPORTANT << "exiting nested diagnostic";
    }

    guard g;

    g_diagnostics.pop_back();
}


string_vector_t get_nested_diagnostics()
{
    return g_diagnostics;
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
