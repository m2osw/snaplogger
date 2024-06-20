// Copyright (c) 2013-2024  Made to Order Software Corp.  All Rights Reserved
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
 * \brief Test extensions.
 *
 * We use Catch2 to write our tests. This file redefines the section
 * macros so the name of the section is added to the nested diagnostic
 * making it possible to display the section name in all the logs.
 *
 * \warning
 * The order in which the headers are included is important because we
 * replace a macro from the snapcatch2.hpp. So for this to work, we first
 * include the default snapcatch2 header here. If you included the
 * snapcatch2.hpp already, it should work either way.
 */

// snaplogger
//
#include    <snaplogger/nested_diagnostic.h>


// catch2
//
#include    <catch2/snapcatch2.hpp>


/** \brief Start a new section with name saved in logger.
 *
 * This macro is an override of the CATCH_START_SECTION() macro defined
 * in snapcatch2.hpp. It is used so we can print include the name of the
 * section in the logger nested diagnostics.
 *
 * \param[in] name  The name of the section.
 */
#ifdef CATCH_START_SECTION
#undef CATCH_START_SECTION
#endif
#define CATCH_START_SECTION(name) \
    CATCH_SECTION(name) \
    { \
        if(SNAP_CATCH2_NAMESPACE::g_progress()) \
        { \
            std::cout << "SECTION: " << name << std::endl; \
        } \
        ::snaplogger::nested_diagnostic snaplogger_diagnostic(name, true);



namespace snaplogger
{



void setup_catch2_nested_diagnostics();



} // namespace snaplogger
// vim: ts=4 sw=4 et
