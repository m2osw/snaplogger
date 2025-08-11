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
#pragma once

/** \file
 * \brief Appenders are used to append data to somewhere.
 *
 * This file declares the base appender class.
 */

// self
//
#include    <snaplogger/message.h>
#include    <snaplogger/utils.h>



namespace snaplogger
{



class nested_diagnostic
{
public:
                            nested_diagnostic(std::string const & diagnostic, bool emit_enter_exit_events = false);
    virtual                 ~nested_diagnostic();

private:
    bool                    f_emit_enter_exit_events = false;
};


string_vector_t             get_nested_diagnostics();
string_vector_t             get_nested_diagnostics(message const & msg);



} // snaplogger namespace
// vim: ts=4 sw=4 et
