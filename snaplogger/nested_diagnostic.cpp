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
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */


// self
//
#include    "snaplogger/nested_diagnostic.h"

#include    "snaplogger/guard.h"
#include    "snaplogger/message.h"
#include    "snaplogger/private_logger.h"


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



nested_diagnostic::nested_diagnostic(std::string const & diagnostic, bool emit_enter_exit_events)
    : f_emit_enter_exit_events(emit_enter_exit_events)
{
    get_private_logger()->push_nested_diagnostic(diagnostic);

    if(f_emit_enter_exit_events)
    {
        SNAP_LOG_UNIMPORTANT
            << section(g_normal_component)
            << section(g_self_component)
            << "entering nested diagnostic"
            << SNAP_LOG_SEND;
    }
}


nested_diagnostic::~nested_diagnostic()
{
    if(f_emit_enter_exit_events)
    {
        SNAP_LOG_UNIMPORTANT
            << section(g_normal_component)
            << section(g_self_component)
            << "exiting nested diagnostic"
            << SNAP_LOG_SEND;
    }

    get_private_logger()->pop_nested_diagnostic();
}


string_vector_t get_nested_diagnostics()
{
    return get_private_logger()->get_nested_diagnostics();
}


string_vector_t get_nested_diagnostics(message const & msg)
{
    return get_private_logger(msg)->get_nested_diagnostics();
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
