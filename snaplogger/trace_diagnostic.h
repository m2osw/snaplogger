// Copyright (c) 2013-2022  Made to Order Software Corp.  All Rights Reserved
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
#pragma once

/** \file
 * \brief Allow for a trace log.
 *
 * This file declares the trace diagnostic class.
 */

// self
//
#include    "snaplogger/message.h"

// C++ lib
//
#include    <deque>


// C lib
//
#include    <sys/time.h>



namespace snaplogger
{


constexpr size_t const      DIAG_TRACE_SIZE = 10;


typedef std::deque<std::string>        trace_diagnostics_t;

void                add_trace_diagnostic(std::string const & diagnostic);
void                clear_trace_diagnostic();
trace_diagnostics_t get_trace_diagnostics();



} // snaplogger namespace
// vim: ts=4 sw=4 et
