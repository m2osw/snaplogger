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


// libexcept lib
//
#include    "libexcept/exception.h"


namespace snaplogger
{


class logger_logic_error
    : public libexcept::logic_exception_t
{
public:
    logger_logic_error(std::string const & msg) : logic_exception_t(msg) {}
};


class logger_error
    : public libexcept::exception_t
{
public:
    logger_error(std::string const & msg) : exception_t(msg) {}
};


class duplicate_error
    : public logger_error
{
public:
    duplicate_error(std::string const & msg) : logger_error(msg) {}
};


class invalid_variable
    : public logger_error
{
public:
    invalid_variable(std::string const & msg) : logger_error(msg) {}
};


class invalid_parameter : public logger_error
{
public:
    invalid_parameter(std::string const & msg) : logger_error(msg) {}
};


class invalid_severity : public logger_error
{
public:
    invalid_severity(std::string const & msg) : logger_error(msg) {}
};


class duplicate_component : public logger_error
{
public:
    duplicate_component(std::string const & msg) : logger_error(msg) {}
};


} // namespace snaplogger
// vim: ts=4 sw=4 et
