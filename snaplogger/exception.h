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
 * \brief Logger exceptions.
 *
 * This files declares a few exceptions that the logger uses when a
 * parameter is wrong. Note that many runtime errors get \em eaten
 * up by the system because there isn't much you can do if the logger
 * itself fails.
 *
 * However, some of the setup can generate exceptions because those
 * errors should be fixed before you go on. For example, if you try
 * to change a system severity level or define two `severity` objects
 * with the same name, then you'll get one of those exceptions. In
 * most cases, those do not need to be handled. Instead, they should
 * be fixed.
 */


// libexcept
//
#include    <libexcept/exception.h>


namespace snaplogger
{



DECLARE_LOGIC_ERROR(logger_logic_error);

DECLARE_MAIN_EXCEPTION(fatal_error);
DECLARE_MAIN_EXCEPTION(logger_error);

DECLARE_EXCEPTION(logger_error, conflict_error);
DECLARE_EXCEPTION(logger_error, duplicate_error);
DECLARE_EXCEPTION(logger_error, invalid_variable);
DECLARE_EXCEPTION(logger_error, invalid_parameter);
DECLARE_EXCEPTION(logger_error, invalid_severity);
DECLARE_EXCEPTION(logger_error, not_a_message);



} // namespace snaplogger
// vim: ts=4 sw=4 et
