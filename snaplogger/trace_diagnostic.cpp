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
 * \brief Trace log diagnostic implementation.
 *
 * This file declares the trace log functions.
 */


// self
//
#include    "snaplogger/trace_diagnostic.h"

#include    "snaplogger/private_logger.h"


// last include
//
#include    <snapdev/poison.h>



namespace snaplogger
{



void add_trace_diagnostic(std::string const & diagnostic)
{
    get_private_logger()->add_trace_diagnostic(diagnostic);
}


void clear_trace_diagnostics()
{
    get_private_logger()->clear_trace_diagnostics();
}


trace_diagnostics_t get_trace_diagnostics()
{
    return get_private_logger()->get_trace_diagnostics();
}



} // snaplogger namespace
// vim: ts=4 sw=4 et
